#include "AdsGatewayServer.h"

// -----------------------------------------------------------------------------
//
//	AdsGatewaySessionAppSignalStateUpdater class implementation
//
// -----------------------------------------------------------------------------

AdsGatewaySessionAppSignalStateUpdater::AdsGatewaySessionAppSignalStateUpdater(AdsGatewaySession& session) :
	m_session(session)
{
}

void AdsGatewaySessionAppSignalStateUpdater::adsConnected()
{
	m_session.setConnectedToAppDataSrv(true);
}

void AdsGatewaySessionAppSignalStateUpdater::adsDisconnected()
{
	m_session.setConnectedToAppDataSrv(false);
}

void AdsGatewaySessionAppSignalStateUpdater::updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply)
{ 
	m_session.updateAppSignalStates(reply); 
}

void AdsGatewaySessionAppSignalStateUpdater::processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply)
{ 
	m_session.processAppSignalStateChanges(reply); 
}

void AdsGatewaySessionAppSignalStateUpdater::processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply)
{ 
	Q_UNUSED(reply);
}

// -----------------------------------------------------------------------------
//	
//	AdsGatewaySession class implementation
//
// -----------------------------------------------------------------------------

AdsGatewaySession::AdsGatewaySession(const SoftwareInfo& swInfo,
	const AppSignals& appSignals,
	const std::vector<HostAddressPort>& serviceAddresses,
	asio::ip::tcp::socket socket,
	CircularLoggerShared log) :
	AsyncTcpSession(swInfo, appSignals, serviceAddresses, std::move(socket), log)
{ 
	m_payload.resize(GCL::GW_MAX_MSG_PAYLOAD_SIZE); 

	int signalCount = TO_INT(appSignals.count());

	m_signalStates.resize(signalCount);
	m_hashToIndex.reserve(signalCount);

	for (int i = 0; i < signalCount; i++)
	{
		const AppSignal* appSignal = appSignals.getSignalByIndex(TO_INT(i));

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

AdsGatewaySession::~AdsGatewaySession()
{
}

void AdsGatewaySession::setConnectedToAppDataSrv(bool connected)
{
	m_connectedToAppDataSrv = connected;

	if (connected == false)
	{
		invalidateSignals();
	}
}

void AdsGatewaySession::updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply)
{
	setConnectedToAppDataSrv(true);

	int statesCount = reply.appsignalstates_size();

	static constexpr int BAD_INDEX = -1;

	m_indexes.resize(statesCount, BAD_INDEX);

	for (int i = 0; i < statesCount; i++)
	{
		const Proto::AppSignalState& state = reply.appsignalstates(i);

		Hash hash = state.hash();

		auto it = m_hashToIndex.find(hash);

		if (it == m_hashToIndex.end())
		{
			m_indexes[i] = BAD_INDEX;
			continue;
		}

		m_indexes[i] = it->second;
	}

	{
		std::lock_guard lg(m_signalStatesMutex);

		for (int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& state = reply.appsignalstates(i);

			int signalIndex = m_indexes[i];

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

void AdsGatewaySession::processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply)
{
	setConnectedToAppDataSrv(true);

	int statesCount = reply.appsignalstates_size();

	if (statesCount == 0)
	{
		return;
	}

	GCL::GwAppSignalState state;

	constexpr size_t MAX_QUEUE_SIZE = 200000;

	{
		std::lock_guard lg(m_signalStateChangesMutex);

		const size_t curSize = m_signalStateChanges.size();

		if (curSize + statesCount > MAX_QUEUE_SIZE)
		{
			const size_t deleteCount = curSize + statesCount - MAX_QUEUE_SIZE;

			if (deleteCount >= curSize)
			{
				m_signalStateChanges.clear();
			}
			else
			{
				m_signalStateChanges.erase(m_signalStateChanges.begin(), m_signalStateChanges.begin() + deleteCount);
			}
		}

		for (int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& s = reply.appsignalstates(i);

			Hash hash = s.hash();

			auto it = m_hashToIndex.find(hash);

			if (it == m_hashToIndex.end())
			{
				continue;
			}

			state.hash = hash;
			state.systemTime = s.systemtime();
			state.localTime = s.localtime();
			state.plantTime = s.planttime();
			state.value = s.value();
			state.flags = s.flags();
			state.reserved = 0;

			m_signalStateChanges.push_back(state);
		}
	}

	updateSignalStatesByChanges(reply);
}

void AdsGatewaySession::invalidateSignals()
{
	std::lock_guard lg(m_signalStatesMutex);

	for (SimpleAppSignalState& st : m_signalStates)
	{
		st.invalidate();
	}
}

void AdsGatewaySession::startGrpcAppDataSrvClient()
{
	std::vector<HostAddressPort> srvAddrs = serviceAdresses();

	auto updater = std::make_shared<AdsGatewaySessionAppSignalStateUpdater>(*this);

	m_adsClient = std::make_unique<GrpcAdsClient>(swInfo(),
												  srvAddrs,
												  QString("GrpcAdsClient GatewayService %1").arg(swInfo().equipmentID()),
												  getLog(),
												  GrpcAdsClient::RequestType::GetAppSignalState,
												  100,
												  GrpcAdsClient::RequestType::GetAppSignalStateChanges,
												  20,
												  updater);

	std::vector<Hash> hashes = appSignals().getHashes();

	m_adsClient->setHashesToRequestStates(hashes);

	m_adsClient->start();
}

void AdsGatewaySession::stopGrpcAppDataSrvClient()
{
	if (m_adsClient != nullptr)
	{
		m_adsClient->stop();
		m_adsClient.reset();
	}
}

void AdsGatewaySession::onStarted()
{ 
	startGrpcAppDataSrvClient(); 
}

void AdsGatewaySession::onStopped()
{ 
	stopGrpcAppDataSrvClient(); 
}

bool AdsGatewaySession::checkRequestID(uint32_t requestID)
{
	GCL::AdsGwRequestId rqID = static_cast<GCL::AdsGwRequestId>(requestID);

	switch (rqID)
	{
	case GCL::AdsGwRequestId::ADSGW_HANDSHAKE:
	case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_START:
	case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT:
	case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START:
	case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
	case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE:
	case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
		return true;
	default:
		;
	}

	return false;
}

bool AdsGatewaySession::isHandshakeRequest(uint32_t requestID)
{ 
	GCL::AdsGwRequestId rqID = static_cast<GCL::AdsGwRequestId>(requestID); 

	return (rqID == GCL::AdsGwRequestId::ADSGW_HANDSHAKE);
}

bool AdsGatewaySession::checkPayloadSize(const GCL::GwMessageHeader& header,
							  const char* recvBuf,
							  const size_t recvBufSize,
							  GCL::GwErrorCode& errCode)
{
	errCode = GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR;

	TEST_PTR_RETURN_FALSE(recvBuf);

	size_t payloadSize = 0;

	switch (static_cast<GCL::AdsGwRequestId>(header.requestID))
	{
	case GCL::AdsGwRequestId::ADSGW_HANDSHAKE:
		payloadSize = GCL::ADS_GW_HANDSHAKE_REQUEST_SIZE;
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_START:
		payloadSize = GCL::ADS_GW_SIGNAL_LIST_START_REQUEST_SIZE;
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT:
		payloadSize = GCL::ADS_GW_SIGNAL_LIST_NEXT_REQUEST_SIZE;
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START:
		payloadSize = GCL::ADS_GW_SIGNAL_PARAM_START_REQUEST_SIZE;
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
		payloadSize = GCL::ADS_GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE;
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE:
		{
			if (recvBufSize < GCL::GW_MSG_HEADER_SIZE + GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE)
			{
				return false;
			}
			GCL::AdsGwSignalStateRequest request;

			std::memcpy(&request, recvBuf + GCL::GW_MSG_HEADER_SIZE, GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE);

			if (request.signalCount > GCL::ADS_GW_MAX_SIGNAL_STATES)
			{
				errCode = GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS;
				return false;
			}

			payloadSize = GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE + static_cast<size_t>(request.signalCount) * GCL::GW_APP_SIGNAL_HASH_SIZE;
		}
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
		payloadSize = GCL::ADS_GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE;
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

bool AdsGatewaySession::processRequest(const GCL::GwMessageHeader& header, char* recvBuf, size_t requestSize)
{
	GCL::AdsGwRequestId requestID = static_cast<GCL::AdsGwRequestId>(header.requestID);

	bool result = false;

	switch (requestID)
	{
	case GCL::AdsGwRequestId::ADSGW_HANDSHAKE:
		result = processHandshakeRequest(header, recvBuf, requestSize);
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_START:
		result = processSignalListStartRequest(header, recvBuf, requestSize);
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT:
		result = processSignalListNextRequest(header, recvBuf, requestSize);
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START:
		result = processSignalParamStartRequest(header, recvBuf, requestSize);
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT:
		result = processSignalParamNextRequest(header, recvBuf, requestSize);
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE:
		result = processSignalStateRequest(header, recvBuf, requestSize);
		break;

	case GCL::AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES:
		result = processSignalStateChangesRequest(header, recvBuf, requestSize);
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

bool AdsGatewaySession::processHandshakeRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::AdsGwHandshakeRequest request;

	std::memcpy(&request, recvBuf, GCL::ADS_GW_HANDSHAKE_REQUEST_SIZE);

	//

	bool nullTerminated = checkNullTerminated(request.clientName, sizeof(request.clientName));

	if (nullTerminated == false)
	{
		logErr("HANDSHAKE request error, clientName is NOT null-terminated");

		sendErrReply(header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	setClientName(QString::fromUtf8(request.clientName));

	if (request.protocolVersion != GCL::ADS_GW_PROTOCOL_VERSION)
	{
		logErr(QString("HANDSHAKE request from %1, WRONG protocol version 0x%2 (required %3)")
				   .arg(clientName())
				   .arg(request.protocolVersion, 4, 16, QChar('0'))
				   .arg(GCL::ADS_GW_PROTOCOL_VERSION, 4, 16, QChar('0')));

		sendErrReply(header, GCL::GwErrorCode::GWC_UNSUPPORTED_VERSION);
		return false;
	}

	logMsg(
		QString("HANDSHAKE request from %1, protocol version 0x%2").arg(clientName()).arg(request.protocolVersion, 4, 16, QChar('0')));

	GCL::AdsGwHandshakeResponse reply;

	reply.protocolVersion = GCL::ADS_GW_PROTOCOL_VERSION;
	reply.reserved = 0;
	reply.maxStateRequest = GCL::ADS_GW_MAX_SIGNAL_STATES;
	reply.sizeof_GwAppSignalParam = GCL::GW_APP_SIGNAL_PARAM_SIZE;
	reply.sizeof_GwAppSignalState = GCL::GW_APP_SIGNAL_STATE_SIZE;

	sendOkReply(header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	setHandshakeCompleted(true);

	return true;
}

bool AdsGatewaySession::processSignalListStartRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	//

	logMsg(QString("SIGNAL_LIST_START request from %1").arg(clientName()));

	GCL::AdsGwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(appSignals().count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT;
	reply.partCount = (signalCount + reply.itemsPerPart - 1) /reply.itemsPerPart;

	sendOkReply(header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	return true;
}

bool AdsGatewaySession::processSignalListNextRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{ 
	Q_UNUSED(requestSize);

	GCL::AdsGwSignalListNextRequest request;
	std::memcpy(&request, recvBuf, GCL::ADS_GW_SIGNAL_LIST_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_LIST_NEXT request from %1, requested part %2").arg(clientName()).arg(request.part));

	char* payloadData = m_payload.data();

	const int signalsCount = TO_INT(appSignals().count());
	const int itemsPerPart = GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT;
	const int partCount = signalsCount / itemsPerPart + (signalsCount % itemsPerPart ? 1 : 0);

	if (request.part >= static_cast<uint32_t>(partCount))
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	GCL::AdsGwSignalListNextResponse reply;

	reply.part = request.part;
	reply.appSignalIdCount = 0;

	int signalStartIndex = request.part * GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT;
	const int signalEndIndex = std::min(TO_INT(signalStartIndex + GCL::ADS_GW_MAX_APP_SIGNAL_ID_COUNT), signalsCount);

	size_t payloadSize = GCL::ADS_GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE;

	for (int i = signalStartIndex; i < signalEndIndex; i++)
	{
		if (payloadSize + GCL::GW_APP_SIGNAL_ID_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			Q_ASSERT(false);
			logErr("AdsGatewayServer::processSignalListNextRequest payload size exceed!");
			sendErrReply(header, GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR);
			return false;
		}

		const AppSignal* appSignal = appSignals().getSignalByIndex(i);

		TEST_PTR_CONTINUE(appSignal);

		copyStr(payloadData + payloadSize, GCL::GW_APP_SIGNAL_ID_SIZE, appSignal->appSignalID());

		payloadSize += GCL::GW_APP_SIGNAL_ID_SIZE;

		reply.appSignalIdCount++;
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE);

	sendOkReply(header, payloadData, payloadSize);

	return true;
}

bool AdsGatewaySession::processSignalParamStartRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{ 
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	logMsg(QString("SIGNAL_PARAM_START request from %1").arg(clientName()));

	GCL::AdsGwSignalListStartResponse reply;

	uint32_t signalCount = static_cast<uint32_t>(appSignals().count());

	reply.totalItemCount = signalCount;
	reply.itemsPerPart = GCL::ADS_GW_MAX_SIGNAL_PARAMS;
	reply.partCount = signalCount / reply.itemsPerPart + (signalCount % reply.itemsPerPart ? 1 : 0);

	sendOkReply(header, reinterpret_cast<const char*>(&reply), sizeof(reply));

	return true;
}

bool AdsGatewaySession::processSignalParamNextRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{ 
	Q_UNUSED(requestSize);

	GCL::AdsGwSignalParamNextRequest request;
	std::memcpy(&request, recvBuf, GCL::ADS_GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE);

	//

	logMsg(QString("SIGNAL_PARAM_NEXT request from %1, requested part %2").arg(clientName()).arg(request.part));

	char* payloadData = m_payload.data();

	const int signalsCount = TO_INT(appSignals().count());
	const int itemsPerPart = GCL::ADS_GW_MAX_SIGNAL_PARAMS;
	const int partCount = signalsCount / itemsPerPart + (signalsCount % itemsPerPart ? 1 : 0);

	if (request.part >= static_cast<uint32_t>(partCount))
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
		return false;
	}

	GCL::AdsGwSignalParamNextResponse reply;

	reply.part = request.part;
	reply.paramCount = 0;

	int signalStartIndex = request.part * GCL::ADS_GW_MAX_SIGNAL_PARAMS;
	const int signalEndIndex = std::min(TO_INT(signalStartIndex + GCL::ADS_GW_MAX_SIGNAL_PARAMS), signalsCount);

	size_t payloadSize = GCL::ADS_GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE;

	for (int i = signalStartIndex; i < signalEndIndex; i++)
	{
		if (payloadSize + GCL::GW_APP_SIGNAL_PARAM_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			Q_ASSERT(false);
			logErr("AdsGatewayServer::processSignalParamNextRequest payload size exceed!");
			sendErrReply(header, GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR);
			return false;
		}

		const AppSignal* appSignal = appSignals().getSignalByIndex(i);

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

		payloadSize += GCL::GW_APP_SIGNAL_PARAM_SIZE;

		reply.paramCount++;
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE);

	sendOkReply(header, payloadData, payloadSize);

	return true;
}

bool AdsGatewaySession::processSignalStateRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{
	Q_UNUSED(requestSize);

	GCL::AdsGwSignalStateRequest request;
	std::memcpy(&request, recvBuf, GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE);

	// logMsg(QString("SIGNAL_STATE request from %1, %2 states requested").
	// 	   arg(stc->clientName).arg(request.signalCount));

	if (request.signalCount > GCL::ADS_GW_MAX_SIGNAL_STATES)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_TOO_MANY_SIGNALS);
		return false;
	}

	if (m_connectedToAppDataSrv.load(std::memory_order_relaxed) == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_ADS_CONNECTION);
		return true;		// this is not request format error!
	}

	char* payloadData = m_payload.data();

	GCL::AdsGwSignalStateResponse reply;

	reply.stateCount = 0;

	const uint32_t hashesCount = request.signalCount;
	size_t hashesOffset = GCL::ADS_GW_SIGNAL_STATE_REQUEST_SIZE;

	size_t payloadSize = GCL::ADS_GW_SIGNAL_STATE_RESPONSE_SIZE;

	{
		std::lock_guard lg(m_signalStatesMutex);

		for (uint32_t i = 0; i < hashesCount; i++)
		{
			if (payloadSize + GCL::GW_APP_SIGNAL_STATE_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
			{
				Q_ASSERT(false);
				logErr("AdsGatewayServer::processSignalStateRequest payload size exceed!");
				sendErrReply(header, GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR);
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

			payloadSize += GCL::GW_APP_SIGNAL_STATE_SIZE;

			reply.stateCount++;
		}
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_STATE_RESPONSE_SIZE);

	sendOkReply(header, payloadData, payloadSize);

	// logMsg(QString("Sent %1 signal states").arg(reply.stateCount));

	return true;
}

bool AdsGatewaySession::processSignalStateChangesRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize)
{ 
	Q_UNUSED(recvBuf);
	Q_UNUSED(requestSize);

	//	logMsg(QString("SIGNAL_STATE_CHANGES request from %1").arg(stc->clientName));

	GCL::AdsGwSignalStateChangesRequest request;
	std::memcpy(&request, recvBuf, GCL::ADS_GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE);

	if (m_connectedToAppDataSrv.load(std::memory_order_relaxed) == false)
	{
		sendErrReply(header, GCL::GwErrorCode::GWC_NO_ADS_CONNECTION);
		return true; // this is not request format error!
	}

	//

	char* payloadData = m_payload.data();

	GCL::AdsGwSignalStateChangesResponse reply;

	reply.stateCount = 0;
	reply.pendingStatesCount = 0;

	size_t payloadSize = GCL::ADS_GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE;

	{
		std::lock_guard lg(m_signalStateChangesMutex);

		GCL::GwAppSignalState state;

		while (m_signalStateChanges.empty() == false && reply.stateCount < GCL::ADS_GW_MAX_SIGNAL_STATE_CHANGES)
		{
			if (payloadSize + GCL::GW_APP_SIGNAL_STATE_SIZE > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
			{
				break;
			}

			state = m_signalStateChanges.front();
			m_signalStateChanges.pop_front();

			std::memcpy(payloadData + payloadSize, &state, GCL::GW_APP_SIGNAL_STATE_SIZE);

			payloadSize += GCL::GW_APP_SIGNAL_STATE_SIZE;

			reply.stateCount++;
		}

		reply.pendingStatesCount = TO_UINT32(m_signalStateChanges.size());
	}

	std::memcpy(payloadData, &reply, GCL::ADS_GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE);

	sendOkReply(header, payloadData, payloadSize);

	// logMsg(QString("Sent %1 signal state changes, pending states %2").
	// 					arg(reply.stateCount).arg(reply.pendingStatesCount));
	return true;
}

void AdsGatewaySession::updateSignalStatesByChanges(const Grpc::GetAppSignalStateChangesReply& reply)
{
	const size_t statesCount = reply.appsignalstates_size();

	std::lock_guard lg(m_signalStatesMutex);

	for (size_t i = 0; i < statesCount; i++)
	{
		const Proto::AppSignalState& s = reply.appsignalstates(TO_INT(i));

		auto it = m_hashToIndex.find(s.hash());

		if (it == m_hashToIndex.end())
		{
			continue;
		}

		size_t signalIndex = it->second;

		SimpleAppSignalState& signalState = m_signalStates[signalIndex];

		if (signalState.systemTime() < s.systemtime())
		{
			signalState.load(s);
		}
	}
}