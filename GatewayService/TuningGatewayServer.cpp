#include "TuningGatewayServer.h"

// -----------------------------------------------------------------------------
// 
// TuningGatewaySession class implementation
// 
// -----------------------------------------------------------------------------

TuningGatewaySession::TuningGatewaySession(const SoftwareInfo& swInfo,
										   const AppSignals& appSignals,
										   const std::vector<HostAddressPort>& serviceAddresses, 
										   asio::ip::tcp::socket socket,
										   CircularLoggerShared log) :
	AsyncTcpSession(swInfo, appSignals, serviceAddresses, std::move(socket), log)
{
}

void TuningGatewaySession::setConnectedToTuningSrv(bool connected) 
{
	m_connectedToTuningSrv = connected;
}

void TuningGatewaySession::setReplyData(const char* data, size_t size)
{
	{
		std::lock_guard<std::mutex> lock(m_condVarMutex);
		m_replyData.assign(data, data + size);
	}

	m_condVar.notify_one();
}

void TuningGatewaySession::onStarted()
{
	startTuningSrvClient();
}

void TuningGatewaySession::onStopped()
{
	stopTuningSrvClient();
}

bool TuningGatewaySession::checkRequestID(uint32_t requestID)
{
	switch (static_cast<GCL::TuningGwRequestId>(requestID))
	{
	case GCL::TuningGwRequestId::TGW_HANDSHAKE:
	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START:
	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT:
	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES:
	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_READ:
	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_WRITE:
	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_APPLY:
	case GCL::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE:
		return true;

	default:
		;
	}

	return false;
}

bool TuningGatewaySession::isHandshakeRequest(uint32_t requestID)
{
	return static_cast<GCL::TuningGwRequestId>(requestID) == GCL::TuningGwRequestId::TGW_HANDSHAKE;
}

