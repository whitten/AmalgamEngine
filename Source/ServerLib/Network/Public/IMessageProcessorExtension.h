#pragma once

#include "MessageType.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace Server
{

/**
 * Defines an extension for the engine's Server::MessageProcessor class.
 *
 * Extensions are implemented by the project, and are given generic functions
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through
 * Application::registerMessageProcessorExtension().
 */
class IMessageProcessorExtension
{
public:
    // Canonical constructor (derived class must implement):
    // MessageProcessorExtension(MessageProcessorExDependencies deps)

    /**
     * Called when a message is received that the engine doesn't have a handler
     * for.
     *
     * @param netID  The network ID of the client that the message came from.
     * @param messageType  The type of the received message.
     * @param messageBuffer A buffer containing a serialized message, starting
     *                      at index 0.
     * @param messageSize  The length in bytes of the message in messageBuffer.
     *
     * @return If the message corresponds to a particular simulation tick,
     *         returns that tick number. If not, returns -1.
     */
    virtual Sint64
        processReceivedMessage(NetworkID netID, MessageType messageType,
                               Uint8* messageBuffer, unsigned int messageSize)
        = 0;
};

} // namespace Server
} // namespace AM
