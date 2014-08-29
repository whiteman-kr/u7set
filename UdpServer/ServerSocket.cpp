#include "ServerSocket.h"


void TestRequestProcessor::processRequest(const UdpRequest& request)
{
    if (request.isEmpty() || request.m_requestDataSize < sizeof(REQUEST_HEADER))
    {
        return;
    }

    qDebug() << "Request processing...";

    UdpRequest newRequest = request;
    REQUEST_HEADER* header = (REQUEST_HEADER*)newRequest.m_requestData;

    switch (header->ID) {
    case RQID_GET_SERVICE_STATE:
        {
            quint64& time = *(quint64*)(newRequest.m_requestData + sizeof(REQUEST_HEADER));
            header->DataLen = sizeof(quint64);
            newRequest.m_requestDataSize = sizeof(REQUEST_HEADER) + header->DataLen;
            if (isRunning)
            {
                time = (QDateTime::currentDateTime().secsTo(lastStartTime));
            }
            else
            {
                time = 0;
            }
            emit ackIsReady(newRequest);
            return;
        }
        break;
    default:
        break;
    }
}

ServerSocket::ServerSocket(const QHostAddress &bindToAddress, quint16 port) :
    UdpServerSocket(bindToAddress, port),
    isRunning(true),
    lastStartTime(QDateTime::currentDateTime())
{
}


ServerSocket::~ServerSocket()
{
}


UdpRequestProcessor* ServerSocket::createUdpRequestProcessor()
{
    UdpRequestProcessor* processor = new TestRequestProcessor(lastStartTime, isRunning);
    connect(processor, SIGNAL(ackIsReady(UdpRequest)), this, SLOT(sendAck(UdpRequest)));
    return processor;
}
