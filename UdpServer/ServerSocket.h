#pragma once

#include "../include/UdpSocket.h"


class TestRequestProcessor : public UdpRequestProcessor
{
    Q_OBJECT

public:
    TestRequestProcessor(QDateTime& lastStartTime, bool& isRunning) : lastStartTime(lastStartTime), isRunning(isRunning) {}

    void processRequest(const UdpRequest& request) override;

signals:
    void ackIsReady(UdpRequest request);

private:
    QDateTime& lastStartTime;
    bool& isRunning;
};


class ServerSocket : public UdpServerSocket
{
    Q_OBJECT

public:
    ServerSocket(const QHostAddress& bindToAddress, quint16 port);
    virtual ~ServerSocket();

    virtual UdpRequestProcessor* createUdpRequestProcessor();

private:
    QDateTime lastStartTime;
    bool isRunning;
};


