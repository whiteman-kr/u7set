#pragma once

#include "../include/UdpSocket.h"


class TestRequestProcessor : public UdpRequestProcessor
{
public:
    TestRequestProcessor() {}

    void processRequest(const UdpRequest& request) override;
};


class ServerSocket : public UdpServerSocket
{
    Q_OBJECT

public:
    ServerSocket(const QHostAddress& bindToAddress, quint16 port);
    virtual ~ServerSocket();

    virtual UdpRequestProcessor* createUdpRequestProcessor();
};


