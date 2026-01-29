#include "AdsGatewayServer.h"

AdsGatewayServer::AdsGatewayServer(const HostAddressPort& listenIP,
									const AppSignals& appSignals,
									CircularLoggerShared log) :
	LogWrapper(log, "AdsGatewayServer"),
	m_listenIP(listenIP),
	m_appSignals(appSignals)
{
	int signalCount = TO_INT(m_appSignals.count());

	m_signalStates.resize(signalCount);
	m_hashToIndex.reserve(signalCount);

	for(int i = 0; i < signalCount; i++)
	{
		const AppSignal* appSignal = m_appSignals.getSignalByIndex(TO_INT(i));

		if (appSignal == nullptr)
		{
			Q_ASSERT(false);
			m_signalStates[i].hash = 0;
			continue;
		}

		Hash hash = appSignal->hash();

		m_signalStates[i].hash = hash;
		m_hashToIndex.emplace(hash, i);
	}
}

AdsGatewayServer::~AdsGatewayServer()
{
	stop();
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
			m_acceptor.reset();
		}
	}

	closeSessions();

	m_io.stop();

	if (m_serverThread.joinable())
	{
		m_serverThread.join();
	}

	joinAllSessions();
}

void AdsGatewayServer::updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply)
{
	setConnectedToAppDataSrv(true);

	thread_local std::vector<int> indexes;

	int statesCount = getStatesReply.appsignalstates_size();

	if (indexes.size() < statesCount)
	{
		indexes.resize(statesCount);
	}

	int notFoundCount = 0;

	for(int i = 0; i < statesCount; i++)
	{
		const Proto::AppSignalState& state = getStatesReply.appsignalstates(i);

		SimpleAppSignalState sass;

		sass.load(state);

		Hash hash = state.hash();

		auto it = m_hashToIndex.find(hash);

		if (it == m_hashToIndex.end())
		{
			notFoundCount++;
			indexes[i] = BAD_INDEX;
			continue;
		}

		indexes[i] = it->second;
	}

	{
		std::lock_guard lg(m_signalStatesMutex);

		for(int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& state = getStatesReply.appsignalstates(i);

			int signalIndex = indexes[i];

			if (signalIndex == BAD_INDEX)
			{
				continue;
			}

			SimpleAppSignalState& signalState = m_signalStates[signalIndex];

			Q_ASSERT(signalState.hash == state.hash());

			signalState.load(state);
		}
	}
}

void AdsGatewayServer::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
{
	setConnectedToAppDataSrv(true);

	int statesCount = getStateChangesReply.appsignalstates_size();

	if (statesCount == 0)
	{
		return;
	}

	AGL::GwAppSignalState state;

	constexpr size_t MAX_QUEUE_SIZE = 200000;

	std::lock_guard lg(m_signalStateChangesMutex);

	const size_t curSize = m_signalStateChanges.size();

	if (curSize + statesCount > MAX_QUEUE_SIZE)
	{
		const size_t deleteCount = curSize + statesCount - MAX_QUEUE_SIZE ;

		if (deleteCount >= curSize)
		{
			m_signalStateChanges.clear();
		}
		else
		{
			m_signalStateChanges.erase(m_signalStateChanges.begin(),
									   m_signalStateChanges.begin() + deleteCount);
		}
	}

	for(int i = 0; i < statesCount; i++)
	{
		const Proto::AppSignalState& s = getStateChangesReply.appsignalstates(i).curstate();

		state.hash = s.hash();
		state.systemTime = s.systemtime();
		state.localTime = s.localtime();
		state.plantTime = s.planttime();
		state.value = s.value();
		state.flags = s.flags();
		state.reserved = 0;

		m_signalStateChanges.push_back(state);
	}
}

void AdsGatewayServer::setConnectedToAppDataSrv(bool connected)
{
	m_connectedToAppDataSrv.store(connected, std::memory_order_relaxed);

	std::lock_guard lg(m_sessionsMutex);

	for(SessionShared session : m_sessions)
	{
		session->connectedToAppDataSrv.store(connected, std::memory_order_relaxed);
	}
}

