#include "../lib/Tcp.h"
#include "../Proto/network.pb.h"
#include <stdlib.h>
#include "version.h"

namespace Tcp
{
	void ConnectionState::dump()
	{
		if (isConnected == false)
		{
			qDebug() << "\nTcp::ConnectionState - is not connected\n";
		}
		else
		{
			qDebug() << "\nTcp::ConnectionState - is connected";
			qDebug() << qPrintable(QString("Peer: %1").arg(peerAddr.addressPortStr()));
			qDebug() << qPrintable(QString("Start time: %1").arg(QDateTime::fromMSecsSinceEpoch(startTime).toString()));
			qDebug() << qPrintable(QString("Sent bytes: %1").arg(sentBytes));
			qDebug() << qPrintable(QString("Received bytes: %1").arg(receivedBytes));
			qDebug() << qPrintable(QString("Request count: %1").arg(requestCount));
			qDebug() << qPrintable(QString("Reply count: %1\n").arg(replyCount));
		}
	}

	// -------------------------------------------------------------------------------------
	//
	// Tcp::SocketWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	SocketWorker::SocketWorker(const SoftwareInfo& softwareInfo) :
		m_mutex(QMutex::Recursive),
		m_watchdogTimer(this)
	{
		m_state.localSoftwareInfo = softwareInfo;

		m_receiveDataBuffer = new char[TCP_MAX_DATA_SIZE];
	}

	SocketWorker::~SocketWorker()
	{
		delete [] m_receiveDataBuffer;
	}

	void SocketWorker::onThreadStarted()
	{
		createSocket();

		connect(&m_watchdogTimer, &QTimer::timeout, this, &SocketWorker::onWatchdogTimerTimeout);
	}

