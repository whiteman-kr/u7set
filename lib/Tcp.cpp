#include "../include/Tcp.h"

namespace Tcp
{

	// -------------------------------------------------------------------------------------
	//
	// Tcp::SocketWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	SocketWorker::SocketWorker()
	{
	}


	SocketWorker::~SocketWorker()
	{
	}


	void SocketWorker::reallocateDataBuffer(int newDataBufferSize)
	{
		if (newDataBufferSize > TCP_PACKET_MAX_DATA_SIZE)
		{
			assert(false);
			return;
		}

		if (m_dataBuffer != nullptr)
		{
			delete [] m_dataBuffer;
		}

		m_dataBuffer = new char[newDataBufferSize];

		m_dataBufferSize = newDataBufferSize;
	}


	void SocketWorker::onThreadStarted()
	{
		assert(m_tcpSocket == nullptr);

		m_tcpSocket = createSocket();

		m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

		connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
		connect(m_tcpSocket, &QTcpSocket::connected, this, &SocketWorker::onSocketConnected);
		connect(m_tcpSocket, &QTcpSocket::disconnected, this, &SocketWorker::onSocketDisconnected);
		connect(m_tcpSocket, &QTcpSocket::readyRead, this, &SocketWorker::onSocketReadyRead);
	}


	void SocketWorker::onThreadFinished()
	{
		m_tcpSocket->close();
		delete m_tcpSocket;

		delete [] m_dataBuffer;
	}


	void SocketWorker::onSocketConnected()
	{
		onConnection();
	}


	void SocketWorker::onSocketDisconnected()
	{
		onDisconnection();

		emit disconnected(this);
	}


	void SocketWorker::onSocketReadyRead()
	{
		if (!m_enableSocketRead)
		{
			assert(false);
			return;
		}

		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		int bytesAvailable = m_tcpSocket->bytesAvailable();

		while(bytesAvailable > 0)
		{
			switch(m_readState)
			{
			case ReadState::WainigForHeader:
				bytesAvailable -= readHeader(bytesAvailable);
				break;

			case ReadState::WainigForData:
				bytesAvailable -= readData(bytesAvailable);
				break;

			default:
				assert(false);
			}

			if (m_headerAndDataReady)
			{
				disableSocketRead();

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
		if (m_readState != ReadState::WainigForHeader)
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

		qDebug() << "Read header bytes " << bytesReaded;

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

			m_readState = ReadState::WainigForHeader;

			return bytesReaded;
		}

		if (m_dataBufferSize < m_header.dataSize)
		{
			if (m_header.dataSize > TCP_PACKET_MAX_DATA_SIZE)
			{
				assert(false);

				closeConnection();

				qDebug() << "Request" << m_header.id << "dataSize too big - " << m_header.dataSize;

				return 0;
			}

			reallocateDataBuffer(m_header.dataSize);
		}

		m_readState = ReadState::WainigForData;

		return bytesReaded;
	}


	int SocketWorker::readData(int bytesAvailable)
	{
		if (m_readState != ReadState::WainigForData)
		{
			assert(false);
			return 0;
		}

		int bytesToRead = m_header.dataSize - m_readedDataSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		if (m_readedDataSize + bytesToRead > m_dataBufferSize)
		{
			assert(false);

			closeConnection();

			qDebug() << "Out of buffer m_requestData";

			return 0;
		}

		int bytesReaded = m_tcpSocket->read(m_dataBuffer + m_readedDataSize, bytesToRead);

		qDebug() << "Read data bytes " << bytesReaded;

		m_readedDataSize += bytesReaded;

		assert(m_readedDataSize <= m_header.dataSize);

		if (m_readedDataSize == m_header.dataSize)
		{
			m_headerAndDataReady = true;
		}

		return bytesReaded;
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


	bool SocketWorker::isUnconnected() const
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return false;
		}

		return m_tcpSocket->state() == QAbstractSocket::UnconnectedState;
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

	int Server::staticId = 1;


	Server::Server()
	{
		m_id = staticId;
		staticId++;
	}


	Server::~Server()
	{
	}


	void Server::onThreadStarted()
	{
		SocketWorker::onThreadStarted();

		onServerThreadStarted();

		onConnection();
	}


	void Server::onThreadFinished()
	{
		onServerThreadFinished();

		SocketWorker::onThreadFinished();
	}


	QTcpSocket* Server::createSocket()
	{
		assert(m_connectedSocketDescriptor != 0);

		QTcpSocket* tcpSocket = new QTcpSocket;

		tcpSocket->setSocketDescriptor(m_connectedSocketDescriptor);

		return tcpSocket;
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

		if (m_autoAck)
		{
			sendAck();
		}

		processRequest(m_header.id, m_dataBuffer, m_header.dataSize);
	}


	void Server::sendAck()
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Ack;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = 0;
		header.calcCRC();

		qint64 written = m_tcpSocket->write(reinterpret_cast<const char*>(&header), sizeof(header));

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < sizeof(header))
		{
			assert(false);
			return;
		}

