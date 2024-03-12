#pragma once

#include <thread>
#include <chrono>

#include "../OnlineLib/CircularLogger.h"

#include "../asio/include/asio.hpp"

using namespace asio;
using namespace asio::ip;

namespace Modbus
{
	inline const quint8 FC_READ_COILS = 0x01;					// Read digital outputs and switching outputs
	inline const quint8 FC_READ_DISCRETE_INPUTS = 0x02;			// Read digital inputs and switching operations
	inline const quint8 FC_READ_HOLDING_REGISTERS = 0x03;		// Read holding registers

#pragma pack(push, 1)

	struct TcpHeader
	{
		quint16 transactionID = 0;
		quint16 protocolID = 0;				// always 0 for Modbus-TCP
		quint16 length = 0;					// length in bytes == sizeof(slaveID) + sizeof(functionCode) + sizeof(modbusData)
	};

	struct TcpFrame
	{
		TcpHeader header;

		quint8	slaveID = 0;
		quint8	functionCode = 0;			// FC_* values

		quint8	modbusData[1];				// variable length
	};

#pragma pack(pop)

	quint8 LRC (const quint8* data, int dataLength);		// Modbus ASCII mode LRC calculation
	quint16 CRC16 (const quint8 *data, int dataLength);		// Modbus RTU mode CRC16 calculation

	//
	// Master/Slave and Client/Server roles in in Modbus-TCP:
	//
	// Modbus Master is TCP Client.	Master (client) sends requests to Slave (server) and receives reply from Slave.
	// Modbus Slave is TCP Server. Slave (server) receive requests from Master (client) and sends relpy to Master.
	//

	class TcpSlaveThread
	{
	private:

		class Worker;

		class Connection
		{
		public:
			Connection(Worker& worker);

			tcp::socket& socket();
			QString peerAddress();

			void startReceive();

		private:
			void onReceiveData(const error_code& error, size_t bytesReceived);

		private:
			Worker& m_worker;
			tcp::socket m_socket;

			static inline const int RECEIVE_BUFFER_SIZE = 256;
			char m_receiveBuffer[RECEIVE_BUFFER_SIZE];
		};

		using ConnectionShared = std::shared_ptr<Connection>;

		class Worker
		{
		public:
			Worker(io_context& ioContext,
				   const HostAddressPort& listeningAddr,
				   std::stop_token stopToken,
				   CircularLoggerShared logger);
			virtual ~Worker();

			void run();

			io_context& ioContext();

		private:
			bool exitIfStopRequested();

			void startTimer500ms();
			void onTimer500ms(const error_code& error);

			void startListening();
			void onAcceptConnection(ConnectionShared newConnection,
									const error_code& error);

		private:
			io_context& m_ioContext;
			HostAddressPort m_listeningAddr;
			std::stop_token m_stopToken;
			CircularLoggerShared m_log;

			steady_timer m_timer;
			tcp::acceptor m_acceptor;

			ConnectionShared m_newConnection;
			std::set<ConnectionShared> m_acceptedConnections;
		};

	public:
		TcpSlaveThread();

		void start(const HostAddressPort& listeningAddr, CircularLoggerShared logger);
		void stop();

	private:
		void run(const HostAddressPort& listeningAddr, CircularLoggerShared logger);

	private:
		std::jthread* m_thread = nullptr;
	};
}
