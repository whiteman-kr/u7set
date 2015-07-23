#include "../include/Tcp.h"

namespace Tcp
{

	// -------------------------------------------------------------------------------------
	//
	// Tcp::SocketWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	SocketWorker::SocketWorker(bool isServer) :
		m_isServer(isServer),
		m_replyTimeoutTimer(this)
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

		m_tcpSocket = new QTcpSocket(this);

		if (isServer())
		{
			// server SocketWorker initialization
			//
			assert(m_connectedSocketDescriptor != 0);

			m_tcpSocket->setSocketDescriptor(m_connectedSocketDescriptor);

			m_state = State::WainigForRequest;
		}
		else
		{
			// client SocketWorker initialization
			//
			m_replyTimeoutTimer.setSingleShot(true);

			connect(&m_replyTimeoutTimer, &QTimer::timeout, this, &SocketWorker::onReplyTimeoutTimer);

			m_state = State::ClearToSendRequest;
		}

		connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
		connect(m_tcpSocket, &QTcpSocket::connected, this, &SocketWorker::onSocketConnected);
		connect(m_tcpSocket, &QTcpSocket::disconnected, this, &SocketWorker::onSocketDisconnected);
		connect(m_tcpSocket, &QTcpSocket::readyRead, this, &SocketWorker::onSocketReadyRead);
	}


	void SocketWorker::onThreadFinished()
	{
		assert(m_tcpSocket != nullptr);

		if (isServer())
		{

		}
		else
		{

		}

		m_tcpSocket->close();
		delete m_tcpSocket;

		if (m_dataBuffer != nullptr)
		{
			delete [] m_dataBuffer;
		}
	}


	void SocketWorker::setConnectedSocketDescriptor(qintptr connectedSocketDescriptor)
	{
		m_connectedSocketDescriptor = connectedSocketDescriptor;
	}


	void SocketWorker::onSocketDisconnected()
	{
		//qDebug() << qPrintable(QString("Socket disconnected from server %1").arg(m_selectedServer.addressPortStr()));

		onDisconnection();

		emit disconnected(this);
	}


	void SocketWorker::onSocketReadyRead()
	{
		if (isServer())
		{
			assert(m_state == State::WainigForRequest);
		}
		else
		{
			assert(m_state == State::WaitingForReply);
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

			if (m_requestReady)
			{
				onRequestReady();
			}
		}
	}


	int SocketWorker::readHeader(int bytesAvailable)
	{
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return 0;
		}

		if (m_readState != ReadState::WainigForHeader)
		{
			assert(false);
			return 0;
		}

		int bytesToRead = sizeof(Header) - m_readedHeaderSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		int bytesReaded = m_tcpSocket->read(reinterpret_cast<char*>(&m_header) + m_readedHeaderSize, bytesToRead);

		qDebug() << "Read header bytes " << bytesReaded;

		m_readedHeaderSize += bytesReaded;

		assert(m_readedHeaderSize <= sizeof(RequestHeader));

		if (m_readedHeaderSize < sizeof(RequestHeader))
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
		if (m_tcpSocket == nullptr)
		{
			assert(false);
			return 0;
		}

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
			m_requestReady = true;
		}

		return bytesReaded;
	}


	void SocketWorker::onRequestReady()
	{
		assert(m_requestReady == true);

		// prepare to read next request
		//
		m_requestReady = false;
		m_readedHeaderSize = 0;
		m_readedDataSize = 0;

		// process request
		//
		switch(m_header.type)
		{
		case Header::Type::Request:
			if (isServer())
			{
				m_state = State::RequestProcessing;

				sendAck();

				processRequest(m_header.id, m_dataBuffer, m_header.dataSize);
			}
			else
			{
				assert(false);
			}
			break;

		case Header::Type::Ack:
			if (!isServer())
			{
				// restart reply timeout timer
				//
				m_replyTimeoutTimer.start(TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT);
			}
			else
			{
				assert(false);
			}
			break;

		case Header::Type::Reply:
			if (!isServer())
			{
				processReply(m_header.id, m_dataBuffer, m_header.dataSize);
			}
			else
			{
				assert(false);
			}
			break;

		default:
			assert(false);
		}
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


	void SocketWorker::onSocketConnected()
	{
		//qDebug() << qPrintable(QString("Socket connected to server %1").arg(m_selectedServer.addressPortStr()));

		onConnection();
	}


	void SocketWorker::sendRequest(quint32 requestID, const QByteArray& requestData)
	{
		sendRequest(requestID, requestData.constData(), requestData.size());
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
		return m_tcpSocket->state() == QAbstractSocket::UnconnectedState;
	}


	bool SocketWorker::isClearToSendRequest() const
	{
		assert(isClient());

		return isConnected() && m_state == State::ClearToSendRequest;
	}


	void SocketWorker::sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
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

		Header requestHeader;

		requestHeader.type = Header::Type::Request;
		requestHeader.id = requestID;
		requestHeader.numerator = m_numerator;
		requestHeader.dataSize = requestDataSize;
		requestHeader.calcCRC();

		m_numerator++;

		qint64 written = m_tcpSocket->write(reinterpret_cast<const char*>(&requestHeader), sizeof(requestHeader));

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_tcpSocket->errorString()));
			return;
		}

		if (written < sizeof(requestHeader))
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

		QTimer::singleShot(TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT, this, SLOT(onRequestReplyTimeout));
	}


	void SocketWorker::sendReply(const char* replyData, quint32 replyDatsSize)
	{
		assert(isServer() && m_state == State::RequestProcessing);

		m_state = State::WainigForRequest;
	}


	void SocketWorker::sendAck()
	{

	}


	void SocketWorker::onReplyTimeoutTimer()
	{
		onReplyTimeout();
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


	void SocketWorker::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
	}

	void SocketWorker::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
	}



	// -------------------------------------------------------------------------------------
	//
	// Tcp::Server class implementation
	//
	// -------------------------------------------------------------------------------------

	int Server::staticId = 0;


	Server::Server() :
		SocketWorker(true)
	{
		m_id = ++staticId;
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
		SocketWorker(false),
		m_periodicTimer(this)
	{
		connect(&m_periodicTimer, &QTimer::timeout, this, &Client::onPeriodicTimer);

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
		onSocketThreadStarted();

		m_periodicTimer.setInterval(1000);
		m_periodicTimer.start();

		connectToServer();
	}


	void Client::onThreadFinished()
	{
		/*m_tcpSocket.disconnectFromHost();
		m_tcpSocket.close();

		onSocketThreadFinished();*/
	}


	void Client::connectToServer()
	{
		//qDebug() << qPrintable(QString("Try connect to server %1").arg(m_selectedServer.addressPortStr()));

		//m_tcpSocket.connectToHost(m_selectedServer.address(), m_selectedServer.port());
	}




	void Client::onPeriodicTimer()
	{
		if (isUnconnected())
		{
			connectToServer();
		}
	}








/*
	void Client::onRequestRelyTimeout()
	{
		qDebug() << "Reply timeout on request " << m_requestHeader.id;

		m_state = State::ClearToSendRequest;

		onRequestTimeout();
	}

	void Client::onReadyRead()
	{

	}*/
}
