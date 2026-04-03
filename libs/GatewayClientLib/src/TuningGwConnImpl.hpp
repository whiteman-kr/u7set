#pragma once

#include "GwConnImpl.hpp"
#include "TuningSources.hpp"

#include <GatewayClientLib/ITuningSignalUpdater.hpp>

#include <queue>

namespace GatewayClientLib
{
	class TuningGwConnImpl : public GwConnImpl<TuningGwRequestId>
	{
	public:
		TuningGwConnImpl(ITuningSignalUpdater& signalUpdater, ILogger& logger);

		void run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId) override;

		// Enqueues a command to activate or deactivate control for a tuning source. The command will be sent to the server in the next
		// request cycle.
		//
		void commandSendActivateTuningSource(uint64_t sourceId, bool activate);

		// Enqueues commands to write tuning signal values. The commands will be sent to the server in the next request cycle.
		//
		void commandWriteSignalValues(std::span<const GwTuningSignalState> states);

		// Enqueues a command to apply(commit) previously written tuning signal values. The command will be sent to the server in the next
		// request cycle.
		//
		void commandApplyWrittenSignalValues();

		// Requests: All requests throw std::runtime_error on communication errors.
		//
	protected:
		void requestHandshake(std::string_view equipmentId, uint16_t protocolVersion = TUNING_GW_PROTOCOL_VERSION);

		void requestTuningSources();
		void requestTuningSourceStates();

		[[nodiscard]] std::vector<GwTuningSignalState> requestSignalStates(std::span<const Radiy::Hash> appSignals);


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
		struct
		{
			std::vector<GwTuningSourceState> tuningSourceStates{};
		} m_state{};

		// Command queue
		//
		std::mutex m_commandQueueMutex{};
		std::condition_variable_any m_commandQueueCv{};

		std::queue<std::function<void()>> m_commandQueue{};
	};
} // namespace GatewayClientLib