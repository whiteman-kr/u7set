#pragma once

#include "IAdsSignalUpdater.hpp"

#include <cstdint>
#include <string_view>
#include <thread>

namespace GatewayClientLib
{
	class ILogger;

	// AdsGwConnection - Manages the connection to the ADS Gateway, including handshake, signal retrieval and state updates.
	//
	class AdsGwConnection final
	{
	public:
		AdsGwConnection(IAdsSignalUpdater& signalUpdater, ILogger& logger);
		~AdsGwConnection();

		void connect(std::string_view address, uint16_t port, std::string_view equipmentId);
		void close();

	private:
		IAdsSignalUpdater& m_signalUpdater;
		ILogger& m_logger;

		std::jthread m_thread;
	};
} // namespace GatewayClientLib