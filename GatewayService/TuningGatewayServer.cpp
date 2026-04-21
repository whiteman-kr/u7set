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

		while (m_running.load(std::memory_order_relaxed)  &&  !isQuitRequested(stc))
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
	stc->tunSrvClientThread->start();
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

		if (requestID != GCL::TuningGwRequestId::TGW_HANDSHAKE &&
			stc->handshakeCompleted == false)
		{
			sendErrReply(stc, header, GCL::GwErrorCode::GWC_HANDSHAKE_REQUIRED);
			stc->errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		switch(requestID)
		{
		case GCL::TuningGwRequestId::TGW_HANDSHAKE:
			result = processHandshakeRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START:
			result = processGetTuningSourcesStartRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT:
			result = processGetTuningSourcesNextRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES:
			result = processGetTuningSourceStatesRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_READ:
			result = processTuningSignalsReadRequest(stc, header, recvBuf, requestSize);
			break;

		case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_WRITE:
		case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_APPLY:
		case GCL::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE:
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

bool TuningGatewayServer::processHandshakeRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::TuningGwHandshakeRequest request;

	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_HANDSHAKE_REQUEST_SIZE);

	//

	bool nullTerminated = checkNullTerminated(request.clientName, sizeof(request.clientName));

	if (nullTerminated == false)
	{
		logErr("HANDSHAKE request error, clientName is NOT null-terminated");

		sendErrReply(stc, header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	stc->clientName = QString::fromUtf8(request.clientName);

	if (request.protocolVersion != GCL::TUNING_GW_PROTOCOL_VERSION)
	{
		logErr(QString("HANDSHAKE request from %1, WRONG protocol version 0x%2 (required %3)").
					arg(stc->clientName).
					arg(request.protocolVersion, 4, 16, QChar('0')).
					arg(GCL::TUNING_GW_PROTOCOL_VERSION, 4, 16, QChar('0')));

		sendErrReply(stc, header, GCL::GwErrorCode::GWC_UNSUPPORTED_VERSION);
		return false;
	}

	logMsg(QString("HANDSHAKE request from %1, protocol version 0x%2").
					arg(stc->clientName).
					arg(request.protocolVersion, 4, 16, QChar('0')));

	GCL::TuningGwHandshakeResponse reply;

	reply.protocolVersion = GCL::TUNING_GW_PROTOCOL_VERSION;
	reply.reserved = 0;

	reply.maxStateRequest = GCL::TUNING_GW_MAX_SIGNAL_STATES;
	reply.maxStateWrite = GCL::TUNING_GW_MAX_WRITE_VALUES;

	reply.sizeof_GwTuningSourceState = GCL::TUNING_GW_TUNING_SOURCE_STATE_SIZE;
	reply.sizeof_GwTuningSignalState = GCL::TUNING_GW_TUNING_SIGNAL_STATE_SIZE;

	sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	stc->handshakeCompleted = true;

	return true;
}

bool TuningGatewayServer::processGetTuningSourcesStartRequest(TgsSessionShared stc,
															  const GCL::GwMessageHeader& header,
															  const char* recvBuf,
															  const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::GwGetTuningSourcesStartRequest request;

	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_GET_TUNING_SOURCES_START_REQUEST_SIZE);

	quint64 fileSize = 0;
	quint64 maxPartSize = 0;
	quint64 partCount = 0;

	if (stc->tunSrvClientThread == nullptr)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	if	(stc->tunSrvClientThread->getTuningSourcesFileMetrics(fileSize, maxPartSize, partCount) == false)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_TUNING_SOURCES_FILE_NOT_READY);
		return false;
	}

	GCL::GwGetTuningSourcesStartResponse reply;

	reply.totalSize = TO_UINT32(fileSize);
	reply.maxPartSize = TO_UINT32(maxPartSize);
	reply.partCount = TO_UINT32(partCount);

	sendOkReply(stc, header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	return true;
}