bool TuningGatewaySession::checkPayloadSize(const GCL::GwMessageHeader& header,
											const char* recvBuf,
											const size_t recvBufSize,
											GCL::GwErrorCode& errCode)
{
	errCode = GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR;

	TEST_PTR_RETURN_FALSE(recvBuf);

	size_t payloadSize = 0;

	switch (static_cast<GCL::TuningGwRequestId>(header.requestID))
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

			payloadSize =
				GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE + static_cast<size_t>(request.count) * GCL::GW_APP_SIGNAL_HASH_SIZE;
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

bool TuningGatewaySession::processRequest(const GCL::GwMessageHeader& header, char* recvBuf, size_t requestSize)
{
	GCL::TuningGwRequestId requestID = static_cast<GCL::TuningGwRequestId>(header.requestID);

	bool result = false;

	switch (requestID)
	{
	case GCL::TuningGwRequestId::TGW_HANDSHAKE:
		result = processHandshakeRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START:
		result = processGetTuningSourcesStartRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT:
		result = processGetTuningSourcesNextRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES:
		result = processGetTuningSourceStatesRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_READ:
		result = processTuningSignalsReadRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_WRITE:
		result = processTuningSignalsWriteRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_TUNING_SIGNALS_APPLY:
		result = processTuningSignalsApplyRequest(header, recvBuf, requestSize);
		break;

	case GCL::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE:
		result = processChangeControlledSourceRequest(header, recvBuf, requestSize);
		break;

	default:
		Q_ASSERT(false);
	}

	if (result == false)
	{
		incErrCount();
	}

	return result;
}

bool TuningGatewaySession::processHandshakeRequest(const GCL::GwMessageHeader& header,
												   const char* recvBuf,
												   const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::TuningGwHandshakeRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_HANDSHAKE_REQUEST_SIZE);

	//

	bool nullTerminated = checkNullTerminated(request.clientName, sizeof(request.clientName));

	if (nullTerminated == false)
	{
		logErr("HANDSHAKE request error, clientName is NOT null-terminated");

		sendErrReply(header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	setClientName(QString::fromUtf8(request.clientName));

	if (request.protocolVersion != GCL::TUNING_GW_PROTOCOL_VERSION)
	{
		logErr(QString("HANDSHAKE request from %1, WRONG protocol version 0x%2 (required %3)")
				   .arg(clientName())
				   .arg(request.protocolVersion, 4, 16, QChar('0'))
				   .arg(GCL::TUNING_GW_PROTOCOL_VERSION, 4, 16, QChar('0')));

		sendErrReply(header, GCL::GwErrorCode::GWC_UNSUPPORTED_VERSION);
		return false;
	}

	logMsg(QString("HANDSHAKE request from %1, protocol version 0x%2").arg(clientName()).arg(request.protocolVersion, 4, 16, QChar('0')));

	GCL::TuningGwHandshakeResponse reply;

	reply.protocolVersion = GCL::TUNING_GW_PROTOCOL_VERSION;
	reply.reserved = 0;

	reply.maxStateRequest = GCL::TUNING_GW_MAX_SIGNAL_STATES;
	reply.maxStateWrite = GCL::TUNING_GW_MAX_WRITE_VALUES;

	reply.sizeof_GwTuningSourceState = GCL::TUNING_GW_TUNING_SOURCE_STATE_SIZE;
	reply.sizeof_GwTuningSignalState = GCL::TUNING_GW_TUNING_SIGNAL_STATE_SIZE;

	sendOkReply(header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	setHandshakeCompleted(true);

	return true;
}

bool TuningGatewaySession::processGetTuningSourcesStartRequest(const GCL::GwMessageHeader& header,
															   const char* recvBuf,
															   const size_t requestSize)
{
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	quint64 fileSize = 0;
	quint64 maxPartSize = 0;
	quint64 partCount = 0;

	if (m_tunSrvClientThread->getTuningSourcesFileMetrics(fileSize, maxPartSize, partCount) == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_TUNING_SOURCES_FILE_NOT_READY);
		return false;
	}

	GCL::GwGetTuningSourcesStartRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_GET_TUNING_SOURCES_START_REQUEST_SIZE);

	GCL::GwGetTuningSourcesStartResponse reply;

	reply.totalSize = TO_UINT32(fileSize);
	reply.maxPartSize = TO_UINT32(maxPartSize);
	reply.partCount = TO_UINT32(partCount);

	sendOkReply(header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	return true;
}

bool TuningGatewaySession::processGetTuningSourcesNextRequest(const GCL::GwMessageHeader& header,
															  const char* recvBuf,
															  const size_t requestSize)
{
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	GCL::GwGetTuningSourcesNextRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_REQUEST_SIZE);

	m_payload.clear();

	if (m_payload.capacity() < GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE + TDS_TUNING_SOURCES_FILE_PART_SIZE)
	{
		m_payload.reserve(GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE + TDS_TUNING_SOURCES_FILE_PART_SIZE);
	}

	// reserve size for GCL::GwGetTuningSourcesNextResponse struct
	//
	m_payload.resize(GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE);

	quint64 partNo = request.part;
	quint64 partSize = 0;

	if (m_tunSrvClientThread->getTuningSourcesFilePart(partNo, m_payload, partSize) == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_TUNING_SOURCES_FILE_NOT_READY);
		return false;
	}

	GCL::GwGetTuningSourcesNextResponse reply;

	reply.part = TO_UINT32(partNo);
	reply.partSize = TO_UINT32(partSize);

	std::memcpy(m_payload.data(), &reply, GCL::TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE);

	sendOkReply(header, m_payload.data(), m_payload.size());

	return true;
}

