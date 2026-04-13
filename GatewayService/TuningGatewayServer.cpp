#include "TuningGatewayServer.h"

TuningGatewayServer::TuningGatewayServer(const SoftwareInfo& swInfo, const HostAddressPort& listenIP,
										const HostAddressPort& tunSrvIP1,
										const HostAddressPort& tunSrvIP2,
										const AppSignals& appSignals,
										CircularLoggerShared log) :
	LogWrapper(log, "TuningGatewayServer"),
	m_swInfo(swInfo),
	m_listenIP(listenIP),
	m_tunSrvIP1(tunSrvIP1),
	m_tunSrvIP2(tunSrvIP2),
	m_appSignals(appSignals)
{
}

TuningGatewayServer::~TuningGatewayServer()
{
}

void TuningGatewayServer::start()
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

void TuningGatewayServer::stop()
{
	if (!m_running.exchange(false))
	{
		return;
	}

	resetAcceptor();

	closeSessions();

	m_io.stop();

	if (m_serverThread.joinable())
	{
		m_serverThread.join();
	}

	joinAllSessions();
}


void TuningGatewayServer::setConnectedToTuningSrv(bool connected)
{
	m_connectedToTuningSrv.store(connected, std::memory_order_relaxed);

	{
		std::lock_guard lg(m_sessionsMutex);

		for(TgsSessionShared session : m_sessions)
		{
			session->connectedToTuningSrv.store(connected, std::memory_order_relaxed);
		}
	}
}

void TuningGatewayServer::runAcceptLoop()
{
	static constexpr int BIND_RETRY_DELAY_MS = 1000;

	while (m_running.load(std::memory_order_relaxed))
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

			while (m_running.load(std::memory_order_relaxed))
			{
				reapFinishedSessions();

				tcp::socket socket(m_io);
				asio::error_code ec;

				std::shared_ptr<tcp::acceptor> acceptor;

				{
					std::lock_guard<std::mutex> lock(m_acceptorMutex);
					acceptor = m_acceptor;
				}

				if (!acceptor)
				{
					break;
				}

				acceptor->accept(socket, ec);

				if (!m_running.load(std::memory_order_relaxed))
				{
					break;
				}

				if (ec)
				{
					if (ec == asio::error::operation_aborted ||
						ec == asio::error::bad_descriptor ||
						ec == asio::error::invalid_argument ||
						ec == asio::error::not_socket)
					{
						logErr(QString("acceptor closed or invalid: %1").
											arg(QString::fromStdString(ec.message())));
						break;
					}

					logErr(QString("accept error: %1").arg(QString::fromStdString(ec.message())));
					continue;
				}

				logMsg(QString("accept new connection from %1").arg(getIpPortStr(socket)));

				// run session thread
				//
				TgsSessionShared stc = std::make_shared<TgsSession>();

				stc->socket = std::make_shared<tcp::socket>(std::move(socket));
				stc->connectedToTuningSrv.store(m_connectedToTuningSrv.load(std::memory_order_relaxed),
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
		catch (const std::exception& ex)
		{
			logErr(QString("accept loop error on %1: %2").
				   arg(m_listenIP.addressPortStr(), ex.what()));
		}

		resetAcceptor();

		reapFinishedSessions();

		if (!m_running.load(std::memory_order_relaxed))
		{
			break;
		}

		logWrn(QString("accept loop will retry bind/listen after delay"));

		std::this_thread::sleep_for(std::chrono::milliseconds(BIND_RETRY_DELAY_MS));
	}

	logMsg(QString("stops"));
}

void TuningGatewayServer::sessionThread(TgsSessionShared stc)
{
	startTuningSrvClient(stc);

	stc->payloadData.resize(GCL::GW_MAX_MSG_PAYLOAD_SIZE);

	std::vector<char> receiveBuffer(GCL::GW_MAX_PAYLOAD_SIZE);
	char* recvBuf = receiveBuffer.data();
	size_t recvBufSize = 0;

	try
	{
		asio::error_code ec;

		stc->socket->set_option(tcp::no_delay(true), ec);

		while (m_running.load(std::memory_order_relaxed)  &&
					!stc->closing.load(std::memory_order_relaxed))
		{
			if (recvBufSize >= GCL::GW_MAX_PAYLOAD_SIZE)
			{
				logErr("receive buffer overflow / invalid request size, receive buffer clearin");
				recvBufSize = 0;
			}

			size_t bytesRead = stc->socket->read_some(asio::buffer(recvBuf + recvBufSize, GCL::GW_MAX_PAYLOAD_SIZE - recvBufSize), ec);

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

	stopTuningSrvClient(stc);
}

void TuningGatewayServer::reapFinishedSessions()
{
	std::vector<TgsSessionShared> finishedSessions;

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (auto it = m_sessions.begin(); it != m_sessions.end(); )
		{
			const TgsSessionShared& session = *it;

			if (session->finished.load(std::memory_order_acquire) == true)
			{
				finishedSessions.push_back(session);

				it = m_sessions.erase(it);
				continue;
			}

			++it;
		}
	}

	for (TgsSessionShared& session : finishedSessions)
	{
		if (session->thread.joinable())
		{
			session->thread.join();
		}
	}
}

void TuningGatewayServer::closeSocket(TgsSessionShared stc)
{
	requestCloseSession(stc);
}

void TuningGatewayServer::requestCloseSession(TgsSessionShared stc)
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
		stc->socket->cancel(ec);
		stc->socket->shutdown(tcp::socket::shutdown_both, ec);
		stc->socket->close(ec);
	}
}

