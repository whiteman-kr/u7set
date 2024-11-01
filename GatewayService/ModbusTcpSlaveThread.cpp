#include "ModbusTcpSlaveThread.h"
#include "ModbusTcpSlaveGatewayHandler.h"

namespace Modbus
{
	// --------------------------------------------------------------------------------------------------------
	//
	// Modbus::TcpSlaveThread::Connection class implementaion
	//
	// --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::Connection::Connection(Listener& listener) :
		m_socket(listener.ioContext()),
		m_listener(listener),
		m_handler(listener.gatewayHandler())
	{
		m_connectionInstance++;
		m_connectionNo = m_connectionInstance;
	}

	tcp::socket& TcpSlaveThread::Connection::socket()
	{
		return m_socket;
	}

	QString TcpSlaveThread::Connection::peerAddress() const
	{
		return QString::fromStdString(m_socket.remote_endpoint().address().to_string());
	}

	int TcpSlaveThread::Connection::connectionNo() const
	{
		return m_connectionNo;
	}

	void TcpSlaveThread::Connection::startReceive()
	{
		m_socket.async_receive(asio::buffer(m_receiveBuffer, RECEIVE_BUFFER_SIZE),
							   bind(&TcpSlaveThread::Connection::onReceiveData, this,
									std::placeholders::_1,
									std::placeholders::_2));
	}

	void TcpSlaveThread::Connection::onReceiveData(const error_code& error, size_t bytesReceived)
	{
		if (error)
		{
			DEBUG_LOG_ERR(m_listener.log(), QString("TcpSlaveThread::Connection::onReceiveData error: %1").
											arg(QString::fromStdString(error.message())));
			m_listener.removeConnection(m_connectionNo);
			return;
		}

		bool result = true;

		switch(m_handler.modbusMode())
		{
		case ::Gateway::E::ModbusMode::ASCII:
			result = asciiRequestProcessing(bytesReceived);
			break;

		case ::Gateway::E::ModbusMode::RTU:
			result = rtuRequestProcessing(bytesReceived);
			break;

		case ::Gateway::E::ModbusMode::TCP_RTU:
			result = tcpRtuRequestProcessing(bytesReceived);
			break;
		}

		startReceive();
	}

	bool TcpSlaveThread::Connection::asciiRequestProcessing(size_t bytesReceived)
	{
		const size_t MIN_REQUEST_SIZE = 1 +		// start marker ':'
										2 +		// slave address 'XX'
										2 +		// function '03'
										4 +		// regs start address 'XXXX'
										4 +		// regs count 'XXXX'
										2 +		// CRC 'XX'
										2;		// end marker CR+LF

		if (bytesReceived < MIN_REQUEST_SIZE)
		{
			Q_ASSERT(false);
			return false;
		}

		unsigned char* request = m_receiveBuffer;

		if (isHexDigits(request + ASCII_START_MARKER_LEN,
						bytesReceived - ASCII_START_MARKER_LEN - ASCII_END_MARKER_LEN) == false)
		{
			Q_ASSERT(false);
			return false;
		}

		if (request[0] != ASCII_START_MARKER)
		{
			Q_ASSERT(false);
			return false;
		}

		bool result = true;

		if ()

		return result;
	}

	bool TcpSlaveThread::Connection::rtuRequestProcessing(size_t bytesReceived)
	{
		Q_ASSERT(false);		// not implemented!
		return false;
	}

	bool TcpSlaveThread::Connection::tcpRtuRequestProcessing(size_t bytesReceived)
	{
		TcpFrame& request = getRequestRef();

		request.reverseBytes();

		Q_ASSERT(request.header.protocolID == 0);

		if (request.header.length + sizeof(request.header) != bytesReceived)
		{
			Q_ASSERT(false);
			return false;
		}

		bool result = true;

		if (request.modbusDeviceID == m_handler.modbusDeviceID())
		{
			int sendBytesCount = 0;

			switch(request.functionCode)
			{
			case FC_READ_HOLDING_REGISTERS:
				sendBytesCount = onFnReadHoldingRegisters(request);
				break;

			default:
				DEBUG_LOG_ERR(m_listener.log(), QString("TcpSlaveThread::Connection::onReceiveData: unknown modbus function code %1. Request ignored.").
												arg(request.functionCode));
				result = false;
			}

			if (result == true && sendBytesCount > 0)
			{
				Q_ASSERT(sendBytesCount <= SEND_BUFFER_SIZE);
				m_socket.write_some(asio::buffer(m_sendBuffer, sendBytesCount));
			}
		}

		return result;
	}

