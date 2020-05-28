#include "Network.h"
#include "Peer.h"
#include "NetworkHelpers.h"
#include <SDL2/SDL_net.h>
#include "Message_generated.h"
#include "GameDefs.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

const std::string Network::SERVER_IP = "127.0.0.1";

Network::Network()
: server(nullptr)
, playerID(0)
, receiveThreadObj()
, exitRequested(false)
{
    SDLNet_Init();
}

Network::~Network()
{
    exitRequested = true;
    receiveThreadObj.join();
    SDLNet_Quit();
}

bool Network::connect()
{
    IPaddress ip;

    // Try to connect.
    server = Peer::initiate(SERVER_IP, SERVER_PORT);

    // Spin up the receive thread.
    if (server != nullptr) {
        receiveThreadObj = std::thread(Network::pollForMessages, this);
    }

    return (server != nullptr);
}

void Network::registerPlayerID(EntityID inPlayerID)
{
    playerID = inPlayerID;
}

void Network::send(BinaryBufferSharedPtr message)
{
    if (!(server->isConnected())) {
        DebugError("Tried to send while server is disconnected.");
    }

    // Send the message.
    NetworkResult result = server->send(message);
    if (result != NetworkResult::Success) {
        DebugError("Send failed.");
    }
}

BinaryBufferSharedPtr Network::receive(MessageType type, Uint64 timeoutMs)
{
    if (!(server->isConnected())) {
        DebugInfo("Tried to receive while server is disconnected.");
        return nullptr;
    }

    MessageQueue* selectedQueue = nullptr;
    switch (type)
    {
        case (MessageType::ConnectionResponse):
            selectedQueue = &connectionResponseQueue;
            break;
        case (MessageType::PlayerUpdate):
            selectedQueue = &playerUpdateQueue;
            break;
        case (MessageType::NpcUpdate):
            selectedQueue = &npcUpdateQueue;
            break;
        default:
            DebugError("Provided unexpected type.");
    }

    BinaryBufferSharedPtr message = nullptr;
    if (timeoutMs == 0) {
        selectedQueue->try_dequeue(message);
    }
    else {
        selectedQueue->wait_dequeue_timed(message, timeoutMs * 1000);
    }

    return message;
}

int Network::pollForMessages(void* inNetwork)
{
    Network* network = static_cast<Network*>(inNetwork);
    std::shared_ptr<Peer> server = network->getServer();
    std::atomic<bool> const* exitRequested = network->getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Wait for a server header.
        ReceiveResult receiveResult = server->receiveBytesWait(SERVER_HEADER_SIZE);

        if (receiveResult.result == NetworkResult::Success) {
            BinaryBufferSharedPtr header = receiveResult.message;

            // Extract the data from the header.
            Sint8 tickOffsetAdjustment =
                header->data()[ServerHeaderIndex::TickOffsetAdjustment];
            Uint8 messageCount = header->data()[ServerHeaderIndex::MessageCount];

            // Receive all of the expected messages.
            for (unsigned int i = 0; i < messageCount; ++i) {
                ReceiveResult receiveResult = server->receiveMessageWait();

                // If we received a message, push it into the appropriate queue.
                if (receiveResult.result == NetworkResult::Success) {
                    BinaryBufferSharedPtr message = receiveResult.message;

                    network->processReceivedMessage(message);
                }
            }
        }
    }

    return 0;
}

void Network::processReceivedMessage(BinaryBufferSharedPtr messageBuffer)
{
    const fb::Message* message = fb::GetMessage(messageBuffer->data());

    /* Funnel the message into the appropriate queue. */
    if (message->content_type() == fb::MessageContent::ConnectionResponse) {
        if (!(connectionResponseQueue.enqueue(std::move(messageBuffer)))) {
            DebugError("Ran out of room in queue and memory allocation failed.");
        }
    }
    else if (message->content_type() == fb::MessageContent::EntityUpdate) {
        auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());
//        DebugInfo("Received message with tick = %u", entityUpdate->currentTick());

        // Pull out the vector of entities.
        auto entities = entityUpdate->entities();

        // Iterate through the entities, checking if there's player or npc data.
        bool playerFound = false;
        bool npcFound = false;
        for (auto entityIt = entities->begin(); entityIt != entities->end(); ++entityIt) {
            EntityID entityID = (*entityIt)->id();

            if (entityID == playerID) {
                // Found the player.
                if (!(playerUpdateQueue.enqueue(std::move(messageBuffer)))) {
                    DebugError("Ran out of room in queue and memory allocation failed.");
                    playerFound = true;
                }
            }
            else if (!npcFound){
                // Found a non-player (npc).
                // TODO: Add npc movement back in.
//                if (!(npcUpdateQueue.enqueue(std::move(messageBuffer)))) {
//                    DebugError("Ran out of room in queue and memory allocation failed.");
//                    npcFound = true;
//                }
            }

            // If we found the player and an npc, we can stop looking.
            if (playerFound && npcFound) {
                return;
            }
        }
    }
}

std::shared_ptr<Peer> Network::getServer()
{
    return server;
}

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Client
} // namespace AM
