#include "TuningGwConnImpl.hpp"

#include <GatewayClientLib/ISignalUpdater.hpp>

#include <algorithm>
#include <cstring>


namespace GatewayClientLib
{
	using AppSignalIdNetworkT = std::array<char, STRING_LENGTH_128>;


	void TuningGwConnImpl::run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId)
	{
		m_isCancelledFunc = [stoken]()
		{
			return stoken.stop_requested();
		};

		while (stoken.stop_requested() == false)
		{
			try
			{
				//m_signalUpdater.reset();
				m_handshakeResponse = {};
				//m_appSignalIds.clear();
				//m_appSignalHashes.clear();

				// Establish TCP Connections
				//
				m_logger.logTrace("Connecting to Tuning Gateway at {}:{}", address, port);
				bool ok = m_conn.connect(address, port, m_isCancelledFunc);
				if (ok == false)
				{
					throw std::runtime_error{std::format("Connect error: {}", m_conn.lastError())};
				}
				m_logger.logMessage("Socket connected to Tuning Gateway at {}:{}, handshake requesting...", address, port);

				// Send Handshake
				//
				requestHandshake(equipmentId);

				// Request tuning sources file (TuningSources.xml) from the server.
				//
				std::vector<char> tuningSourcesXmlContent = requestTuningSources();



				//// Getting signal list
				////
				//m_appSignalIds = requestSignalList();
				//m_logger.logMessage("Received {} signal IDs.", m_appSignalIds.size());

				//m_appSignalHashes.reserve(m_appSignalIds.size());
				//for (const auto& appSignalId : m_appSignalIds)
				//{
				//	m_appSignalHashes.push_back(Radiy::calcHash(appSignalId));
				//}

				//// Getting signal params
				////
				//std::vector<GwAppSignalParam> appSignalParams = requestSignalParams();
				//m_logger.logMessage("Received {} signal params.", appSignalParams.size());

				//assert(m_appSignalIds.size() == appSignalParams.size());

				//m_signalUpdater.addSignals(appSignalParams);

				// Main communication loop
				//
				while (stoken.stop_requested() == false)
				{
					// Request state changes, several requests if needed.
					//
					//requestStateChanges();

					// Periodically refresh the complete signal state set.
					// requestStateChanges (ARGW_SIGNAL_STATE_CHANGES) relies on the aperture mechanism,
					// so we should request the full snapshot to prevent state discrepancies with the LogicModule.
					//
					//requestSignalStates();

					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
			catch (const std::runtime_error& e)
			{
				m_conn.close();

				m_logger.logError(e.what());

				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}

		if (m_conn.isOpen() == true)
		{
			m_conn.close();
		}

		m_isCancelledFunc = {};

		return;
	}

	// Performs handshake with the Tuning Gateway (TGW_HANDSHAKE).
	// Throws std::runtime_error on errors.
	// 
	void TuningGwConnImpl::requestHandshake(std::string_view equipmentId, uint16_t protocolVersion)
	{
		TuningGwHandshakeRequest request{};
		TuningGwHandshakeResponse response{};
		m_handshakeResponse = {};

		request.protocolVersion = protocolVersion;
		
		const size_t clientNameLen = std::min(equipmentId.size(), sizeof(request.clientName) - 1);
		std::memcpy(request.clientName, equipmentId.data(), clientNameLen);
		request.clientName[clientNameLen] = '\0';

		GwErrorCode requestResult{};

		try
		{
			m_logger.logTrace("Sending handshake request to Tuning Gateway...");
			requestResult = sendRequest(TuningGwRequestId::TGW_HANDSHAKE, request, response, m_isCancelledFunc);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("Handshake error: {}", e.what())};
		}

		if (requestResult != GwErrorCode::GWC_SUCCESS)
		{
			throw std::runtime_error{std::format("Handshake server error: {}", requestResult)};
		}

		if (response.protocolVersion != protocolVersion)
		{
			m_conn.close();
			throw std::runtime_error{std::format("Handshake error: Unsupported protocol version {}", response.protocolVersion)};
		}

		if (response.sizeof_GwTuningSourceState != sizeof(GwTuningSourceState))
		{
			m_conn.close();
			throw std::runtime_error{
				std::format("Handshake error: Incompatible GwTuningSourceState size {}", response.sizeof_GwTuningSourceState)};
		}

		if (response.sizeof_GwTuningSignalState != sizeof(GwTuningSignalState))
		{
			m_conn.close();
			throw std::runtime_error{
				std::format("Handshake error: Incompatible GwTuningSignalState size {}", response.sizeof_GwTuningSignalState)};
		}


		m_logger.logTrace("Handshake successful. MaxStateRequest={}, MaxStateWrite={}, GwTuningSourceStateSize={}, GwTuningSignalStateSize={}",
						  response.maxStateRequest,
						  response.maxStateWrite,
						  response.sizeof_GwTuningSourceState,
						  response.sizeof_GwTuningSignalState);

		m_handshakeResponse = response;

		return;
	}

	// Requests the list of available tuning sources from the Tuning Gateway (TGW_GET_TUNING_SOURCES_START/TGW_GET_TUNING_SOURCES_NEXT).
	// Retrieve the TuningSources.xml configuration file contents from the Gateway.
	// Throws std::runtime_error on errors.
	//
	std::vector<char> TuningGwConnImpl::requestTuningSources()
	{
		std::vector<char> tuningSourcesXmlContent{};
		uint32_t totalSize{};
		uint32_t maxPartSize{};
		uint32_t partCount{};

		// Start
		//
		try
		{
			GwGetTuningSourcesStartRequest startRequest{};
			GwGetTuningSourcesStartResponse startResponse{};

			GwErrorCode requestResult =
				sendRequest(TuningGwRequestId::TGW_GET_TUNING_SOURCES_START, startRequest, startResponse, m_isCancelledFunc);

			if (requestResult != GwErrorCode::GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", requestResult)};
			}

			totalSize = startResponse.totalSize;
			maxPartSize = startResponse.maxPartSize;
			partCount = startResponse.partCount;
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_GET_TUNING_SOURCES_START error: {}", e.what())};
		}

		// Next
		//
		try
		{
			tuningSourcesXmlContent.resize(totalSize, 0);

			for (uint32_t part = 0; part < partCount; part++)
			{
				GwGetTuningSourcesNextRequest request{};
				GwGetTuningSourcesNextResponse response{};
				request.part = part;

				size_t currentPartSize = std::min(maxPartSize, totalSize - part * maxPartSize);
				auto currentPartSpan = std::span{tuningSourcesXmlContent.data() + part * maxPartSize, currentPartSize};

				auto requestResult = sendRequest(TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT,
												 request,
												 std::span<const char>{},
												 response,
												 currentPartSpan,
												 m_isCancelledFunc);
				if (requestResult != GwErrorCode::GWC_SUCCESS)
				{
					throw std::runtime_error{std::format("server error {}", requestResult)};
				}

				if (response.part != part)
				{
					throw std::runtime_error{std::format("part mismatch: requested {}, got {}", part, response.part)};
				}

				if (response.partSize != currentPartSize)
				{
					throw std::runtime_error{std::format("part size mismatch: expected {}, got {}", currentPartSize, response.partSize)};
				}
			}
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_GET_TUNING_SOURCES_NEXT error: {}", e.what())};
		}

		return tuningSourcesXmlContent;
	}

	// Requests the list of signal parameters from the ADS Gateway.
	// Throws std::runtime_error on errors.
	//
	//std::vector<GwAppSignalParam> TuningGwConnImpl::requestSignalParams()
	//{
	//	std::vector<GwAppSignalParam> result{};
	//	uint32_t totalItems{};
	//	uint32_t itemsPerPart{};
	//	uint32_t partsCount{};

	//	// Start
	//	//
	//	try
	//	{
	//		AdsGwSignalParamStartRequest request{};
	//		AdsGwSignalParamStartResponse startResponse{};

	//		m_logger.logTrace("Sending request ADSGW_SIGNAL_PARAM_START...");
	//		GwErrorCode requestResult = sendRequest(AdsGwRequestId::ADSGW_SIGNAL_PARAM_START, request, startResponse, m_isCancelledFunc);

	//		if (requestResult != GwErrorCode::GWC_SUCCESS)
	//		{
	//			throw std::runtime_error{std::format("server error {}", requestResult)};
	//		}

	//		totalItems = startResponse.totalItemCount;
	//		itemsPerPart = startResponse.itemsPerPart;
	//		partsCount = startResponse.partCount;
	//	}
	//	catch (const std::runtime_error& e)
	//	{
	//		throw std::runtime_error{std::format("ADSGW_SIGNAL_PARAM_START error: {}", e.what())};
	//	}

	//	// Next
	//	//
	//	try
	//	{
	//		result.reserve(totalItems);

	//		std::vector<GwAppSignalParam> responseVariablePartBuffer{};

	//		for (uint32_t part = 0; part < partsCount; part++)
	//		{
	//			AdsGwSignalParamNextRequest request{};
	//			AdsGwSignalParamNextResponse response{};
	//			request.part = part;

	//			responseVariablePartBuffer.resize(itemsPerPart);
	//			std::fill(std::begin(responseVariablePartBuffer), std::end(responseVariablePartBuffer), GwAppSignalParam{});

	//			m_logger.logTrace("Sending request ADSGW_SIGNAL_PARAM_NEXT, part {}/{}...", part + 1, partsCount);

	//			GwErrorCode requestResult = sendRequest(AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT,
	//													request,
	//													std::span<const std::byte>{},
	//													response,
	//													std::span<GwAppSignalParam>{responseVariablePartBuffer},
	//													m_isCancelledFunc);
	//			if (requestResult != GwErrorCode::GWC_SUCCESS)
	//			{
	//				throw std::runtime_error{std::format("server error {}", requestResult)};
	//			}

	//			if (response.part != part)
	//			{
	//				throw std::runtime_error{std::format("part mismatch: requested {}, got {}", part, response.part)};
	//			}

	//			if (response.paramCount > itemsPerPart)
	//			{
	//				throw std::runtime_error{std::format("invalid paramCount: {}", response.paramCount)};
	//			}

	//			std::copy_n(std::begin(responseVariablePartBuffer), static_cast<size_t>(response.paramCount), std::back_inserter(result));
	//		}
	//	}
	//	catch (const std::runtime_error& e)
	//	{
	//		throw std::runtime_error{std::format("ADSGW_SIGNAL_PARAM_NEXT error: {}", e.what())};
	//	}

	//	if (result.size() != totalItems)
	//	{
	//		assert(result.size() == totalItems);
	//		throw std::runtime_error{std::format("Getting signal params error: total expected {}, got {}", totalItems, result.size())};
	//	}

	//	return result;
	//}

	//void TuningGwConnImpl::requestStateChanges()
	//{
	//	try
	//	{
	//		m_statesBuffer.resize(m_handshakeResponse.maxStateRequest); // We do not expect more than maxStateRequest states in one request.

	//		const uint32_t RepeatRequestThreshold =
	//			m_handshakeResponse.maxStateRequest / 4;                // The real m_handshakeResponse.maxStateRequest is about 40K.

	//		uint32_t pendingChangesCount = 0;
	//		int attempts = 0;                                           // Just for safety to avoid infinite loops
	//		const int MaxAttempts = 10;                                 // Safety cap: limit the number of repeat requests per call

	//		do
	//		{
	//			if (m_isCancelledFunc && m_isCancelledFunc() == true)
	//			{
	//				break;
	//			}

	//			AdsGwSignalStateChangesRequest request{};
	//			AdsGwSignalStateChangesResponse response{};

	//			GwErrorCode requestResult = sendRequest(AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES,
	//													request,
	//													std::span<const std::byte>{},
	//													response,
	//													std::span{m_statesBuffer},
	//													m_isCancelledFunc);

	//			if (requestResult == GwErrorCode::GWC_NO_ADS_CONNECTION)
	//			{
	//				return;
	//			}

	//			if (requestResult != GwErrorCode::GWC_SUCCESS)
	//			{
	//				throw std::runtime_error{std::format("server error {}", requestResult)};
	//			}

	//			m_signalUpdater.setStates(std::span{m_statesBuffer.data(), response.stateCount});

	//			pendingChangesCount = response.pendingStatesCount;
	//			attempts++;

	//		} while (pendingChangesCount >= RepeatRequestThreshold && attempts < MaxAttempts);

	//		m_logger.logTrace("ADSGW_SIGNAL_STATE_CHANGES completed. Attempts={}, PendingChanges={}", attempts, pendingChangesCount);
	//	}
	//	catch (const std::runtime_error& e)
	//	{
	//		throw std::runtime_error{std::format("ADSGW_SIGNAL_STATE_CHANGES error: {}", e.what())};
	//	}
	//}

	// Request the next page of signal states from the ADS Gateway.
	//
//	void TuningGwConnImpl::requestSignalStates()
//	{
//		if (m_appSignalHashes.empty() == true)
//		{
//			return;
//		}
//
//		try
//		{
//			if (m_nextStateIndexToRequest >= m_appSignalHashes.size())
//			{
//				m_nextStateIndexToRequest = 0;
//			}
//
//#if 0
//			const size_t partSize = m_handshakeResponse.maxStateRequest; // is abou 40K, it can overload the network
//#else
//			const size_t partSize = 2500; // Reasonable value, not too big to overload the network (2500 * 48bytes * 10rps = ~1.2Mbytes/s).
//										  // This value can be tuned if needed.
//										  // If we have 100K signals, and request 2500 per 100ms, full refresh takes 4 seconds.
//										  // If signal value changes during this time to the value >= aperture,
//										  // it will be caught by requestStateChanges().
//#endif
//			auto requestCount = std::min(partSize, m_appSignalHashes.size() - m_nextStateIndexToRequest);
//
//			AdsGwSignalStateRequest request{};
//			request.signalCount = static_cast<uint32_t>(requestCount);
//
//			m_hashBuffer.clear();
//			m_hashBuffer.reserve(requestCount);
//			std::copy_n(m_appSignalHashes.data() + m_nextStateIndexToRequest, requestCount, std::back_inserter(m_hashBuffer));
//
//			AdsGwSignalStateResponse response{};
//			m_statesBuffer.clear();
//			m_statesBuffer.resize(requestCount);
//
//			m_nextStateIndexToRequest += requestCount;
//
//			// Send request
//			//
//			GwErrorCode requestResult = sendRequest(AdsGwRequestId::ADSGW_SIGNAL_STATE,
//													request,
//													std::span<const Radiy::Hash>{m_hashBuffer},
//													response,
//													std::span{m_statesBuffer},
//													m_isCancelledFunc);
//
//			if (requestResult == GwErrorCode::GWC_NO_ADS_CONNECTION)
//			{
//				return;
//			}
//
//			if (requestResult != GwErrorCode::GWC_SUCCESS)
//			{
//				throw std::runtime_error{std::format("server error {}", requestResult)};
//			}
//
//			m_signalUpdater.setStates(std::span{m_statesBuffer.data(), response.stateCount});
//		}
//		catch (const std::runtime_error& e)
//		{
//			throw std::runtime_error{std::format("ADSGW_SIGNAL_STATE error: {}", e.what())};
//		}
//	}

} // namespace GatewayClientLib