	int TcpSlaveThread::Connection::onFnReadHoldingRegisters(TcpFrame& request)
	{
		if (request.functionCode != FC_READ_HOLDING_REGISTERS)
		{
			Q_ASSERT(false);
			return 0;
		}

		Fn03_ReadHoldingRegisters_Request& fn03Request = request.fn03Request;

		fn03Request.reverseBytes();

		// Human readable value for regsStartAddr == 1 in request decremented by 1, i.e. send as 0!
		//
		int regsStartAddr = fn03Request.regsStartAddr;
		int regsCount = fn03Request.regsCount;

		Q_ASSERT(regsCount <= 127);

		TcpFrame& reply = getReplyRef();

		int bytesCount = m_handler.getRegistersValues(regsStartAddr, regsCount,
											reply.fn03Reply.regValues, FN03_MAX_REGS_COUNT,
											QThread::currentThread());
		Q_ASSERT(bytesCount < 256);

		// copy request header fields to reply
		//
		reply.header.transactionID = request.header.transactionID;
		reply.header.protocolID = request.header.protocolID;

		// set size of reply data after header
		//
		reply.header.length = sizeof(reply.modbusDeviceID) +
							  sizeof(reply.functionCode) +
							  sizeof(reply.fn03Reply.bytesCount) +
							  bytesCount;

		// copy request function params to reply
		//
		reply.modbusDeviceID = request.modbusDeviceID;
		reply.functionCode = request.functionCode;

		// fill reply bytes count
		//
		reply.fn03Reply.bytesCount = static_cast<quint8>(bytesCount);

		//

		int sendBytesCount = sizeof(reply.header) + reply.header.length;

		reply.reverseBytes();		// translate header fields to BE

		return sendBytesCount;
	}

	TcpFrame& TcpSlaveThread::Connection::getRequestRef()
	{
		Q_ASSERT(sizeof(Modbus::TcpFrame) < sizeof(m_receiveBuffer));
		return *reinterpret_cast<TcpFrame*>(m_receiveBuffer);
	}

	TcpFrame& TcpSlaveThread::Connection::getReplyRef()
	{
		Q_ASSERT(sizeof(Modbus::TcpFrame) < sizeof(m_sendBuffer));
		return *reinterpret_cast<TcpFrame*>(m_sendBuffer);
	}

	bool TcpSlaveThread::Connection::isHexDigits(const unsigned char* ptr, int len) const
	{
		int result = 1;

		for(int i = 0; i < len; i++)
		{
			result &= std::isxdigit(ptr[i]);
		}

		return (result == 0 ? false : true);
	}

	quint8 TcpSlaveThread::Connection::asciiDecodeXX(const unsigned char* ptr)
	{
		Q_ASSERT(std::isxdigit(ptr[0]));		// high
		Q_ASSERT(std::isxdigit(ptr[1]));		// low

		quint8 result = ptr[0] - '0';

		result <<= 4;

		result += ptr[1] - '0';

		return result;
	}

	quint16 TcpSlaveThread::Connection::asciiDecodeXXXX(unsigned char* ptr)
	{
'lvme;rbmew;rbmbbrer
	}

	quint64 asciiDecode(const unsigned char* ptr, int len) const
	{
		TEST_PTR_RETURN_VALUE(ptr, 0);

		if(len <= 0 || len > sizeof(quint64) * 2)
		{
			Q_ASSERT(false);
			return 0;
		}

		quint64 result = 0;

		for(int i = 0; i < len; i++)
		{
			unsigned char ch = ptr[i];

			if (ch >= '0' && ch <= '9')
			{
				ch -= '0';
			}
			else
			{
				if (ch >= 'A' && ch <= 'F')
				{
					ch = 0x0A + ch - 'A';
				}
				else
				{
					if (ch >= 'a' && ch <= 'f')
					{
						ch = 0x0A + ch - 'a';
					}
					else
					{
						Q_ASSERT(false);		// ch is not a hex digit!
						return 0;
					}
				}
			}

			result <<= 4;
			result += ch;
		}

		return result;
	}

