#include "AdsGwConnImpl.hpp"
#include "ISignalUpdater.hpp"

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

				// Establish TCP Connections
				//
				m_logger.logTraceFormat("Connecting to ADS Gateway at {}:{}", address, port);
				bool ok = m_conn.connect(address, port, m_isCancelledFunc);
				if (ok == false)
				{
					throw std::runtime_error{std::format("Connect error: {}", m_conn.lastError())};
				}
				m_logger.logTraceFormat("Connected to ADS Gateway {}:{}", address, port);

				// Send Handshake
				//
				requestHandshake(equipmentId);

				// Getting signal list
				//
				std::vector<std::string> appSignalIds = requestSignalList();
				m_logger.logTraceFormat("Received {} signal IDs.", appSignalIds.size());

				// Getting signal params
				//
				std::vector<GwAppSignalParam> appSignalParams = requestSignalParams();
				m_logger.logTraceFormat("Received {} signal params.", appSignalParams.size());

				assert(appSignalIds.size() == appSignalParams.size());

				m_signalUpdater.addSignals(appSignalParams);

				// Main communication loop
				//
				while (stoken.stop_requested() == false)
				{
					requestStateChanges();

					// TODO: Main communication loop
					//
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

		request.protocolVersion = ADSGW_PROTOCOL_VERSION;
		std::snprintf(request.clientName, sizeof(request.clientName), "%s", equipmentId.data());

		GwErrorCode requestResult{};

		try
		{
			m_logger.logTraceFormat("Sending handshake request to ADS Gateway...");
			requestResult = sendRequest(ADSGW_HANDSHAKE, request, response, m_isCancelledFunc);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("Handshake error: {}", e.what())};
		}

		if (requestResult != GWC_SUCCESS)
		{
			throw std::runtime_error{std::format("Handshake server error: {}", static_cast<int>(requestResult))};
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

			m_logger.logTraceFormat("Sending ADSGW_SIGNAL_LIST_START request to ADS Gateway...");
			GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_LIST_START, request, startResponse, m_isCancelledFunc);

			if (requestResult != GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", static_cast<int>(requestResult))};
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
					throw std::runtime_error{std::format("server error {}", static_cast<int>(requestResult))};
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

			m_logger.logTraceFormat("Sending ADSGW_SIGNAL_PARAM_START request to ADS Gateway...");
			GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_PARAM_START, request, startResponse, m_isCancelledFunc);

			if (requestResult != GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", static_cast<int>(requestResult))};
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

				GwErrorCode requestResult = sendRequest(ADSGW_SIGNAL_PARAM_NEXT,
														request,
														std::span<const std::byte>{},
														response,
														std::span<GwAppSignalParam>{responseVariablePartBuffer},
														m_isCancelledFunc);
				if (requestResult != GWC_SUCCESS)
				{
					throw std::runtime_error{std::format("server error {}", static_cast<int>(requestResult))};
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

	void AdsGwConnImpl::requestStateChanges() {}
} // namespace AdsGatewayLib