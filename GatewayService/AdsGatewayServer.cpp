#include "AdsGatewayServer.h"

AdsGatewayServer::AdsGatewayServer(const HostAddressPort& listenIP,
									const AppSignals& appSignals,
									CircularLoggerShared log) :
	LogWrapper(log, "AdsGatewayServer"),
	m_listenIP(listenIP),
	m_appSignals(appSignals)
{
}

AdsGatewayServer::~AdsGatewayServer()
{
}

void AdsGatewayServer::run()
{
	bool expected = false;

	if (!m_running.compare_exchange_strong(expected, true))
	{
		Q_ASSERT(false);
		return;
	}

	m_serverThread = std::thread(
		[this]()
		{
			runAcceptLoop();
		});
}

void AdsGatewayServer::stop()
{
	if (!m_running.exchange(false))
	{
		return;
	}

	// cancel acceptor
	//
	{
		std::lock_guard<std::mutex> lock(m_acceptorMutex);

		if (m_acceptor)
		{
			asio::error_code ec;
			m_acceptor->cancel(ec);
			m_acceptor->close(ec);
		}
	}

	// close active sessions
	//
	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (auto& sock : m_sessionSockets)
		{
			if (sock)
			{
				asio::error_code ec;
				sock->shutdown(tcp::socket::shutdown_both, ec);
				sock->close(ec);
			}
		}
	}

	if (m_serverThread.joinable())
	{
		m_serverThread.join();
	}

	// wait while all sessions stopped
	//
	joinAllSessions();
}

void AdsGatewayServer::runAcceptLoop()
{
	try
	{
		asio::io_context io;

		tcp::endpoint ep(asio::ip::address_v4::from_string(m_listenIP.addressStr().toStdString()), m_listenIP.port());
		tcp::acceptor acceptor(io);

		acceptor.open(ep.protocol());
		acceptor.set_option(tcp::acceptor::reuse_address(true));
		acceptor.bind(ep);
		acceptor.listen();

		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);
			m_acceptor = &acceptor;
		}

		logMsg(QString("starts listening %1").arg(m_listenIP.addressPortStr()));

		while (m_running.load())
		{
			reapFinishedSessions();

			tcp::socket socket(io);
			asio::error_code ec;

			acceptor.accept(socket, ec);

			if (!m_running.load())
			{
				break;
			}

			if (ec)
			{
				continue;
			}

			SessionThreadContextShared stc = std::make_shared<SessionThreadContext>();

			stc->socket = std::make_shared<tcp::socket>(std::move(socket));

			logMsg(QString("accept new connectio from %1").arg(getIpPortStr(stc->socket)));

			{
				std::lock_guard<std::mutex> lock(m_sessionsMutex);
				m_sessionSockets.push_back(stc->socket);
			}

			// start session thread
			//
			auto finished = std::make_shared<std::atomic<bool>>(false);

			{
				std::lock_guard<std::mutex> lock(m_threadsMutex);

				m_sessionThreads.push_back(
					SessionThread
					{
						std::thread(
							[this, stc, finished]()
							{
								const QString remoteIpPort = getIpPortStr(stc->socket);

								logMsg(QString("session thread for %1 started").arg(remoteIpPort));

								sessionThread(stc);
								removeSessionSocket(stc->socket);
								finished->store(true);

								logMsg(QString("session thread for %1 finished").arg(remoteIpPort));
							}),
						finished
					});
			}
		}

		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);
			m_acceptor = nullptr;
		}
	}
	catch (const std::exception& ex)
	{
		logErr(QString("accept loop error: %1").arg(ex.what()));
	}

	logMsg(QString("stops"));
}

void AdsGatewayServer::sessionThread(SessionThreadContextShared stc)
{
	char* recvBuf = new char[AGL::ADSGW_MAX_PAYLOAD_SIZE];
	std::size_t recvBufIndex = 0;

	try
	{
		asio::error_code ec;

		stc->socket->set_option(tcp::no_delay(true), ec);

		while (m_running.load())
		{
			recvBufIndex += stc->socket->read_some(
				asio::buffer(recvBuf + recvBufIndex, AGL::ADSGW_MAX_PAYLOAD_SIZE - recvBufIndex), ec);

			if (ec)
			{
				break;
			}

			processRequest(stc, recvBuf, recvBufIndex);
		}
	}
	catch (const std::exception& ex)
	{
		logErr(QString("session error: %1").arg(ex.what()));
	}

	delete recvBuf;

	{
		asio::error_code ec;
		stc->socket->shutdown(tcp::socket::shutdown_both, ec);
		stc->socket->close(ec);
	}
}

