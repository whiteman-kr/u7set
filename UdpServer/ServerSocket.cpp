#include "ServerSocket.h"


ServerSocket::ServerSocket(const QHostAddress &bindToAddress, quint16 port) :
    UdpServerSocket(bindToAddress, port)
{
}


ServerSocket::~ServerSocket()
{
}