void AdsGatewayServer::runAcceptLoop()
{
	try
	{
		m_io.restart();

		tcp::endpoint ep(asio::ip::address_v4::from_string(m_listenIP.addressStr().toStdString()), m_listenIP.port());

		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);

			m_acceptor = std::make_shared<tcp::acceptor>(m_io);

			m_acceptor->open(ep.protocol());
			m_acceptor->set_option(tcp::acceptor::reuse_address(true));
			m_acceptor->bind(ep);
			m_acceptor->listen();
		}

		logMsg(QString("starts listening %1").arg(m_listenIP.addressPortStr()));

		while (m_running.load())
		{
			reapFinishedSessions();

			tcp::socket socket(m_io);
			asio::error_code ec;

			std::shared_ptr<tcp::acceptor> acceptor = nullptr;

			{
				std::lock_guard<std::mutex> lock(m_acceptorMutex);
				acceptor = m_acceptor;
			}

			if (!acceptor)
			{
				break;
			}

			acceptor->accept(socket, ec);

			if (!m_running.load())
			{
				break;
			}

			if (ec)
			{
				if (ec == asio::error::operation_aborted ||
					ec == asio::error::bad_descriptor ||
					ec == asio::error::invalid_argument ||
					ec == asio::error::not_socket ||
					ec == asio::error::no_descriptors)
				{
					break;
				}

				logErr(QString("accept error: %1").arg(QString::fromStdString(ec.message())));
				continue;
			}

			logMsg(QString("accept new connectio from %1").arg(getIpPortStr(socket)));

			{
				SessionShared stc = std::make_shared<Session>();

				stc->socket = std::make_shared<tcp::socket>(std::move(socket));
				stc->connectedToAppDataSrv.store(m_connectedToAppDataSrv.load(std::memory_order_relaxed),
												std::memory_order_relaxed);

				{
					std::lock_guard<std::mutex> lg(m_sessionsMutex);
					m_sessions.insert(stc);
				}

				stc->thread = std::thread(
					[this, stc]()
					{
						const QString remoteIpPort = getIpPortStr(*stc->socket);

						logMsg(QString("session thread for %1 started").arg(remoteIpPort));

						sessionThread(stc);

						stc->finished.store(true, std::memory_order_release);

						logMsg(QString("session thread for %1 finished").arg(remoteIpPort));
					});
			}
		}

		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);
			m_acceptor.reset();
		}
	}
	catch (const std::exception& ex)
	{
		{
			std::lock_guard<std::mutex> lock(m_acceptorMutex);
			m_acceptor.reset();
		}

		logErr(QString("accept loop error: %1").arg(ex.what()));
	}

	logMsg(QString("stops"));
}

void AdsGatewayServer::sessionThread(SessionShared stc)
{
	stc->payloadData.resize(AGL::GW_MAX_MSG_PAYLOAD_SIZE);

	std::vector<char> receiveBuffer(AGL::ADSGW_MAX_PAYLOAD_SIZE);
	char* recvBuf = receiveBuffer.data();
	size_t recvBufSize = 0;

	try
	{
		asio::error_code ec;

		stc->socket->set_option(tcp::no_delay(true), ec);

		while (m_running.load()  && !stc->closing.load())
		{
			if (recvBufSize >= AGL::ADSGW_MAX_PAYLOAD_SIZE)
			{
				logErr("receive buffer overflow / invalid request size, receive buffer clearin");
				recvBufSize = 0;
			}

			size_t bytesRead = stc->socket->read_some(
				asio::buffer(recvBuf + recvBufSize, AGL::ADSGW_MAX_PAYLOAD_SIZE - recvBufSize), ec);

			if (ec)
			{
				break;
			}

			recvBufSize += bytesRead;

			processRequest(stc, recvBuf, recvBufSize);

			if (stc->errCount > MAX_SESSION_ERRORS)
			{
				break;
			}
		}
	}
	catch (const std::exception& ex)
	{
		logErr(QString("session error: %1").arg(ex.what()));
	}

	closeSocket(stc);
}