bool TuningGatewaySession::processGetTuningSourceStatesRequest(const GCL::GwMessageHeader& header,
															   const char* recvBuf,
															   const size_t requestSize)
{
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	GCL::GwGetTuningSourceStatesRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_REQUEST_SIZE);

	clearReplyData();

	m_tunSrvClientThread->getTuningSourcesState();

	WaitResult wr = waitForOrQuit(500);

	if (wr == WaitResult::QuitRequested)
	{
		return true;
	}

	Network::GetTuningSourcesStatesReply tsStates;

	bool res = false;
	
	{
		std::lock_guard lg(m_condVarMutex);
		res = tsStates.ParseFromArray(m_replyData.data(), TO_INT(m_replyData.size()));
	}

	if (res == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_PARSE_REQUEST_ERROR);
		return true;
	}

	GCL::GwGetTuningSourceStatesResponse reply;

	reply.count = 0;
	reply.clientIsActive = QString::fromStdString(tsStates.activeclientid()) == swInfo().equipmentID() ? 1 : 0;

	if (tsStates.tuningsourcesstate_size() == 0)
	{
		sendOkReply(header, reinterpret_cast<const char*>(&reply), GCL::TUNING_GW_GET_TUNING_SOURCE_STATES_RESPONSE_SIZE);
		return true;
	}

	reply.count = TO_UINT32(tsStates.tuningsourcesstate_size());

	m_payload.clear();

	const char* replyPtr = toConstCharPtr(&reply);

	m_payload.assign(replyPtr, replyPtr + sizeof(reply));

	for (const Network::TuningSourceState& tss : tsStates.tuningsourcesstate())
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
		m_payload.insert(m_payload.end(), stPtr, stPtr + sizeof(st));
	}

	sendOkReply(header, m_payload.data(), m_payload.size());

	return true;
}

bool TuningGatewaySession::processTuningSignalsReadRequest(const GCL::GwMessageHeader& header,
														   const char* recvBuf,
														   const size_t requestSize)
{
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	GCL::GwTuningSignalsReadRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE);

	quint32 hashesCount = request.count;

	if (hashesCount > GCL::TUNING_GW_MAX_SIGNAL_STATES)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS);
		return false;
	}

	const char* hashPtr = recvBuf + GCL::TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE;

	m_readRequestID++;

	std::vector<Hash> hashes;

	hashes.reserve(hashesCount);

	for (quint32 i = 0; i < hashesCount; i++)
	{
		Hash h;

		std::memcpy(&h, hashPtr, sizeof(Hash));
		hashPtr += sizeof(Hash);

		hashes.push_back(h);
	}

	clearReplyData();

	m_tunSrvClientThread->tuningSignalsRead(m_readRequestID, hashes);

	WaitResult wr = waitForOrQuit(2000);

	if (wr == WaitResult::QuitRequested)
	{
		return true;
	}

	GCL::GwTuningSignalsReadResponse reply;

	reply.count = 0;
	reply.reserved = 0;

	m_payload.clear();

	const char* replyPtr = toConstCharPtr(&reply);

	m_payload.assign(replyPtr, replyPtr + sizeof(reply));

	if (wr == WaitResult::Timeout || m_replyData.size() == 0)
	{
		sendOkReply(header, m_payload.data(), m_payload.size());
		return true;
	}

	Network::TuningSignalsReadReply prp;
	bool res = false;
	
	{
		std::lock_guard lg(m_condVarMutex);
		res = prp.ParseFromArray(m_replyData.data(), TO_INT(m_replyData.size()));
	}

	if (res == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_PARSE_REQUEST_ERROR);
		return true;
	}

	if (prp.error() != 0)
	{
		sendErrReply(header, static_cast<GCL::GwErrorCode>(prp.error()));
		return true;
	}

	reply.count = prp.tuningsignalstate_size();

	std::memcpy(m_payload.data(), &reply, sizeof(reply));

	GCL::GwTuningSignalState st;
	TuningValue tv;

	for (const Network::TuningSignalState& tst : prp.tuningsignalstate())
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
		m_payload.insert(m_payload.end(), stPtr, stPtr + sizeof(st));
	}

	sendOkReply(header, m_payload.data(), m_payload.size());

	return true;
}