bool TuningGatewayServer::processGetTuningSourcesNextRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf,
	const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::GwGetTuningSourcesNextRequest request;

	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_REQUEST_SIZE);

	if (stc->tunSrvClientThread == nullptr)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	thread_local std::vector<char> payload;

	if (payload.capacity() < GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE +
							TDS_TUNING_SOURCES_FILE_PART_SIZE)
	{
		payload.reserve(GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE +
						TDS_TUNING_SOURCES_FILE_PART_SIZE);
	}

	// reserve size for GCL::GwGetTuningSourcesNextResponse struct
	//
	payload.resize(GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE);

	quint64 partNo = request.part;
	quint64 partSize = 0;

	if (stc->tunSrvClientThread->getTuningSourcesFilePart(partNo, payload, partSize) == false)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_TUNING_SOURCES_FILE_NOT_READY);
		return false;
	}

	GCL::GwGetTuningSourcesNextResponse reply;

	reply.part = TO_UINT32(partNo);
	reply.partSize = TO_UINT32(partSize);

	std::memcpy(payload.data(), &reply, GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE);

	sendOkReply(stc, header, payload.data(), payload.size());

	return true;
}

bool TuningGatewayServer::processGetTuningSourceStatesRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf,
	const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::GwGetTuningSourceStatesRequest request;

	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_REQUEST_SIZE);

	if (stc->tunSrvClientThread == nullptr)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	thread_local std::vector<char> replyData;

	replyData.clear();

	stc->tunSrvClientThread->getTuningSourceStatesReply(replyData);

	thread_local Network::GetTuningSourcesStatesReply tsStates;

	tsStates.Clear();

	bool result = false;

	if (replyData.size() > 0)
	{
		result = tsStates.ParseFromArray(replyData.data(), TO_INT(replyData.size()));
	}

	GCL::GwGetTuningSourceStatesResponse reply;

	reply.count = 0;
	reply.clientIsActive = 0;

	if (result == false)
	{
		sendOkReply(stc, header, reinterpret_cast<const char*>(&reply),
							GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_RESPONSE_SIZE);
		return true;
	}

	reply.clientIsActive = QString::fromStdString(tsStates.activeclientid()) == m_swInfo.equipmentID() ? 1 : 0;

	if (tsStates.tuningsourcesstate_size() == 0)
	{
		sendOkReply(stc, header, reinterpret_cast<const char*>(&reply),
					GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_RESPONSE_SIZE);
		return true;
	}

	reply.count = TO_UINT32(tsStates.tuningsourcesstate_size());

	thread_local std::vector<char> payload;

	payload.clear();

	payload.resize(GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_RESPONSE_SIZE);

	std::memcpy(payload.data(), &reply, GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_RESPONSE_SIZE);

	for(const Network::TuningSourceState& tss : tsStates.tuningsourcesstate())
	{
		GCL::GwTuningSourceState st;

		st.sourceId = tss.sourceid();
		copyStr(st.moduleEquipmentId, GCL::STRING_LENGTH_128, tss.moduleequipmentid());
		copyStr(st.lanEquipmentId, GCL::STRING_LENGTH_128, tss.lanequipmentid());

		st.isReplying = (tss.isreply() ? 1 : 0);
		st.controlIsActive = (tss.controlisactive() ? 1 : 0);
		st.setSOR = (tss.setsor() ? 1 : 0);
		st.writingDisabled = (tss.writingdisabled() ? 1 : 0);
		st.buildMismatch = (tss.buildmismatch() ? 1 : 0);
		st.hasUnappliedParams = (tss.hasunappliedparams() ? 1 : 0);
		st.reservedFlags[0] = 0;
		st.reservedFlags[1] = 0;
		st.lmTime = tss.lmtime();

		const char* stPtr = reinterpret_cast<const char*>(&st);
		payload.insert(payload.end(), stPtr, stPtr+ sizeof(st));
	}

	sendOkReply(stc, header, payload.data(), payload.size());

	return true;
}