void AdsGatewayServer::reapFinishedSessions()
{
	std::vector<SessionShared> finishedSessions;

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (auto it = m_sessions.begin(); it != m_sessions.end(); )
		{
			const SessionShared& session = *it;

			if (session->finished.load(std::memory_order_acquire) == true)
			{
				finishedSessions.push_back(session);

				it = m_sessions.erase(it);
				continue;
			}

			++it;
		}
	}

	for (SessionShared& session : finishedSessions)
	{
		if (session->thread.joinable())
		{
			session->thread.join();
		}
	}
}

void AdsGatewayServer::closeSocket(SessionShared stc)
{
	if (!stc)
	{
		return;
	}

	bool expected = false;

	if (!stc->closing.compare_exchange_strong(expected, true))
	{
		return;
	}

	if (stc->socket)
	{
		asio::error_code ec;
		stc->socket->shutdown(tcp::socket::shutdown_both, ec);
		stc->socket->close(ec);
	}
}

void AdsGatewayServer::requestCloseSession(SessionShared stc)
{
	if (!stc)
	{
		return;
	}

	stc->closing.store(true, std::memory_order_relaxed);

	if (stc->socket)
	{
		asio::error_code ec;
		stc->socket->cancel(ec);
	}
}

void AdsGatewayServer::closeSessions()
{
	std::vector<SessionShared> sessions;

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (SessionShared session : m_sessions)
		{
			sessions.push_back(session);
		}
	}

	for(SessionShared session: sessions)
	{
		requestCloseSession(session);
	}
}

void AdsGatewayServer::joinAllSessions()
{
	std::set<SessionShared> sessions;

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);
		sessions.swap(m_sessions);
	}

	for (SessionShared session : sessions)
	{
		if (session->thread.joinable())
		{
			session->thread.join();
		}
	}
}

