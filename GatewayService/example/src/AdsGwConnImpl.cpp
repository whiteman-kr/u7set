#include "AdsGwConnImpl.hpp"
#include "ISignalUpdater.hpp"

#include <algorithm>
#include <cstring>


namespace AdsGatewayLib
{
	void AdsGwConnImpl::run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId)
	{
		m_isCancelledFunc = [stoken]()
		{
			return stoken.stop_requested();
		};

		while (stoken.stop_requested() == false)
		{
			try
			{
				m_signalUpdater.reset();
				m_handshakeResponse = {};
				m_appSignalIds.clear();
				m_appSignalHashes.clear();

				// Establish TCP Connections
				//
				m_logger.logTrace("Connecting to ADS Gateway at {}:{}", address, port);
				bool ok = m_conn.connect(address, port, m_isCancelledFunc);
				if (ok == false)
				{
					throw std::runtime_error{std::format("Connect error: {}", m_conn.lastError())};
				}
				m_logger.logTrace("Connected to ADS Gateway {}:{}", address, port);

				// Send Handshake
				//
				requestHandshake(equipmentId);

				// Getting signal list
				//
				m_appSignalIds = requestSignalList();
				m_logger.logTrace("Received {} signal IDs.", m_appSignalIds.size());

				m_appSignalHashes.reserve(m_appSignalIds.size());
				for (const auto& appSignalId : m_appSignalIds)
				{
					m_appSignalHashes.push_back(Radiy::calcHash(appSignalId));
				}

				// Getting signal params
				//
				std::vector<GwAppSignalParam> appSignalParams = requestSignalParams();
				m_logger.logTrace("Received {} signal params.", appSignalParams.size());

				assert(m_appSignalIds.size() == appSignalParams.size());

				m_signalUpdater.addSignals(appSignalParams);

				// Main communication loop
				//
				while (stoken.stop_requested() == false)
				{
					// Request state changes, several requests if needed.
					//
					requestStateChanges();

					// Periodically refresh the complete signal state set.
					// requestStateChanges (ARGW_SIGNAL_STATE_CHANGES) relies on the aperture mechanism,
					// so we should request the full snapshot to prevent state discrepancies with the LogicModule.
					//
					requestSignalStates();

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
			catch (const std::runtime_error& e)
			{
				m_conn.close();

				// todo: Logger, now just print to stdout.
				//
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

	// Performs handshake with the ADS Gateway.
	// Throws std::runtime_error on errors.
	//
	void AdsGwConnImpl::requestHandshake(std::string_view equipmentId)
	{
		GwHandshakeRequest request{};
		GwHandshakeResponse response{};
		m_handshakeResponse = {};

		request.protocolVersion = ADSGW_PROTOCOL_VERSION;
		std::snprintf(request.clientName, sizeof(request.clientName), "%s", equipmentId.data());

		GwErrorCode requestResult{};

		try
		{
			m_logger.logTrace("Sending handshake request to ADS Gateway...");
			requestResult = sendRequest(ADSGW_HANDSHAKE, request, response, m_isCancelledFunc);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("Handshake error: {}", e.what())};
		}

		if (requestResult != GWC_SUCCESS)
		{
			throw std::runtime_error{std::format("Handshake server error: {}", requestResult)};
		}

		if (response.protocolVersion != ADSGW_PROTOCOL_VERSION)
		{
			m_conn.close();
			throw std::runtime_error{std::format("Handshake error: Unsupported protocol version {}", response.protocolVersion)};
		}

		if (response.sizeof_GwAppSignalParam != sizeof(GwAppSignalParam))
		{
			m_conn.close();
			throw std::runtime_error{
				std::format("Handshake error: Incompatible GwAppSignalParam size {}", response.sizeof_GwAppSignalParam)};
		}

		if (response.sizeof_GwAppSignalState != sizeof(GwAppSignalState))
		{
			m_conn.close();
			throw std::runtime_error{
				std::format("Handshake error: Incompatible GwAppSignalState size {}", response.sizeof_GwAppSignalState)};
		}


		m_logger.logTrace("Handshake successful. MaxStateRequest={}, GwAppSignalParamSize={}, GwAppSignalStateSize={}",
						  response.maxStateRequest,
						  response.sizeof_GwAppSignalParam,
						  response.sizeof_GwAppSignalState);

		m_handshakeResponse = response;

		return;
	}


	// Requests the list of available signals from the ADS Gateway.
	// Throws std::runtime_error on errors.
	//
	std::vector<std::string> AdsGwConnImpl::requestSignalList()
	{
		std::vector<std::string> result{};
		uint32_t totalItems{};
		uint32_t itemsPerPart{};
		uint32_t partsCount{};

		// Start
		//
		try
		{
			GwSignalListStartRequest request{};
			GwSignalListStartResponse startResponse{};

			GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_LIST_START, request, startResponse, m_isCancelledFunc);

			if (requestResult != GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", requestResult)};
			}

			totalItems = startResponse.totalItemCount;
			itemsPerPart = startResponse.itemsPerPart;
			partsCount = startResponse.partCount;
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("ARGW_SIGNAL_LIST_START error: {}", e.what())};
		}

		// Next
		//
		try
		{
			result.reserve(totalItems);

			std::vector<AppSignalIdNetworkT> responseVariablePartBuffer{};

			for (uint32_t part = 0; part < partsCount; part++)
			{
				GwSignalListNextRequest request{};
				GwSignalListNextResponse response{};
				request.part = part;

				responseVariablePartBuffer.resize(itemsPerPart);
				std::memset(responseVariablePartBuffer.data(), 0, responseVariablePartBuffer.size() * sizeof(AppSignalIdNetworkT));

				GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_LIST_NEXT,
														request,
														std::span<const std::byte>{},
														response,
														std::span<AppSignalIdNetworkT>{responseVariablePartBuffer},
														m_isCancelledFunc);
				if (requestResult != GWC_SUCCESS)
				{
					throw std::runtime_error{std::format("server error {}", requestResult)};
				}

				if (response.part != part)
				{
					throw std::runtime_error{std::format("part mismatch: requested {}, got {}", part, response.part)};
				}

				if (response.appSignalIdCount > itemsPerPart)
				{
					throw std::runtime_error{std::format("invalid appSignalIdCount: {}", response.appSignalIdCount)};
				}

				for (uint32_t i = 0; i < response.appSignalIdCount; ++i)
				{
					auto& id = responseVariablePartBuffer[i];
					id[id.size() - 1] = '\0'; // Ensure null-termination
					result.emplace_back(id.data());
				}
			}
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("ADSGW_SIGNAL_LIST_NEXT error: {}", e.what())};
		}

		if (result.size() != totalItems)
		{
			assert(result.size() == totalItems);
			throw std::runtime_error{std::format("Getting signal list error: total expected {}, got {}", totalItems, result.size())};
		}

		return result;
	}

	// Requests the list of signal parameters from the ADS Gateway.
	// Throws std::runtime_error on errors.
	//
	std::vector<GwAppSignalParam> AdsGwConnImpl::requestSignalParams()
	{
		std::vector<GwAppSignalParam> result{};
		uint32_t totalItems{};
		uint32_t itemsPerPart{};
		uint32_t partsCount{};

		// Start
		//
		try
		{
			GwSignalParamStartRequest request{};
			GwSignalParamStartResponse startResponse{};

			m_logger.logTrace("Sending request ADSGW_SIGNAL_PARAM_START...");
			GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_PARAM_START, request, startResponse, m_isCancelledFunc);

			if (requestResult != GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", requestResult)};
			}

			totalItems = startResponse.totalItemCount;
			itemsPerPart = startResponse.itemsPerPart;
			partsCount = startResponse.partCount;
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("ADSGW_SIGNAL_PARAM_START error: {}", e.what())};
		}

		// Next
		//
		try
		{
			result.reserve(totalItems);

			std::vector<GwAppSignalParam> responseVariablePartBuffer{};

			for (uint32_t part = 0; part < partsCount; part++)
			{
				GwSignalParamNextRequest request{};
				GwSignalParamNextResponse response{};
				request.part = part;

				responseVariablePartBuffer.resize(itemsPerPart);
				std::fill(std::begin(responseVariablePartBuffer), std::end(responseVariablePartBuffer), GwAppSignalParam{});

				m_logger.logTrace("Sending request ADSGW_SIGNAL_PARAM_NEXT, part {}/{}...", part + 1, partsCount);

				GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_PARAM_NEXT,
														request,
														std::span<const std::byte>{},
														response,
														std::span<GwAppSignalParam>{responseVariablePartBuffer},
														m_isCancelledFunc);
				if (requestResult != GWC_SUCCESS)
				{
					throw std::runtime_error{std::format("server error {}", requestResult)};
				}

				if (response.part != part)
				{
					throw std::runtime_error{std::format("part mismatch: requested {}, got {}", part, response.part)};
				}

				if (response.paramCount > itemsPerPart)
				{
					throw std::runtime_error{std::format("invalid paramCount: {}", response.paramCount)};
				}

				std::copy_n(std::begin(responseVariablePartBuffer), static_cast<size_t>(response.paramCount), std::back_inserter(result));
			}
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("ADSGW_SIGNAL_PARAM_NEXT error: {}", e.what())};
		}

		if (result.size() != totalItems)
		{
			assert(result.size() == totalItems);
			throw std::runtime_error{std::format("Getting signal params error: total expected {}, got {}", totalItems, result.size())};
		}

		return result;
	}

	void AdsGwConnImpl::requestStateChanges()
	{
		try
		{
			m_statesBuffer.resize(m_handshakeResponse.maxStateRequest); // We do not expect more than maxStateRequest states in one request.

			const uint32_t RepeatRequestThreshold =
				m_handshakeResponse.maxStateRequest / 4;                // The real m_handshakeResponse.maxStateRequest is about 40K.

			uint32_t pendingChangesCount = 0;
			int attempts = 0;                                           // Just for safety to avoid infinite loops
			const uint32_t MaxAttempts = 10;                            // Safety cap: limit the number of repeat requests per call

			do
			{
				if (m_isCancelledFunc && m_isCancelledFunc() == true)
				{
					break;
				}

				GwSignalStateChangesRequest request{};
				GwSignalStateChangesResponse response{};

				GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_STATE_CHANGES,
														request,
														std::span<const std::byte>{},
														response,
														std::span{m_statesBuffer},
														m_isCancelledFunc);

				if (requestResult == GWC_NO_ADS_CONNECTION)
				{
					return;
				}

				if (requestResult != GWC_SUCCESS)
				{
					throw std::runtime_error{std::format("server error {}", requestResult)};
				}

				m_signalUpdater.setStates(std::span{m_statesBuffer.data(), response.stateCount});

				pendingChangesCount = response.pendingStatesCount;
				attempts++;

			} while (pendingChangesCount >= RepeatRequestThreshold && attempts < MaxAttempts);

			m_logger.logTrace("ADSGW_SIGNAL_STATE_CHANGES completed. Attempts={}, PendingChanges={}", attempts, pendingChangesCount);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("ADSGW_SIGNAL_STATE_CHANGES error: {}", e.what())};
		}
	}

	void AdsGwConnImpl::requestSignalStates()
	{
		if (m_appSignalHashes.empty() == true)
		{
			return;
		}

		try
		{
			if (m_nextStateIndexToRequest >= m_appSignalHashes.size())
			{
				m_nextStateIndexToRequest = 0;
			}

			auto requestCount =
				std::min(static_cast<size_t>(m_handshakeResponse.maxStateRequest), m_appSignalHashes.size() - m_nextStateIndexToRequest);

			GwSignalStateRequest request{};
			request.signalCount = static_cast<uint32_t>(requestCount);

			m_hashBuffer.clear();
			m_hashBuffer.reserve(requestCount);
			std::copy_n(m_appSignalHashes.data() + m_nextStateIndexToRequest, requestCount, std::back_inserter(m_hashBuffer));

			GwSignalStateResponse response{};
			m_statesBuffer.clear();
			m_statesBuffer.resize(requestCount);

			m_nextStateIndexToRequest += requestCount;

			// Send request
			//
			GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_STATE,
													request,
													std::span<const Radiy::Hash>{m_hashBuffer},
													response,
													std::span{m_statesBuffer},
													m_isCancelledFunc);

			if (requestResult == GWC_NO_ADS_CONNECTION)
			{
				return;
			}

			if (requestResult != GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", requestResult)};
			}

			m_signalUpdater.setStates(std::span{m_statesBuffer.data(), response.stateCount});
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("ADSGW_SIGNAL_STATE error: {}", e.what())};
		}
	}

} // namespace AdsGatewayLib