#include "../include/UdpSocket.h"
#include <QByteArray>


// -----------------------------------------------------------------------------
// UdpSocket class implementation
//

bool UdpSocket::metaTypesRegistered = false;

UdpSocket::UdpSocket() :
	m_socket(this),
	m_timer(this)
{
	registerMetaTypes();
}


UdpSocket::~UdpSocket()
{

}

void UdpSocket::registerMetaTypes()
{
	if (metaTypesRegistered)
	{
		return;
	}

	qRegisterMetaType<UdpRequest>("UdpRequest");

	metaTypesRegistered = true;
}


// -----------------------------------------------------------------------------
// UdpClientSocket class implementation
//


UdpClientSocket::UdpClientSocket(const QHostAddress &serverAddress, quint16 port) :
    m_serverAddress(serverAddress),
	m_port(port)
{
	m_timer.setSingleShot(true);

	// generate unique clientID
	//
	m_clientID = qHash(QUuid::createUuid());

	connect(&m_timer, &QTimer::timeout, this, &UdpClientSocket::onAckTimerTimeout);
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
}


void UdpClientSocket::onSocketThreadFinishedSlot()
{
    onSocketThreadFinished();

    deleteLater();
}


void UdpClientSocket::onSocketThreadFinished()
{
}


void UdpClientSocket::onSocketReadyRead()
{
    m_mutex.lock();

    assert(m_state == UdpClientSocketState::waitingForAck);

	m_timer.stop();

	QHostAddress address;
	quint16 port = 0;

	qint64 recevedDataSize = m_socket.readDatagram(m_ack.rawData(), MAX_DATAGRAM_SIZE, &address, &port);

	if (recevedDataSize == -1)
	{
		assert(false);
		return;
	}

	m_ack.setAddress(address);
	m_ack.setPort(port);
	m_ack.setRawDataSize(recevedDataSize);

	assert(m_ack.dataSize() == m_ack.headerDataSize());

	m_ack.initRead();

	m_state = UdpClientSocketState::readyToSend;

	bool unknownAck = true;

	if (m_request.ID() == m_ack.ID() &&
		m_request.clientID() == m_ack.clientID() &&
		m_request.no() == m_ack.no())
	{
		unknownAck = false;
	}

	m_mutex.unlock();


	if (unknownAck)
    {
		emit unknownAckReceived(m_ack);
    }
    else
    {
		emit ackReceived(m_ack);
    }
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


quint16 UdpClientSocket::port() const
{
    return m_port;
}


void UdpClientSocket::setPort(quint16 port)
{
    m_mutex.lock();

    m_port = port;

    m_mutex.unlock();
}


void UdpClientSocket::sendRequest(UdpRequest request)
{
    QMutexLocker m(&m_mutex);

	if (m_state != UdpClientSocketState::readyToSend)
	{
		assert(m_state == UdpClientSocketState::readyToSend);
		return;
	}

	m_request.setAddress(m_serverAddress);
	m_request.setPort(m_port);

	m_request.setID(request.ID());

	m_request.setClientID(m_clientID);
	m_request.setVersion(m_protocolVersion);
	m_request.setNo(m_requestNo);
	m_request.setErrorCode(RQERROR_OK);

	m_request.initWrite();

	if (request.dataSize() > 0)
	{
		m_request.writeData(request.data(), request.dataSize());
	}

	qint64 sent = m_socket.writeDatagram(m_request.rawData(), m_request.rawDataSize(), m_serverAddress, m_port);

    if (sent == -1)
    {
		assert(false);
    }

    m_requestNo++;

    m_state = UdpClientSocketState::waitingForAck;

    m_retryCtr = 0;

	m_timer.start(m_msTimeout);
}


void UdpClientSocket::retryRequest()
{
    assert(m_state == UdpClientSocketState::waitingForAck);

	m_socket.writeDatagram(m_request.rawData(), m_request.rawDataSize(), m_serverAddress, m_port);

	m_timer.start(m_msTimeout);
}


void UdpClientSocket::onAckTimerTimeout()
{
    m_mutex.lock();

    m_ackTimeoutCtr++;

    m_retryCtr++;

    if (m_retryCtr < m_retryCount)
    {
        retryRequest();
    }
    else
    {
       m_retryCtr = 0;
       m_state = UdpClientSocketState::readyToSend;

	   emit ackTimeout(m_request);
    }

	m_mutex.unlock();
}


void UdpClientSocket::onAckTimeout()
{
}


void UdpClientSocket::onRequestTimeout(const RequestHeader& /* requestHeader */)
{
}


// -----------------------------------------------------------------------------
// UDP Server classes implementation
//


// -----------------------------------------------------------------------------
// UdpRequest class implementation
//

UdpRequest::UdpRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 receivedDataSize) :
    m_address(senderAddress),
	m_port(senderPort)
{
	memset(m_header, sizeof(RequestHeader), 0);

    if (receivedData != nullptr && (receivedDataSize >= sizeof(RequestHeader) && receivedDataSize <= MAX_DATAGRAM_SIZE))
    {
		memcpy(m_rawData, receivedData, m_rawDataSize);

		m_rawDataSize = receivedDataSize;
    }
    else
    {
        assert(false);
    }
}


