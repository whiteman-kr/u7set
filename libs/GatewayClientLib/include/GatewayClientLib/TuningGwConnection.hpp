#pragma once

#include "ITuningSignalUpdater.hpp"

#include <cstdint>
#include <string_view>
#include <thread>

namespace GatewayClientLib
{
	class ILogger;

	class TuningGwConnection final
	{
	public:
		TuningGwConnection(ITuningSignalUpdater& signalUpdater, ILogger& logger);
		~TuningGwConnection();

		void connect(std::string_view address, uint16_t port, std::string_view equipmentId);
		void close();

	private:
		ITuningSignalUpdater& m_signalUpdater;
		ILogger& m_logger;

		std::jthread m_thread;
	};
} // namespace GatewayClientLib