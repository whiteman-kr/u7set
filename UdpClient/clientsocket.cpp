#include "clientsocket.h"

ClientSocket::ClientSocket(const QHostAddress& serverAddres, quint16 port) :
    UdpClientSocket(serverAddres, port)
{
}
