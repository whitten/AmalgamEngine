#include "Acceptor.h"
#include "Log.h"

namespace AM
{
Acceptor::Acceptor(Uint16 port, const std::shared_ptr<SocketSet>& inClientSet)
: socket(port)
, listenerSet(1)
, clientSet(inClientSet)
{
    listenerSet.addSocket(socket);
}

Acceptor::~Acceptor() {}

std::unique_ptr<Peer> Acceptor::accept()
{
    listenerSet.checkSockets(0);

    if (socket.isReady()) {
        std::unique_ptr<TcpSocket> newSocket{socket.accept()};
        if (newSocket != nullptr) {
            return std::make_unique<Peer>(std::move(newSocket), clientSet);
        }
        else {
            LOG_FATAL("Listener socket showed ready, but accept() failed.");
        }
    }

    return nullptr;
}

bool Acceptor::reject()
{
    listenerSet.checkSockets(0);

    bool peerWasWaiting{false};
    if (socket.isReady()) {
        std::unique_ptr<TcpSocket> newSocket{socket.accept()};
        if (newSocket != nullptr) {
            peerWasWaiting = true;
        }
        else {
            LOG_FATAL("Listener socket showed ready, but accept() failed.");
        }
    }

    return peerWasWaiting;
}

} // namespace AM