UdpRequest::UdpRequest()
{
	memset(m_header, sizeof(RequestHeader), 0);
}


void UdpRequest::initAck(const UdpRequest& request)
{
    assert(!request.isEmpty());

    m_address = request.m_address;
    m_port = request.m_port;

	memset(m_header, sizeof(RequestHeader), 0);

	// copy request header
	//
	memcpy(m_rawData, request.m_rawData, sizeof(RequestHeader));

	m_rawDataSize = sizeof(RequestHeader);

	m_header->errorCode = RQERROR_OK;
	m_header->dataSize = 0;

	m_writeDataPtr = m_data;		// initWrite
}


bool UdpRequest::writeDword(quint32 dw)
{
	if (m_rawDataSize + sizeof(quint32) > MAX_DATAGRAM_SIZE)
    {
		assert(m_rawDataSize + sizeof(quint32) <= MAX_DATAGRAM_SIZE);
        return false;
    }

	*reinterpret_cast<quint32*>(m_writeDataPtr) = dw;

	m_writeDataPtr += sizeof(quint32);

	m_header->dataSize += sizeof(quint32);

	m_rawDataSize += sizeof(quint32);

    return true;
}


bool UdpRequest::writeData(const char* data, quint32 dataSize)
{
	if (data == nullptr)
	{
		assert(data != nullptr);
		return false;
	}

	if (m_rawDataSize + dataSize > MAX_DATAGRAM_SIZE)
    {
		assert(m_rawDataSize + dataSize <= MAX_DATAGRAM_SIZE);
        return false;
    }

	memcpy(m_writeDataPtr, data, dataSize);

	m_writeDataPtr += dataSize;

	m_header->dataSize += dataSize;

	m_rawDataSize += dataSize;

    return true;
}


bool UdpRequest::writeStruct(Serializable* s)
{
	if (s == nullptr)
	{
		assert(s != nullptr);
		return false;
	}

	m_writeDataPtr = s->serializeTo(m_writeDataPtr);

	m_header->dataSize += s->size();
	m_rawDataSize += s->size();

	assert(m_rawDataSize <= MAX_DATAGRAM_SIZE);

	return true;
}


quint32 UdpRequest::readDword()
{
	if (m_readDataPtr - m_data + sizeof(quint32) > m_header->dataSize)
	{
		assert(m_readDataPtr - m_data + sizeof(quint32) <= m_header->dataSize);
		return 0;
	}

	quint32 result = *reinterpret_cast<quint32*>(m_readDataPtr);

	m_readDataPtr += sizeof(quint32);

	return result;
}