void AdsGatewayServer::removeSessionSocket(const TCP_SOCKET_SHARED& socket)
{
	std::lock_guard<std::mutex> lock(m_sessionsMutex);

	for (auto it = m_sessionSockets.begin(); it != m_sessionSockets.end(); ++it)
	{
		if (*it == socket)
		{
			m_sessionSockets.erase(it);
			break;
		}
	}
}

void AdsGatewayServer::reapFinishedSessions()
{
	std::lock_guard<std::mutex> lock(m_threadsMutex);

	for (std::size_t i = 0; i < m_sessionThreads.size();)
	{
		SessionThread& st = m_sessionThreads[i];

		if (st.finished && st.finished->load())
		{
			if (st.thread.joinable())
			{
				st.thread.join();
			}

			m_sessionThreads.erase(m_sessionThreads.begin() + static_cast<std::ptrdiff_t>(i));
			continue;
		}

		++i;
	}
}

void AdsGatewayServer::joinAllSessions()
{
	std::vector<SessionThread> threads;

	{
		std::lock_guard<std::mutex> lock(m_threadsMutex);
		threads.swap(m_sessionThreads);
	}

	for (auto& st : threads)
	{
		if (st.thread.joinable())
		{
			st.thread.join();
		}
	}
}

void AdsGatewayServer::processRequest(SessionThreadContextShared stc, char* recvBuf, std::size_t& recvBufIndex)
{
	if (recvBufIndex < AGL::GW_MSG_HEADER_SIZE)
	{
		return;
	}

	AGL::GwMessageHeader header;

	std::memcpy(&header, recvBuf, AGL::GW_MSG_HEADER_SIZE);

	if (recvBufIndex < AGL::GW_MSG_HEADER_SIZE +
						header.payloadSize +
						AGL::GW_MSG_CRC_SIZE)
	{
		return;
	}

	uint32_t calcCrc = Radiy::CRC32(recvBuf, AGL::GW_MSG_HEADER_SIZE + header.payloadSize);

	uint32_t receivedCrc;
	std::memcpy(&receivedCrc, recvBuf + AGL::GW_MSG_HEADER_SIZE + header.payloadSize, sizeof(receivedCrc));

	if (calcCrc != receivedCrc)
	{
		logErr(QString("request %1 error CRC 0x%2 (expected 0x%3)").
			   arg(header.requestID).arg(receivedCrc, 8, 16, QChar('0')).arg(calcCrc, 8, 16, QChar('0')));

		sendErrReply(stc, header, AGL::GWC_CRC_ERROR);
		recvBufIndex = 0;
		return;
	}

	std::size_t requestSize = 0;

	switch(static_cast<AGL::GwRequestId>(header.requestID))
	{
	case AGL::GwRequestId::ADSGW_HANDSHAKE:
		requestSize = processHandshakeRequest(stc, header, recvBuf, recvBufIndex);
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_LIST_START:
		requestSize = processSignalListStartRequest(stc, header, recvBuf, recvBufIndex);
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_LIST_NEXT:
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_START:
	case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
	case AGL::GwRequestId::ADSGW_SIGNAL_STATE:
	case AGL::GwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
		Q_ASSERT(false);
		break;

	default:
		sendErrReply(stc, header, AGL::GWC_INVALID_REQUEST);
		recvBufIndex = 0;
		return;
	}

	if (requestSize == CONTINUE_RECEIVE)
	{
		return;
	}

	if (requestSize == 0)		// error request processing!
	{
		recvBufIndex = 0;
		return;
	}

	requestSize += AGL::GW_MSG_CRC_SIZE;

	if (requestSize == recvBufIndex)
	{
		recvBufIndex = 0;
		return;
	}

	if (requestSize < recvBufIndex)
	{
		// move rest of data in buffer to 0 position
		//
		std::memcpy(recvBuf, recvBuf + requestSize, recvBufIndex - requestSize);
		recvBufIndex -= requestSize;
	}
}

