#include "Simulation.h"
#include "Network.h"
#include "SpriteData.h"
#include "EnttGroups.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Sprite.h"
#include "Camera.h"
#include "InputHistory.h"
#include "ISimulationExtension.h"
#include "NeedsAdjacentChunks.h"
#include "Name.h"
#include "ScreenRect.h"
#include "Transforms.h"
#include "Paths.h"
#include "Config.h"
#include "SharedConfig.h"
#include "UserConfig.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include <memory>
#include <string>

namespace AM
{
namespace Client
{
Simulation::Simulation(EventDispatcher& inUiEventDispatcher, Network& inNetwork,
                       SpriteData& inSpriteData)
: network{inNetwork}
, spriteData{inSpriteData}
, world(inSpriteData)
, currentTick(0)
, connectionResponseQueue(network.getEventDispatcher())
, extension{nullptr}
, chunkUpdateSystem(*this, world, network)
, tileUpdateSystem(world, inUiEventDispatcher, network)
, npcLifetimeSystem(*this, world, spriteData, network.getEventDispatcher())
, playerInputSystem(*this, world, network)
, playerMovementSystem(*this, world, network.getEventDispatcher())
, npcMovementSystem(*this, world, network, spriteData)
, cameraSystem(world)
{
    // Initialize our entt groups.
    EnttGroups::init(world.registry);

    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Simulation::connect()
{
    if (Config::RUN_OFFLINE) {
        // No need to connect if we're running offline. Just need mock player
        // data.
        fakeConnection();
        return;
    }

    while (!(network.connect())) {
        LOG_INFO("Network failed to connect. Retrying.");
    }

    // Wait for the data from the server.
    ConnectionResponse connectionResponse{};
    if (!(connectionResponseQueue.waitPop(connectionResponse,
                                          CONNECTION_RESPONSE_WAIT_US))) {
        LOG_FATAL("Server did not respond.");
    }

    // Save our player entity info.
    entt::entity playerEntity = connectionResponse.entity;
    LOG_INFO(
        "Received connection response. ID: %u, tick: %u, pos: (%.4f, %.4f)",
        playerEntity, connectionResponse.tickNum, connectionResponse.x,
        connectionResponse.y);

    // Resize the world's tile map.
    world.tileMap.setMapSize(connectionResponse.mapXLengthChunks,
                             connectionResponse.mapYLengthChunks);
    world.worldSignals.tileMapExtentChanged.publish(
        world.tileMap.getTileExtent());
    LOG_INFO("Setting map size to: (%u, %u)ch.",
             connectionResponse.mapXLengthChunks,
             connectionResponse.mapYLengthChunks);

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick = connectionResponse.tickNum + Config::INITIAL_TICK_OFFSET;

    // Create the player entity using the ID we received.
    entt::registry& registry = world.registry;
    entt::entity newEntity = registry.create(playerEntity);
    if (newEntity != playerEntity) {
        LOG_FATAL("Created entity doesn't match received entity. Created: %u, "
                  "received: %u",
                  newEntity, playerEntity);
    }

    // Save the player entity ID for convenience.
    world.playerEntity = newEntity;

    // Set up the player's sim components.
    registry.emplace<Name>(newEntity,
                           std::to_string(static_cast<Uint32>(newEntity)));
    Position& playerPosition{registry.emplace<Position>(
        newEntity, connectionResponse.x, connectionResponse.y, 0.0f)};
    registry.emplace<PreviousPosition>(newEntity, connectionResponse.x,
                                       connectionResponse.y, 0.0f);
    registry.emplace<Velocity>(newEntity, 0.0f, 0.0f, 20.0f, 20.0f);
    registry.emplace<Input>(newEntity);

    // Set up the player's visual components.
    // TODO: Switch to logical screen size and do scaling in Renderer.
    UserConfig& userConfig{UserConfig::get()};
    Sprite& playerSprite{
        registry.emplace<Sprite>(newEntity, spriteData.get("roberto_0"))};
    registry.emplace<Camera>(newEntity, Camera::CenterOnEntity, Position{},
                             PreviousPosition{}, userConfig.getWindowSize());

    // Set up the player's bounding box, based on their sprite.
    registry.emplace<BoundingBox>(
        newEntity, Transforms::modelToWorldCentered(playerSprite.modelBounds,
                                                    playerPosition));

    // Set up the player's InputHistory component.
    registry.emplace<InputHistory>(newEntity);

    // Flag that we just moved and need to request all map data.
    registry.emplace<NeedsAdjacentChunks>(newEntity);
}

void Simulation::fakeConnection()
{
    // Create the player entity.
    entt::registry& registry = world.registry;
    entt::entity newEntity = registry.create();

    // Save the player entity ID for convenience.
    world.playerEntity = newEntity;

    // Set up the player's sim components.
    registry.emplace<Name>(newEntity,
                           std::to_string(static_cast<Uint32>(newEntity)));
    Position& playerPosition{
        registry.emplace<Position>(newEntity, 0.0f, 0.0f, 0.0f)};
    registry.emplace<PreviousPosition>(newEntity, 0.0f, 0.0f, 0.0f);
    registry.emplace<Velocity>(newEntity, 0.0f, 0.0f, 20.0f, 20.0f);
    registry.emplace<Input>(newEntity);

    // Set up the player's visual components.
    // TODO: Switch to logical screen size and do scaling in Renderer.
    UserConfig& userConfig{UserConfig::get()};
    Sprite& playerSprite{
        registry.emplace<Sprite>(newEntity, spriteData.get("roberto_0"))};
    registry.emplace<Camera>(newEntity, Camera::CenterOnEntity, Position{},
                             PreviousPosition{}, userConfig.getWindowSize());

    // Set up the player's bounding box, based on their sprite.
    registry.emplace<BoundingBox>(
        newEntity, Transforms::modelToWorldCentered(playerSprite.modelBounds,
                                                    playerPosition));
    // TODO: Update our placement in the spatial partition.

    // Set up the player's InputHistory component.
    registry.emplace<InputHistory>(newEntity);
}

void Simulation::tick()
{
    /* Calculate what tick we should be on. */
    // Increment the tick to the next.
    Uint32 targetTick{currentTick + 1};

    // If we're online, apply any adjustments that we receive from the
    // server.
    if (!Config::RUN_OFFLINE) {
        int adjustment{network.transferTickAdjustment()};
        if (adjustment != 0) {
            targetTick += adjustment;

            // Make sure NPC replication takes the adjustment into account.
            replicationTickOffset.applyAdjustment(adjustment);
        }
    }

    /* Process ticks until we match what the server wants.
       This may cause us to not process any ticks, or to process multiple
       ticks. */
    while (currentTick < targetTick) {
        /* Run all systems. */
        // Call the project's pre-everything logic.
        if (extension != nullptr) {
            extension->beforeAll();
        }

        // Process chunk updates from the server.
        chunkUpdateSystem.updateChunks();

        // Process tile updates from the UI and server.
        tileUpdateSystem.updateTiles();

        // Process entities that need to be constructed or destructed.
        npcLifetimeSystem.processUpdates();

        // Call the project's pre-movement logic.
        if (extension != nullptr) {
            extension->afterMapAndLifetimeUpdates();
        }

        // Process the held user input state and send change requests to the
        // server.
        // Note: Mouse and momentary inputs are processed through our OS event
        //       handling, prior to this tick.
        playerInputSystem.processHeldInputs();

        // Push the new input state into the player's history.
        playerInputSystem.addCurrentInputsToHistory();

        // Process player movement.
        playerMovementSystem.processMovement();

        // Process NPC movement.
        npcMovementSystem.updateNpcs();

        // Move all cameras to their new positions.
        cameraSystem.moveCameras();

        // Call the project's post-movement logic.
        if (extension != nullptr) {
            extension->afterMovement();
        }

        currentTick++;
    }
}

World& Simulation::getWorld()
{
    return world;
}

Uint32 Simulation::getCurrentTick()
{
    return currentTick;
}

Uint32 Simulation::getReplicationTick()
{
    return (currentTick + replicationTickOffset.get());
}

bool Simulation::handleOSEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEMOTION: {
            playerInputSystem.processMouseState(event.motion);
            return true;
        }
        default: {
            // Default to assuming it's a momentary input.
            playerInputSystem.processMomentaryInput(event);
            return true;
        }
    }

    return false;
}

void Simulation::setExtension(std::unique_ptr<ISimulationExtension> inExtension)
{
    extension = std::move(inExtension);
}

} // namespace Client
} // namespace AM