		m_tcpSocket->flush();
	}


	void Server::sendReply(const QByteArray& replyData)
	{
		sendReply(replyData.constData(), replyData.size());
	}


	void Server::sendReply(const char* replyData, quint32 replyDatsSize)
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return;
		}

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Reply;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = replyDatsSize;
		header.calcCRC();

		qint64 written = m_tcpSocket->write(reinterpret_cast<const char*>(&header), sizeof(header));

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < sizeof(header))
		{
			assert(false);
			return;
		}

		if (replyDatsSize > 0)
		{
			qint64 written = m_tcpSocket->write(replyData, replyDatsSize);

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

		m_tcpSocket->flush();

		m_serverState = ServerState::WainigForRequest;
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

		//connect(&m_tcpServer, &QTcpServer::newConnection, this, &Listener::onNewConnection);
		//connect(&m_tcpServer, &QTcpServer::acceptError, this, &Listener::onAcceptError);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Listener::onPeriodicTimer);
	}


	Listener::~Listener()
	{
		// close all conection threads
		//
		for(SimpleThread* connectionThread : m_runningServices)
		{
			connectionThread->quit();
			delete connectionThread;
		}

		m_runningServices.clear();

		delete m_serverInstance;
	}


	void Listener::onThreadStarted()
	{
		m_periodicTimer.setInterval(1000);
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

		m_runningServices.insert(newServerInstance, newThread);

		newThread->start();

		qDebug() << "Accept new connection #" << newServerInstance->id();
	}


	void Listener::onAcceptError()
	{
	}


	void Listener::onServerDisconnected(const SocketWorker* server)
	{
		if (!m_runningServices.contains(server))
		{
			assert(false);
			return;
		}

		SimpleThread* thread = m_runningServices[server];

		m_runningServices.remove(server);

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!! reimplement
		//qDebug() << "Connection closed #" << server->id();

		thread->quit();

		delete thread;


	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ServerThread class implementation
	//
	// -------------------------------------------------------------------------------------

	ServerThread::ServerThread(const HostAddressPort& listenAddressPort, Server* server) :
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


	Client::Client() :
		m_periodicTimer(this),
		m_replyTimeoutTimer(this)
	{
	}


	Client::~Client()
	{
	}


	void Client::setServer(const HostAddressPort& serverAddressPort)
	{
		m_serversAddressPort[0] = serverAddressPort;
		m_serversAddressPort[1] = serverAddressPort;

		selectServer1();
	}


	void Client::setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)
	{
		m_serversAddressPort[0] = serverAddressPort1;
		m_serversAddressPort[1] = serverAddressPort2;

		selectServer1();
	}


	void Client::onThreadStarted()
	{
		onClientThreadStarted();

		SocketWorker::onThreadStarted();

		connect(&m_replyTimeoutTimer, &QTimer::timeout, this, &Client::onReplyTimeoutTimer);

		m_replyTimeoutTimer.setSingleShot(true);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Client::onPeriodicTimer);

		m_periodicTimer.setInterval(1000);
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
		assert(m_clientState == ClientState::WaitingForReply);

		switch(m_header.type)
		{
		case Header::Type::Ack:
			processAck();
			break;

		case Header::Type::Reply:
			processReply(m_header.id, m_dataBuffer, m_header.dataSize);
			break;

		default:
			assert(false);
		}
	}


	void Client::processAck()
	{
		if (m_header.id == m_sentRequestHeader.id &&
			m_header.numerator == m_sentRequestHeader.numerator)
		{
			restartReplyTimeoutTimer();
			onAck();
		}
		else
		{
			assert(false);
		}
	}


	void Client::connectToServer()
	{
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
		if (isUnconnected())
		{
			connectToServer();
		}
	}


	void Client::onReplyTimeoutTimer()
	{
		onReplyTimeout();

		//closeConnection();
	}


	bool Client::isClearToSendRequest() const
	{
		return isConnected() && m_clientState == ClientState::ClearToSendRequest;
	}


	void Client::restartReplyTimeoutTimer()
	{
		m_replyTimeoutTimer.start(TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT);
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

		qint64 written = m_tcpSocket->write(reinterpret_cast<const char*>(&m_sentRequestHeader), sizeof(m_sentRequestHeader));

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < sizeof(m_sentRequestHeader))
		{
			assert(false);
			return;
		}

		written = m_tcpSocket->write(requestData, requestDataSize);

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

		m_tcpSocket->flush();		//	?

		restartReplyTimeoutTimer();

		m_clientState = ClientState::WaitingForReply;
	}

}