void UdpRequest::readStruct(Serializable* s)
{
	m_readDataPtr = s->serializeFrom(m_readDataPtr);
}


// -----------------------------------------------------------------------------
// UdpRequestProcessor class implementation
//


UdpRequestProcessor::UdpRequestProcessor() :
    m_clientRequestHandler(nullptr)
{
}


void UdpRequestProcessor::setClientRequestHandler(UdpClientRequestHandler* clientRequestHandler)
{
    m_clientRequestHandler = clientRequestHandler;
}


void UdpRequestProcessor::onThreadStartedSlot()
{
    onThreadStarted();
}


void UdpRequestProcessor::onThreadFinishedSlot()
{
    onThreadFinished();
    deleteLater();
}


void UdpRequestProcessor::onRequestQueueIsNotEmpty()
{
    assert(m_clientRequestHandler != nullptr);

    int count = 0;

    do
    {
        UdpRequest request = m_clientRequestHandler->getRequest();

        if (request.isEmpty())
        {
            break;
        }

        processRequest(request);

        count++;

    } while (m_clientRequestHandler->hasRequest() && count < 5);
}


UdpClientRequestHandler::UdpClientRequestHandler(UdpRequestProcessor* udpRequestProcessor) :
    m_lastRequestTime(0)
{
    udpRequestProcessor->setClientRequestHandler(this);
    udpRequestProcessor->moveToThread(&m_handlerThread);

    connect(&m_handlerThread, &QThread::started, udpRequestProcessor, &UdpRequestProcessor::onThreadStartedSlot);
    connect(&m_handlerThread, &QThread::finished, udpRequestProcessor, &UdpRequestProcessor::onThreadFinishedSlot);

    connect(this, &UdpClientRequestHandler::requestQueueIsNotEmpty, udpRequestProcessor, &UdpRequestProcessor::onRequestQueueIsNotEmpty);

    m_handlerThread.start();
}


// -----------------------------------------------------------------------------
// UdpClientRequestHandler class implementation
//


UdpClientRequestHandler::~UdpClientRequestHandler()
{
    m_handlerThread.quit();
    m_handlerThread.wait();
}


qint64 UdpClientRequestHandler::lastRequestTime() const
{
    return m_lastRequestTime;
}


void UdpClientRequestHandler::putRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 recevedDataSize)
{
    m_queueMutex.lock();

    m_lastRequestTime = QDateTime::currentMSecsSinceEpoch();

    requestQueue.enqueue(UdpRequest(senderAddress, senderPort, receivedData, recevedDataSize));

    m_queueMutex.unlock();

    emit requestQueueIsNotEmpty();
}


UdpRequest UdpClientRequestHandler::getRequest()
{
    m_queueMutex.lock();

    if (requestQueue.isEmpty())
    {
        m_queueMutex.unlock();

        return UdpRequest();
    }

    UdpRequest request = requestQueue.dequeue();

    m_queueMutex.unlock();

    return request;
}


bool UdpClientRequestHandler::hasRequest()
{
    bool result = false;

    m_queueMutex.lock();

    result = !requestQueue.isEmpty();

    m_queueMutex.unlock();

    return result;
}


// -----------------------------------------------------------------------------
// UdpServerSocket class implementation
//

UdpServerSocket::UdpServerSocket(const QHostAddress &bindToAddress, quint16 port) :
    m_bindToAddress(bindToAddress),
	m_port(port)
{
}


UdpServerSocket::~UdpServerSocket()
{
}


void UdpServerSocket::onSocketThreadStartedSlot()
{
    m_timer.start(1000);

    connect(&m_timer, &QTimer::timeout, this, &UdpServerSocket::onTimer);
	connect(&m_socket, &QUdpSocket::readyRead, this, &UdpServerSocket::onSocketReadyRead);

	bind();

    onSocketThreadStarted();
}


