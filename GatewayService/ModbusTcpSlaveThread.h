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

			void logRequest(const error_code& error, size_t bytesReceived);
			void logAsciiRequest(const error_code& error, size_t bytesReceived);
			void logRtuRequest(const error_code& error, size_t bytesReceived);
			void logTcpRequest(const error_code& error, size_t bytesReceived);

			void logReply(size_t sendBytes);
			void logAsciiReply(size_t sendBytes);
			void logRtuReply(size_t sendBytes);
			void logTcpReply(size_t sendBytes);

			size_t asciiRequestProcessing(size_t bytesReceived);
			size_t rtuRequestProcessing(size_t bytesReceived);
			size_t tcpRequestProcessing(size_t bytesReceived);

			int onFnReadHoldingRegisters(TcpFrame& request);

			TcpFrame& getRequestRef();
			TcpFrame& getReplyRef();

			int onAsciiFnReadHoldingRegisters(quint16 regsStartAddr, quint16 regsCount);

			bool isHexDigits(const quint8* ptr, int len) const;
			quint8 asciiDecodeXX(const quint8* ptr, bool* ok) const;
			quint16 asciiDecodeXXXX(const quint8* ptr, bool* ok) const;
			quint64 asciiDecode(const quint8* ptr, int len, bool* ok) const;
			quint8 asciiDecodeX(quint8 ch, bool* ok) const;

			quint8* asciiEncodeXX(quint8 v8, quint8* ptr);
			quint8* asciiEncodeXXXX(quint16 v16, quint8* ptr);
			quint8 asciiEncodeX(quint8 ch);

			quint8 nonStandardModbusCrcCalculation(const quint8* ptr, int len);

		private:
			Listener& m_listener;
			::Gateway::ModbusTcpSlaveHandler& m_handler;
			tcp::socket m_socket;

			bool m_enableLogging = false;
			QString m_logStr;

			int m_connectionNo = 0;
			bool m_firstStartReceive = true;

			static inline const size_t RECEIVE_BUFFER_SIZE = 1024;
			unsigned char m_receiveBuffer[RECEIVE_BUFFER_SIZE];

			static inline const size_t SEND_BUFFER_SIZE = 1024;
			unsigned char m_sendBuffer[SEND_BUFFER_SIZE];

			inline static const int ASCII_REG_VALUES_COUNT = 256;

			RegisterValue m_asciiRegValues[ASCII_REG_VALUES_COUNT];

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
