#include "ServerSocket.h"


void TestRequestProcessor::processRequest(const UdpRequest& request)
{
    if (request.isEmpty())
    {
        return;
    }

    qDebug() << "Request processing...";
}

ServerSocket::ServerSocket(const QHostAddress &bindToAddress, quint16 port) :
    UdpServerSocket(bindToAddress, port)
{
}


ServerSocket::~ServerSocket()
{
}


UdpRequestProcessor* ServerSocket::createUdpRequestProcessor()
{
    return new TestRequestProcessor;
}
