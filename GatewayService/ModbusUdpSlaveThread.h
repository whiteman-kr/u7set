#pragma once

#include <thread>
#include <asio.hpp>
#include <QtTypes>
#include "../OnlineLib/CircularLogger.h"
#include "ModbusProtocol.h"

using namespace asio;
using namespace asio::ip;

namespace Gateway
{
	class ModbusSlaveHandler;
}

namespace Modbus
{
	class UdpSlaveThread
	{
	public:
		UdpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusSlaveHandler& handler);

		void start();
		void stop();

	private:
		void run();

		void startTimer();
		void onTimer(const error_code& error);
		bool exitIfStopRequested();

		bool createAndBindSocket();
		bool isSocketWorkable() const;
		void closeSocket();

		void startReceive();
		void onReceiveData(const error_code& error, size_t bytesReceived);
		void initIndexes();

	private:
		HostAddressPort m_recvIP;
		::Gateway::ModbusSlaveHandler& m_handler;
		CircularLoggerShared m_log;

		//

		io_context* m_ioContext = nullptr;

		steady_timer* m_timer = nullptr;

		std::jthread* m_thread = nullptr;
		std::stop_token m_stopToken;

		udp::endpoint m_recvEndpoint;
		udp::socket* m_socket = nullptr;
		bool m_socketBound = false;

		static const size_t RECV_BUFFER_SIZE = 1024;
		quint8 m_recvBuffer[RECV_BUFFER_SIZE];
		udp::endpoint m_recvFromIP;
		int m_recvBufferIndex = 0;
		int m_startMarkerIndex = -1;
	};

}
