#pragma once

#include "OSEventHandler.h"

namespace AM
{
namespace Client
{

/**
 * Defines an extension for the engine's Client::Simulation class.
 *
 * Extensions are implemented by the project, and are given generic functions
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through
 * Application::registerSimulationExtension().
 */
class ISimulationExtension : public OSEventHandler
{
public:
    // Canonical constructor (derived class must implement):
    // SimulationExtension(SimulationExDependencies deps)

    /**
     * Called before any systems are ran.
     */
    virtual void beforeAll() = 0;

    /**
     * Called after the tile map is updated and NPCs are added/removed.
     */
    virtual void afterMapAndLifetimeUpdates() = 0;

    /**
     * Called after all entity movement has been processed.
     */
    virtual void afterMovement() = 0;

    /**
     * See OSEventHandler for details.
     *
     * Note: Simulation will pass events to this class first. If the event is
     *       not handled, then Simulation will attempt to handle it.
     */
    bool handleOSEvent(SDL_Event& event) override = 0;
};

} // namespace Client
} // namespace AM
