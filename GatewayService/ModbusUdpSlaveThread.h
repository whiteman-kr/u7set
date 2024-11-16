#pragma once

#include <thread>
#include <asio.hpp>
#include <QtTypes>
#include "../OnlineLib/CircularLogger.h"
#include "ModbusProtocol.h"
#include "ModbusSlaveGatewayHandler.h"

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
		void restartScan();


	private:
		HostAddressPort m_listeningIP;
		udp::endpoint m_listeningEndpoint;
		::Gateway::ModbusSlaveHandler& m_handler;
		CircularLoggerShared m_log;

		//

		io_context* m_ioContext = nullptr;

		steady_timer* m_timer = nullptr;

		std::jthread* m_thread = nullptr;
		std::stop_token m_stopToken;

		udp::socket* m_socket = nullptr;
		bool m_socketBound = false;
		udp::endpoint m_recvFromEndpoint;

		static const int TEMP_BUFFER_SIZE = 512;
		quint8 m_tempBuffer[TEMP_BUFFER_SIZE];

		static const int RECV_BUFFER_SIZE = 256;
		quint8 m_recvBuffer[RECV_BUFFER_SIZE];

		int m_endMarkerCount = 0;
		int m_recvBufferIndex = -1;

		static const int SEND_BUFFER_SIZE = 2048;
		quint8 m_sendBuffer[SEND_BUFFER_SIZE];

		Gateway::MbshProcData m_mpd;
	};

}