bool TuningGatewaySession::processTuningSignalsWriteRequest(const GCL::GwMessageHeader& header,
															const char* recvBuf,
															const size_t requestSize)
{
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	GCL::GwTuningSignalsWriteRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE);

	quint32 valuesCount = request.count;

	if (valuesCount > GCL::TUNING_GW_MAX_WRITE_VALUES)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS);
		return false;
	}

	QString user = QString::fromUtf8(request.user);

	bool apply = request.apply;

	const char* valuePtr = recvBuf + GCL::TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE;

	m_writeRequestID++;

	std::vector<Hash> hashes;
	std::vector<double> values;

	hashes.reserve(valuesCount);
	values.reserve(valuesCount);

	for (quint32 i = 0; i < valuesCount; i++)
	{
		const GCL::GwTuningWriteValue* valPtr =
			reinterpret_cast<const GCL::GwTuningWriteValue*>(valuePtr + i * sizeof(GCL::GwTuningWriteValue));

		hashes.push_back(valPtr->hash);
		values.push_back(valPtr->value);
	}

	clearReplyData();

	m_tunSrvClientThread->tuningSignalsWrite(m_writeRequestID, user.toStdString(), apply, hashes, values);

	WaitResult wr = waitForOrQuit(5000);

	if (wr == WaitResult::QuitRequested)
	{
		return true;
	}

	GCL::GwTuningSignalsWriteResponse reply;

	reply.count = 0;
	reply.reserved = 0;

	m_payload.clear();

	const char* replyPtr = reinterpret_cast<const char*>(&reply);

	m_payload.assign(replyPtr, replyPtr + sizeof(reply));

	if (wr == WaitResult::Timeout || m_replyData.size() == 0)
	{
		sendOkReply(header, m_payload.data(), m_payload.size());
		return true;
	}

	Network::TuningSignalsWriteReply prp;
	bool res = false;

	{
		std::lock_guard lg(m_condVarMutex);
		res = prp.ParseFromArray(m_replyData.data(), TO_INT(m_replyData.size()));
	}

	if (res == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_PARSE_REQUEST_ERROR);
		return true;
	}

	if (prp.error() != 0)
	{
		sendErrReply(header, static_cast<GCL::GwErrorCode>(prp.error()));
		return true;
	}

	reply.count = prp.writeresult_size();

	std::memcpy(m_payload.data(), &reply, sizeof(reply));

	GCL::GwTuningSignalWriteResult writeResult;

	writeResult.reserved = 0;

	const char* writeResultPtr = reinterpret_cast<const char*>(&writeResult);

	for (const Network::TuningSignalWriteResult& twr : prp.writeresult())
	{
		writeResult.hash = twr.signalhash();
		writeResult.status = twr.error();
		m_payload.insert(m_payload.end(), writeResultPtr, writeResultPtr + sizeof(writeResult));
	}

	sendOkReply(header, m_payload.data(), m_payload.size());

	return true;
}

bool TuningGatewaySession::processTuningSignalsApplyRequest(const GCL::GwMessageHeader& header,
															const char* recvBuf,
															const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	clearReplyData();

	m_tunSrvClientThread->tuningSignalsApply();

	WaitResult wr = waitForOrQuit(500);

	if (wr == WaitResult::QuitRequested)
	{
		return true;
	}

	GCL::GwTuningSignalsApplyResponse reply;

	reply.reserved = 0;

	m_payload.clear();

	const char* replyPtr = reinterpret_cast<const char*>(&reply);

	m_payload.assign(replyPtr, replyPtr + sizeof(reply));

	if (wr == WaitResult::Timeout || m_replyData.size() == 0)
	{
		sendOkReply( header, m_payload.data(), m_payload.size());
		return true;
	}

	Network::TuningSignalsApplyReply arp;
	bool res = false;

	{
		std::lock_guard lg(m_condVarMutex);
		res = arp.ParseFromArray(m_replyData.data(), TO_INT(m_replyData.size()));
	}

	if (res == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_PARSE_REQUEST_ERROR);
		return true;
	}

	if (arp.error() != 0)
	{
		sendErrReply(header, static_cast<GCL::GwErrorCode>(arp.error()));
		return true;
	}

	sendOkReply(header, m_payload.data(), m_payload.size());

	return true;
}