std::size_t AdsGatewayServer::processHandshakeRequest(SessionThreadContextShared stc,
													const AGL::GwMessageHeader& header,
													const char* recvBuf, std::size_t& recvBufIndex)
{
	constexpr std::size_t REQUEST_SIZE = AGL::GW_MSG_HEADER_SIZE + AGL::GW_HANDSHAKE_REQUEST_SIZE;

	if (recvBufIndex < REQUEST_SIZE)
	{
		return CONTINUE_RECEIVE;
	}

	AGL::GwHandshakeRequest request;

	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_HANDSHAKE_REQUEST_SIZE);

	//

	bool nullTerminated = checkNullTerminated(request.clientName, sizeof(request.clientName));

	if (nullTerminated == false)
	{
		logErr("HANDSHAKE request error, clientName is NOT null-terminated");

		sendErrReply(stc, header, AGL::GWC_REQUEST_FORMAT_ERROR);
		return REQUEST_SIZE;
	}

	stc->clientName = QString::fromStdString(std::string(request.clientName));

	if (request.protocolVersion != AGL::ADSGW_PROTOCOL_VERSION)
	{
		logErr(QString("HANDSHAKE request from %1, WRONG protocol version 0x%2 (required %3)").
					arg(QString::fromStdString(std::string(request.clientName))).
					arg(request.protocolVersion, 4, 16, QChar('0')).
					arg(AGL::ADSGW_PROTOCOL_VERSION, 4, 16, QChar('0')));

		sendErrReply(stc, header, AGL::GWC_UNSUPPORTED_VERSION);
		return REQUEST_SIZE;
	}

	logMsg(QString("HANDSHAKE request from %1, protocol version 0x%2").
					arg(QString::fromStdString(std::string(request.clientName))).
					arg(request.protocolVersion, 4, 16, QChar('0')));

	AGL::GwHandshakeResponse reply;

	reply.protocolVersion = AGL::ADSGW_PROTOCOL_VERSION;
	reply.reserved = 0;
	reply.maxStateRequest = AGL::GW_MAX_SIGNAL_STATES;
	reply.sizeof_GwAppSignalParam = AGL::GW_APP_SIGNAL_PARAM_SIZE;
	reply.sizeof_GwAppSignalState = AGL::GW_APP_SIGNAL_STATE_SIZE;

	bool res = sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	if (res == false)
	{
		return 0;
	}

	stc->handshakeCompleted = true;

	return REQUEST_SIZE;
}

std::size_t AdsGatewayServer::processSignalListStartRequest(SessionThreadContextShared stc,
															const AGL::GwMessageHeader& header,
															const char* recvBuf, std::size_t& recvBufIndex)
{
	Q_UNUSED(recvBuf);

	constexpr std::size_t REQUEST_SIZE = AGL::GW_MSG_HEADER_SIZE + AGL::GW_SIGNAL_LIST_START_REQUEST_SIZE;

	if (recvBufIndex < REQUEST_SIZE)
	{
		return CONTINUE_RECEIVE;
	}

	if (stc->handshakeCompleted == false)
	{
		sendErrReply(stc, header, AGL::GWC_HANDSHAKE_REQUIRED);
		return REQUEST_SIZE;
	}

//	AGL::GwSignalListStartRequest request;
//	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_LIST_START_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_LIST_START request from %1").arg(stc->clientName));

	AGL::GwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(m_appSignals.count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = AGL::GW_MAX_APP_SIGNAL_ID_COUNT;
	reply.partCount = signalCount / reply.itemsPerPart + (signalCount % reply.itemsPerPart ? 1 : 0);

	bool res = sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	if (res == false)
	{
		return 0;
	}

	return REQUEST_SIZE;
}

std::size_t AdsGatewayServer::processSignalListNextRequest(SessionThreadContextShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, std::size_t& recvBufIndex)
{
	Q_UNUSED(recvBuf);

	std::size_t REQUEST_SIZE = AGL::GW_MSG_HEADER_SIZE + AGL::GW_SIGNAL_LIST_NEXT_REQUEST_SIZE;

	if (recvBufIndex < REQUEST_SIZE)
	{
		return CONTINUE_RECEIVE;
	}

	if (stc->handshakeCompleted == false)
	{
		sendErrReply(stc, header, AGL::GWC_HANDSHAKE_REQUIRED);
		return REQUEST_SIZE;
	}

	AGL::GwSignalListNextRequest request;
	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_LIST_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_LIST_NEXT request from %1, requested part %2").
						arg(stc->clientName).arg(request.part));

	AGL::GwSignalListNextResponse reply;

	reply.part = request.part;
	reply.appSignalIdCount = 0;

	std::vector<char> payloadData();

	int signalsCount = TO_INT(m_appSignals.count());
	int signalStartIndex = request.part * AGL::GW_MAX_APP_SIGNAL_ID_COUNT;

	for(int i = signalStartIndex; i < signalsCount; i++)
	{

	}

	bool res = sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	if (res == false)
	{
		return 0;
	}

	return REQUEST_SIZE;
}