void TuningGatewayServer::closeSessions()
{
	std::vector<TgsSessionShared> sessions;

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (TgsSessionShared session : m_sessions)
		{
			sessions.push_back(session);
		}
	}

	for(TgsSessionShared session: sessions)
	{
		requestCloseSession(session);
	}
}

void TuningGatewayServer::joinAllSessions()
{
	std::set<TgsSessionShared> sessions;

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);
		sessions.swap(m_sessions);
	}

	for (TgsSessionShared session : sessions)
	{
		if (session->thread.joinable())
		{
			session->thread.join();
		}
	}
}

void TuningGatewayServer::resetAcceptor()
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

void TuningGatewayServer::startTuningSrvClient(TgsSessionShared stc)
{
	TEST_PTR_RETURN(stc);

	Q_ASSERT(stc->tunSrvClientThread == nullptr);

	stc->tunSrvClientThread = std::make_unique<TuningSrvClientThread>(stc, m_swInfo, m_tunSrvIP1, m_tunSrvIP2,
							QString("TuningGateway %1").arg(m_swInfo.equipmentID()), Separator::EMPTY_STR);
}

void TuningGatewayServer::stopTuningSrvClient(TgsSessionShared stc)
{
	TEST_PTR_RETURN(stc);

	if (stc->tunSrvClientThread == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	stc->tunSrvClientThread->quitAndWait(5000);
	stc->tunSrvClientThread.reset();
}

void TuningGatewayServer::processRequest(TgsSessionShared stc, char* recvBuf, size_t& recvBufSize)
{
	while(recvBufSize >= GCL::GW_MSG_HEADER_SIZE)
	{
		GCL::GwMessageHeader header;

		std::memcpy(&header, recvBuf, GCL::GW_MSG_HEADER_SIZE);

		if (header.payloadSize > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
			stc->errCount++;
			recvBufSize = 0;
			return;
		}

		const size_t requestSize = GCL::GW_MSG_HEADER_SIZE +
								   header.payloadSize +
								   GCL::GW_MSG_CRC_SIZE;

		if (recvBufSize < requestSize)
		{
			return;
		}

		if (requestSize > GCL::GW_MAX_PAYLOAD_SIZE)
		{
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
			stc->errCount++;
			recvBufSize = 0;
			return;
		}

		GCL::TuningGwRequestId requestID = static_cast<GCL::TuningGwRequestId>(header.requestID);

		switch(requestID)
		{
		case GCL::TuningGwRequestId::TGW_HANDSHAKE:
		case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START:
		case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT:
		case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES:
		case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_READ:
		case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_WRITE:
		case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_APPLY:
		case GCL::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE:
			break;
		default:
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_INVALID_REQUEST);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		GCL::GwErrorCode errCode = GCL::GwErrorCode::GWC_SUCCESS;

		if (checkPayloadSize(header, recvBuf, recvBufSize, errCode) == false)
		{
			sendErrReply(stc, header, errCode);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		uint32_t calcCrc = Radiy::CRC32(recvBuf, GCL::GW_MSG_HEADER_SIZE + header.payloadSize);

		uint32_t receivedCrc;
		std::memcpy(&receivedCrc, recvBuf + GCL::GW_MSG_HEADER_SIZE + header.payloadSize, sizeof(receivedCrc));

		if (calcCrc != receivedCrc)
		{
			logErr(QString("request %1 error CRC 0x%2 (expected 0x%3)").
				   arg(header.requestID).arg(receivedCrc, 8, 16, QChar('0')).arg(calcCrc, 8, 16, QChar('0')));

			sendErrReply(stc, header, GCL::GwErrorCode::GWC_CRC_ERROR);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		bool result = true;

/*		if (requestID != GCL::AdsGwRequestId::ADSGW_HANDSHAKE &&
			stc->handshakeCompleted == false)
		{
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_HANDSHAKE_REQUIRED);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}


		switch(requestID)
		{
		case GCL::AdsGwRequestId::ADSGW_HANDSHAKE:
			result = processHandshakeRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_START:
			result = processSignalListStartRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT:
			result = processSignalListNextRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START:
			result = processSignalParamStartRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
			result = processSignalParamNextRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE:
			result = processSignalStateRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
			result = processSignalStateChangesRequest(stc, header, recvBuf, requestSize);
			break;

		default:
			Q_ASSERT(false);
		}*/

		if (result == false)
		{
			stc->errCount++;
		}

		recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
	}
}

bool TuningGatewayServer::processHandshakeRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
/*	Q_UNUSED(requestSize);

	GCL::AdsGwHandshakeRequest request;

	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::ADS_GW_HANDSHAKE_REQUEST_SIZE);

	//

	bool nullTerminated = checkNullTerminated(request.clientName, sizeof(request.clientName));

	if (nullTerminated == false)
	{
		logErr("HANDSHAKE request error, clientName is NOT null-terminated");

		sendErrReply(stc, header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	stc->clientName = QString::fromUtf8(request.clientName);

	if (request.protocolVersion != GCL::ADS_GW_PROTOCOL_VERSION)
	{
		logErr(QString("HANDSHAKE request from %1, WRONG protocol version 0x%2 (required %3)").
					arg(stc->clientName).
					arg(request.protocolVersion, 4, 16, QChar('0')).
					arg(GCL::ADS_GW_PROTOCOL_VERSION, 4, 16, QChar('0')));

		sendErrReply(stc, header, GCL::GwErrorCode::GWC_UNSUPPORTED_VERSION);
		return false;
	}

	logMsg(QString("HANDSHAKE request from %1, protocol version 0x%2").
					arg(stc->clientName).
					arg(request.protocolVersion, 4, 16, QChar('0')));

	GCL::AdsGwHandshakeResponse reply;

	reply.protocolVersion = GCL::ADS_GW_PROTOCOL_VERSION;
	reply.reserved = 0;
	reply.maxStateRequest = GCL::ADS_GW_MAX_SIGNAL_STATES;
	reply.sizeof_GwAppSignalParam = GCL::GW_APP_SIGNAL_PARAM_SIZE;
	reply.sizeof_GwAppSignalState = GCL::GW_APP_SIGNAL_STATE_SIZE;

	sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	stc->handshakeCompleted = true;
*/
	return true;
}
/*
bool TuningGatewayServer::processSignalListStartRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	//

	logMsg(QString("SIGNAL_LIST_START request from %1").arg(stc->clientName));

	GCL::AdsGwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(m_appSignals.count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT;
	reply.partCount = signalCount / reply.itemsPerPart + (signalCount % reply.itemsPerPart ? 1 : 0);

	char* payloadData = stc->payloadData.data();
	const size_t payloadSize = sizeof(reply);

	std::memcpy(payloadData, &reply, payloadSize);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool TuningGatewayServer::processSignalListNextRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::AdsGwSignalListNextRequest request;
	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::ADS_GW_SIGNAL_LIST_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_LIST_NEXT request from %1, requested part %2").
						arg(stc->clientName).arg(request.part));

	char* payloadData = stc->payloadData.data();

	const int signalsCount = TO_INT(m_appSignals.count());
	const int itemsPerPart = GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT;
	const int partCount = signalsCount / itemsPerPart + (signalsCount % itemsPerPart ? 1 : 0);

	if (request.part >= static_cast<uint32_t>(partCount))
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	GCL::AdsGwSignalListNextResponse reply;

	reply.part = request.part;
	reply.appSignalIdCount = 0;

	int signalStartIndex = request.part * GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT;
	const int signalEndIndex = std::min(TO_INT(signalStartIndex + GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT), signalsCount);

	size_t payloadSize = GCL::ADS_GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE;

	for(int i = signalStartIndex; i < signalEndIndex; i++)
	{
		if (payloadSize + GCL::GW_APP_SIGNAL_ID_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			Q_ASSERT(false);
			logErr("TuningGatewayServer::processSignalListNextRequest payload size exceed!");
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR);
			return false;
		}

		const AppSignal* appSignal = m_appSignals.getSignalByIndex(i);

		TEST_PTR_CONTINUE(appSignal);

		copyStr(payloadData + payloadSize, GCL::GW_APP_SIGNAL_ID_SIZE, appSignal->appSignalID());

		payloadSize +=  GCL::GW_APP_SIGNAL_ID_SIZE;

		reply.appSignalIdCount++;
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool TuningGatewayServer::processSignalParamStartRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	logMsg(QString("SIGNAL_PARAM_START request from %1").arg(stc->clientName));

	GCL::AdsGwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(m_appSignals.count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = GCL::ADS_GW_MAX_SIGNAL_PARAMS;
	reply.partCount = signalCount / reply.itemsPerPart + (signalCount % reply.itemsPerPart ? 1 : 0);

	char* payloadData = stc->payloadData.data();
	const size_t payloadSize = sizeof(reply);

	std::memcpy(payloadData, &reply, payloadSize);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool TuningGatewayServer::processSignalParamNextRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	GCL::AdsGwSignalParamNextRequest request;
	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::ADS_GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_PARAM_NEXT request from %1, requested part %2").
		   arg(stc->clientName).arg(request.part));

	char* payloadData = stc->payloadData.data();

	const int signalsCount = TO_INT(m_appSignals.count());
	const int itemsPerPart = GCL::ADS_GW_MAX_SIGNAL_PARAMS;
	const int partCount = signalsCount / itemsPerPart + (signalsCount % itemsPerPart ? 1 : 0);

	if (request.part >= static_cast<uint32_t>(partCount))
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	GCL::AdsGwSignalParamNextResponse reply;

	reply.part = request.part;
	reply.paramCount = 0;

	int signalStartIndex = request.part * GCL::ADS_GW_MAX_SIGNAL_PARAMS;
	const int signalEndIndex = std::min(TO_INT(signalStartIndex + GCL::ADS_GW_MAX_SIGNAL_PARAMS), signalsCount);

	size_t payloadSize = GCL::ADS_GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE;

	for(int i = signalStartIndex; i < signalEndIndex; i++)
	{
		if (payloadSize + GCL::GW_APP_SIGNAL_PARAM_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			Q_ASSERT(false);
			logErr("TuningGatewayServer::processSignalParamNextRequest payload size exceed!");
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR);
			return false;
		}

		const AppSignal* appSignal = m_appSignals.getSignalByIndex(i);

		TEST_PTR_CONTINUE(appSignal);

		GCL::GwAppSignalParam p{};

		p.hash = TO_UINT64(appSignal->hash());
		copyStr(p.appSignalId, GCL::STRING_LENGTH_128, appSignal->appSignalID());
		copyStr(p.customSignalId, GCL::STRING_LENGTH_128, appSignal->customAppSignalID());
		copyStr(p.caption, GCL::STRING_LENGTH_256, appSignal->caption());
		copyStr(p.equipmentId, GCL::STRING_LENGTH_128, appSignal->equipmentID());
		copyStr(p.lmEquipmentId, GCL::STRING_LENGTH_128, appSignal->lmEquipmentID());
		copyStr(p.units, GCL::STRING_LENGTH_128, appSignal->unit());
		copyStr(p.tags, GCL::STRING_LENGTH_256, appSignal->tagsStr());
		p.channel = static_cast<GCL::Channel>(appSignal->channel());
		p.inOutType = static_cast<GCL::InOutType>(appSignal->inOutType());
		
		switch (appSignal->signalType())
		{
		case E::SignalType::Discrete:
			p.type = GCL::SignalType::Discrete;
			break;
		case E::SignalType::Analog:
			switch (appSignal->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				p.type = GCL::SignalType::SignedInt32;
				break;
			case E::AnalogAppSignalFormat::Float32:
				p.type = GCL::SignalType::Float32;
				break;
			default:
				Q_ASSERT(false);
				p.type = GCL::SignalType::Discrete;
			}
			break; 
		default:
			Q_ASSERT(false);
			p.type = GCL::SignalType::Discrete;
		}

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

		std::memcpy(payloadData + payloadSize, &p, GCL::GW_APP_SIGNAL_PARAM_SIZE);

		payloadSize +=  GCL::GW_APP_SIGNAL_PARAM_SIZE;

		reply.paramCount++;
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	return true;
}

bool TuningGatewayServer::processSignalStateRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	GCL::AdsGwSignalStateRequest request;
	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE);

	// logMsg(QString("SIGNAL_STATE request from %1, %2 states requested").
	// 	   arg(stc->clientName).arg(request.signalCount));

	if (request.signalCount > GCL::ADS_GW_MAX_SIGNAL_STATES)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS);
		return false;
	}

	if (stc->connectedToTuningSrv.load(std::memory_order_relaxed) == false)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_NO_ADS_CONNECTION);
		return true;		// this is not request format error!
	}

	char* payloadData = stc->payloadData.data();

	GCL::AdsGwSignalStateResponse reply;

	reply.stateCount = 0;

	const uint32_t hashesCount = request.signalCount;
	size_t hashesOffset = GCL::GW_MSG_HEADER_SIZE + GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE;

	size_t payloadSize = GCL::ADS_GW_SIGNAL_STATE_RESPONSE_SIZE;

	{
		std::lock_guard lg(m_signalStatesMutex);

		for(uint32_t i = 0; i < hashesCount; i++)
		{
			if (payloadSize + GCL::GW_APP_SIGNAL_STATE_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
			{
				Q_ASSERT(false);
				logErr("TuningGatewayServer::processSignalStateRequest payload size exceed!");
				sendErrReply(stc, header, GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR);
				return false;
			}

			Hash hash;

			std::memcpy(&hash, recvBuf + hashesOffset, sizeof(hash));
			hashesOffset += GCL::GW_APP_SIGNAL_HASH_SIZE;

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

			GCL::GwAppSignalState state;

			state.hash = st.hash;
			state.systemTime = st.systemTime();
			state.localTime = st.localTime();
			state.plantTime = st.plantTime();
			state.value = st.value;
			state.flags = st.flags.all;
			state.reserved = 0;

			std::memcpy(payloadData + payloadSize, &state, GCL::GW_APP_SIGNAL_STATE_SIZE);

			payloadSize +=  GCL::GW_APP_SIGNAL_STATE_SIZE;

			reply.stateCount++;
		}
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_STATE_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	// logMsg(QString("Sent %1 signal states").arg(reply.stateCount));

	return true;
}

bool TuningGatewayServer::processSignalStateChangesRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

//	logMsg(QString("SIGNAL_STATE_CHANGES request from %1").arg(stc->clientName));

	GCL::AdsGwSignalStateChangesRequest request;
	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::ADS_GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE);

	if (stc->connectedToTuningSrv.load(std::memory_order_relaxed) == false)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_NO_ADS_CONNECTION);
		return true;		// this is not request format error!
	}

	//

	char* payloadData = stc->payloadData.data();

	GCL::AdsGwSignalStateChangesResponse reply;

	reply.stateCount = 0;
	reply.pendingStatesCount = 0;

	size_t payloadSize = GCL::ADS_GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE;

	{
		std::lock_guard lg(m_signalStateChangesMutex);

		GCL::GwAppSignalState state;

		while(m_signalStateChanges.empty() == false &&
			   reply.stateCount < GCL::ADS_GW_MAX_SIGNAL_STATE_CHANGES)
		{
			if (payloadSize + GCL::GW_APP_SIGNAL_STATE_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
			{
				break;
			}

			state = m_signalStateChanges.front();
			m_signalStateChanges.pop_front();

			std::memcpy(payloadData + payloadSize, &state, GCL::GW_APP_SIGNAL_STATE_SIZE);

			payloadSize +=  GCL::GW_APP_SIGNAL_STATE_SIZE;

			reply.stateCount++;
		}

		reply.pendingStatesCount = TO_UINT32(m_signalStateChanges.size());
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE);

	sendOkReply(stc, header, payloadData, payloadSize);

	// logMsg(QString("Sent %1 signal state changes, pending states %2").
	// 					arg(reply.stateCount).arg(reply.pendingStatesCount));
	return true;
}
*/
bool TuningGatewayServer::checkPayloadSize(const GatewayClientLib::GwMessageHeader& header,
	const char* recvBuf, const size_t recvBufSize, GatewayClientLib::GwErrorCode& errCode)
{
	errCode = GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR;

	TEST_PTR_RETURN_FALSE(recvBuf);

	size_t payloadSize = 0;

	switch(static_cast<GCL::TuningGwRequestId>(header.requestID))
	{
	case GCL::TuningGwRequestId::TGW_HANDSHAKE:
		payloadSize = GCL::TUNING_GW_HANDSHAKE_REQUEST_SIZE;
		break;

	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START:
		payloadSize = GCL::TUNING_GW_GET_TUNING_SOURCES_START_REQUEST_SIZE;
		break;

	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT:
		payloadSize = GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_REQUEST_SIZE;
		break;

	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES:
		payloadSize = GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_REQUEST_SIZE;
		break;

	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_READ:
		{
			if (recvBufSize < GCL::GW_MSG_HEADER_SIZE + GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE)
			{
				return false;
			}

			GCL::GwTuningSignalsReadRequest request;

			std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE);

			if (request.count > GCL::TUNING_GW_MAX_SIGNAL_STATES)
			{
				errCode = GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS;
				return false;
			}

			payloadSize = GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE +
						  static_cast<size_t>(request.count) * GCL::GW_APP_SIGNAL_HASH_SIZE;
		}
		break;

	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_WRITE:
		{
			if (recvBufSize < GCL::GW_MSG_HEADER_SIZE + GCL::TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE)
			{
				return false;
			}

			GCL::GwTuningSignalsWriteRequest request;

			std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE);

			if (request.count > GCL::TUNING_GW_MAX_WRITE_VALUES)
			{
				errCode = GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS;
				return false;
			}

			payloadSize = GCL::TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE +
						  static_cast<size_t>(request.count) * GCL::TUNING_GW_TUNING_WRITE_VALUE_SIZE;
		}
		break;

	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_APPLY:
		payloadSize = GCL::TUNING_GW_TUNING_SIGNALS_APPLY_REQUEST_SIZE;
		break;

	case GCL::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE:
		payloadSize = GCL::TUNING_GW_CHANGE_CONTROLLED_TUNING_SOURCE_REQUEST_SIZE;
		break;

	default:
		Q_ASSERT(false);
		return false;
	}

	if (header.payloadSize != payloadSize)
	{
		return false;
	}

	errCode = GCL::GwErrorCode::GWC_SUCCESS;
	return true;
}