	void SocketWorker::createSocket()
	{
		AUTO_LOCK(m_mutex)

		deleteSocket();

		m_tcpSocket = new QTcpSocket;

		m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));

		connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
		connect(m_tcpSocket, &QTcpSocket::connected, this, &SocketWorker::onSocketConnected);
		connect(m_tcpSocket, &QTcpSocket::disconnected, this, &SocketWorker::onSocketDisconnected);
		connect(m_tcpSocket, &QTcpSocket::readyRead, this, &SocketWorker::onSocketReadyRead);
		connect(m_tcpSocket, &QTcpSocket::bytesWritten, this, &SocketWorker::onSocketBytesWritten);
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

		setStateConnected(HostAddressPort(m_tcpSocket->peerAddress(), m_tcpSocket->peerPort()));

		onInitConnection();
	}


	void SocketWorker::onSocketDisconnected()
	{
		onDisconnection();

		setStateDisconnected();

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

		addReceivedBytes(bytesAvailable);

		int bytesRead = 0;

		while(bytesAvailable > 0)
		{
			switch(m_readState)
			{
			case ReadState::WaitingNothing:
				assert(false);
				return;

			case ReadState::WaitingForHeader:
				bytesRead = readHeader(bytesAvailable);
				break;

			case ReadState::WaitingForData:
				bytesRead = readData(bytesAvailable);
				break;

			default:
				assert(false);
			}

			bytesAvailable -= bytesRead;

			if (m_headerAndDataReady)
			{
				// prepare to read next request
				//
				m_headerAndDataReady = false;
				m_readHeaderSize = 0;
				m_readDataSize = 0;

				onHeaderAndDataReady();
			}
		}
	}

	void SocketWorker::onSocketBytesWritten()
	{
		m_bytesWritten = true;
	}

	void SocketWorker::onWatchdogTimerTimeout()
	{
		if (m_watchdogTimerEnable == true)
		{
			qDebug() << "Tcp connection WatchdogTimer timeout";
		}
	}


	void SocketWorker::enableWatchdogTimer(bool enable)
	{
		m_watchdogTimerEnable = enable;

		if (enable == false)
		{
			m_watchdogTimer.stop();
		}
	}


	void SocketWorker::restartWatchdogTimer()
	{
		if (m_watchdogTimerEnable == true)
		{
			m_watchdogTimer.setSingleShot(true);
			m_watchdogTimer.start(m_watchdogTimerTimeout);
		}
	}


	int SocketWorker::readHeader(int bytesAvailable)
	{
		if (m_readState != ReadState::WaitingForHeader)
		{
			assert(false);
			return 0;
		}

		int bytesToRead = sizeof(SocketWorker::Header) - m_readHeaderSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		int bytesRead = m_tcpSocket->read(reinterpret_cast<char*>(&m_header) + m_readHeaderSize, bytesToRead);

		m_readHeaderSize += bytesRead;

		assert(m_readHeaderSize <= sizeof(SocketWorker::Header));

		if (m_readHeaderSize < sizeof(SocketWorker::Header))
		{
			return bytesRead;
		}

		// Full requestHeader is read
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

			m_readState = ReadState::WaitingNothing;

			return bytesRead;
		}

		if (m_header.dataSize > TCP_MAX_DATA_SIZE)
		{
			assert(false);

			closeConnection();

			qDebug() << "Request" << m_header.id << "dataSize too big - " << m_header.dataSize;

			return 0;
		}

		m_readState = ReadState::WaitingForData;

		return bytesRead;
	}


	int SocketWorker::readData(int bytesAvailable)
	{
		if (m_readState != ReadState::WaitingForData)
		{
			assert(false);
			return 0;
		}

		int bytesToRead = m_header.dataSize - m_readDataSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		if (m_readDataSize + bytesToRead > TCP_MAX_DATA_SIZE)
		{
			assert(false);

			closeConnection();

			qDebug() << "Out of buffer m_requestData";

			return 0;
		}

		int bytesRead = m_tcpSocket->read(m_receiveDataBuffer + m_readDataSize, bytesToRead);

		m_readDataSize += bytesRead;

		assert(m_readDataSize <= m_header.dataSize);

		if (m_readDataSize == m_header.dataSize)
		{
			m_headerAndDataReady = true;

			m_readState = ReadState::WaitingNothing;
		}

		return bytesRead;
	}


	qint64 SocketWorker::socketWrite(const char* data, qint64 size)
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return -1;
		}

		//assert(m_bytesWritten == true);

		qint64 written = m_tcpSocket->write(data, size);

		//qDebug() << "Socket written bytes  =" << written;

		if (written == -1)
		{
			return -1;
		}

		// m_tcpSocket->flush();

		m_bytesWritten = false;

		//m_tcpSocket->waitForBytesWritten(TCP_BYTES_WRITTEN_TIMEOUT);

		addSentBytes(size);

		return written;
	}


	qint64 SocketWorker::socketWrite(const Header& header)
	{
		return socketWrite(reinterpret_cast<const char*>(&header), sizeof(header));
	}


	void SocketWorker::onSocketStateChanged(QAbstractSocket::SocketState newState)
	{
		return;			// its Ok!

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
		AUTO_LOCK(m_mutex)

		if (m_tcpSocket == nullptr)
		{
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


	void SocketWorker::onInitConnection()
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		qDebug() << C_STR(QString(tr("Socket connected with %1 (descriptor = %2)")).
						  arg(peerAddr().addressStr()).
						  arg(m_tcpSocket->socketDescriptor()));
	}


	void SocketWorker::onConnection()
	{
		qDebug() << C_STR(QString(tr("Socket connected (descriptor = %1)")).
						  arg(m_tcpSocket->socketDescriptor()));
	}


	void SocketWorker::onDisconnection()
	{
		qDebug() << C_STR(QString(tr("Socket disconnected (descriptor = %1)")).
						  arg(m_tcpSocket->socketDescriptor()));
	}


	ConnectionState SocketWorker::getConnectionState() const
	{
		m_stateMutex.lock();

		ConnectionState state = m_state;

		m_stateMutex.unlock();

		return state;
	}

	SoftwareInfo SocketWorker::localSoftwareInfo() const
	{
		return getConnectionState().localSoftwareInfo;
	}

	SoftwareInfo SocketWorker::connectedSoftwareInfo() const
	{
		return getConnectionState().connectedSoftwareInfo;
	}

	HostAddressPort SocketWorker::peerAddr() const
	{
		m_stateMutex.lock();

		HostAddressPort peerAddr = m_state.peerAddr;

		m_stateMutex.unlock();

		return peerAddr;
	}


	void SocketWorker::setStateConnected(const HostAddressPort& peerAddr)
	{
		AUTO_LOCK(m_stateMutex);

		m_state.isConnected = true;
		m_state.peerAddr = peerAddr;
		m_state.startTime = QDateTime::currentMSecsSinceEpoch();
		m_state.sentBytes = 0;
		m_state.receivedBytes = 0;
		m_state.requestCount = 0;
		m_state.replyCount = 0;
	}


	void SocketWorker::setStateDisconnected()
	{
		AUTO_LOCK(m_stateMutex);

		m_state.isConnected = false;
		m_state.peerAddr.clear();
		m_state.startTime = 0;
		m_state.sentBytes = 0;
		m_state.receivedBytes = 0;
		m_state.requestCount = 0;
		m_state.replyCount = 0;
	}


	void SocketWorker::addSentBytes(int bytes)
	{
		AUTO_LOCK(m_stateMutex);

		m_state.sentBytes += bytes;
	}


	void SocketWorker::addReceivedBytes(int bytes)
	{
		AUTO_LOCK(m_stateMutex);

		m_state.receivedBytes += bytes;
	}

	void SocketWorker::addRequest()
	{
		AUTO_LOCK(m_stateMutex);

		m_state.requestCount++;
	}


	void SocketWorker::addReply()
	{
		AUTO_LOCK(m_stateMutex);

		m_state.replyCount++;
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Server class implementation
	//
	// -------------------------------------------------------------------------------------

	int Server::staticId = 0;


	Server::Server(const SoftwareInfo& sotwareInfo) :
		SocketWorker(sotwareInfo),
		m_autoAckTimer(this)
	{
		m_id = staticId;
		staticId++;

		initReadStatusVariables();

		m_autoAckTimer.setSingleShot(false);
	}


	Server::~Server()
	{
		if (m_protobufBuffer != nullptr)
		{
			delete [] m_protobufBuffer;
		}
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

		// added 20_01_2017 by WhiteMan
		//
		setStateConnected(HostAddressPort(m_tcpSocket->peerAddress(), m_tcpSocket->peerPort()));
	}


	void Server::initReadStatusVariables()
	{
		m_serverState = ServerState::WainigForRequest;
		m_readState = ReadState::WaitingForHeader;
		m_readHeaderSize = 0;
		m_readDataSize = 0;
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

		addRequest();

		if (m_header.id == RQID_INTRODUCE_MYSELF)
		{
			Network::SoftwareInfo inMessage;

			bool result = inMessage.ParseFromArray(m_receiveDataBuffer, m_header.dataSize);

			if (result == false)
			{
				assert(false);
				return;
			}

			m_stateMutex.lock();

			m_state.connectedSoftwareInfo.serializeFrom(inMessage);

			Network::SoftwareInfo outMessage;

			m_state.localSoftwareInfo.serializeTo(&outMessage);

			m_stateMutex.unlock();

			sendReply(outMessage);

			emit connectedSoftwareInfoChanged();
		}
		else
		{
			processRequest(m_header.id, m_receiveDataBuffer, m_header.dataSize);
		}
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


	void Server::updateClientsInfo(const std::list<Tcp::ConnectionState> connectionStates)
	{
		m_statesMutex.lock();

		m_connectionStates = connectionStates;

		m_statesMutex.unlock();
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


	bool Server::sendReply()
	{
		return sendReply(nullptr, 0);
	}


	bool Server::sendReply(const QByteArray& replyData)
	{
		return sendReply(replyData.constData(), replyData.size());
	}


	bool Server::sendReply(google::protobuf::Message& protobufMessage)
	{
		int messageSize = protobufMessage.ByteSize();

		if (messageSize > TCP_MAX_DATA_SIZE)
		{
			assert(false);
			return false;
		}

		if (m_protobufBuffer == nullptr)
		{
			m_protobufBuffer = new char [TCP_MAX_DATA_SIZE];

			assert(m_protobufBuffer != nullptr);
		}

		protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_protobufBuffer));

		return sendReply(m_protobufBuffer, messageSize);
	}


	bool Server::sendReply(const char* replyData, quint32 replyDataSize)
	{
		m_autoAckTimer.stop();

		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return false;
		}

		if (m_serverState != ServerState::RequestProcessing)
		{
			assert(false);
			return false;
		}

		addReply();

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Reply;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = replyDataSize;
		header.requestProcessingPorgress = 100;
		header.calcCRC();

		qint64 written = socketWrite(header);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return false;
		}

		if (written < static_cast<qint64>(sizeof(header)))
		{
			assert(false);
			return false;
		}

		if (replyDataSize > 0)
		{
			written = socketWrite(replyData, replyDataSize);



			if (written == -1)
			{
				qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
				return false;
			}

			if (written < replyDataSize)
			{
				assert(false);
				return false;
			}
		}

		initReadStatusVariables();

		return true;
	}


	void Server::sendClientList()
	{
		Network::ServiceClients message;

		m_statesMutex.lock();

		for(const Tcp::ConnectionState& state : m_connectionStates)
		{
			const SoftwareInfo& si = state.connectedSoftwareInfo;

			if (E::containes<E::SoftwareType>(TO_INT(si.softwareType())) == false)
			{
				continue;
			}

			Network::ServiceClientInfo* clientInfo = message.add_clients();

			Network::SoftwareInfo* newSoftwareInfo = new Network::SoftwareInfo();

			si.serializeTo(newSoftwareInfo);

			clientInfo->set_allocated_softwareinfo(newSoftwareInfo);

			clientInfo->set_ip(state.peerAddr.address32());

			clientInfo->set_uptime(QDateTime::currentMSecsSinceEpoch() - state.startTime);
			clientInfo->set_isactual(state.isActual);
			clientInfo->set_replyquantity(state.replyCount);
		}

		m_statesMutex.unlock();

		sendReply(message);
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::TcpServer class implementation
	//
	// -------------------------------------------------------------------------------------

	void TcpServer::incomingConnection(qintptr socketDescriptor)
	{
		emit newConnection(socketDescriptor);
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Listener class implementation
	//
	// -------------------------------------------------------------------------------------

	Listener::Listener(const HostAddressPort& listenAddressPort, Server* server, std::shared_ptr<CircularLogger> logger) :
		m_listenAddressPort(listenAddressPort),
		m_logger(logger),
		m_periodicTimer(this),
		m_serverInstance(server)
	{
		assert(m_serverInstance != nullptr);

		qRegisterMetaType<std::list<ConnectionState>>("std::list<ConnectionState>");

		m_serverInstance->setParent(this);
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

	void Listener::onNewConnectionAccepted(const HostAddressPort& peerAddr, int connectionNo)
	{
		Q_UNUSED(peerAddr)
		Q_UNUSED(connectionNo)
	}


	void Listener::onStartListening(const HostAddressPort& addr, bool startOk, const QString& errStr)
	{
		if (startOk == true)
		{
			DEBUG_LOG_MSG(m_logger, QString("Start listening %1 OK").arg(addr.addressPortStr()));
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString("Error on start listening %1: %2").arg(addr.addressPortStr()).arg(errStr));
		}
	}


	void Listener::onThreadStarted()
	{
		m_periodicTimer.setInterval(TCP_PERIODIC_TIMER_INTERVAL);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Listener::onPeriodicTimer);
		connect(&m_periodicTimer, &QTimer::timeout, this, &Listener::updateClientsList);

		m_periodicTimer.start();

		startListening();

		qDebug() << "Start listening: " << C_STR(m_listenAddressPort.addressPortStr());

		onListenerThreadStarted();
	}


	void Listener::onThreadFinished()
	{
		onListenerThreadFinished();

		if (m_tcpServer != nullptr)
		{
			m_tcpServer->close();
			delete m_tcpServer;
		}
	}


	void Listener::startListening()
	{
		if (m_tcpServer == nullptr)
		{
			m_tcpServer = new TcpServer();

			connect(m_tcpServer, &TcpServer::newConnection, this, &Listener::onNewConnection);
		}

		if (m_tcpServer->listen(m_listenAddressPort.address(), m_listenAddressPort.port()))
		{
			onStartListening(m_listenAddressPort, true, "");
		}
		else
		{
			onStartListening(m_listenAddressPort, false, m_tcpServer->errorString());
		}
	}


	void Listener::onNewConnection(qintptr socketDescriptor)
	{
		// accept new connection
		//
		Server* newServerInstance = m_serverInstance->getNewInstance();

		connect(this, &Listener::connectedClientsListChanged, newServerInstance, &Server::updateClientsInfo);

		connect(newServerInstance, &Server::disconnected, this, &Listener::onServerDisconnected);
		connect(newServerInstance, &Server::connectedSoftwareInfoChanged, this, &Listener::updateClientsList);

		newServerInstance->setConnectedSocketDescriptor(socketDescriptor);

		onNewConnectionAccepted(newServerInstance->peerAddr(), newServerInstance->id());

		SimpleThread* newThread = new SimpleThread(newServerInstance);

		m_runningServers.insert(newServerInstance, newThread);

		newThread->start();

		updateClientsList();
	}


	void Listener::updateClientsList()
	{
		std::list<ConnectionState> clientsInfo;

		QList<const SocketWorker*>&& servers = m_runningServers.keys();

		for (const SocketWorker* server : servers)
		{
			clientsInfo.push_back(server->getConnectionState());
		}

		emit connectedClientsListChanged(clientsInfo);
	}


	void Listener::onPeriodicTimer()
	{
		if (!m_tcpServer->isListening())
		{
			startListening();
		}
	}


	void Listener::onServerDisconnected(const SocketWorker* server)
	{
		SimpleThread* thread = m_runningServers.value(server, nullptr);

		if (thread == nullptr)
		{
			assert(false);
			return;
		}

		m_runningServers.remove(server);

		thread->quit();
		delete thread;

		updateClientsList();
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ServerThread class implementation
	//
	// -------------------------------------------------------------------------------------

	ServerThread::ServerThread(const HostAddressPort &listenAddressPort,
							   Server* server,
							   std::shared_ptr<CircularLogger> logger) :
		SimpleThread(new Listener(listenAddressPort, server, logger))
	{
	}


	ServerThread::ServerThread(Listener* listener) :
		SimpleThread(listener)
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

	Client::Client(const SoftwareInfo& softwareInfo,
				   const HostAddressPort &serverAddressPort) :
		SocketWorker(softwareInfo),
		m_periodicTimer(this),
		m_replyTimeoutTimer(this)
	{
		setServer(serverAddressPort, false);
		initReadStatusVariables();
	}


	Client::Client(const SoftwareInfo& softwareInfo,
				   const HostAddressPort& serverAddressPort1,
				   const HostAddressPort& serverAddressPort2) :
		SocketWorker(softwareInfo),
		m_periodicTimer(this),
		m_replyTimeoutTimer(this)
	{
		setServers(serverAddressPort1, serverAddressPort2, false);
		initReadStatusVariables();
	}


	Client::~Client()
	{
		if (m_protobufBuffer != nullptr)
		{
			delete [] m_protobufBuffer;
		}
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

	QString Client::equipmentID() const
	{
		return localSoftwareInfo().equipmentID();
	}

	HostAddressPort Client::currentServerAddressPort()
	{
		return m_selectedServer;
	}

	HostAddressPort Client::serverAddressPort(int serverIndex)
	{
		if (serverIndex < 0 || serverIndex > 1)
		{
			assert(false);
			return HostAddressPort();
		}

		return m_serversAddressPort[serverIndex];
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

		restartWatchdogTimer();

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
		m_readState = ReadState::WaitingNothing;
		m_readHeaderSize = 0;
		m_readDataSize = 0;
		m_connectTimeout = 0;
	}


	void Client::onInitConnection()
	{
		qDebug() << qPrintable(QString("Socket connected to server %1").arg(m_selectedServer.addressPortStr()));

		SoftwareInfo locSoftwareInfo = localSoftwareInfo();

		Network::SoftwareInfo message;

		locSoftwareInfo.serializeTo(&message);

		sendRequest(RQID_INTRODUCE_MYSELF, message);
	}


	void Client::onDisconnection()
	{
		qDebug() << qPrintable(QString("Socket disconnected from server %1").arg(m_selectedServer.addressPortStr()));
	}


	void Client::onTryConnectToServer(const HostAddressPort& serverAddr)
	{
		qDebug() << qPrintable(QString("Try connect to server %1").arg(serverAddr.addressPortStr()));
	}


	void Client::onAck(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		Q_UNUSED(requestID);
		Q_UNUSED(replyData);
		Q_UNUSED(replyDataSize);
	}


	void Client::onHeaderAndDataReady()
	{
		if (m_clientState != ClientState::WaitingForReply)
		{
			assert(false);
			closeConnection();
			return;
		}

		restartWatchdogTimer();

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

			onAck(m_header.id, m_receiveDataBuffer, m_header.dataSize);

			m_readState = ReadState::WaitingForHeader;

			break;

		case Header::Type::Reply:
			stopReplyTimeoutTimer();

			initReadStatusVariables();

			addReply();

			if (m_header.id == RQID_INTRODUCE_MYSELF)
			{
				onConnection();
			}
			else
			{
				processReply(m_header.id, m_receiveDataBuffer, m_header.dataSize);
			}

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

		onTryConnectToServer(m_selectedServer);

		m_tcpSocket->connectToHost(m_selectedServer.address(), m_selectedServer.port());
	}


	void Client::onPeriodicTimer()
	{
		AUTO_LOCK(m_mutex);

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


	void Client::stopReplyTimeoutTimer()
	{
		m_replyTimeoutTimer.stop();
	}

	bool Client::sendRequest(quint32 requestID)
	{
		return sendRequest(requestID, nullptr, 0);
	}


	bool Client::sendRequest(quint32 requestID, const QByteArray& requestData)
	{
		return sendRequest(requestID, requestData.constData(), requestData.size());
	}


	bool Client::sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		AUTO_LOCK(m_mutex);

		restartWatchdogTimer();

		if (!isClearToSendRequest())
		{
			assert(false);
			return false;
		}

		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return false;
		}

		// qDebug() << "Send request" << requestID;

		addRequest();

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
			return false;
		}

		if (written < static_cast<qint64>(sizeof(m_sentRequestHeader)))
		{
			assert(false);
			return false;
		}

		if (requestDataSize > 0)
		{
			if (requestData == nullptr)
			{
				assert(false);
				return false;
			}

			written = socketWrite(requestData, requestDataSize);

			if (written == -1)
			{
				qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
				return false;
			}

			if (written < requestDataSize)
			{
				assert(false);
				return false;
			}
		}

		restartReplyTimeoutTimer();

		m_clientState = ClientState::WaitingForReply;
		m_readState = ReadState::WaitingForHeader;

		return true;
	}


	bool Client::sendRequest(quint32 requestID, google::protobuf::Message& protobufMessage)
	{
		int messageSize = protobufMessage.ByteSize();

		if (messageSize > TCP_MAX_DATA_SIZE)
		{
			return false;
		}

		if (m_protobufBuffer == nullptr)
		{
			m_protobufBuffer = new char [TCP_MAX_DATA_SIZE];

			assert(m_protobufBuffer != nullptr);
		}

		protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_protobufBuffer));

		return sendRequest(requestID, m_protobufBuffer, messageSize);
	}


	void Client::onWatchdogTimerTimeout()
	{
		SocketWorker::onWatchdogTimerTimeout();

		closeConnection();
	}


}
