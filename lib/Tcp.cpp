#include "../include/Tcp.h"

namespace Tcp
{

	// -------------------------------------------------------------------------------------
	//
	// Tcp::SocketWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	SocketWorker::SocketWorker() :
		m_mutex(QMutex::Recursive)
	{
		m_dataBuffer = new char[TCP_MAX_DATA_SIZE];
	}


	SocketWorker::~SocketWorker()
	{
		delete [] m_dataBuffer;
	}

	void SocketWorker::onThreadStarted()
	{
		createSocket();
	}


	void SocketWorker::createSocket()
	{
		deleteSocket();

		m_tcpSocket = new QTcpSocket;

		m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 0);

		connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
		connect(m_tcpSocket, &QTcpSocket::connected, this, &SocketWorker::onSocketConnected);
		connect(m_tcpSocket, &QTcpSocket::disconnected, this, &SocketWorker::onSocketDisconnected);
		connect(m_tcpSocket, &QTcpSocket::readyRead, this, &SocketWorker::onSocketReadyRead);
	}


	void SocketWorker::deleteSocket()
	{
		if (m_tcpSocket != nullptr)
		{
			m_tcpSocket->close();
			delete m_tcpSocket;
			m_tcpSocket = nullptr;
		}
	}


	void SocketWorker::onThreadFinished()
	{
		deleteSocket();
	}


	void SocketWorker::onSocketConnected()
	{
		initReadStatusVariables();
		onConnection();
	}


	void SocketWorker::onSocketDisconnected()
	{
		onDisconnection();

		emit disconnected(this);
	}


	void SocketWorker::onSocketReadyRead()
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		int bytesAvailable = m_tcpSocket->bytesAvailable();
		int bytesReaded = 0;

		while(bytesAvailable > 0)
		{
			switch(m_readState)
			{
			case ReadState::WaitingAnything:
				assert(false);
				return;

			case ReadState::WaitingForHeader:
				bytesReaded = readHeader(bytesAvailable);
				break;

			case ReadState::WaitingForData:
				bytesReaded = readData(bytesAvailable);
				break;

			default:
				assert(false);
			}

			bytesAvailable -= bytesReaded;

			if (m_headerAndDataReady)
			{
				// prepare to read next request
				//
				m_headerAndDataReady = false;
				m_readedHeaderSize = 0;
				m_readedDataSize = 0;

				onHeaderAndDataReady();
			}
		}
	}


	int SocketWorker::readHeader(int bytesAvailable)
	{
		if (m_readState != ReadState::WaitingForHeader)
		{
			assert(false);
			return 0;
		}

		int bytesToRead = sizeof(SocketWorker::Header) - m_readedHeaderSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		int bytesReaded = m_tcpSocket->read(reinterpret_cast<char*>(&m_header) + m_readedHeaderSize, bytesToRead);

		//qDebug() << "Read header bytes " << bytesReaded;

		m_readedHeaderSize += bytesReaded;

		assert(m_readedHeaderSize <= sizeof(SocketWorker::Header));

		if (m_readedHeaderSize < sizeof(SocketWorker::Header))
		{
			return bytesReaded;
		}

		// Full requestHeader is readed
		//

		if (m_header.checkCRC() == false)
		{
			assert(false);

			closeConnection();

			qDebug() << "Request header CRC error!";

			return 0;
		}

		if (m_header.dataSize == 0)
		{
			m_headerAndDataReady = true;

			m_readState = ReadState::WaitingAnything;

			return bytesReaded;
		}

		if (m_header.dataSize > TCP_MAX_DATA_SIZE)
		{
			assert(false);

			closeConnection();

			qDebug() << "Request" << m_header.id << "dataSize too big - " << m_header.dataSize;

			return 0;
		}

		m_readState = ReadState::WaitingForData;

		return bytesReaded;
	}


	int SocketWorker::readData(int bytesAvailable)
	{
		if (m_readState != ReadState::WaitingForData)
		{
			assert(false);
			return 0;
		}

		int bytesToRead = m_header.dataSize - m_readedDataSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		if (m_readedDataSize + bytesToRead > TCP_MAX_DATA_SIZE)
		{
			assert(false);

			closeConnection();

			qDebug() << "Out of buffer m_requestData";

			return 0;
		}

		int bytesReaded = m_tcpSocket->read(m_dataBuffer + m_readedDataSize, bytesToRead);

		//qDebug() << "Read data bytes " << bytesReaded;

		m_readedDataSize += bytesReaded;

		assert(m_readedDataSize <= m_header.dataSize);

		if (m_readedDataSize == m_header.dataSize)
		{
			m_headerAndDataReady = true;

			m_readState = ReadState::WaitingAnything;
		}

		return bytesReaded;
	}


	qint64 SocketWorker::socketWrite(const char* data, qint64 size)
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return -1;
		}

		qint64 written = m_tcpSocket->write(data, size);

		if (written == -1)
		{
			return -1;
		}

		m_tcpSocket->waitForBytesWritten(TCP_BYTES_WRITTEN_TIMEOUT);

		return written;
	}


	qint64 SocketWorker::socketWrite(const Header& header)
	{
		return socketWrite(reinterpret_cast<const char*>(&header), sizeof(header));
	}


	void SocketWorker::onSocketStateChanged(QAbstractSocket::SocketState newState)
	{
		QString stateStr;

		switch(newState)
		{
		case QAbstractSocket::UnconnectedState:
			stateStr = "Socket state: UnconnectedState";
			break;

		case QAbstractSocket::HostLookupState:
			stateStr = "Socket state: HostLookupState";
			break;

		case QAbstractSocket::ConnectingState:
			stateStr = "Socket state: ConnectingState";
			break;

		case QAbstractSocket::ConnectedState:
			stateStr = "Socket state: ConnectedState";
			break;

		case QAbstractSocket::BoundState:
			stateStr = "Socket state: BoundState";
			break;

		case QAbstractSocket::ClosingState:
			stateStr = "Socket state: ClosingState";
			break;

		default:
			assert(false);
		}

		qDebug() << qPrintable(stateStr);
	}


	bool SocketWorker::isConnected() const
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return false;
		}

		return m_tcpSocket->state() == QAbstractSocket::ConnectedState;
	}


	void SocketWorker::closeConnection()
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		m_tcpSocket->close();
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Server class implementation
	//
	// -------------------------------------------------------------------------------------

	int Server::staticId = 0;


	Server::Server() :
		m_autoAckTimer(this)
	{
		m_id = staticId;
		staticId++;

		initReadStatusVariables();

		m_autoAckTimer.setSingleShot(false);
	}


	Server::~Server()
	{
	}


	void Server::onThreadStarted()
	{
		connect(&m_autoAckTimer, &QTimer::timeout, this, &Server::onAutoAckTimer);

		SocketWorker::onThreadStarted();

		onServerThreadStarted();

		onConnection();
	}


	void Server::onThreadFinished()
	{
		onServerThreadFinished();

		SocketWorker::onThreadFinished();
	}


	void Server::createSocket()
	{
		assert(m_connectedSocketDescriptor != 0);

		SocketWorker::createSocket();

		m_tcpSocket->setSocketDescriptor(m_connectedSocketDescriptor);
	}


	void Server::initReadStatusVariables()
	{
		m_serverState = ServerState::WainigForRequest;
		m_readState = ReadState::WaitingForHeader;
		m_readedHeaderSize = 0;
		m_readedDataSize = 0;
	}

	void Server::onConnection()
	{
	}


	void Server::onDisconnection()
	{
	}


	void Server::setConnectedSocketDescriptor(qintptr connectedSocketDescriptor)
	{
		m_connectedSocketDescriptor = connectedSocketDescriptor;
	}


	void Server::onHeaderAndDataReady()
	{
		assert(m_serverState == ServerState::WainigForRequest);

		if (m_header.type != Header::Request)
		{
			assert(false);
			return;
		}

		m_serverState = ServerState::RequestProcessing;

		if (m_autoAck == true)
		{
			m_autoAckTimer.start(TCP_AUTO_ACK_TIMER_INTERVAL);
		}

		m_requestProcessingPorgress = 0;

		processRequest(m_header.id, m_dataBuffer, m_header.dataSize);
	}


	void Server::onAutoAckTimer()
	{
		if (m_autoAck == false || m_serverState != ServerState::RequestProcessing)
		{
			m_autoAckTimer.stop();

			return;
		}

		sendAck();
	}


	void Server::sendAck()
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		if (m_serverState != Server::ServerState::RequestProcessing)
		{
			return;
		}

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Ack;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = 0;
		header.requestProcessingPorgress = m_requestProcessingPorgress;
		header.calcCRC();

		qint64 written = socketWrite(header);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < static_cast<qint64>(sizeof(header)))
		{
			assert(false);
			return;
		}
	}


	void Server::sendReply()
	{
		sendReply(nullptr, 0);
	}


	void Server::sendReply(const QByteArray& replyData)
	{
		sendReply(replyData.constData(), replyData.size());
	}


	void Server::sendReply(const char* replyData, quint32 replyDatsSize)
	{
		m_autoAckTimer.stop();

		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		if (m_serverState != ServerState::RequestProcessing)
		{
			assert(false);
			return;
		}

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Reply;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = replyDatsSize;
		header.requestProcessingPorgress = 100;
		header.calcCRC();

		qint64 written = socketWrite(header);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < static_cast<qint64>(sizeof(header)))
		{
			assert(false);
			return;
		}

		if (replyDatsSize > 0)
		{
			written = socketWrite(replyData, replyDatsSize);

			if (written == -1)
			{
				qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
				return;
			}

			if (written < replyDatsSize)
			{
				assert(false);
				return;
			}
		}

		initReadStatusVariables();
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::TcpServer class implementation
	//
	// -------------------------------------------------------------------------------------

	TcpServer::TcpServer(Listener* parent) :
		QTcpServer(parent)
	{
	}


	void TcpServer::incomingConnection(qintptr socketDescriptor)
	{
		Listener* listener = dynamic_cast<Listener*>(parent());

		if (listener == nullptr)
		{
			assert(false);
			return;
		}

		listener->onNewConnection(socketDescriptor);
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Listener class implementation
	//
	// -------------------------------------------------------------------------------------

	Listener::Listener(const HostAddressPort& listenAddressPort, Server* server) :
		m_listenAddressPort(listenAddressPort),
		m_tcpServer(this),
		m_periodicTimer(this),
		m_serverInstance(server)
	{
		assert(m_serverInstance != nullptr);

		m_serverInstance->setParent(this);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Listener::onPeriodicTimer);
	}


	Listener::~Listener()
	{
		// close all conection threads
		//
		for(SimpleThread* connectionThread : m_runningServers)
		{
			connectionThread->quit();
			delete connectionThread;
		}

		m_runningServers.clear();

		delete m_serverInstance;
	}


	void Listener::onThreadStarted()
	{
		m_periodicTimer.setInterval(TCP_PERIODIC_TIMER_INTERVAL);
		m_periodicTimer.start();

		startListening();
	}


	void Listener::onThreadFinished()
	{
		m_tcpServer.close();
	}


	void Listener::startListening()
	{
		if (m_tcpServer.listen(m_listenAddressPort.address(), m_listenAddressPort.port()))
		{
			qDebug() << qPrintable(QString("Start listening %1 OK").arg(m_listenAddressPort.addressPortStr()));
		}
		else
		{
			qDebug() << qPrintable(QString("Error on start listening %1: %2").
									arg(m_listenAddressPort.addressPortStr()).
									arg(m_tcpServer.errorString()));
		}
	}


	void Listener::onPeriodicTimer()
	{
		if (!m_tcpServer.isListening())
		{
			startListening();
		}
	}


	void Listener::onNewConnection(qintptr socketDescriptor)
	{
		// accept new connection
		//
		Server* newServerInstance = m_serverInstance->getNewInstance();

		connect(newServerInstance, &Server::disconnected, this, &Listener::onServerDisconnected);

		newServerInstance->setConnectedSocketDescriptor(socketDescriptor);

		SimpleThread* newThread = new SimpleThread(newServerInstance);

		m_runningServers.insert(newServerInstance, newThread);

		newThread->start();

		qDebug() << "Accept new connection #" << newServerInstance->id();
	}


	void Listener::onAcceptError()
	{
	}


	void Listener::onServerDisconnected(const SocketWorker* server)
	{
		if (!m_runningServers.contains(server))
		{
			assert(false);
			return;
		}

		SimpleThread* thread = m_runningServers[server];

		m_runningServers.remove(server);

		thread->quit();

		delete thread;
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ServerThread class implementation
	//
	// -------------------------------------------------------------------------------------

	ServerThread::ServerThread(const HostAddressPort &listenAddressPort, Server* server) :
		SimpleThread(new Listener(listenAddressPort, server))
	{
	}


	ServerThread::~ServerThread()
	{
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ClientWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	Client::Client(const HostAddressPort &serverAddressPort) :
		m_periodicTimer(this),
		m_replyTimeoutTimer(this)
	{
		setServer(serverAddressPort, false);
		initReadStatusVariables();
	}


	Client::Client(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
		m_periodicTimer(this),
		m_replyTimeoutTimer(this)
	{
		setServers(serverAddressPort1, serverAddressPort2, false);
		initReadStatusVariables();
	}


	Client::~Client()
	{
	}


	void Client::setServer(const HostAddressPort& serverAddressPort, bool reconnect)
	{
		AUTO_LOCK(m_mutex)

		m_serversAddressPort[0] = serverAddressPort;
		m_serversAddressPort[1] = serverAddressPort;

		selectServer1(reconnect);
	}


	void Client::setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2, bool reconnect)
	{
		AUTO_LOCK(m_mutex)

		m_serversAddressPort[0] = serverAddressPort1;
		m_serversAddressPort[1] = serverAddressPort2;

		selectServer1(reconnect);
	}


	void Client::onThreadStarted()
	{
		onClientThreadStarted();

		SocketWorker::onThreadStarted();

		connect(&m_replyTimeoutTimer, &QTimer::timeout, this, &Client::onReplyTimeoutTimer);

		m_replyTimeoutTimer.setSingleShot(true);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Client::onPeriodicTimer);

		m_periodicTimer.setInterval(TCP_PERIODIC_TIMER_INTERVAL);
		m_periodicTimer.start();

		connectToServer();
	}


	void Client::onThreadFinished()
	{
		onClientThreadFinished();

		if (m_tcpSocket != nullptr)
		{
			m_tcpSocket->disconnectFromHost();
			m_tcpSocket->close();
		}
		else
		{
			assert(false);
		}

		SocketWorker::onThreadFinished();
	}


	void Client::initReadStatusVariables()
	{
		m_clientState = ClientState::ClearToSendRequest;
		m_readState = ReadState::WaitingAnything;
		m_readedHeaderSize = 0;
		m_readedDataSize = 0;
		m_connectTimeout = 0;
	}


	void Client::onConnection()
	{
		qDebug() << qPrintable(QString("Socket connected to server %1").arg(m_selectedServer.addressPortStr()));
	}


	void Client::onDisconnection()
	{
		qDebug() << qPrintable(QString("Socket disconnected from server %1").arg(m_selectedServer.addressPortStr()));
	}


	void Client::onHeaderAndDataReady()
	{
		if (m_clientState != ClientState::WaitingForReply)
		{
			assert(false);
			closeConnection();
			return;
		}

		if (m_header.id != m_sentRequestHeader.id ||
			m_header.numerator != m_sentRequestHeader.numerator)
		{
			assert(false);
			closeConnection();
			return;
		}

		switch(m_header.type)
		{
		case Header::Type::Ack:
			restartReplyTimeoutTimer();

			onAck();

			m_readState = ReadState::WaitingForHeader;

			break;

		case Header::Type::Reply:
			initReadStatusVariables();

			processReply(m_header.id, m_dataBuffer, m_header.dataSize);

			break;

		default:
			assert(false);
		}
	}


	void Client::autoSwitchServer()
	{
		AUTO_LOCK(m_mutex)

		if (m_autoSwitchServer == true)
		{
			if (m_selectedServerIndex == 0)
			{
				m_selectedServerIndex = 1;
			}
			else
			{
				m_selectedServerIndex = 0;
			}

			m_selectedServer = m_serversAddressPort[m_selectedServerIndex];
		}
	}


	void Client::selectServer(int serverIndex, bool reconnect)
	{
		AUTO_LOCK(m_mutex)

		if (serverIndex < 0 || serverIndex > 1)
		{
			assert(false);
			serverIndex = 0;
		}

		m_selectedServerIndex = serverIndex;
		m_selectedServer = m_serversAddressPort[m_selectedServerIndex];

		if (reconnect == true)
		{
			closeConnection();
		}
	}


	void Client::connectToServer()
	{
		AUTO_LOCK(m_mutex);

		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		qDebug() << qPrintable(QString("Try connect to server %1").arg(m_selectedServer.addressPortStr()));

		m_tcpSocket->connectToHost(m_selectedServer.address(), m_selectedServer.port());
	}


	void Client::onPeriodicTimer()
	{
		if (isConnected() == false)
		{
			m_connectTimeout++;

			if (m_connectTimeout >= TCP_CONNECT_TIMEOUT)
			{
				autoSwitchServer();
				createSocket();
				connectToServer();

				m_connectTimeout = 0;
			}
		}
	}


	void Client::onReplyTimeoutTimer()
	{
		onReplyTimeout();
		closeConnection();
	}


	bool Client::isClearToSendRequest() const
	{
		return isConnected() && m_clientState == ClientState::ClearToSendRequest;
	}


	void Client::restartReplyTimeoutTimer()
	{
		m_replyTimeoutTimer.start(TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT);
	}


	void Client::sendRequest(quint32 requestID)
	{
		sendRequest(requestID, nullptr, 0);
	}


	void Client::sendRequest(quint32 requestID, const QByteArray& requestData)
	{
		sendRequest(requestID, requestData.constData(), requestData.size());
	}


	void Client::sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		if (!isClearToSendRequest())
		{
			assert(false);
			return;
		}

		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		m_sentRequestHeader.type = Header::Type::Request;
		m_sentRequestHeader.id = requestID;
		m_sentRequestHeader.numerator = m_requestNumerator;
		m_sentRequestHeader.dataSize = requestDataSize;
		m_sentRequestHeader.calcCRC();

		m_requestNumerator++;

		qint64 written = socketWrite(m_sentRequestHeader);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < static_cast<qint64>(sizeof(m_sentRequestHeader)))
		{
			assert(false);
			return;
		}

		if (requestDataSize > 0)
		{
			if (requestData == nullptr)
			{
				assert(false);
				return;
			}

			written = socketWrite(requestData, requestDataSize);

			if (written == -1)
			{
				qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
				return;
			}

			if (written < requestDataSize)
			{
				assert(false);
				return;
			}
		}

		restartReplyTimeoutTimer();

		m_clientState = ClientState::WaitingForReply;
		m_readState = ReadState::WaitingForHeader;
	}

}
