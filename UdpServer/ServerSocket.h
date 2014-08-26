#pragma once

#include "../include/UdpSocket.h"

class ServerSocket : public UdpServerSocket
{
    Q_OBJECT

public:
    ServerSocket(const QHostAddress& bindToAddress, quint16 port);
    virtual ~ServerSocket();

};


