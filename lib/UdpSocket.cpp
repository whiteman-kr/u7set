#include "../include/UdpSocket.h"



// -----------------------------------------------------------------------------
// UdpServerSocketWorker class implementation
//


UdpClientSocket::UdpClientSocket(const QHostAddress &serverAddress, quint16 port) :
    m_state(readyToSend),
    m_serverAddress(serverAddress),
    m_port(port),
    m_msTimeout(100),
    m_retryCount(0),
    m_retryCtr(0),
    m_ackTimeoutCtr(0),
    m_protocolVersion(1),
    m_ackTimer(this),
    m_socket(this),
    m_recevedDataSize(0),
    m_sentDataSize(0),
    m_senderPort(0),
    m_requestNo(0),
    m_receivedHeader(reinterpret_cast<REQUEST_HEADER*>(m_receivedData)),
    m_sentHeader(reinterpret_cast<REQUEST_HEADER*>(m_sentData))
{
    m_ackTimer.setSingleShot(true);

    m_clientID = qHash(QUuid::createUuid());            // generate unique clientID

    connect(&m_ackTimer, SIGNAL(timeout()), this, SLOT(onAckTimerTimeout()));
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
}


UdpClientSocket::~UdpClientSocket()
{
}


void UdpClientSocket::onSocketThreadStartedSlot()
{
    onSocketThreadStarted();
}


void UdpClientSocket::onSocketThreadStarted()
{
    qDebug() << "Called UdpClientSocket::onSocketThreadStarted()";
}


void UdpClientSocket::onSocketThreadFinishedSlot()
{
    onSocketThreadFinished();

    this->deleteLater();
}


void UdpClientSocket::onSocketThreadFinished()
{
    qDebug() << "Called UdpClientSocket::onSocketThreadFinished()";
}


void UdpClientSocket::onSocketReadyRead()
{
    m_mutex.lock();

    assert(m_state == UdpClientSocketState::waitingForAck);

    m_ackTimer.stop();

    m_recevedDataSize = m_socket.readDatagram(m_receivedData, MAX_DATAGRAM_SIZE, &m_senderHostAddr, &m_senderPort);

    if (m_sentHeader->ID == m_receivedHeader->ID &&
        m_sentHeader->ClientID == m_receivedHeader->ClientID &&
        m_sentHeader->No == m_receivedHeader->No)
    {
        assert(m_receivedHeader->DataLen + sizeof(REQUEST_HEADER) <= MAX_DATAGRAM_SIZE);

        onRequestAck(*m_receivedHeader, m_receivedData + sizeof(REQUEST_HEADER), m_receivedHeader->DataLen);
    }
    else
    {
        onUnknownRequestAck(*m_receivedHeader, m_receivedData + sizeof(REQUEST_HEADER), m_receivedHeader->DataLen);
    }

    m_state = UdpClientSocketState::readyToSend;

    m_mutex.unlock();
}


void UdpClientSocket::onRequestAck(const REQUEST_HEADER& /* ackHeader */, char* /* ackData */, quint32 /* ackDataSize */)
{
    qDebug() << "Called UdpClientSocket::onRequestAck()";
}


void UdpClientSocket::onUnknownRequestAck(const REQUEST_HEADER& /* ackHeader */, char* /* ackData */, quint32 /* ackDataSize */)
{
    qDebug() << "Called UdpClientSocket::onUnknownRequestAck()";
}


const QHostAddress& UdpClientSocket::serverAddress() const
{
    return m_serverAddress;
}


void UdpClientSocket::setServerAddress(const QHostAddress& serverAddress)
{
    m_mutex.lock();

    m_serverAddress = serverAddress;

    m_mutex.unlock();
}


const quint16 UdpClientSocket::port() const
{
    return m_port;
}


void UdpClientSocket::setPort(quint16 port)
{
    m_mutex.lock();

    m_port = port;

    m_mutex.unlock();
}


void UdpClientSocket::sendRequest(quint32 requestID, char *requestData, quint32 requestDataSize)
{
    QMutexLocker m(&m_mutex);

    assert(m_state == UdpClientSocketState::readyToSend);

    m_sentHeader->ID = requestID;
    m_sentHeader->ClientID = m_clientID;
    m_sentHeader->Version = m_protocolVersion;
    m_sentHeader->No = m_requestNo;
    m_sentHeader->ErrorCode = 0;

    if (requestData != nullptr && requestDataSize > 0)
    {
        memcpy(m_sentData + sizeof(REQUEST_HEADER), requestData, requestDataSize);
    }
    else
    {
        requestDataSize = 0;
    }

    m_sentHeader->DataLen = requestDataSize;

    m_sentDataSize = sizeof(REQUEST_HEADER) + requestDataSize;

    qint64 sent = m_socket.writeDatagram(m_sentData, m_sentDataSize, m_serverAddress, m_port);

    if (sent == -1)
    {
        qDebug() << "UdpClientSocket::sendRequest writeDatagram error!";

    }
    else
    {
        qDebug() << "UdpClientSocket::sendRequest OK";
    }

    m_requestNo++;

    m_state = UdpClientSocketState::waitingForAck;

    m_retryCtr = 0;

    m_ackTimer.start(m_msTimeout);
}