bool TuningGatewayServer::processTuningSignalsReadRequest(TgsSessionShared stc,
	const GCL::GwMessageHeader& header,
	const char* recvBuf,
	const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::GwTuningSignalsReadRequest request;

	std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE);

	if (stc->tunSrvClientThread == nullptr)
	{
		sendErrReply(stc, header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	quint32 hashesCount = request.count;

	const char* hashPtr = recvBuf + GCL::GW_MSG_HEADER_SIZE +
						  GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE;

	thread_local std::vector<Hash> hashes;
	thread_local quint64 readRequestID = 0;

	readRequestID++;

	hashes.clear();

	for(quint32 i = 0; i < hashesCount; i++)
	{
		Hash h;

		std::memcpy(&h, hashPtr, sizeof(Hash));
		hashPtr += sizeof(Hash);

		hashes.push_back(h);
	}

	stc->replyData.clear();

	stc->tunSrvClientThread->tuningSignalsRead(readRequestID, hashes,
											   &stc->condVarMutex, &stc->condVar, &stc->replyData);

	QElapsedTimer et;
	et.start();

	WaitResult wr = waitForOrQuit(stc, 500);

	qint64 time = et.elapsed();

	if (wr == WaitResult::QuitRequested)
	{
		return true;
	}

	GCL::GwTuningSignalsReadResponse reply;

	reply.count = 0;
	reply.reserved = 0;

	thread_local std::vector<char> payload;

	payload.resize(sizeof(reply));

	std::memcpy(payload.data(), &reply, sizeof(reply));

	if (wr == WaitResult::Timeout || stc->replyData.size() == 0)
	{
		sendOkReply(stc, header, payload.data(), payload.size());
		return true;
	}

	thread_local Network::TuningSignalsReadReply prp;

	bool res = prp.ParseFromArray(stc->replyData.data(), TO_INT(stc->replyData.size()));

	if (res == false || prp.error() != 0)
	{
		sendOkReply(stc, header, payload.data(), payload.size());
		return true;
	}

	reply.count = prp.tuningsignalstate_size();

	std::memcpy(payload.data(), &reply, sizeof(reply));

	GCL::GwTuningSignalState st;
	TuningValue tv;

	for(const Network::TuningSignalState& tst : prp.tuningsignalstate())
	{
		std::memset(&st, 0, sizeof(st));

		st.hash = tst.signalhash();
		st.errorCode = tst.error();

		uint32_t flags = 0;

		flags |= tst.valid() ? GCL::TGWF_VALID : 0;
		flags |= tst.writeinprogress() ? GCL::TGWF_WRITE_IN_PROGRESS : 0;
		flags |= tst.writingdisabled() ? 0 : GCL::TGWF_WRITING_IS_ENABLED;
		flags |= tst.tuningdefault() ? GCL::TGWF_TUNING_DEFAULT : 0;

		st.flags = flags;

		tv.load(tst.value());

		st.value = tv.toDouble();
		st.successfulReadTime = tst.successfulreadtime();
		st.writeRequestTime = tst.writerequesttime();
		st.successfulWriteTime = tst.successfulwritetime();
		st.unsuccessfulWriteTime = tst.unsuccessfulwritetime();
		st.lmTime = tst.lmtime();
		st.fotipProcessingNumerator = tst.fotipprocessingnumerator();

		const char* stPtr = reinterpret_cast<const char*>(&st);
		payload.insert(payload.end(), stPtr, stPtr + sizeof(st));
	}

	sendOkReply(stc, header, payload.data(), payload.size());

	return true;
}

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

	if (isQuitRequested(stc))
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

void TuningGatewayServer::copyStr(char* toStr, size_t toStrLen, const std::string& fromStr) const
{
	TEST_PTR_RETURN(toStr);

	size_t fromLen = fromStr.size();

	if (fromLen > toStrLen - 1)
	{
		std::memset(toStr, 0, toStrLen);
	}
	else
	{
		std::memcpy(toStr, fromStr.data(), fromLen);
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

TuningGatewayServer::WaitResult TuningGatewayServer::waitForOrQuit(TgsSessionShared stc,
																const int64_t timeoutMs)
{
	TEST_PTR_RETURN_VALUE(stc, WaitResult::QuitRequested);

	std::chrono::milliseconds timeout(timeoutMs);

	std::unique_lock<std::mutex> ul(stc->condVarMutex);

	bool conditionMet = stc->condVar.wait_for(ul, timeout, [this, stc]()
								  {
									  return isQuitRequested(stc) || !stc->replyData.empty();
								  });

	if (!conditionMet)
	{
		return WaitResult::Timeout;
	}

	if (isQuitRequested(stc))
	{
		return WaitResult::QuitRequested;
	}

	return WaitResult::DataReady;
}

bool TuningGatewayServer::isQuitRequested(TgsSessionShared stc) const
{
	return stc->closing.load(std::memory_order_relaxed);
}

