#pragma once

#include <thread>
#include <asio.hpp>
#include "../OnlineLib/CircularLogger.h"
#include "ModbusProtocol.h"

using namespace asio;
using namespace asio::ip;

namespace Gateway
{
	class ModbusTcpSlaveHandler;
}

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

			int onFnReadHoldingRegisters(TcpFrame& request);

			TcpFrame& getRequestRef();
			TcpFrame& getReplyRef();

		private:
			Listener& m_listener;
			::Gateway::ModbusTcpSlaveHandler& m_handler;
			tcp::socket m_socket;

			int m_connectionNo = 0;
			bool m_firstStartReceive = true;

			static inline const int RECEIVE_BUFFER_SIZE = 1024;
			char m_receiveBuffer[RECEIVE_BUFFER_SIZE];

			static inline const int SEND_BUFFER_SIZE = 1024;
			char m_sendBuffer[SEND_BUFFER_SIZE];

			static inline int m_connectionInstance = 0;
		};

		using ConnectionShared = std::shared_ptr<Connection>;

		class Listener
		{
		public:
			Listener(const HostAddressPort& listeningIP,
					::Gateway::ModbusTcpSlaveHandler& handler,
					io_context& ioContext,
					std::stop_token stopToken);
			virtual ~Listener();

			void run();

			::Gateway::ModbusTcpSlaveHandler& gatewayHandler();
			io_context& ioContext();
			CircularLoggerShared log();

			void removeConnection(int connectionNo);

		private:
			bool exitIfStopRequested();

			void startTimer500ms();
			void onTimer500ms(const error_code& error);

			void startListening();
			void onAcceptConnection(ConnectionShared newConnection,
									const error_code& error);

		private:
			HostAddressPort m_listeningIP;
			::Gateway::ModbusTcpSlaveHandler& m_handler;

			io_context& m_ioContext;
			std::stop_token m_stopToken;
			CircularLoggerShared m_log;

			steady_timer m_timer;
			tcp::acceptor m_acceptor;

			ConnectionShared m_newConnection;
			std::map<int, ConnectionShared> m_acceptedConnections;
		};

	public:
		TcpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusTcpSlaveHandler& handler);

		void start();
		void stop();

	private:
		void run();

	private:
		HostAddressPort m_listeningIP;
		::Gateway::ModbusTcpSlaveHandler& m_handler;
		CircularLoggerShared m_log;

		std::jthread* m_thread = nullptr;
	};
}
