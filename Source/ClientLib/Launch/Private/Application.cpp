#include "Application.h"
#include "Config.h"
#include "SharedConfig.h"
#include "UserConfig.h"
#include "Paths.h"
#include "Log.h"

#include <SDL.h>

#include <memory>
#include "Ignore.h"

namespace AM
{
namespace Client
{
Application::Application()
: sdl(SDL_INIT_VIDEO)
, userConfigInitializer()
, sdlWindow("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            static_cast<int>(UserConfig::get().getWindowSize().width),
            static_cast<int>(UserConfig::get().getWindowSize().height),
            SDL_WINDOW_SHOWN)
, sdlRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)
, assetCache(sdlRenderer.Get())
, spriteData(assetCache)
, userInterface()
, uiCaller(std::bind_front(&UserInterface::tick, &userInterface),
           Config::UI_TICK_TIMESTEP_S, "UserInterface", true)
, network()
, networkCaller(std::bind_front(&Network::tick, &network),
                SharedConfig::NETWORK_TICK_TIMESTEP_S, "Network", true)
, simulation(userInterface.getEventDispatcher(), network, spriteData)
, simCaller(std::bind_front(&Simulation::tick, &simulation),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, renderer(sdlRenderer.Get(), simulation.getWorld(), userInterface, spriteData,
           std::bind_front(&PeriodicCaller::getProgress, &simCaller))
, rendererCaller(std::bind_front(&Renderer::render, &renderer),
                 UserConfig::get().getFrameTimestepS(), "Renderer", true)
, eventHandlers{this, &renderer, &userInterface, &simulation}
, exitRequested(false)
{
    // Set fullscreen mode.
    unsigned int fullscreenMode{UserConfig::get().getFullscreenMode()};
    switch (fullscreenMode) {
        case 0: {
            sdlWindow.SetFullscreen(0);
            break;
        }
        case 1: {
            sdlWindow.SetFullscreen(SDL_WINDOW_FULLSCREEN_DESKTOP);
            break;
        }
        // Note: We removed real fullscreen because it was behaving weirdly and
        //       causing network timeouts, and having just windowed and
        //       borderless seems fine.
        default: {
            LOG_FATAL("Invalid fullscreen value: %d", fullscreenMode);
            break;
        }
    }

    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);

    // Set up our event filter.
    SDL_SetEventFilter(&Application::filterEvents, this);
}

void Application::start()
{
    // Connect to the server (waits for connection response).
    simulation.connect();

    // Prime the timers so they don't start at 0.
    simCaller.initTimer();
    uiCaller.initTimer();
    networkCaller.initTimer();
    rendererCaller.initTimer();
    while (!exitRequested) {
        // Let the simulation process an iteration if it needs to.
        simCaller.update();

        // Let the UI widgets tick if they need to.
        uiCaller.update();

        // Send a heartbeat if necessary.
        networkCaller.update();

        // Let the renderer render if it needs to.
        rendererCaller.update();

        // If we have enough time, dispatch events.
        if (enoughTimeTillNextCall(DISPATCH_MINIMUM_TIME_S)) {
            dispatchOSEvents();
        }

        // If we have enough time, sleep.
        if (enoughTimeTillNextCall(Config::SLEEP_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

bool Application::handleOSEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_QUIT: {
            exitRequested = true;
            return true;
        }
    }

    return false;
}

void Application::dispatchOSEvents()
{
    // Dispatch all waiting SDL events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Pass the event to each handler in order, stopping if it returns as
        // handled.
        for (OSEventHandler* handler : eventHandlers) {
            if (handler->handleOSEvent(event)) {
                break;
            }
        }
    }
}

bool Application::enoughTimeTillNextCall(double minimumTime)
{
    if ((simCaller.getTimeTillNextCall() > minimumTime)
        && (networkCaller.getTimeTillNextCall() > minimumTime)
        && (rendererCaller.getTimeTillNextCall() > minimumTime)) {
        return true;
    }
    else {
        return false;
    }
}

int Application::filterEvents(void* userData, SDL_Event* event)
{
    Application* app{static_cast<Application*>(userData)};
    ignore(event);
    ignore(app);

    // Currently no events that we care to filter.
    //
    // switch (event->type) {
    // }

    return 1;
}

} // End namespace Client
} // End namespace AM
