#pragma once

#include <string>
#include <thread>

namespace AdsGatewayLib
{
	class IMiniLogger;

	class AdsGwConnection
	{
	public:
		AdsGwConnection(IMiniLogger& logger);

		void connect(std::string_view address, uint16_t port, std::string_view equipmentId);
		void close();

	private:
		IMiniLogger& m_logger;
		std::jthread m_thread;
	};
} // namespace AdsGatewayLib