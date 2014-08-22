#include "../include/UdpSocket.h"


// -------------------------------------------------------------------
// UDP Client classes implementations
//

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

    connect(&m_ackTimer, &QTimer::timeout, this, &UdpClientSocket::onAckTimerTimeout);
    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpClientSocket::onSocketReadyRead);
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
    qDebug() << "Called UdpClientSocketWorker::onSocketThreadStarted()";
}


void UdpClientSocket::onSocketThreadFinishedSlot()
{
    onSocketThreadFinished();

    this->deleteLater();
}


void UdpClientSocket::onSocketThreadFinished()
{
    qDebug() << "Called UdpClientSocketWorker::onSocketThreadFinished()";
}


void UdpClientSocket::onSocketReadyRead()
{
    m_mutex.lock();

    assert(m_state == UdpClientSocketState::waitingForAck);

    m_recevedDataSize = m_socket.readDatagram(reinterpret_cast<char*>(m_receivedData), MAX_DATAGRAM_SIZE, &m_senderHostAddr, &m_senderPort);

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
    qDebug() << "Called UdpClientSocketWorker::onRequestAck()";
}


void UdpClientSocket::onUnknownRequestAck(const REQUEST_HEADER& /* ackHeader */, char* /* ackData */, quint32 /* ackDataSize */)
{
    qDebug() << "Called UdpClientSocketWorker::onUnknownRequestAck()";
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

    m_socket.writeDatagram(QByteArray(m_sentData, m_sentDataSize), m_serverAddress, m_port);

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
    qDebug() << "Called UdpClientSocketWorker::onAckTimeout()";
}


void UdpClientSocket::onRequestTimeout(const REQUEST_HEADER& /* requestHeader */)
{
    qDebug() << "Called UdpClientSocketWorker::onRequestTimeout()";
}


// UdpSocketThread class implementation
//

UdpSocketThread::UdpSocketThread()
{
}


void UdpSocketThread::runClientSocket(UdpClientSocket* clientSocket)
{
    clientSocket->moveToThread(&m_socketThread);

    connect(&m_socketThread, &QThread::started, clientSocket, &UdpClientSocket::onSocketThreadStartedSlot);
    connect(&m_socketThread, &QThread::finished, clientSocket, &UdpClientSocket::onSocketThreadFinishedSlot);

    m_socketThread.start();
}


UdpSocketThread::~UdpSocketThread()
{
    m_socketThread.quit();
    m_socketThread.wait();
}



// -------------------------------------------------------------------
// UDP Server classes implementations
//


// UdpServerSocketWorker class implementation
//


UdpServerSocketWorker::UdpServerSocketWorker(const QHostAddress &bindToAddress, quint16 port) :
    m_bindToAddress(bindToAddress),
    m_port(port),
    m_secondsTimer(this),
    m_socket(this),
    m_recevedDatagramSize(0),
    m_senderPort(0)
{
}


UdpServerSocketWorker::~UdpServerSocketWorker()
{
}


void UdpServerSocketWorker::onSocketThreadStarted()
{
    m_secondsTimer.start(1000);

    connect(&m_secondsTimer, &QTimer::timeout, this, &UdpServerSocketWorker::onSecondsTimer);

    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpServerSocketWorker::onSocketReadyRead);

    qDebug() << "Socket Thread started";
}


void UdpServerSocketWorker::onSecondsTimer()
{
    if (!m_socket.isValid())
    {
        qDebug() << "Socket is not valid";
    }

    if (m_socket.state() == QAbstractSocket::BoundState)
    {
        qDebug() << "Socket is bounded";
    }
    else
    {
        if (m_socket.bind(m_bindToAddress, m_port) == true)
        {
            qDebug() << "Bind OK";
        }
        else
        {
            qDebug() << "Bind failed";
        }
    }

}


void UdpServerSocketWorker::onSocketReadyRead()
{
    m_recevedDatagramSize = m_socket.readDatagram(m_receivedDatagram, MAX_DATAGRAM_SIZE, &m_senderHostAddr, &m_senderPort);

    datagramReceived();
}


// m_recevedDatagramSize
// m_receivedDatagram
//
void UdpServerSocketWorker::datagramReceived()
{
}


// UdpServerSocket class implementation
//

UdpServerSocket::UdpServerSocket(const QHostAddress& address, qint16 port)
{
    UdpServerSocketWorker* serverWorker = new UdpServerSocketWorker(address, port);

    serverWorker->moveToThread(&m_socketThread);

    connect(&m_socketThread, &QThread::started, serverWorker, &UdpServerSocketWorker::onSocketThreadStarted);
    connect(&m_socketThread, &QThread::finished, serverWorker, &QObject::deleteLater);

    m_socketThread.start();
}


UdpServerSocket::~UdpServerSocket()
{
    m_socketThread.quit();
    m_socketThread.wait();
}
