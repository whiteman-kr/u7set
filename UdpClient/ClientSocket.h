#pragma once

#include "../lib/UdpSocket.h"

class ClientSocket : public UdpClientSocket
{
public:
    ClientSocket(const QHostAddress& serverAddres, quint16 port);
};