bool TuningGatewaySession::processChangeControlledSourceRequest(const GCL::GwMessageHeader& header,
																const char* recvBuf,
																const size_t requestSize)
{
	Q_UNUSED(requestSize);

	if (m_tunSrvClientThread == nullptr || m_connectedToTuningSrv == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_TS_CONNECTION);
		return false;
	}

	GCL::GwChangeControlledTuningSourceRequest request;

	std::memcpy(&request, recvBuf, GCL::TUNING_GW_CHANGE_CONTROLLED_TUNING_SOURCE_REQUEST_SIZE);

	QString moduleEquipmentID = QString::fromUtf8(request.moduleEquipmentId);

	bool activateControl = request.activateControl;

	clearReplyData();

	m_tunSrvClientThread->tuningChangeControlledSource(moduleEquipmentID.toStdString(), activateControl);

	WaitResult wr = waitForOrQuit(500);

	if (wr == WaitResult::QuitRequested)
	{
		return true;
	}

	Network::ChangeConrolledTuningSourceReply crp;
	bool res = false;

	{
		std::lock_guard lg(m_condVarMutex);
		res = crp.ParseFromArray(m_replyData.data(), TO_INT(m_replyData.size()));
	}

	if (res == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_PARSE_REQUEST_ERROR);
		return true;
	}

	if (crp.error() != 0)
	{
		sendErrReply(header, static_cast<GCL::GwErrorCode>(crp.error()));
		return true;
	}

	GCL::GwChangeControlledTuningSourceResponse reply;

	copyStr(reply.controlledModuleEquipmentId, GCL::STRING_LENGTH_128, crp.controlledtuningsourceequipmentid());
	reply.controlIsActive = crp.controlisactive();

	reply.reserved[0] = 0;
	reply.reserved[1] = 0;
	reply.reserved[2] = 0;

	sendOkReply(header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	return true;
}

void TuningGatewaySession::startTuningSrvClient()
{
	Q_ASSERT(m_tunSrvClientThread == nullptr);

	HostAddressPort tunSrvIP1;
	HostAddressPort tunSrvIP2;

	const std::vector<HostAddressPort>& srvAddrs = serviceAdresses();

	if (srvAddrs.size() > 0)
	{
		tunSrvIP1 = srvAddrs[0];
	}

	if (srvAddrs.size() > 1)
	{
		tunSrvIP2 = srvAddrs[1];
	}

	m_tunSrvClientThread = std::make_unique<TuningSrvClientThread>(*this, swInfo(), tunSrvIP1, tunSrvIP2,
																   QString("TuningGateway %1").arg(swInfo().equipmentID()),
																   Separator::EMPTY_STR, appSignals());
	m_tunSrvClientThread->start();
}

void TuningGatewaySession::stopTuningSrvClient()
{
	if (m_tunSrvClientThread == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_tunSrvClientThread->quitAndWait(5000);
	m_tunSrvClientThread.reset();
}

TuningGatewaySession::WaitResult TuningGatewaySession::waitForOrQuit(const int64_t timeoutMs)
{
	std::chrono::milliseconds timeout(timeoutMs);

	std::unique_lock<std::mutex> ul(m_condVarMutex);

	bool conditionMet = m_condVar.wait_for(ul, timeout,
											  [this]()
											  {
												  return isQuitRequested() || !m_replyData.empty();
											  });

	if (!conditionMet)
	{
		return WaitResult::Timeout;
	}

	if (isQuitRequested())
	{
		return WaitResult::QuitRequested;
	}

	return WaitResult::DataReady;
}

void TuningGatewaySession::clearReplyData() 
{
	std::lock_guard<std::mutex> lock(m_condVarMutex);
	m_replyData.clear();
}	