void AdsGatewayServer::processRequest(SessionShared stc, char* recvBuf, size_t& recvBufSize)
{
	while(recvBufSize >= AGL::GW_MSG_HEADER_SIZE)
	{
		AGL::GwMessageHeader header;

		std::memcpy(&header, recvBuf, AGL::GW_MSG_HEADER_SIZE);

		if (header.payloadSize > AGL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			sendErrReply(stc, header, AGL::GWC_REQUEST_FORMAT_ERROR);
			stc->errCount++;
			recvBufSize = 0;
			return;
		}

		const size_t requestSize = AGL::GW_MSG_HEADER_SIZE +
								   header.payloadSize +
								   AGL::GW_MSG_CRC_SIZE;

		if (recvBufSize < requestSize)
		{
			return;
		}

		if (requestSize > AGL::ADSGW_MAX_PAYLOAD_SIZE)
		{
			sendErrReply(stc, header, AGL::GWC_REQUEST_FORMAT_ERROR);
			stc->errCount++;
			recvBufSize = 0;
			return;
		}

		AGL::GwRequestId requestID = static_cast<AGL::GwRequestId>(header.requestID);

		switch(requestID)
		{
		case AGL::GwRequestId::ADSGW_HANDSHAKE:
		case AGL::GwRequestId::ADSGW_SIGNAL_LIST_START:
		case AGL::GwRequestId::ADSGW_SIGNAL_LIST_NEXT:
		case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_START:
		case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
		case AGL::GwRequestId::ADSGW_SIGNAL_STATE:
		case AGL::GwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
			break;
		default:
			sendErrReply(stc, header, AGL::GWC_INVALID_REQUEST);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		AGL::GwErrorCode errCode = AGL::GwErrorCode::GWC_SUCCESS;

		if (checkPayloadSize(header, recvBuf, recvBufSize, errCode) == false)
		{
			sendErrReply(stc, header, errCode);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		uint32_t calcCrc = Radiy::CRC32(recvBuf, AGL::GW_MSG_HEADER_SIZE + header.payloadSize);

		uint32_t receivedCrc;
		std::memcpy(&receivedCrc, recvBuf + AGL::GW_MSG_HEADER_SIZE + header.payloadSize, sizeof(receivedCrc));

		if (calcCrc != receivedCrc)
		{
			logErr(QString("request %1 error CRC 0x%2 (expected 0x%3)").
				   arg(header.requestID).arg(receivedCrc, 8, 16, QChar('0')).arg(calcCrc, 8, 16, QChar('0')));

			sendErrReply(stc, header, AGL::GWC_CRC_ERROR);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		if (requestID != AGL::GwRequestId::ADSGW_HANDSHAKE &&
			stc->handshakeCompleted == false)
		{
			sendErrReply(stc, header, AGL::GWC_HANDSHAKE_REQUIRED);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		bool result = true;

		switch(requestID)
		{
		case AGL::GwRequestId::ADSGW_HANDSHAKE:
			result = processHandshakeRequest(stc, header, recvBuf, requestSize);
			break;

		case AGL::GwRequestId::ADSGW_SIGNAL_LIST_START:
			result = processSignalListStartRequest(stc, header, recvBuf, requestSize);
			break;

		case AGL::GwRequestId::ADSGW_SIGNAL_LIST_NEXT:
			result = processSignalListNextRequest(stc, header, recvBuf, requestSize);
			break;

		case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_START:
			result = processSignalParamStartRequest(stc, header, recvBuf, requestSize);
			break;

		case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
			result = processSignalParamNextRequest(stc, header, recvBuf, requestSize);
			break;

		case AGL::GwRequestId::ADSGW_SIGNAL_STATE:
			result = processSignalStateRequest(stc, header, recvBuf, requestSize);
			break;

		case AGL::GwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
			result = processSignalStateChangesRequest(stc, header, recvBuf, requestSize);
			break;

		default:
			Q_ASSERT(false);
		}

		if (result == false)
		{
			stc->errCount++;
		}

		recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
	}
}

bool AdsGatewayServer::processHandshakeRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(requestSize);

	AGL::GwHandshakeRequest request;

	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_HANDSHAKE_REQUEST_SIZE);

	//

	bool nullTerminated = checkNullTerminated(request.clientName, sizeof(request.clientName));

	if (nullTerminated == false)
	{
		logErr("HANDSHAKE request error, clientName is NOT null-terminated");

		sendErrReply(stc, header, AGL::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	stc->clientName = QString::fromUtf8(request.clientName);

	if (request.protocolVersion != AGL::ADSGW_PROTOCOL_VERSION)
	{
		logErr(QString("HANDSHAKE request from %1, WRONG protocol version 0x%2 (required %3)").
					arg(stc->clientName).
					arg(request.protocolVersion, 4, 16, QChar('0')).
					arg(AGL::ADSGW_PROTOCOL_VERSION, 4, 16, QChar('0')));

		sendErrReply(stc, header, AGL::GWC_UNSUPPORTED_VERSION);
		return false;
	}

	logMsg(QString("HANDSHAKE request from %1, protocol version 0x%2").
					arg(stc->clientName).
					arg(request.protocolVersion, 4, 16, QChar('0')));

	AGL::GwHandshakeResponse reply;

	reply.protocolVersion = AGL::ADSGW_PROTOCOL_VERSION;
	reply.reserved = 0;
	reply.maxStateRequest = AGL::GW_MAX_SIGNAL_STATES;
	reply.sizeof_GwAppSignalParam = AGL::GW_APP_SIGNAL_PARAM_SIZE;
	reply.sizeof_GwAppSignalState = AGL::GW_APP_SIGNAL_STATE_SIZE;

	sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	stc->handshakeCompleted = true;

	return true;
}

bool AdsGatewayServer::processSignalListStartRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	//

	logMsg(QString("SIGNAL_LIST_START request from %1").arg(stc->clientName));

	AGL::GwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(m_appSignals.count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = AGL::GW_MAX_APP_SIGNAL_ID_COUNT;
	reply.partCount = signalCount / reply.itemsPerPart + (signalCount % reply.itemsPerPart ? 1 : 0);

	char* payloadData = stc->payloadData.data();
	const size_t payloadSize = sizeof(reply);

	std::memcpy(payloadData, &reply, payloadSize);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool AdsGatewayServer::processSignalListNextRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(requestSize);

	AGL::GwSignalListNextRequest request;
	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_LIST_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_LIST_NEXT request from %1, requested part %2").
						arg(stc->clientName).arg(request.part));

	char* payloadData = stc->payloadData.data();

	const int signalsCount = TO_INT(m_appSignals.count());
	const int itemsPerPart = AGL::GW_MAX_APP_SIGNAL_ID_COUNT;
	const int partCount = signalsCount / itemsPerPart + (signalsCount % itemsPerPart ? 1 : 0);

	if (request.part >= static_cast<uint32_t>(partCount))
	{
		sendErrReply(stc, header, AGL::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	AGL::GwSignalListNextResponse reply;

	reply.part = request.part;
	reply.appSignalIdCount = 0;

	int signalStartIndex = request.part * AGL::GW_MAX_APP_SIGNAL_ID_COUNT;
	const int signalEndIndex = std::min(TO_INT(signalStartIndex + AGL::GW_MAX_APP_SIGNAL_ID_COUNT), signalsCount);

	size_t payloadSize = AGL::GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE;

	for(int i = signalStartIndex; i < signalEndIndex; i++)
	{
		if (payloadSize + AGL::GW_APP_SIGNAL_ID_SIZE > AGL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			Q_ASSERT(false);
			logErr("AdsGatewayServer::processSignalListNextRequest payload size exceed!");
			sendErrReply(stc, header, AGL::GWC_INTERNAL_ERROR);
			return false;
		}

		const AppSignal* appSignal = m_appSignals.getSignalByIndex(i);

		TEST_PTR_CONTINUE(appSignal);

		copyStr(payloadData + payloadSize, AGL::GW_APP_SIGNAL_ID_SIZE, appSignal->appSignalID());

		payloadSize +=  AGL::GW_APP_SIGNAL_ID_SIZE;

		reply.appSignalIdCount++;
	}

	std::memcpy(payloadData, &reply, AGL::GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool AdsGatewayServer::processSignalParamStartRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	logMsg(QString("SIGNAL_PARAM_START request from %1").arg(stc->clientName));

	AGL::GwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(m_appSignals.count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = AGL::GW_MAX_SIGNAL_PARAMS;
	reply.partCount = signalCount / reply.itemsPerPart + (signalCount % reply.itemsPerPart ? 1 : 0);

	char* payloadData = stc->payloadData.data();
	const size_t payloadSize = sizeof(reply);

	std::memcpy(payloadData, &reply, payloadSize);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool AdsGatewayServer::processSignalParamNextRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	AGL::GwSignalParamNextRequest request;
	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_PARAM_NEXT request from %1, requested part %2").
		   arg(stc->clientName).arg(request.part));

	char* payloadData = stc->payloadData.data();

	const int signalsCount = TO_INT(m_appSignals.count());
	const int itemsPerPart = AGL::GW_MAX_SIGNAL_PARAMS;
	const int partCount = signalsCount / itemsPerPart + (signalsCount % itemsPerPart ? 1 : 0);

	if (request.part >= static_cast<uint32_t>(partCount))
	{
		sendErrReply(stc, header, AGL::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	AGL::GwSignalParamNextResponse reply;

	reply.part = request.part;
	reply.paramCount = 0;

	int signalStartIndex = request.part * AGL::GW_MAX_SIGNAL_PARAMS;
	const int signalEndIndex = std::min(TO_INT(signalStartIndex + AGL::GW_MAX_SIGNAL_PARAMS), signalsCount);

	size_t payloadSize = AGL::GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE;

	for(int i = signalStartIndex; i < signalEndIndex; i++)
	{
		if (payloadSize + AGL::GW_APP_SIGNAL_PARAM_SIZE > AGL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			Q_ASSERT(false);
			logErr("AdsGatewayServer::processSignalParamNextRequest payload size exceed!");
			sendErrReply(stc, header, AGL::GWC_INTERNAL_ERROR);
			return false;
		}

		const AppSignal* appSignal = m_appSignals.getSignalByIndex(i);

		TEST_PTR_CONTINUE(appSignal);

		AGL::GwAppSignalParam p;

		p.hash = TO_UINT64(appSignal->hash());
		copyStr(p.appSignalId, AGL::STRING_LENGTH_128, appSignal->appSignalID());
		copyStr(p.customSignalId, AGL::STRING_LENGTH_128, appSignal->customAppSignalID());
		copyStr(p.caption, AGL::STRING_LENGTH_256, appSignal->caption());
		copyStr(p.equipmentId, AGL::STRING_LENGTH_128, appSignal->equipmentID());
		copyStr(p.lmEquipmentId, AGL::STRING_LENGTH_128, appSignal->lmEquipmentID());
		copyStr(p.units, AGL::STRING_LENGTH_128, appSignal->unit());
		copyStr(p.tags, AGL::STRING_LENGTH_256, appSignal->tagsStr());
		p.channel = channelChar(appSignal->channel());
		p.inOutType = TO_UINT8(appSignal->inOutType());
		p.type = TO_UINT8(appSignal->signalType());
		p.decimalPlaces = TO_UINT8(appSignal->decimalPlaces());
		p.tuning = TO_UINT8(appSignal->enableTuning());
		p.reserved1 = 0;
		p.reserved2 = 0;
		p.reserved3 = 0;
		p.lowValidRange = appSignal->lowValidRange();
		p.highValidRange = appSignal->highValidRange();
		p.tuningDefaultValue = appSignal->tuningDefaultValue().toDouble();
		p.tuningLowBound = appSignal->tuningLowBound().toDouble();
		p.tuningHighBound = appSignal->tuningHighBound().toDouble();

		std::memcpy(payloadData + payloadSize, &p, AGL::GW_APP_SIGNAL_PARAM_SIZE);

		payloadSize +=  AGL::GW_APP_SIGNAL_PARAM_SIZE;

		reply.paramCount++;
	}

	std::memcpy(payloadData, &reply, AGL::GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool AdsGatewayServer::processSignalStateRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	AGL::GwSignalStateRequest request;
	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_STATE_REQUEST_SIZE);

	logMsg(QString("SIGNAL_STATE request from %1, %2 states requested").
		   arg(stc->clientName).arg(request.signalCount));

	if (request.signalCount > AGL::GW_MAX_SIGNAL_STATES)
	{
		sendErrReply(stc, header, AGL::GWC_TOO_MANY_SIGNALS);
		return false;
	}

	if (stc->connectedToAppDataSrv.load(std::memory_order_relaxed) == false)
	{
		sendErrReply(stc, header, AGL::GWC_NO_ADS_CONNECTION);
		return true;		// this is not request format error!
	}

	char* payloadData = stc->payloadData.data();

	AGL::GwSignalStateResponse reply;

	reply.stateCount = 0;

	const uint32_t hashesCount = request.signalCount;
	size_t hashesOffset = AGL::GW_MSG_HEADER_SIZE + AGL::GW_SIGNAL_STATE_REQUEST_SIZE;

	size_t payloadSize = AGL::GW_SIGNAL_STATE_RESPONSE_SIZE;

	{
		std::lock_guard lg(m_signalStatesMutex);

		for(uint32_t i = 0; i < hashesCount; i++)
		{
			if (payloadSize + AGL::GW_APP_SIGNAL_STATE_SIZE > AGL::GW_MAX_MSG_PAYLOAD_SIZE)
			{
				Q_ASSERT(false);
				logErr("AdsGatewayServer::processSignalStateRequest payload size exceed!");
				sendErrReply(stc, header, AGL::GWC_INTERNAL_ERROR);
				return false;
			}

			Hash hash;

			std::memcpy(&hash, recvBuf + hashesOffset, sizeof(hash));
			hashesOffset += AGL::GW_APP_SIGNAL_HASH_SIZE;

			auto it = m_hashToIndex.find(hash);

			if (it == m_hashToIndex.end())
			{
				continue;
			}

			int stateIndex = it->second;

			if (stateIndex < 0 || stateIndex >= TO_INT(m_signalStates.size()))
			{
				Q_ASSERT(false);
				continue;
			}

			const SimpleAppSignalState& st = m_signalStates[stateIndex];

			Q_ASSERT(st.hash == hash);

			AGL::GwAppSignalState state;

			state.hash = st.hash;
			state.systemTime = st.systemTime();
			state.localTime = st.localTime();
			state.plantTime = st.plantTime();
			state.value = st.value;
			state.flags = st.flags.all;
			state.reserved = 0;

			std::memcpy(payloadData + payloadSize, &state, AGL::GW_APP_SIGNAL_STATE_SIZE);

			payloadSize +=  AGL::GW_APP_SIGNAL_STATE_SIZE;

			reply.stateCount++;
		}
	}

	std::memcpy(payloadData, &reply, AGL::GW_SIGNAL_STATE_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	logMsg(QString("Sent %1 signal states").arg(reply.stateCount));

	return true;
}

bool AdsGatewayServer::processSignalStateChangesRequest(SessionShared stc,
	const AGL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	logMsg(QString("SIGNAL_STATE_CHANGES request from %1").arg(stc->clientName));

	AGL::GwSignalStateChangesRequest request;
	std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE);

	if (stc->connectedToAppDataSrv.load(std::memory_order_relaxed) == false)
	{
		sendErrReply(stc, header, AGL::GWC_NO_ADS_CONNECTION);
		return true;		// this is not request format error!
	}

	//

	char* payloadData = stc->payloadData.data();

	AGL::GwSignalStateChangesResponse reply;

	reply.stateCount = 0;
	reply.pendingStatesCount = 0;

	size_t payloadSize = AGL::GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE;

	{
		std::lock_guard lg(m_signalStateChangesMutex);

		AGL::GwAppSignalState state;

		while(m_signalStateChanges.empty() == false &&
			   reply.stateCount < AGL::GW_MAX_SIGNAL_STATE_CHANGES)
		{
			if (payloadSize + AGL::GW_APP_SIGNAL_STATE_SIZE > AGL::GW_MAX_MSG_PAYLOAD_SIZE)
			{
				break;
			}

			state = m_signalStateChanges.front();
			m_signalStateChanges.pop_front();

			std::memcpy(payloadData + payloadSize, &state, AGL::GW_APP_SIGNAL_STATE_SIZE);

			payloadSize +=  AGL::GW_APP_SIGNAL_STATE_SIZE;

			reply.stateCount++;
		}

		reply.pendingStatesCount = TO_UINT32(m_signalStateChanges.size());
	}

	std::memcpy(payloadData, &reply, AGL::GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	logMsg(QString("Sent %1 signal state changes, pending states %2").
						arg(reply.stateCount).arg(reply.pendingStatesCount));
	return true;
}

bool AdsGatewayServer::checkPayloadSize(const AdsGatewayLib::GwMessageHeader& header,
	const char *recvBuf, const size_t recvBufSize, AdsGatewayLib::GwErrorCode& errCode)
{
	errCode = AGL::GWC_REQUEST_FORMAT_ERROR;

	TEST_PTR_RETURN_FALSE(recvBuf);

	size_t payloadSize = 0;

	switch(static_cast<AGL::GwRequestId>(header.requestID))
	{
	case AGL::GwRequestId::ADSGW_HANDSHAKE:
		payloadSize = AGL::GW_HANDSHAKE_REQUEST_SIZE;
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_LIST_START:
		payloadSize = AGL::GW_SIGNAL_LIST_START_REQUEST_SIZE;
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_LIST_NEXT:
		payloadSize = AGL::GW_SIGNAL_LIST_NEXT_REQUEST_SIZE;
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_START:
		payloadSize = AGL::GW_SIGNAL_PARAM_START_REQUEST_SIZE;
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
		payloadSize = AGL::GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE;
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_STATE:
		{
			if (recvBufSize < AGL::GW_MSG_HEADER_SIZE + AGL::GW_SIGNAL_STATE_REQUEST_SIZE)
			{
				return false;
			}
			AGL::GwSignalStateRequest request;

			std::memcpy(&request, recvBuf + AGL::GW_MSG_HEADER_SIZE, AGL::GW_SIGNAL_STATE_REQUEST_SIZE);

			if (request.signalCount > AGL::GW_MAX_SIGNAL_STATES)
			{
				errCode = AGL::GwErrorCode::GWC_TOO_MANY_SIGNALS;
				return false;
			}

			payloadSize = AGL::GW_SIGNAL_STATE_REQUEST_SIZE +
						  static_cast<size_t>(request.signalCount) * AGL::GW_APP_SIGNAL_HASH_SIZE;
		}
		break;

	case AGL::GwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
		payloadSize = AGL::GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE;
		break;

	default:
		Q_ASSERT(false);
		return false;
	}

	if (header.payloadSize != payloadSize)
	{
		return false;
	}

	errCode = AGL::GwErrorCode::GWC_SUCCESS;
	return true;
}

size_t AdsGatewayServer::skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize)
{
	if (recvBufSize < requestSize)
	{
		Q_ASSERT(false);
		return recvBufSize;
	}

	const size_t restSize = recvBufSize - requestSize;

	if (restSize > 0)
	{
		std::memmove(recvBuf, recvBuf + requestSize, restSize);
	}

	return restSize;
}

void AdsGatewayServer::sendErrReply(SessionShared stc,
	const AdsGatewayLib::GwMessageHeader& requestHeader,
	AGL::GwErrorCode errCode)
{
	sendReply(stc, requestHeader.requestID, errCode, nullptr, 0);
}

void AdsGatewayServer::sendOkReply(SessionShared stc,
	const AdsGatewayLib::GwMessageHeader& requestHeader,
	const char* payloadData, size_t payloadSize)
{
	sendReply(stc, requestHeader.requestID, AGL::GWC_SUCCESS, payloadData, payloadSize);
}

void AdsGatewayServer::sendReply(SessionShared stc,
	uint32_t requestID, AGL::GwErrorCode errCode,
	const char* payloadData, size_t payloadSize)
{
	TEST_PTR_RETURN(stc);

	thread_local std::vector<char> sendBuf(AGL::ADSGW_MAX_PAYLOAD_SIZE);

	if (AGL::GW_MSG_HEADER_SIZE + payloadSize + AGL::GW_MSG_CRC_SIZE > AGL::ADSGW_MAX_PAYLOAD_SIZE)
	{
		Q_ASSERT(false);
		payloadData = nullptr;
		payloadSize = 0;
		errCode = AGL::GWC_INTERNAL_ERROR;
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

	if (stc->closing.load(std::memory_order_relaxed))
	{
		return;
	}

	asio::error_code ec;

	asio::write(*stc->socket, asio::buffer(sendBuf.data(), AGL::GW_MSG_HEADER_SIZE + payloadSize + AGL::GW_MSG_CRC_SIZE), ec);

	if (ec)
	{
		logErr(QString("send error: %1").arg(QString::fromStdString(ec.message())));
		requestCloseSession(stc);
	}
}

bool AdsGatewayServer::checkNullTerminated(const char* str, size_t size) const
{
	TEST_PTR_RETURN_FALSE(str);

	for(size_t i = 0; i < size; i++)
	{
		if (str[i] == 0)
		{
			return true;
		}
	}

	return false;
}

QString AdsGatewayServer::getIpPortStr(const tcp::socket& socket) const
{
	tcp::endpoint remote = socket.remote_endpoint();

	return QString("%1:%2").arg(QString::fromStdString(remote.address().to_string())).arg(remote.port());
}

void AdsGatewayServer::copyStr(char* toStr, size_t toStrLen, const QString& fromStr) const
{
	TEST_PTR_RETURN(toStr);

	const QByteArray fromData = fromStr.toUtf8();

	size_t fromLen = fromData.size();

	if (fromLen > toStrLen - 1)
	{
		std::memset(toStr, 0, toStrLen);
	}
	else
	{
		std::memcpy(toStr, fromData.constData(), fromLen);
		std::memset(toStr + fromLen, 0, toStrLen - fromLen);
	}
}

uint8_t AdsGatewayServer::channelChar(E::Channel ch) const
{
	switch(ch)
	{
	case E::Channel::A: return TO_UINT8('A');
	case E::Channel::B: return TO_UINT8('B');
	case E::Channel::C: return TO_UINT8('C');
	case E::Channel::D: return TO_UINT8('D');
	default: ;
	}

	return 0;
}