size_t TuningGatewayServer::skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize)
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

void TuningGatewayServer::sendErrReply(TgsSessionShared stc,
	const GatewayClientLib::GwMessageHeader& requestHeader,
	GCL::GwErrorCode errCode)
{
	sendReply(stc, requestHeader.requestID, errCode, nullptr, 0);
}

void TuningGatewayServer::sendOkReply(TgsSessionShared stc,
	const GatewayClientLib::GwMessageHeader& requestHeader,
	const char* payloadData, size_t payloadSize)
{
	sendReply(stc, requestHeader.requestID, GCL::GwErrorCode::GWC_SUCCESS, payloadData, payloadSize);
}

void TuningGatewayServer::sendReply(TgsSessionShared stc,
	uint32_t requestID, GCL::GwErrorCode errCode,
	const char* payloadData, size_t payloadSize)
{
	TEST_PTR_RETURN(stc);

	thread_local std::vector<char> sendBuf(GCL::GW_MAX_PAYLOAD_SIZE);

	if (GCL::GW_MSG_HEADER_SIZE + payloadSize + GCL::GW_MSG_CRC_SIZE > GCL::GW_MAX_PAYLOAD_SIZE)
	{
		Q_ASSERT(false);
		payloadData = nullptr;
		payloadSize = 0;
		errCode = GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR;
	}

	if (errCode != GCL::GwErrorCode::GWC_SUCCESS)
	{
		Q_ASSERT(payloadData == nullptr);
		Q_ASSERT(payloadSize == 0);
		payloadSize = 0;
	}

	GCL::GwMessageHeader header;

	header.requestID = requestID;
	header.payloadSize = static_cast<uint32_t>(payloadSize);
	header.statusCode = static_cast<uint32_t>(errCode);

	std::memcpy(sendBuf.data(), &header, GCL::GW_MSG_HEADER_SIZE);

	if (payloadSize > 0)
	{
		std::memcpy(sendBuf.data() + GCL::GW_MSG_HEADER_SIZE, payloadData, payloadSize);
	}

	uint32_t crc = Radiy::CRC32(sendBuf.data(), GCL::GW_MSG_HEADER_SIZE + payloadSize);

	std::memcpy(sendBuf.data() + GCL::GW_MSG_HEADER_SIZE + payloadSize, &crc, GCL::GW_MSG_CRC_SIZE);

	if (stc->closing.load(std::memory_order_relaxed))
	{
		return;
	}

	asio::error_code ec;

	asio::write(*stc->socket, asio::buffer(sendBuf.data(), GCL::GW_MSG_HEADER_SIZE + payloadSize + GCL::GW_MSG_CRC_SIZE), ec);

	if (ec)
	{
		logErr(QString("send error: %1").arg(QString::fromStdString(ec.message())));
		requestCloseSession(stc);
	}
}

bool TuningGatewayServer::checkNullTerminated(const char* str, size_t size) const
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

QString TuningGatewayServer::getIpPortStr(const tcp::socket& socket) const
{
	asio::error_code ec;
	tcp::endpoint remote = socket.remote_endpoint(ec);

	if (ec)
	{
		return "unknown";
	}

	return QString("%1:%2").arg(QString::fromStdString(remote.address().to_string())).arg(remote.port());
}

void TuningGatewayServer::copyStr(char* toStr, size_t toStrLen, const QString& fromStr) const
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

uint8_t TuningGatewayServer::channelChar(E::Channel ch) const
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
