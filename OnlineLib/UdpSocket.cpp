#ifndef ONLINE_LIB_DOMAIN
#error Don't include this file in the project! Link OnlineLib instead.
#endif

#include "UdpSocket.h"

// -----------------------------------------------------------------------------
//
// UdpRequest class implementation
//
// -----------------------------------------------------------------------------

UdpRequest::UdpRequest()
{
	memset(header(), 0, sizeof(RequestHeader));
}

UdpRequest::UdpRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 receivedDataSize) :
	m_address(senderAddress),
	m_port(senderPort)
{
	memset(header(), 0, sizeof(RequestHeader));

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

UdpRequest&UdpRequest::operator =(const UdpRequest& request)
{
	m_address = request.address();
	m_port = request.port();
	m_rawDataSize = request.rawDataSize();

	if (m_rawDataSize < sizeof(RequestHeader))
	{
		assert(m_rawDataSize >= sizeof(RequestHeader));
		m_rawDataSize = sizeof(RequestHeader);
	}
	if (m_rawDataSize > sizeof(m_rawData))
	{
		assert(m_rawDataSize > sizeof(m_rawData));
		m_rawDataSize = sizeof(m_rawData);
	}

	memcpy(m_rawData, request.rawData(), m_rawDataSize);

	m_readDataIndex = request.m_readDataIndex;
	m_writeDataIndex = request.m_writeDataIndex;

	return *this;
}

void UdpRequest::initAck(const UdpRequest& request)
{
	assert(!request.isEmpty());

	m_address = request.m_address;
	m_port = request.m_port;

	memset(header(), 0, sizeof(RequestHeader));

	// copy request header
	//
	memcpy(m_rawData, request.m_rawData, sizeof(RequestHeader));

	m_rawDataSize = sizeof(RequestHeader);

	header()->errorCode = RQERROR_OK;
	header()->dataSize = 0;

	m_writeDataIndex = 0;		// initWrite
}

void UdpRequest::initWrite()
{
	m_writeDataIndex = 0;
	header()->dataSize = 0;
	m_rawDataSize = sizeof(RequestHeader);
}

bool UdpRequest::writeData(const char* data, qint64 dataSize)
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

	memcpy(writeDataPtr(), data, dataSize);

	m_writeDataIndex += static_cast<unsigned int>(dataSize);

	header()->dataSize += static_cast<quint32>(dataSize);

	m_rawDataSize += dataSize;

	return true;
}

bool UdpRequest::writeDword(quint32 dw)
{
	if (m_rawDataSize + sizeof(quint32) > MAX_DATAGRAM_SIZE)
	{
		assert(m_rawDataSize + sizeof(quint32) <= MAX_DATAGRAM_SIZE);
		return false;
	}

	*reinterpret_cast<quint32*>(writeDataPtr()) = dw;

	m_writeDataIndex += sizeof(quint32);

	header()->dataSize += sizeof(quint32);

	m_rawDataSize += sizeof(quint32);

	return true;
}

bool UdpRequest::writeData(const QByteArray& data)
{
	return writeData(data.constData(), data.size());
}

bool UdpRequest::writeData(google::protobuf::Message& protobufMessage)
{
	int messageSize = static_cast<int>(protobufMessage.ByteSizeLong());

	if (m_rawDataSize + messageSize > MAX_DATAGRAM_SIZE)
	{
		assert(m_rawDataSize + messageSize <= MAX_DATAGRAM_SIZE);
		return false;
	}

	protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(writeDataPtr()));

	m_writeDataIndex += messageSize;

	header()->dataSize += messageSize;

	m_rawDataSize += messageSize;

	return true;
}

void UdpRequest::initRead()
{
	m_readDataIndex = 0;
}

quint32 UdpRequest::readDword()
{
	if (readDataPtr() - data() + sizeof(quint32) > header()->dataSize)
	{
		assert(readDataPtr() - data() + sizeof(quint32) <= header()->dataSize);
		return 0;
	}

	quint32 result = *reinterpret_cast<quint32*>(readDataPtr());

	m_readDataIndex += sizeof(quint32);

	return result;
}

// -----------------------------------------------------------------------------
//
// UdpSocket class implementation
//
// -----------------------------------------------------------------------------

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
	if (metaTypesRegistered == true)
	{
		return;
	}

	qRegisterMetaType<UdpRequest>("UdpRequest");

	metaTypesRegistered = true;
}

// -----------------------------------------------------------------------------
//
// UdpClientSocket class implementation
//
// -----------------------------------------------------------------------------

UdpClientSocket::UdpClientSocket(const QHostAddress &serverAddress, quint16 port) :
	m_serverAddress(serverAddress),
	m_port(port)
{
}

