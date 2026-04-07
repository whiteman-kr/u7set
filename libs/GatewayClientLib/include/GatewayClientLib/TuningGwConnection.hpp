#pragma once

#include "GwClient.hpp"
#include "GwHash.hpp"
#include "ITuningSignalUpdater.hpp"

#include <cstdint>
#include <future>
#include <memory>
#include <string_view>
#include <thread>


namespace GatewayClientLib
{
	class ILogger;
	class TuningGwConnImpl;

	struct WriteValueResult
	{
		WriteValueResult(GatewayClientLib::GwErrorCode errorCode = GwErrorCode::GWC_SUCCESS) :
			errorCode(errorCode)
		{
		}

		GwErrorCode errorCode{};
		operator GwErrorCode() const { return errorCode; }

		std::vector<GwTuningSignalWriteResult> signalResults;
	};

	// Manages the connection to the TuningGateway and provides methods for sending commands, such as
	// activating or deactivating tuning sources and writing or applying tuning signal values.
	//
	// Threading model: This class is not thread-safe and is intended to be used from a single thread. Communication
	// with the server is performed on a separate thread managed by this class.
	//
	// Command functions are non-blocking. They enqueue commands for transmission to the server, and the result can be
	// retrieved later from the returned `std::future`.
	//
	// The returned `std::future` does not propagate exceptions. Instead, it yields either an error code reported by the
	// Tuning Service through the Gateway, `GwErrorCode::GWC_COMMUNICATION_ERROR` if a communication failure occurs, or
	// `GwErrorCode::GWC_COMMAND_CANCELED` if the command is canceled.
	//
	class TuningGwConnection final
	{
	public:
		TuningGwConnection(GatewayClientLib::ITuningSignalUpdater& signalUpdater, GatewayClientLib::ILogger& logger);
		~TuningGwConnection();

		void connect(std::string_view address, uint16_t port, std::string_view equipmentId);
		void close();

		// Enqueues a command to activate or deactivate control of a tuning source.
		//
		std::future<GatewayClientLib::GwErrorCode> commandSendActivateTuningSource(std::string_view tuningSourceId, bool activate);

		// Enqueues commands to write tuning signal values. The contents of `states` are copied, so the
		// caller does not need to keep them alive after this function returns.
		//
		std::future<GatewayClientLib::WriteValueResult> commandWriteSignalValues(std::span<const GwTuningWriteValue> states,
																				 std::string_view user,
																				 bool apply);

		// Enqueues a command to apply(commit) previously written tuning signal values.
		//
		std::future<GatewayClientLib::GwErrorCode> commandApplyWrittenSignals();

	private:
		ITuningSignalUpdater& m_signalUpdater;
		ILogger& m_logger;

		std::jthread m_thread;
		std::unique_ptr<GatewayClientLib::TuningGwConnImpl> m_conn;
	};
} // namespace GatewayClientLib