void UdpServerSocket::onSocketThreadStarted()
{
    qDebug() << "Called UdpServerSocket::onSocketThreadStarted";
}


void UdpServerSocket::onSocketThreadFinishedSlot()
{
    onSocketThreadFinished();

    deleteLater();
}


void UdpServerSocket::bind()
{
	if (m_socket.state() != QAbstractSocket::BoundState)
	{
		bool res = m_socket.bind(m_bindToAddress, m_port);

		if (res)
		{
			qDebug() << "Socket bound on port " << m_port;
		}
	}
}


void UdpServerSocket::sendAck(UdpRequest request)
{
	qint64 sent = m_socket.writeDatagram(request.rawData(), request.rawDataSize(), request.address(), request.port());

    if (sent == -1)
    {
		assert(false);
    }
}


void UdpServerSocket::onSocketThreadFinished()
{
    qDebug() << "Called UdpServerSocket::onSocketThreadFinished";
}


void UdpServerSocket::onTimer()
{
	bind();

	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    m_clientMapMutex.lock();

    QHashIterator<quint32, UdpClientRequestHandler*> i(clientRequestHandlerMap);

    while (i.hasNext())
    {
        i.next();

        UdpClientRequestHandler* clientHandler = i.value();

        if (clientHandler == nullptr)
        {
            assert(false);
        }
        else
        {
            qint64 dtime = currentTime - clientHandler->lastRequestTime();

            if (dtime > 5 * 1000)
            {
                // time from last request more then 5 sec
                //
                quint32 clientID = i.key();

                clientRequestHandlerMap.remove(clientID);

                delete clientHandler;
            }
        }
    }

    m_clientMapMutex.unlock();
}


void UdpServerSocket::onSocketReadyRead()
{
	QHostAddress address;
	quint16 port = 0;

	qint64 recevedDataSize = m_socket.readDatagram(m_request.rawData(), MAX_DATAGRAM_SIZE, &address, &port);

	if (recevedDataSize == -1)
	{
		assert(false);
		return;
	}

	m_request.setAddress(address);
	m_request.setPort(port);
	m_request.setRawDataSize(recevedDataSize);

	assert(m_request.dataSize() == m_request.headerDataSize());

	m_request.initRead();

	emit receiveRequest(m_request);

/*    UdpClientRequestHandler* clientRequestHandler = nullptr;

    quint32 clientID = requestHeader->ClientID;

    m_clientMapMutex.lock();

    if (clientRequestHandlerMap.contains(clientID))
    {
        clientRequestHandler = clientRequestHandlerMap.value(clientID);

        qDebug() << "clientRequestHandler found";
    }
    else
    {
        clientRequestHandler = new UdpClientRequestHandler(createUdpRequestProcessor());

        clientRequestHandlerMap.insert(clientID, clientRequestHandler);

        qDebug() << "clientRequestHandler created";
    }

    clientRequestHandler->putRequest(m_senderHostAddr, m_senderPort, m_receivedData, m_recevedDataSize);

    m_clientMapMutex.unlock();*/
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

    connect(&m_socketThread, &QThread::started, clientSocket, &UdpClientSocket::onSocketThreadStartedSlot);
    connect(&m_socketThread, &QThread::finished, clientSocket, &UdpClientSocket::onSocketThreadFinishedSlot);

    m_socketThread.start();
}


void UdpSocketThread::run(UdpServerSocket* serverSocket)
{
    serverSocket->moveToThread(&m_socketThread);

    connect(&m_socketThread, &QThread::started, serverSocket, &UdpServerSocket::onSocketThreadStartedSlot);
    connect(&m_socketThread, &QThread::finished, serverSocket, &UdpServerSocket::onSocketThreadFinishedSlot);

    m_socketThread.start();
}



UdpSocketThread::~UdpSocketThread()
{
    m_socketThread.quit();
    m_socketThread.wait();
}

