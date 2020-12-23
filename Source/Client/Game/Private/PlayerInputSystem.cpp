#include "PlayerInputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "Input.h"
#include "PlayerState.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Client
{
PlayerInputSystem::PlayerInputSystem(Game& inGame, World& inWorld)
: game(inGame)
, world(inWorld)
{
}

void PlayerInputSystem::processMomentaryInput(SDL_Event& event)
{
    // We currently don't have any momentary inputs that we care about.
    ignore(event);
}

void PlayerInputSystem::processHeldInputs()
{
    const Uint8* keyStates = SDL_GetKeyboardState(nullptr);
    Input::StateArr newInputStates{};

    // Get the latest state of all the keys that we care about.
    if (keyStates[SDL_SCANCODE_W]) {
        newInputStates[Input::Up] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_A]) {
        newInputStates[Input::Left] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_S]) {
        newInputStates[Input::Down] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_D]) {
        newInputStates[Input::Right] = Input::Pressed;
    }

    // Update our saved input state.
    entt::registry& registry = world.registry;
    Input::StateArr& playerInputs = registry.get<Input>(world.playerEntity).inputStates;
    for (unsigned int inputType = 0; inputType < Input::Type::NumTypes;
         ++inputType) {
        // If the saved state doesn't match the latest.
        if (newInputStates[inputType] != playerInputs[inputType]) {
            // Save the new state.
            playerInputs[inputType] = newInputStates[inputType];

            // Mark the player as dirty.
            registry.patch<PlayerState>(world.playerEntity,
                [](PlayerState& state) {state.isDirty = true;});
        }
    }
}

void PlayerInputSystem::addCurrentInputsToHistory()
{
    CircularBuffer<Input::StateArr, PlayerState::INPUT_HISTORY_LENGTH>& playerInputHistory =
        world.registry.get<PlayerState>(world.playerEntity).inputHistory;
    Input::StateArr& playerInputs = world.registry.get<Input>(world.playerEntity).inputStates;

    // Push the player's current inputs into the player's input history.
    playerInputHistory.push(playerInputs);
}

} // namespace Client
} // namespace AM
