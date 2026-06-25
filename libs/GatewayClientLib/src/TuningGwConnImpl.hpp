#pragma once

#include "GwConnImpl.hpp"
#include "TuningSources.hpp"

#include <GatewayClientLib/ITuningSignalUpdater.hpp>
#include <GatewayClientLib/TuningGwConnection.hpp>

#include <future>
#include <queue>

namespace GatewayClientLib
{
	class TuningGwConnImpl : public GwConnImpl<TuningGwRequestId>
	{
	public:
		TuningGwConnImpl(ITuningSignalUpdater& signalUpdater, ILogger& logger);

		void run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId) override;

		// Enqueues a command to activate or deactivate control for a tuning source.
		//
		std::future<GwErrorCode> commandActivateTuningSource(std::string_view tuningSourceId, bool activate);

		// Enqueues commands to write tuning signal values.
		//
		std::future<WriteValueResult> commandWriteSignalValues(std::span<const GwTuningWriteValue> states,
															   std::string_view user,
															   bool apply);

		// Enqueues a command to apply(commit) previously written tuning signal values.
		//
		std::future<GwErrorCode> commandApplyWrittenSignals();

	public:
		[[nodiscard]] bool clientIsActive() const;
		[[nodiscard]] std::vector<GatewayClientLib::GwTuningSourceState> tuningSources() const;

		// Requests: All requests throw std::runtime_error on communication errors.
		//
	protected:
		void requestHandshake(std::string_view equipmentId, uint16_t protocolVersion = TUNING_GW_PROTOCOL_VERSION);
		void requestTuningSources();
		void requestTuningSourceStates();
		[[nodiscard]]
		std::vector<GwTuningSignalState> requestSignalStates(std::span<const Radiy::Hash> appSignals);

		// Command requests error handling:
		// * TuningService errors are returned to the caller as std::future<GwErrorCode>, so the caller can handle them (e.g. by showing an
		//   error message to the user, etc.).
		// * all other errors (communication or Gateway-side) are reported via std::runtime_error,
		//   which is handled by the communication loop and causes the connection to be reset.
		//

		template<typename SharedPromiseT, typename RequestFunc, typename... RequestArgs>
		void doCommandRequest(SharedPromiseT&& promise, RequestFunc&& requestFunc, RequestArgs&&... args)
		{
			try
			{
				auto result = std::invoke(std::forward<RequestFunc>(requestFunc), this, std::forward<RequestArgs>(args)...);

				GwErrorCode errorCode = static_cast<GwErrorCode>(result);
				if (errorCode > GwErrorCode::GWC_GATEWAY_SERVICE_ERROR_BASE)
				{
					// Treat GatewayService errors as runtime errors, they likely indicate a problem that cannot be resolved
					// by retrying the command (e.g. invalid request, unsupported protocol version, etc.)
					//
					throw std::runtime_error{std::format("Command failed with gateway error: {}", errorCode)};
				}
				// This result was returned by the TuningService and should be reported to the caller.
				//
				promise->set_value(result);
			}
			catch (const std::runtime_error&)
			{
				promise->set_value(GwErrorCode::GWC_COMMUNICATION_ERROR);

				// Rethrowing exception to the main loop to trigger connection reset, as this error likely indicates a
				// communication problem that need s to be handled by re-establishing the connection.
				//
				throw;
			}
		}

		[[nodiscard]] GwErrorCode requestActivateTuningSource(std::string_view tuningSourceId, bool activate);
		[[nodiscard]] WriteValueResult requestWriteSignalValues(std::span<const GwTuningWriteValue> states,
																std::string_view user,
																bool apply);
		[[nodiscard]] WriteValueResult requestWriteSignalValuesPart(std::span<const GwTuningWriteValue> states,
																	std::string_view user,
																	bool apply);
		[[nodiscard]] GwErrorCode requestApplyWrittenSignals();

		// --
		//
		void clear();
		void updateSignalStates(); // Throws on communication errors

	protected:
		ITuningSignalUpdater& m_signalUpdater;
		std::function<bool()> m_isCancelledFunc;

	protected:
		struct
		{
			TuningGwHandshakeResponse handshakeResponse{};

			Project project{};                          // Content/BuildInfo
			std::vector<TuningSource> tuningSources{};  // Content/DataSources/DataSource
			std::vector<Radiy::Hash> appSignalHashes{}; // Fill this buffer once with the hashes of all signals from TuningSources.xml, then
														// use it in requestSignalStates() to get the states of all signals in one request.
		} m_workset{};

		// Operative buffers used in requests
		//
		mutable std::mutex m_stateMutex;
		struct
		{
			bool clientIsActive{};
			std::vector<GwTuningSourceState> tuningSourceStates{};
		} m_state{};

		// Command queue
		//
		std::mutex m_commandQueueMutex{};
		std::condition_variable_any m_commandQueueCv{};

		std::queue<std::function<void(bool)>> m_commandQueue{}; // arg bool is to cancel command.
	};
} // namespace GatewayClientLib