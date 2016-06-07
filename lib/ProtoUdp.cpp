#include "../lib/ProtoUdp.h"

namespace ProtoUdp
{
	// -------------------------------------------------------------------------------------
	//
	// ProtoUdp::Socket class implementation
	//
	// -------------------------------------------------------------------------------------


	Socket::Socket() :
		m_socket(this),
		m_timer(this)
	{
	}


	Socket::~Socket()
	{
	}


	quint32 Socket::getTotalFramesNumber(quint32 dataSize)
	{
		return dataSize / FRAME_DATA_SIZE +
				(dataSize % FRAME_DATA_SIZE ? 1 : 0);
	}


	quint32 Socket::getFrameDataSize(quint32 frameNumber, quint32 dataSize)
	{
		quint32 totalFramesNumber = getTotalFramesNumber(dataSize);

		if (frameNumber >= totalFramesNumber)
		{
			assert(false);
			return 0;
		}

		if (frameNumber == totalFramesNumber - 1)
		{
			// is a last frame
			//
			return dataSize % FRAME_DATA_SIZE;
		}

		return FRAME_DATA_SIZE;
	}


	void Socket::copyDataToFrame(Frame& frame, const QByteArray& data)
	{
		const char* dataPtr = data.constData() + frame.header.frameNumber * FRAME_DATA_SIZE;

		quint32 frameDataSize = getFrameDataSize(frame.header.frameNumber, data.size());

		memcpy(frame.data, dataPtr, static_cast<size_t>(frameDataSize));
	}


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdp::Client class implementation
	//
	// -------------------------------------------------------------------------------------


	Client::Client(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress) :
		m_firstServerAddress(firstServerAddress),
		m_secondServerAddress(secondServerAddress)
	{
		m_serverAddress = firstServerAddress;
		m_communicateWithFirstServer = true;

		m_clientID = qHash(QUuid::createUuid());

	//	m_socket.bind();

		connect(&m_timer, &QTimer::timeout, this, &Client::onTimerTimeout);
		connect(&m_socket, &QUdpSocket::readyRead, this, &Client::onSocketReadyRead);

		connect(this, &Client::startSendRequest, this, &Client::onStartSendRequest);
		connect(this, &Client::restartCommunication, this, &Client::onRestartCommunication);
	}


	Client::~Client()
	{
	}


	void Client::setServersAddresses(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress)
	{
		m_sync.lock();

		m_firstServerAddress = firstServerAddress;
		m_secondServerAddress = secondServerAddress;

		emit restartCommunication();

		m_sync.unlock();
	}


	void Client::switchServer()
	{
		m_sync.lock();

		m_communicateWithFirstServer = !m_communicateWithFirstServer;

		if (m_communicateWithFirstServer)
		{
			m_serverAddress = m_firstServerAddress;
		}
		else
		{
			m_serverAddress = m_secondServerAddress;
		}

		emit restartCommunication();

		m_sync.unlock();
	}


	void Client::switchToFirstServer()
	{
		m_sync.lock();

		m_communicateWithFirstServer = true;
		m_serverAddress = m_firstServerAddress;

		emit restartCommunication();

		m_sync.unlock();
	}


	void Client::switchToSecondServer()
	{
		m_sync.lock();

		m_communicateWithFirstServer = false;
		m_serverAddress = m_secondServerAddress;

		emit restartCommunication();

		m_sync.unlock();
	}


	void Client::sendRequest(quint32 requestID, QByteArray& requestData)
	{
		m_sync.lock();

		if (state() == ClientState::ReadyToSendRequest)
		{
			m_requestData.swap(requestData);

			m_requestID = requestID;

			emit startSendRequest();
		}
		else
		{
			// m_clientState != ClientState::ReadyToSendRequest
			//
			assert(false);
		}

		m_sync.unlock();
	}


	void Client::onStartSendRequest()
	{
		m_requestFrame.header.type = FrameHeader::Type::Request;
		m_requestFrame.header.clientID = m_clientID;
		m_requestFrame.header.sessionID = 0;					// must be assign on server side
		m_requestFrame.header.requestID = m_requestID;
		m_requestFrame.header.frameNumber = 0;
		m_requestFrame.header.errorCode = 0;

		copyDataToFrame(m_requestFrame, m_requestData);

		setState(ClientState::RequestSending);
		setError(Error::Ok);

		calcRequestSendingProgress();
		setReplyReceivingProgress(0);

		sendRequestFrame();
	}


	void Client::continueSendRequest()
	{
		m_requestFrame.header.frameNumber++;

		copyDataToFrame(m_requestFrame, m_requestData);

		sendRequestFrame();
	}


	void Client::calcRequestSendingProgress()
	{
		quint32 sentDataSize = (m_requestFrame.header.frameNumber + 1) * FRAME_DATA_SIZE;

		if (sentDataSize >= m_requestFrame.header.totalDataSize)
		{
			setRequestSendingProgress(100);
		}
		else
		{
			setRequestSendingProgress(static_cast<int>(double(sentDataSize * 100) / double(m_requestFrame.header.totalDataSize)));
		}
	}


	void Client::sendRequestFrame()
	{
		qint64 senBytesCount = m_socket.writeDatagram(reinterpret_cast<const char*>(&m_requestFrame), sizeof(m_requestFrame), m_serverAddress.address(), m_serverAddress.port());

		assert(senBytesCount == sizeof(m_requestFrame));

		m_timer.start(m_replayTimeout);
	}


