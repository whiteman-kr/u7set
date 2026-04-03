#include "TuningGwConnImpl.hpp"

#include <algorithm>
#include <cstring>
#include <ranges>


namespace GatewayClientLib
{
	TuningGwConnImpl::TuningGwConnImpl(ITuningSignalUpdater& signalUpdater, ILogger& logger) :
		GwConnImpl{logger},
		m_signalUpdater{signalUpdater}
	{
	}

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
				clear();

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
				requestTuningSources();

				m_logger.logMessage("Project '{}', buildNo {}, buildDate '{}', buildUser '{}'",
									m_workset.project.name,
									m_workset.project.buildNo,
									m_workset.project.buildDate,
									m_workset.project.buildUser);
				m_logger.logMessage("Received {} tuning sources.", m_workset.tuningSources.size());
				for (const auto& tuningSource : m_workset.tuningSources)
				{
					m_logger.logMessage(
						"\tTuning source: EquipmentID='{}', ModuleCaption='{}', SubsystemID='{}', SubsystemChannel='{}', SignalCount={}",
						tuningSource.moduleEquipmentId,
						tuningSource.moduleCaption,
						tuningSource.subsystemId,
						tuningSource.channel,
						tuningSource.signalIds.size());
				}

				// Main communication loop
				//
				while (stoken.stop_requested() == false)
				{
					// Update tuning source states
					//
					requestTuningSourceStates();

					// Update tuning signal states
					//
					updateSignalStates();

					// Check command queue, if not empty execute commands until queue is empty
					//
					bool thereAreMoreCommands = false;
					do
					{
						std::unique_lock locker{m_commandQueueMutex};

						m_commandQueueCv.wait_for(locker,
												  stoken,
												  std::chrono::milliseconds(100),
												  [this]()
												  {
													  return m_commandQueue.empty() == false;
												  });

						if (stoken.stop_requested() == true)
						{
							break;
						}

						if (m_commandQueue.empty() == false)
						{
							auto command = std::move(m_commandQueue.front());
							m_commandQueue.pop();

							thereAreMoreCommands = m_commandQueue.empty() == false;

							locker.unlock(); // Explicitly unlock before executing command.

							// Execute command.
							//
							command();
						}
					} while (thereAreMoreCommands);
				} // while (stoken.stop_requested() == false)
			}
			catch (const std::runtime_error& e)
			{
				m_conn.close();

				m_logger.logError(e.what());

				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}

		clear();

		if (m_conn.isOpen() == true)
		{
			m_conn.close();
		}

		m_isCancelledFunc = {};

		return;
	}

	void TuningGwConnImpl::commandSendActivateTuningSource(uint64_t sourceId, bool activate)
	{
		m_logger.logTrace("Enqueue command: {} control for tuning source {}", activate ? "Activate" : "Deactivate", sourceId);

		std::lock_guard lock{m_commandQueueMutex};

		m_commandQueue.push(
			[sourceId, activate]()
			{
				return;
			});

		m_commandQueueCv.notify_one();
		return;
	}

	void TuningGwConnImpl::commandWriteSignalValues(std::span<const GwTuningSignalState> states)
	{
		m_logger.logTrace("Enqueue command: Write values for {} tuning signals", states.size());

		std::vector<GwTuningSignalState> s{states.begin(), states.end()};

		std::lock_guard lock{m_commandQueueMutex};

		m_commandQueue.push(
			[states = std::move(s)]()
			{
				return;
			});

		m_commandQueueCv.notify_one();
		return;
	}

	void TuningGwConnImpl::commandApplyWrittenSignalValues()
	{
		m_logger.logTrace("Enqueue command: Apply written tuning signal values");

		std::lock_guard lock{m_commandQueueMutex};

		m_commandQueue.push(
			[]()
			{
				return;
			});

		m_commandQueueCv.notify_one();
		return;
	}

	// Performs handshake with the Tuning Gateway (TGW_HANDSHAKE).
	// Throws std::runtime_error on errors.
	//
	void TuningGwConnImpl::requestHandshake(std::string_view equipmentId, uint16_t protocolVersion)
	{
		TuningGwHandshakeRequest request{};
		TuningGwHandshakeResponse response{};
		m_workset.handshakeResponse = {};

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


		m_logger.logTrace(
			"Handshake successful. MaxStateRequest={}, MaxStateWrite={}, GwTuningSourceStateSize={}, GwTuningSignalStateSize={}",
			response.maxStateRequest,
			response.maxStateWrite,
			response.sizeof_GwTuningSourceState,
			response.sizeof_GwTuningSignalState);

		m_workset.handshakeResponse = response;

		return;
	}

	// Requests the list of available tuning sources from the Tuning Gateway (TGW_GET_TUNING_SOURCES_START/TGW_GET_TUNING_SOURCES_NEXT).
	// Retrieve the TuningSources.xml configuration file contents from the Gateway.
	// Throws std::runtime_error on errors.
	//
	void TuningGwConnImpl::requestTuningSources()
	{
		m_workset.project = {};
		m_workset.tuningSources.clear();
		m_state.tuningSourceStates.clear();

		std::vector<std::byte> tuningSourcesXmlContent{};
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

			m_logger.logTrace("TuningSources.xml total size: {}, max part size: {}, part count: {}", totalSize, maxPartSize, partCount);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_GET_TUNING_SOURCES_START error: {}", e.what())};
		}

		// Next
		//
		try
		{
			tuningSourcesXmlContent.resize(totalSize, {});

			for (uint32_t part = 0; part < partCount; part++)
			{
				GwGetTuningSourcesNextRequest request{};
				GwGetTuningSourcesNextResponse response{};
				request.part = part;

				m_logger.logTrace("Requesting TuningSources.xml part {}/{} (offset {}, size {})...",
								  part + 1,
								  partCount,
								  part * maxPartSize,
								  std::min(maxPartSize, totalSize - part * maxPartSize));

				size_t currentPartSize = std::min(maxPartSize, totalSize - part * maxPartSize);
				auto currentPartSpan = std::span{tuningSourcesXmlContent.data() + part * maxPartSize, currentPartSize};

				auto requestResult = sendRequest(TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT,
												 request,
												 std::span<const std::byte>{},
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

		// Parse received TuningSources.xml content
		//
		auto result = GatewayClientLib::parseTuningSourcesXml(tuningSourcesXmlContent);
		if (result.errors.empty() == false)
		{
			std::string allErrors;
			for (const auto& error : result.errors)
			{
				allErrors += error + "; ";
			}

			throw std::runtime_error{std::format("Parsing TuningSources.xml {} error(s): {}", result.errors.size(), allErrors)};
		}

		// Save the result for further use
		//
		m_workset.project = std::move(result.project);
		m_workset.tuningSources = std::move(result.tuningSources);

		// Fill appSignalHashes with all signal hashes.
		//
		{
			auto calcHash = [](const std::string& appSignalId)
			{
				return Radiy::calcHash(appSignalId);
			};

			for (const auto& tuningSource : m_workset.tuningSources)
			{
				m_workset.appSignalHashes.append_range(tuningSource.signalIds | std::views::transform(calcHash));
			}
		}

		return;
	}

	void TuningGwConnImpl::requestTuningSourceStates()
	{
		try
		{
			GwGetTuningSourceStatesRequest request{};
			GwGetTuningSourceStatesResponse response{};
			std::vector<GwTuningSourceState> tuningSourceStates{m_workset.tuningSources.size(), GwTuningSourceState{}};

			GwErrorCode requestResult = sendRequest(TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES,
													request,
													std::span<const std::byte>{},
													response,
													std::span{tuningSourceStates},
													m_isCancelledFunc);

			if (requestResult != GwErrorCode::GWC_SUCCESS)
			{
				throw std::runtime_error{std::format("server error {}", requestResult)};
			}

			if (response.count != m_workset.tuningSources.size())
			{
				throw std::runtime_error{std::format("tuning source states count mismatch: expected {}, got {}",
													 m_workset.tuningSources.size(),
													 response.count)};
			}

			m_state.tuningSourceStates = std::move(tuningSourceStates);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_GET_TUNING_SOURCE_STATES error: {}", e.what())};
		}

		return;
	}

	std::vector<GwTuningSignalState> TuningGwConnImpl::requestSignalStates(std::span<const Radiy::Hash> appSignals)
	{
		std::vector<GwTuningSignalState> tuningSignalStates{};
		tuningSignalStates.resize(appSignals.size(), {});

		size_t stateOffset = 0;

		if (appSignals.empty() == true)
		{
			return tuningSignalStates;
		}

		try
		{
			const size_t maxPartSize = m_workset.handshakeResponse.maxStateRequest;
			if (maxPartSize == 0)
			{
				throw std::runtime_error{"Invalid maxStateRequest value from handshake response: 0"};
			}

			for (size_t requestOffset = 0; requestOffset < appSignals.size(); requestOffset += maxPartSize)
			{
				GwTuningSignalsReadRequest request{};
				GwTuningSignalsReadResponse response{};

				size_t partSize = std::min(maxPartSize, appSignals.size() - requestOffset);
				request.count = static_cast<uint32_t>(partSize);

				GwErrorCode requestResult = sendRequest(TuningGwRequestId::TGW_TUNING_SIGNALS_READ,
														request,
														std::span{appSignals.cbegin() + requestOffset, partSize},
														response,
														std::span{tuningSignalStates.begin() + stateOffset, partSize},
														m_isCancelledFunc);

				if (requestResult != GwErrorCode::GWC_SUCCESS)
				{
					throw std::runtime_error{std::format("server error {}", requestResult)};
				}

				if (response.count > partSize)
				{
					throw std::runtime_error{std::format("response count {} exceeds requested part size {}", response.count, partSize)};
				}

				// Section 5.4: Missing signals: If a requested signal hash is not found in the system, no error is reported. The signal is
				// simply skipped in the response.
				//
				stateOffset += response.count;
			}
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_TUNING_SIGNALS_READ error: {}", e.what())};
		}

		tuningSignalStates.resize(
			stateOffset); // Resize to actual received states count, which can be less than requested if some signals were missing.

		return tuningSignalStates;
	}

	void TuningGwConnImpl::clear()
	{
		m_workset = {};
		m_state = {};

		std::lock_guard lock{m_commandQueueMutex};
		m_commandQueue = {};

		return;
	}

	void TuningGwConnImpl::updateSignalStates()
	{
		auto states = requestSignalStates(m_workset.appSignalHashes);
		m_signalUpdater.setStates(states);

		return;
	}

	// Requests the list of signal parameters from the ADS Gateway.
	// Throws std::runtime_error on errors.
	//
	// std::vector<GwAppSignalParam> TuningGwConnImpl::requestSignalParams()
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

	// void TuningGwConnImpl::requestStateChanges()
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
	// #if 0
	//			const size_t partSize = m_handshakeResponse.maxStateRequest; // is abou 40K, it can overload the network
	// #else
	//			const size_t partSize = 2500; // Reasonable value, not too big to overload the network (2500 * 48bytes * 10rps =
	//~1.2Mbytes/s).
	//										  // This value can be tuned if needed.
	//										  // If we have 100K signals, and request 2500 per 100ms, full refresh takes 4 seconds.
	//										  // If signal value changes during this time to the value >= aperture,
	//										  // it will be caught by requestStateChanges().
	// #endif
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