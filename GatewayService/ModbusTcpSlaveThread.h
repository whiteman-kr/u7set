#pragma once

#include <thread>
#include <QtTypes>
#include "../OnlineLib/CircularLogger.h"
#include "ModbusProtocol.h"
#include "ModbusSlaveGatewayHandler.h"

#include <asio/ip/tcp.hpp>

using namespace asio::ip;

namespace Modbus
{
	//
	// Master/Slave and Client/Server roles in Modbus-TCP:
	//
	// Modbus Master is TCP Client.	Master (client) sends requests to Slave (server) and receives reply from Slave.
	// Modbus Slave is TCP Server. Slave (server) receive requests from Master (client) and sends relpy to Master.
	//

	class TcpSlaveThread
	{
	private:

		class Listener;

		class Connection
		{
		public:
			Connection(Listener& listener);

			tcp::socket& socket();
			QString peerAddress() const;
			int connectionNo() const;

			void startReceive();

		private:
			void onReceiveData(const error_code& error, size_t bytesReceived);

		private:
			Listener& m_listener;
			::Gateway::ModbusSlaveHandler& m_handler;
			tcp::socket m_socket;
			QString m_peerAddr;

			static inline const size_t RECV_BUFFER_SIZE = 256;
			quint8 m_recvBuffer[RECV_BUFFER_SIZE];

			static inline const size_t SEND_BUFFER_SIZE = 1024;
			quint8 m_sendBuffer[SEND_BUFFER_SIZE];

			int m_connNo = 0;
			bool m_firstStartReceive = true;

			Gateway::MbshProcData m_mpd;

			static inline int m_connectionInstance = 0;
		};

		using ConnectionShared = std::shared_ptr<Connection>;

		class Listener
		{
		public:
			Listener(const HostAddressPort& listeningIP,
					::Gateway::ModbusSlaveHandler& handler,
					io_context& ioContext,
					std::stop_token stopToken);
			virtual ~Listener();

			void run();

			::Gateway::ModbusSlaveHandler& gatewayHandler();
			io_context& ioContext();
			CircularLoggerShared log();

			void removeConnection(int connectionNo);

		private:
			bool exitIfStopRequested();

			void startTimer();
			void onTimer(const error_code& error);

			void startListening();
			void onAcceptConnection(ConnectionShared newConnection,
									const error_code& error);

		private:
			HostAddressPort m_listeningIP;
			::Gateway::ModbusSlaveHandler& m_handler;

			io_context& m_ioContext;
			std::stop_token m_stopToken;
			CircularLoggerShared m_log;

			steady_timer m_timer;
			tcp::acceptor m_acceptor;

			ConnectionShared m_newConnection;
			std::map<int, ConnectionShared> m_acceptedConnections;
		};

	public:
		TcpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusSlaveHandler& handler);

		void start();
		void stop();

	private:
		void run();

	private:
		HostAddressPort m_listeningIP;
		::Gateway::ModbusSlaveHandler& m_handler;
		CircularLoggerShared m_log;

		std::jthread* m_thread = nullptr;
	};
}