UdpClientSocket::~UdpClientSocket()
{
}

const QHostAddress& UdpClientSocket::serverAddress() const
{
	return m_serverAddress;
}

void UdpClientSocket::setServerAddress(const QHostAddress& serverAddress)
{
	QMutexLocker locker(&m_mutex);

	m_serverAddress = serverAddress;
}

quint16 UdpClientSocket::port() const
{
	return m_port;
}

void UdpClientSocket::setPort(quint16 port)
{
	QMutexLocker locker(&m_mutex);

	m_port = port;
}

bool UdpClientSocket::isWaitingForAck() const
{
	AUTO_LOCK(m_mutex)

	bool result = (m_state == WaitingForAck);

	return result;
}

bool UdpClientSocket::isReadyToSend() const
{
	AUTO_LOCK(m_mutex)

	return m_state == ReadyToSend;
}

void UdpClientSocket::sendRequest(const UdpRequest& udpRequest)
{
	emit sendRequestSignal(udpRequest);
}

void UdpClientSocket::sendRequest(quint32 requestID)
{
	UdpRequest request;

	request.setID(requestID);

	emit sendRequestSignal(request);
}

void UdpClientSocket::onSendRequest(UdpRequest request)
{
	AUTO_LOCK(m_mutex)

	if (m_state != State::ReadyToSend)
	{
		//qDebug() << "request: " << request.ID() << " last ack: " << m_ack.ID() << " last request: " << m_request.ID();
		assert(m_state == State::ReadyToSend);
		return;
	}

	m_request.setAddress(m_serverAddress);
	m_request.setPort(m_port);

	assert(request.ID() != 0);
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
		//assert(false);
	}

	m_requestNo++;

	m_state = State::WaitingForAck;

	m_retryCtr = 0;

	m_timer.start(m_msTimeout);
}

void UdpClientSocket::onSocketReadyRead()
{
	AUTO_LOCK(m_mutex);

	if (m_state != State::WaitingForAck)
	{
		return;
	}

	QHostAddress address;
	quint16 port = 0;

	qint64 recevedDataSize = m_socket.readDatagram(m_ack.rawData(), MAX_DATAGRAM_SIZE, &address, &port);

	m_state = State::ReadyToSend;

	if (recevedDataSize == -1)
	{
		QAbstractSocket::SocketError err = m_socket.error();
		Q_UNUSED(err)

		return;
	}

	m_timer.stop();

	m_ack.setAddress(address);
	m_ack.setPort(port);
	m_ack.setRawDataSize(recevedDataSize);

	assert(m_ack.dataSize() == m_ack.headerDataSize());

	m_ack.initRead();

	bool unknownAck = true;

	if (m_request.ID() == m_ack.ID() &&
		m_request.clientID() == m_ack.clientID() &&
		m_request.no() == m_ack.no())
	{
		unknownAck = false;
	}

	if (unknownAck)
	{
		emit unknownAckReceived(m_ack);
	}
	else
	{
		assert(m_ack.data() == m_ack.readDataPtr());
		emit ackReceived(m_ack);
	}
}

void UdpClientSocket::onAckTimerTimeout()
{
	QMutexLocker locker(&m_mutex);

	m_ackTimeoutCtr++;

	m_retryCtr++;

	if (m_retryCtr < m_retryCount)
	{
		retryRequest();
	}
	else
	{
	   m_retryCtr = 0;
	   m_state = State::ReadyToSend;

	   emit ackTimeout(m_request);

	   // qDebug() << "Ack timeout: server " << m_request.address().toString() << " : " << m_request.port();
	}
}

void UdpClientSocket::retryRequest()
{
	assert(m_state == State::WaitingForAck);

	m_socket.writeDatagram(m_request.rawData(), m_request.rawDataSize(), m_serverAddress, m_port);

	m_timer.start(m_msTimeout);
}

void UdpClientSocket::onThreadStarted()
{
	m_timer.setSingleShot(true);

	// generate unique clientID
	//
	m_clientID = qHash(QUuid::createUuid());

	connect(&m_timer, &QTimer::timeout, this, &UdpClientSocket::onAckTimerTimeout);
	connect(&m_socket, &QUdpSocket::readyRead, this, &UdpClientSocket::onSocketReadyRead);

	connect(this, &UdpClientSocket::sendRequestSignal, this, &UdpClientSocket::onSendRequest);

	onSocketThreadStarted();
}

void UdpClientSocket::onThreadFinished()
{
	onSocketThreadFinished();
}

// -----------------------------------------------------------------------------
//
// UdpRequestProcessor class implementation
//
// -----------------------------------------------------------------------------