   // --------------------------------------------------------------------------------------------------------
   //
   // Modbus::TcpSlaveThread::Listener class implementaion
   //
   // --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::Listener::Listener(const HostAddressPort& listeningIP,
									   ::Gateway::ModbusTcpSlaveHandler& handler,
										io_context& ioContext,
										std::stop_token stopToken) :
		m_listeningIP(listeningIP),
		m_handler(handler),
		m_ioContext(ioContext),
		m_stopToken(stopToken),
		m_log(handler.log()),
		m_timer(ioContext),
		m_acceptor(ioContext, tcp::endpoint(ip::address::from_string(listeningIP.addressStr().toStdString()),
											listeningIP.port()))
	{
	}

	TcpSlaveThread::Listener::~Listener()
	{
	}

	void TcpSlaveThread::Listener::run()
	{
		startTimer500ms();
		startListening();

		m_ioContext.run();
	}

	::Gateway::ModbusTcpSlaveHandler& TcpSlaveThread::Listener::gatewayHandler()
	{
		return m_handler;
	}

	io_context& TcpSlaveThread::Listener::ioContext()
	{
		return m_ioContext;
	}

	CircularLoggerShared TcpSlaveThread::Listener::log()
	{
		return m_log;
	}

	void TcpSlaveThread::Listener::removeConnection(int connectionNo)
	{
		size_t removedCount = m_acceptedConnections.erase(connectionNo);

		if (removedCount == 1)
		{
				DEBUG_LOG_MSG(m_log, QString("TcpSlaveThread::Listener connection #%1 removed").
								 arg(connectionNo));
		}
		else
		{
			Q_ASSERT(false);
		}
	}

	bool TcpSlaveThread::Listener::exitIfStopRequested()
	{
		if (m_stopToken.stop_requested() == false)
		{
			return false;
		}

		m_ioContext.stop();

		return true;
	}

	void TcpSlaveThread::Listener::startTimer500ms()
	{
		m_timer.expires_after(asio::chrono::milliseconds(1000));
		m_timer.async_wait(bind(&TcpSlaveThread::Listener::onTimer500ms, this,
								std::placeholders::_1));
	}

	void TcpSlaveThread::Listener::onTimer500ms(const error_code& error)
	{
		Q_UNUSED(error);

		if (exitIfStopRequested() == true)
		{
			return;
		}

		startTimer500ms();
	}

	void TcpSlaveThread::Listener::startListening()
	{
		Q_ASSERT(m_newConnection == nullptr);

		m_newConnection = std::make_shared<Connection>(*this);

		m_acceptor.async_accept(m_newConnection->socket(),
								std::bind(&Listener::onAcceptConnection, this, m_newConnection,
										  std::placeholders::_1));

		DEBUG_LOG_MSG(m_log, QString("Modbus::TcpSlaveThread wait conections on %1").
							 arg(m_listeningIP.addressPortStr()));
	}

	void TcpSlaveThread::Listener::onAcceptConnection(ConnectionShared newConnection,
													const error_code& error)
	{
		if (error)
		{
			DEBUG_LOG_ERR(m_log, QString("Modbus::TcpSlaveThread::Listener::onAcceptConnection error: %1").
								 arg(QString::fromStdString(error.message())));
			return;
		}

		Q_ASSERT(m_newConnection == newConnection);
		Q_ASSERT(m_acceptedConnections.contains(newConnection->connectionNo()) == false);

		DEBUG_LOG_MSG(m_log, QString("Modbus::TcpSlaveThread on %1 accept new connection #%2 from %3").
							 arg(m_listeningIP.addressPortStr()).
							 arg(newConnection->connectionNo()).
							 arg(newConnection->peerAddress()));

		m_acceptedConnections.emplace(newConnection->connectionNo(), m_newConnection);

		m_newConnection->startReceive();

		m_newConnection.reset();

		startListening();
	}

   // --------------------------------------------------------------------------------------------------------
   //
   // Modbus::TcpSlaveThread class implementaion
   //
   // --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::TcpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusTcpSlaveHandler& handler) :
		m_listeningIP(listeningIP),
		m_handler(handler),
		m_log(handler.log())
	{
	}

	void TcpSlaveThread::start()
	{
		Q_ASSERT(m_thread == nullptr);

		m_thread = new std::jthread(&TcpSlaveThread::run, this);
	}

	void TcpSlaveThread::stop()
	{
		TEST_PTR_RETURN(m_thread);

		m_thread->request_stop();

		delete m_thread;

		m_thread = nullptr;
	}

	void TcpSlaveThread::run()
	{
		DEBUG_LOG_MSG(m_log, "Modbus::TcpSlaveThread started");

		try
		{
			io_context ioContext;

			Listener listener(m_listeningIP, m_handler, ioContext, m_thread->get_stop_token());

			listener.run();
		}

		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

		DEBUG_LOG_MSG(m_log, "Modbus::TcpSlaveThread stoped");
	}
}