	void Client::onTimerTimeout()
	{
		m_sync.lock();

		switch(state())
		{
		case ClientState::RequestSending:

			if (m_retryCount < RETRY_COUNT)
			{
				sendRequestFrame();
				m_retryCount++;
			}
			else
			{
				m_timer.stop();

				m_requestFrame.header.clear();

				setError(Error::NoReplayFromServer);
				setState(ClientState::ReadyToSendRequest);

				qDebug() << "NoReplayFromServer";
			}

			break;

		default:
			assert(false);
		}

		m_sync.unlock();
	}


	void Client::onSocketReadyRead()
	{

	}


	void Client::onRestartCommunication()
	{
		assert(false);		// must be implemented!!!!
	}


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdp::ClientThread class implementation
	//
	// -------------------------------------------------------------------------------------


	ClientThread::ClientThread(const HostAddressPort& serverAddress)
	{
		m_client = new Client(serverAddress, serverAddress);
		SimpleThread::addWorker(m_client);
	}


	ClientThread::ClientThread(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress)
	{
		m_client = new Client(firstServerAddress, secondServerAddress);
		SimpleThread::addWorker(m_client);
	}


	/*void ClientThread::run()
	{
		m_client->moveToThread(&m_thread);

		connect(&m_thread, &QThread::started, m_client, &Client::onThreadStarted);
		connect(&m_thread, &QThread::finished, m_client, &Client::onThreadFinished);

		m_thread.start();
	}


	ClientThread::~ClientThread()
	{
		m_thread.quit();
		m_thread.wait();
	}*/


	void ClientThread::sendRequest(quint32 requestID, QByteArray& requestData)
	{
		m_client->sendRequest(requestID, requestData);
	}


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdp::RequestProcessingThread class implementation
	//
	// -------------------------------------------------------------------------------------


	RequestProcessingThread::RequestProcessingThread(Server* server, RequestProcessor* requestProcessor) :
		m_server(server),
		m_requestProcessor(requestProcessor)
	{
		assert(m_server != nullptr);
		assert(m_requestProcessor != nullptr);
	}


	RequestProcessingThread::~RequestProcessingThread()
	{

	}


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdp::Server class implementation
	//
	// -------------------------------------------------------------------------------------


	Server::Server(const HostAddressPort& serverAddress)
	{
		m_serverAddress = serverAddress;

		connect(&m_timer, &QTimer::timeout, this, &Server::onTimerTimeout);
		connect(&m_socket, &QUdpSocket::readyRead, this, &Server::onSocketReadyRead);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Server::onPeriodicTimer);
	}


	Server::~Server()
	{

	}


	bool Server::bind()
	{
		if (m_socket.bind(m_serverAddress.address(), m_serverAddress.port()))
		{
			m_binded = true;

			qDebug() << "Successful bind to address: " << m_serverAddress.address().toString() << " : " << m_serverAddress.port();
		}
		else
		{
			m_binded = false;

			qDebug() << "Bind error to address: " << m_serverAddress.address().toString() << " : " << m_serverAddress.port();
		}

		return m_binded;
	}


	void Server::onTimerTimeout()
	{

	}


	void Server::onSocketReadyRead()
	{
		QHostAddress clientAddress;
		quint16 clientPort = 0;

		quint64 size = m_socket.readDatagram(reinterpret_cast<char*>(&m_requestFrame), sizeof(m_requestFrame), &clientAddress, &clientPort);

		if (size != sizeof(m_requestFrame))
		{
			assert(false);
			return;
		}

		int clientID = m_requestFrame.header.clientID;

		RequestProcessingThread* processingThread = nullptr;

		if (m_requestProcesingThreads.contains(clientID))
		{
			processingThread = m_requestProcesingThreads[clientID];
		}
		else
		{
			quint32 requestID = m_requestFrame.header.requestID;

			if (m_requestProcesingThreads.count() < m_maxProcessingThreadsCount)
			{
				RequestProcessor* processor = createRequestProcessor(requestID);

				if (processor != nullptr)
				{
					processingThread = new RequestProcessingThread(this, processor);

					m_requestProcesingThreads.insert(clientID, processingThread);
				}
				else
				{
					assert(false);

					qDebug() << "Not found RequestProcessor for request: " << requestID;
				}
			}
			else
			{
				assert(false);

				qDebug() << "Maximum number of RequestProcessingThreads reached!";
			}
		}

		//processingThread->push(m_requestFrame);
	}


	RequestProcessor* Server::createRequestProcessor(quint32 /*requestID*/)
	{
		return nullptr;
	}

	void Server::onPeriodicTimer()
	{
		if (!m_binded)
		{
			bind();
		}
	}


	void Server::onThreadStarted()
	{
		bind();
	}


	void Server::onThreadFinished()
	{
		Socket::onThreadFinished();
	}


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdp::ServerThread class implementation
	//
	// -------------------------------------------------------------------------------------


	ServerThread::ServerThread(const HostAddressPort& serverAddress)
	{
		m_server = new Server(serverAddress);
	}


	ServerThread::~ServerThread()
	{
		m_thread.quit();
		m_thread.wait();
	}


	void ServerThread::run()
	{
		m_server->moveToThread(&m_thread);

		connect(&m_thread, &QThread::started, m_server, &Server::onThreadStarted);
		connect(&m_thread, &QThread::finished, m_server, &Server::onThreadFinished);

		m_thread.start();
	}

}