void UdpClientSocket::retryRequest()
{
    assert(m_state == UdpClientSocketState::waitingForAck);

    m_socket.writeDatagram(QByteArray(m_sentData, m_sentDataSize), m_serverAddress, m_port);

    m_ackTimer.start(m_msTimeout);
}


void UdpClientSocket::onAckTimerTimeout()
{
    m_mutex.lock();

    m_ackTimeoutCtr++;

    m_retryCtr++;

    if (m_retryCtr < m_retryCount)
    {
        retryRequest();

        m_mutex.unlock();

        onAckTimeout();
    }
    else
    {
       m_retryCtr = 0;
       m_state = UdpClientSocketState::readyToSend;

       REQUEST_HEADER requestHeader;

       memcpy(&requestHeader, m_sentData, sizeof(REQUEST_HEADER));

       m_mutex.unlock();

       onRequestTimeout(requestHeader);
    }
}


void UdpClientSocket::onAckTimeout()
{
    qDebug() << "Called UdpClientSocket::onAckTimeout()";
}


void UdpClientSocket::onRequestTimeout(const REQUEST_HEADER& /* requestHeader */)
{
    qDebug() << "Called UdpClientSocket::onRequestTimeout()";
}


// -----------------------------------------------------------------------------
// UdpServerSocket class implementation
//

UdpRequest::UdpRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 receivedDataSize) :
    m_senderAddress(senderAddress),
    m_senderPort(senderPort),
    m_requestDataSize(0)
{
    if (receivedData != nullptr && (receivedDataSize >= sizeof(REQUEST_HEADER) && receivedDataSize <= MAX_DATAGRAM_SIZE))
    {
        m_requestDataSize = receivedDataSize;
        memcpy(m_requestData, receivedData, m_requestDataSize);
    }
    else
    {
        assert(false);
    }
}


UdpRequestProcessor::UdpRequestProcessor()
{
}


void UdpRequestProcessor::PutRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 recevedDataSize)
{

}


UdpServerSocket::UdpServerSocket(const QHostAddress &bindToAddress, quint16 port) :
    m_bindToAddress(bindToAddress),
    m_port(port),
    m_secondsTimer(this),
    m_socket(this),
    m_recevedDataSize(0),
    m_senderPort(0),
    requestHeader(reinterpret_cast<REQUEST_HEADER*>(m_receivedData))
{
}


UdpServerSocket::~UdpServerSocket()
{
}


void UdpServerSocket::onSocketThreadStartedSlot()
{
    m_secondsTimer.start(1000);

    connect(&m_secondsTimer, &QTimer::timeout, this, &UdpServerSocket::onSecondsTimer);
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyReadSlot()));

    m_socket.bind(m_bindToAddress, m_port);

    onSocketThreadStarted();
}


void UdpServerSocket::onSocketThreadStarted()
{
    qDebug() << "Called UdpServerSocket::onSocketThreadStarted";
}


void UdpServerSocket::onSocketThreadFinishedSlot()
{
    onSocketThreadFinished();

    this->deleteLater();
}


void UdpServerSocket::onSocketThreadFinished()
{
    qDebug() << "Called UdpServerSocket::onSocketThreadFinished";
}


void UdpServerSocket::onSecondsTimer()
{
}


void UdpServerSocket::onSocketReadyReadSlot()
{
    QMutexLocker m(&m_mutex);

    m_recevedDataSize = m_socket.readDatagram(m_receivedData, MAX_DATAGRAM_SIZE, &m_senderHostAddr, &m_senderPort);

    datagramReceived();

    UdpRequestProcessor* requestProcessor = nullptr;

    quint32 clientID = requestHeader->ClientID;

    if (requestProcessorMap.contains(clientID))
    {
        requestProcessor = requestProcessorMap.value(clientID);
    }
    else
    {
        requestProcessor = new UdpRequestProcessor;

        requestProcessorMap.insert(clientID, requestProcessor);
    }

    requestProcessor->PutRequest(m_senderHostAddr, m_senderPort, m_receivedData, m_recevedDataSize);

    //m_socket.writeDatagram(m_receivedDatagram, m_recevedDatagramSize, m_senderHostAddr, m_senderPort);
}


void UdpServerSocket::datagramReceived()
{
     qDebug() << "Called UdpServerSocket::datagramReceived";
}


// -----------------------------------------------------------------------------
// UdpSocketThread class implementation
//

UdpSocketThread::UdpSocketThread()
{
}


void UdpSocketThread::run(UdpClientSocket* clientSocket)
{
    clientSocket->moveToThread(&m_socketThread);

    connect(&m_socketThread, SIGNAL(started()), clientSocket, SLOT(onSocketThreadStartedSlot()));
    connect(&m_socketThread, SIGNAL(finished()), clientSocket, SLOT(onSocketThreadFinishedSlot()));

    m_socketThread.start();
}


void UdpSocketThread::run(UdpServerSocket* serverSocket)
{
    serverSocket->moveToThread(&m_socketThread);

    connect(&m_socketThread, SIGNAL(started()), serverSocket, SLOT(onSocketThreadStartedSlot()));
    connect(&m_socketThread, SIGNAL(finished()), serverSocket, SLOT(onSocketThreadFinishedSlot()));

    m_socketThread.start();
}



UdpSocketThread::~UdpSocketThread()
{
    m_socketThread.quit();
    m_socketThread.wait();
}