UdpRequestProcessor::UdpRequestProcessor()
{
}

UdpRequestProcessor::~UdpRequestProcessor()
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

// -----------------------------------------------------------------------------
//
// UdpClientRequestHandler class implementation
//
// -----------------------------------------------------------------------------

UdpClientRequestHandler::UdpClientRequestHandler(UdpRequestProcessor* udpRequestProcessor)
{
	udpRequestProcessor->setClientRequestHandler(this);
	udpRequestProcessor->moveToThread(&m_handlerThread);

	connect(&m_handlerThread, &QThread::started, udpRequestProcessor, &UdpRequestProcessor::onThreadStartedSlot);
	connect(&m_handlerThread, &QThread::finished, udpRequestProcessor, &UdpRequestProcessor::onThreadFinishedSlot);

	connect(this, &UdpClientRequestHandler::requestQueueIsNotEmpty, udpRequestProcessor, &UdpRequestProcessor::onRequestQueueIsNotEmpty);

	m_handlerThread.start();
}

UdpClientRequestHandler::~UdpClientRequestHandler()
{
	m_handlerThread.quit();
	m_handlerThread.wait();
}

void UdpClientRequestHandler::putRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 recevedDataSize)
{
	QMutexLocker locker(&m_queueMutex);

	m_lastRequestTime = QDateTime::currentMSecsSinceEpoch();

	requestQueue.enqueue(UdpRequest(senderAddress, senderPort, receivedData, recevedDataSize));

	locker.unlock();

	emit requestQueueIsNotEmpty();
}

UdpRequest UdpClientRequestHandler::getRequest()
{
	QMutexLocker locker(&m_queueMutex);

	if (requestQueue.isEmpty())
	{
		return UdpRequest();
	}

	UdpRequest request = requestQueue.dequeue();

	return request;
}

bool UdpClientRequestHandler::hasRequest()
{
	bool result = false;

	QMutexLocker locker(&m_queueMutex);

	result = !requestQueue.isEmpty();

	return result;
}

qint64 UdpClientRequestHandler::lastRequestTime() const
{
	return m_lastRequestTime;
}

// -----------------------------------------------------------------------------
//
// UdpServerSocket class implementation
//
// -----------------------------------------------------------------------------

UdpServerSocket::UdpServerSocket(const QHostAddress &bindToAddress, quint16 port, std::shared_ptr<CircularLogger> logger) :
	m_bindToAddress(bindToAddress),
	m_port(port),
	m_logger(logger)
{
}

UdpServerSocket::~UdpServerSocket()
{
}

void UdpServerSocket::onSocketThreadStarted()
{
}

void UdpServerSocket::onSocketThreadFinished()
{
}

void UdpServerSocket::sendAck(UdpRequest request)
{
	qint64 sent = m_socket.writeDatagram(request.rawData(), request.rawDataSize(), request.address(), request.port());

	if (sent == -1)
	{
		assert(false);
	}
}

void UdpServerSocket::onTimer()
{
	bind();
}

void UdpServerSocket::onSocketReadyRead()
{
	QHostAddress address;
	quint16 port = 0;

	qint64 recevedDataSize = m_socket.readDatagram(m_request.rawData(), MAX_DATAGRAM_SIZE, &address, &port);

	if (recevedDataSize == -1)
	{
		return;
	}

	m_request.setAddress(address);
	m_request.setPort(port);
	m_request.setRawDataSize(recevedDataSize);

	assert(m_request.dataSize() == m_request.headerDataSize());

	m_request.initRead();

	emit receiveRequest(m_request);
}


void UdpServerSocket::onThreadStarted()
{
	m_timer.start(1000);

	connect(&m_timer, &QTimer::timeout, this, &UdpServerSocket::onTimer);
	connect(&m_socket, &QUdpSocket::readyRead, this, &UdpServerSocket::onSocketReadyRead);

	bind();

	onSocketThreadStarted();
}

void UdpServerSocket::onThreadFinished()
{
	onSocketThreadFinished();

	DEBUG_LOG_MSG(m_logger, QString(tr("UdpServerSocket thread finished (listen %1)")).
				  arg(HostAddressPort(m_bindToAddress, m_port).addressPortStr()));
}

void UdpServerSocket::bind()
{
	if (m_socket.state() == QAbstractSocket::BoundState)
	{
		return;
	}

	bool result = m_socket.bind(m_bindToAddress, m_port);

	if (result == true)
	{
		DEBUG_LOG_MSG(m_logger, QString(tr("UdpServerSocket listen on  %1").
							  arg(HostAddressPort(m_bindToAddress, m_port).addressPortStr())));
	}
}
