#include "TuningGwConnImpl.hpp"

#include <algorithm>
#include <cstring>
#include <ranges>

namespace
{
	template<size_t N>
	size_t cStringBufferLength(const char (&buffer)[N])
	{
		return static_cast<size_t>(std::find(buffer, buffer + N, '\0') - buffer);
	}

	template<size_t N1, size_t N2>
	bool cStringBuffersEqual(const char (&lhs)[N1], const char (&rhs)[N2])
	{
		const size_t lhsLen = cStringBufferLength(lhs);
		const size_t rhsLen = cStringBufferLength(rhs);

		if (lhsLen != rhsLen)
		{
			return false;
		}

		return std::memcmp(lhs, rhs, lhsLen) == 0;
	}
} // namespace

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
							command(false);
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

	std::future<GwErrorCode> TuningGwConnImpl::commandActivateTuningSource(std::string_view tuningSourceId, bool activate)
	{
		m_logger.logTrace("Enqueue command: {} control for tuning source {}", activate ? "Activate" : "Deactivate", tuningSourceId);

		// Use shared_ptr for promise because std::function must be copyable (std::move_only_function is c++23, which can be unavailable for
		// some customers).
		//
		auto sharedPromise = std::make_shared<std::promise<GwErrorCode>>();
		auto future = sharedPromise->get_future();

		{
			std::lock_guard lock{m_commandQueueMutex};

			m_commandQueue.push(
				[this, sharedPromise, tuningSourceIdStr = std::string{tuningSourceId}, activate](bool cancel)
				{
					if (cancel == true)
					{
						sharedPromise->set_value(GwErrorCode::GWC_COMMAND_CANCELED);
						return;
					}
					return doCommandRequest(sharedPromise, &TuningGwConnImpl::requestActivateTuningSource, tuningSourceIdStr, activate);
				});
		}

		m_commandQueueCv.notify_one();
		return future;
	}

	std::future<WriteValueResult> TuningGwConnImpl::commandWriteSignalValues(std::span<const GwTuningWriteValue> states,
																			 std::string_view user,
																			 bool apply)
	{
		m_logger.logTrace("Enqueue command: Write values for {} tuning signals", states.size());

		auto sharedPromise = std::make_shared<std::promise<WriteValueResult>>();
		auto future = sharedPromise->get_future();

		{
			std::lock_guard lock{m_commandQueueMutex};

			m_commandQueue.push(
				[this,
				 sharedPromise,
				 m_states = std::vector<GwTuningWriteValue>{states.begin(), states.end()},
				 m_user = std::string{user},
				 m_apply = apply](bool cancel)
				{
					if (cancel == true)
					{
						sharedPromise->set_value(GwErrorCode::GWC_COMMAND_CANCELED);
						return;
					}
					return doCommandRequest(sharedPromise, &TuningGwConnImpl::requestWriteSignalValues, m_states, m_user, m_apply);
				});
		}

		m_commandQueueCv.notify_one();
		return future;
	}

	std::future<GwErrorCode> TuningGwConnImpl::commandApplyWrittenSignals()
	{
		m_logger.logTrace("Enqueue command: Apply written tuning signal values");

		auto sharedPromise = std::make_shared<std::promise<GwErrorCode>>();
		auto future = sharedPromise->get_future();

		{
			std::lock_guard lock{m_commandQueueMutex};

			m_commandQueue.push(
				[this, sharedPromise](bool cancel)
				{
					if (cancel == true)
					{
						sharedPromise->set_value(GwErrorCode::GWC_COMMAND_CANCELED);
						return;
					}

					return doCommandRequest(sharedPromise, &TuningGwConnImpl::requestApplyWrittenSignals);
				});
		}

		m_commandQueueCv.notify_one();
		return future;
	}

	bool TuningGwConnImpl::clientIsActive() const
	{
		std::lock_guard locker{m_stateMutex};
		return m_state.clientIsActive;
	}

	std::vector<GatewayClientLib::GwTuningSourceState> TuningGwConnImpl::tuningSources() const
	{
		std::lock_guard locker{m_stateMutex};
		return m_state.tuningSourceStates;
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

		if (response.maxStateWrite == 0)
		{
			m_conn.close();
			throw std::runtime_error{"Handshake error: MaxStateWrite is zero"};
		}

		if (response.maxStateRequest == 0)
		{
			m_conn.close();
			throw std::runtime_error{"Handshake error: MaxStateRequest is zero"};
		}

		m_workset.handshakeResponse = response;

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

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

		{
			std::lock_guard locker{m_stateMutex};
			m_state.tuningSourceStates.clear();
		}

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

			GwErrorCode requestResult = GwErrorCode::GWC_SUCCESS;

			int count = 0;

			while (count < 30)
			{
				requestResult =
					sendRequest(TuningGwRequestId::TGW_GET_TUNING_SOURCES_START, startRequest, startResponse, m_isCancelledFunc);

				if (requestResult != GwErrorCode::GWC_NO_TS_CONNECTION && requestResult != GwErrorCode::GWC_TUNING_SOURCES_FILE_NOT_READY)
				{
					break;
				}

				if (m_isCancelledFunc && m_isCancelledFunc() == true)
				{
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				count++;
			}

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
		ParseTuningSourceXmlResult result = parseTuningSourcesXml(tuningSourcesXmlContent);
		if (result.errors.empty() == false)
		{
			std::string allErrors;
			for (const auto& error : result.errors)
			{
				allErrors += error + "; ";
			}

			throw std::runtime_error{std::format("Parsing TuningSources.xml {} error(s): {}", result.errors.size(), allErrors)};
		}

		// Add signals from sources.
		//
		for (const auto& src : result.tuningSources)
		{
			for (const auto [_, signalParam] : src.signals)
			{
				m_signalUpdater.addSignals({&signalParam, 1});
			}
		}

		// Save the result for further use
		//
		m_workset.project = std::move(result.project);
		m_workset.tuningSources = std::move(result.tuningSources);

		// Fill appSignalHashes with all signal hashes.
		//
		for (const auto& tuningSource : m_workset.tuningSources)
		{
			std::transform(tuningSource.signalIds.begin(),
						   tuningSource.signalIds.end(),
						   std::back_inserter(m_workset.appSignalHashes),
						   [](const std::string& appSignalId)
						   {
							   return Radiy::calcHash(appSignalId);
						   });
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

			{
				std::lock_guard locker{m_stateMutex};
				m_state.clientIsActive = response.clientIsActive != 0;
				m_state.tuningSourceStates = std::move(tuningSourceStates);
			}
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
														std::span{appSignals.begin() + requestOffset, partSize},
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

	GwErrorCode TuningGwConnImpl::requestActivateTuningSource(std::string_view tuningSourceId, bool activate)
	{
		try
		{
			GwChangeControlledTuningSourceRequest request{};
			GwChangeControlledTuningSourceResponse response{};

			const size_t idLen = std::min(tuningSourceId.size(), sizeof(request.moduleEquipmentId) - 1);
			std::copy_n(tuningSourceId.data(), idLen, request.moduleEquipmentId);
			request.moduleEquipmentId[idLen] = '\0';
			request.activateControl = activate ? 1 : 0;

			auto result = sendRequest(TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE, request, response, m_isCancelledFunc);

			if (result == GwErrorCode::GWC_SUCCESS)
			{
				// Check echoed values in response to detect possible errors.
				//
				if (response.controlIsActive != request.activateControl ||
					cStringBuffersEqual(request.moduleEquipmentId, response.controlledModuleEquipmentId) == false)
				{
					// Treat this as an internal error. Later all gateway errors (code > GWC_GATEWAY_SERVICE_ERROR_BASE) are treated
					// as runtime errors in the communication loop, which triggers connection reset and thus is a way to recover from
					// this error.
					//
					result = GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR;
				}
			}

			return result;
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_CHANGE_CONTROLLED_TUNING_SOURCE error: {}", e.what())};
		}
	}

	WriteValueResult TuningGwConnImpl::requestWriteSignalValues(std::span<const GwTuningWriteValue> states,
																std::string_view user,
																bool apply)
	{
		WriteValueResult result{};
		result.signalResults.reserve(states.size());

		// Split write requests in parts if they exceed maxStateWrite limit from handshake response
		//
		for (size_t offset = 0; offset < states.size(); offset += m_workset.handshakeResponse.maxStateWrite)
		{
			size_t partSize = std::min(static_cast<size_t>(m_workset.handshakeResponse.maxStateWrite), states.size() - offset);

			auto partResult = requestWriteSignalValuesPart(std::span{states.data() + offset, partSize}, user, apply);

			std::copy(partResult.signalResults.begin(), partResult.signalResults.end(), std::back_inserter(result.signalResults));

			if (partResult.errorCode != GwErrorCode::GWC_SUCCESS)
			{
				result.errorCode = partResult.errorCode;
				return result;
			}
		}

		return result;
	}

	WriteValueResult TuningGwConnImpl::requestWriteSignalValuesPart(std::span<const GwTuningWriteValue> states,
																	std::string_view user,
																	bool apply)
	{
		assert(states.size() <= m_workset.handshakeResponse.maxStateWrite);

		try
		{
			WriteValueResult result;

			GwTuningSignalsWriteRequest request{};
			GwTuningSignalsWriteResponse response{};

			// Safe set for user name and signal values count, as the server will validate them and return an error code if they exceed
			// limits.
			const size_t userLen = std::min(user.size(), sizeof(request.user) - 1);
			std::copy_n(user.data(), userLen, request.user);
			request.user[userLen] = '\0';
			request.apply = apply ? 1 : 0;
			request.count = static_cast<uint32_t>(states.size());

			result.signalResults.resize(states.size());

			result.errorCode = sendRequest(TuningGwRequestId::TGW_TUNING_SIGNALS_WRITE,
										   request,
										   std::span{states},
										   response,
										   std::span{result.signalResults},
										   m_isCancelledFunc);
			return result;
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_TUNING_SIGNALS_WRITE error: {}", e.what())};
		}
	}

	GwErrorCode TuningGwConnImpl::requestApplyWrittenSignals()
	{
		try
		{
			GwTuningSignalsApplyRequest request{};
			GwTuningSignalsApplyResponse response{};

			return sendRequest(TuningGwRequestId::TGW_TUNING_SIGNALS_APPLY, request, response, m_isCancelledFunc);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("TGW_TUNING_SIGNALS_APPLY error: {}", e.what())};
		}
	}

	void TuningGwConnImpl::clear()
	{
		m_workset = {};

		{
			std::lock_guard locker{m_stateMutex};
			m_state = {};
		}

		// Clear command queue, set all pending command promises to canceled.
		//
		{
			std::lock_guard lock{m_commandQueueMutex};
			while (m_commandQueue.empty() == false)
			{
				m_commandQueue.front()(true);
				m_commandQueue.pop();
			}
		}

		return;
	}

	void TuningGwConnImpl::updateSignalStates()
	{
		auto states = requestSignalStates(m_workset.appSignalHashes);
		m_signalUpdater.setStates(states);

		return;
	}
} // namespace GatewayClientLib