bool AdsGatewayServer::sendErrReply(SessionThreadContextShared stc,
	const AdsGatewayLib::GwMessageHeader& requestHeader,
	AGL::GwErrorCode errCode)
{
	return sendReply(stc, requestHeader.requestID, errCode, nullptr, 0);
}

bool AdsGatewayServer::sendOkReply(SessionThreadContextShared stc,
	const AdsGatewayLib::GwMessageHeader& requestHeader,
	const char* payloadData, std::size_t payloadSize)
{
	return sendReply(stc, requestHeader.requestID, AGL::GWC_SUCCESS, payloadData, payloadSize);
}

bool AdsGatewayServer::sendReply(SessionThreadContextShared stc,
	uint32_t requestID, AGL::GwErrorCode errCode,
	const char* payloadData, std::size_t payloadSize)
{
	thread_local std::vector<char> sendBuf(AGL::ADSGW_MAX_PAYLOAD_SIZE);

	if (AGL::GW_MSG_HEADER_SIZE + payloadSize + AGL::GW_MSG_CRC_SIZE > AGL::ADSGW_MAX_PAYLOAD_SIZE)
	{
		return false;
	}

	if (errCode != AGL::GWC_SUCCESS)
	{
		Q_ASSERT(payloadData == nullptr);
		Q_ASSERT(payloadSize == 0);
		payloadSize = 0;
	}

	AGL::GwMessageHeader header;

	header.requestID = requestID;
	header.payloadSize = static_cast<uint32_t>(payloadSize);
	header.statusCode = static_cast<uint32_t>(errCode);

	std::memcpy(sendBuf.data(), &header, AGL::GW_MSG_HEADER_SIZE);

	if (payloadSize > 0)
	{
		std::memcpy(sendBuf.data() + AGL::GW_MSG_HEADER_SIZE, payloadData, payloadSize);
	}

	uint32_t crc = Radiy::CRC32(sendBuf.data(), AGL::GW_MSG_HEADER_SIZE + payloadSize);

	std::memcpy(sendBuf.data() + AGL::GW_MSG_HEADER_SIZE + payloadSize, &crc, AGL::GW_MSG_CRC_SIZE);

	asio::error_code ec;

	asio::write(*stc->socket, asio::buffer(sendBuf.data(), AGL::GW_MSG_HEADER_SIZE + payloadSize + AGL::GW_MSG_CRC_SIZE), ec);

	if (ec)
	{
		Q_ASSERT(false);
		return false;
	}

	return true;
}

bool AdsGatewayServer::checkNullTerminated(const char* str, std::size_t size)
{
	TEST_PTR_RETURN_FALSE(str);

	for(std::size_t i = 0; i < size; i++)
	{
		if (str[i] == 0)
		{
			return true;
		}
	}

	return false;
}

QString AdsGatewayServer::getIpPortStr(const std::shared_ptr<tcp::socket>& socket)
{
	if (socket == nullptr)
	{
		Q_ASSERT(false);
		return Separator::EMPTY_STR;
	}

	tcp::endpoint remote = socket->remote_endpoint();

	return QString("%1:%2").arg(QString::fromStdString(remote.address().to_string())).arg(remote.port());
}

