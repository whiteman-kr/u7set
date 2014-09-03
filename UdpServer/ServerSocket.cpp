#include "ServerSocket.h"
#include <QBuffer>



TestRequestProcessor::TestRequestProcessor(QDateTime &lastStartTime, QDateTime &runTime, bool &isRunning) :
    lastStartTime(lastStartTime),
    runTime(runTime),
    isRunning(isRunning)
{
}


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
    case RQID_GET_SERVICE_INFO:
        {
            QByteArray array;/* = QByteArray::fromRawData(newRequest.m_requestData + sizeof(REQUEST_HEADER), MAX_DATAGRAM_SIZE - sizeof(REQUEST_HEADER));*/
            QBuffer buffer(&array);
            buffer.open(QBuffer::WriteOnly);
            QDataStream out(&buffer);
            QDateTime currentTime = QDateTime::currentDateTime();
            out << RQSTP_CONFIG;
            out << quint32(0);   // Major version
            out << quint32(1);   // Minor version
            out << quint32(2);   // BuildNo
            out << quint32(0xffffffff);  // CRC
            out << quint32(runTime.secsTo(currentTime));
            out << quint32(isRunning ? SS_MF_WORK : SS_MF_STOPPED);
            out << quint32(isRunning ? lastStartTime.secsTo(currentTime) : 0);
            header->DataLen = array.size();
            memcpy(newRequest.m_requestData + sizeof(REQUEST_HEADER), array.constData(), array.size());
            newRequest.m_requestDataSize = sizeof(REQUEST_HEADER) + header->DataLen;
            emit ackIsReady(newRequest);
            return;
        }
    case RQID_SERVICE_MF_START:
        {
            if (isRunning)
            {
                return;
            }
            isRunning = true;
            lastStartTime = QDateTime::currentDateTime();
            emit ackIsReady(newRequest);
            return;
        }
        break;
    case RQID_SERVICE_MF_STOP:
        {
            if (!isRunning)
            {
                return;
            }
            isRunning = false;
            emit ackIsReady(newRequest);
            return;
        }
        break;
    case RQID_SERVICE_MF_RESTART:
        {
            if (!isRunning)
            {
                return;
            }
            lastStartTime = QDateTime::currentDateTime();
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
    lastStartTime(QDateTime::currentDateTime()),
    runTime(QDateTime::currentDateTime())
{
}


ServerSocket::~ServerSocket()
{
}


UdpRequestProcessor* ServerSocket::createUdpRequestProcessor()
{
    UdpRequestProcessor* processor = new TestRequestProcessor(lastStartTime, runTime, isRunning);
    connect(processor, SIGNAL(ackIsReady(UdpRequest)), this, SLOT(sendAck(UdpRequest)));
    return processor;
}
