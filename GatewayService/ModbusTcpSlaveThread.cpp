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

		case ::Gateway::E::ModbusMode::TCP:
			result = tcpRtuRequestProcessing(bytesReceived);
			break;
		}

		startReceive();
	}

	bool TcpSlaveThread::Connection::asciiRequestProcessing(size_t bytesReceived)
	{
		const size_t MIN_REQUEST_SIZE = ASCII_START_MARKER_LEN +	// marker ':'
										ASCII_DEVICE_ID_LEN +		// modbus deviceID 'XX'
										ASCII_FUNCTION_LEN +		// function '03'
										ASCII_REG_START_ADDR_LEN +	// regs start address 'XXXX'
										ASCII_REG_COUNT_LEN +		// regs count 'XXXX'
										ASCII_CRC_LEN +				// CRC 'XX'
										ASCII_END_MARKER_LEN;		// end marker CR+LF

		if (bytesReceived < MIN_REQUEST_SIZE)
		{
			Q_ASSERT(false);
			return false;
		}

		quint8* request = m_receiveBuffer;
		quint8* ptr = request;

		if (*ptr != Modbus::ASCII_START_MARKER)
		{
			Q_ASSERT(false);
			return false;
		}

		bool result = true;

		ptr += Modbus::ASCII_START_MARKER_LEN;

		quint8 modbusDeviceID = asciiDecodeXX(ptr, &result);

		RETURN_IF_FALSE(result);

		if (modbusDeviceID != m_handler.modbusDeviceID())
		{
			return true;			// its Ok, request to another device
		}

		ptr += Modbus::ASCII_DEVICE_ID_LEN;

		quint8 function = asciiDecodeXX(ptr, &result);

		RETURN_IF_FALSE(result);

		if (function != FC_READ_HOLDING_REGISTERS)
		{
			Q_ASSERT(false);
			return false;
		}

		ptr += Modbus::ASCII_FUNCTION_LEN;

		quint16 regsStartAddr = asciiDecodeXXXX(ptr, &result);

		RETURN_IF_FALSE(result);

		ptr += Modbus::ASCII_REG_START_ADDR_LEN;

		quint16 regsCount = asciiDecodeXXXX(ptr, &result);

		RETURN_IF_FALSE(result);

		ptr += Modbus::ASCII_REG_COUNT_LEN;

		quint8 receivedCrc = asciiDecodeXX(ptr, &result);

		RETURN_IF_FALSE(result);

		quint8 calculatedCrc = nonStandardModbusCrcCalculation(request + ASCII_START_MARKER_LEN,
																ASCII_DEVICE_ID_LEN +
																ASCII_FUNCTION_LEN +
																ASCII_REG_START_ADDR_LEN +
																ASCII_REG_COUNT_LEN);
		if (receivedCrc != calculatedCrc)
		{
			return false;
		}

		int sendBytesCount = onAsciiFnReadHoldingRegisters(regsStartAddr, regsCount);

		Q_ASSERT(sendBytesCount <= SEND_BUFFER_SIZE);
		m_socket.write_some(asio::buffer(m_sendBuffer, sendBytesCount));

		return result;
	}

	bool TcpSlaveThread::Connection::rtuRequestProcessing(size_t bytesReceived)
	{
		Q_ASSERT(false);		// not implemented!

		Q_UNUSED(bytesReceived);
		return false;
	}

	bool TcpSlaveThread::Connection::tcpRtuRequestProcessing(size_t bytesReceived)
	{
		TcpFrame& request = getRequestRef();

		request.reverseBytes();

		Q_ASSERT(request.header.protocolID == 0);

		if (request.header.length + sizeof(request.header) != bytesReceived)
		{
			//Q_ASSERT(false);
			return false;
		}

		if (request.modbusDeviceID != m_handler.modbusDeviceID())
		{
			return true;					// its Ok, request to another device
		}

		bool result = true;

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

	int TcpSlaveThread::Connection::onAsciiFnReadHoldingRegisters(quint16 regsStartAddr, quint16 regsCount)
	{
		Q_ASSERT(regsCount <= ASCII_REG_VALUES_COUNT);

		int bytesCount = m_handler.getRegistersValues(regsStartAddr, regsCount,
													  m_asciiRegValues, ASCII_REG_VALUES_COUNT,
													  QThread::currentThread());

		Q_ASSERT(bytesCount == regsCount * sizeof(quint16));
		Q_ASSERT(bytesCount <= 0xFF);

		quint8* ptr = m_sendBuffer;

		*ptr = ':';
		ptr++;

		ptr = asciiEncodeXX(FC_READ_HOLDING_REGISTERS, ptr);

		ptr = asciiEncodeXX(bytesCount, ptr);

		for(quint16 i = 0; i < regsCount; i++)
		{
			ptr = asciiEncodeXXXX(m_asciiRegValues[i], ptr);
		}

		quint8 crc = nonStandardModbusCrcCalculation(m_sendBuffer + ASCII_START_MARKER_LEN,	// skip start marker
													 ASCII_FUNCTION_LEN +
													 ASCII_BYTES_COUNT_LEN +
													 bytesCount * 2);		// 2 chars on 1 byte

		ptr = asciiEncodeXX(crc, ptr);

		*ptr = ASCII_END_MARKER[0];
		ptr++;

		*ptr = ASCII_END_MARKER[1];
		ptr++;

		int sendBytesCount = ptr - m_sendBuffer;

		return sendBytesCount;
	}

	bool TcpSlaveThread::Connection::isHexDigits(const quint8* ptr, int len) const
	{
		int result = 1;

		for(int i = 0; i < len; i++)
		{
			result &= std::isxdigit(ptr[i]);
		}

		return (result == 0 ? false : true);
	}

	quint8 TcpSlaveThread::Connection::asciiDecodeXX(const quint8* ptr, bool* ok) const
	{
		quint64 result = asciiDecode(ptr, sizeof(quint8) * 2, ok);
		Q_ASSERT(result <= 0xFF);
		return static_cast<quint8>(result);
	}

	quint16 TcpSlaveThread::Connection::asciiDecodeXXXX(const quint8* ptr, bool* ok) const
	{
		quint64 result = asciiDecode(ptr, sizeof(quint16) * 2, ok);
		Q_ASSERT(result <= 0xFFFF);
		return static_cast<quint16>(result);
	}

	quint64 TcpSlaveThread::Connection::asciiDecode(const quint8* ptr, int len, bool* ok) const
	{
		TEST_PTR_RETURN_VALUE(ptr, 0);
		TEST_PTR_RETURN_VALUE(ok, 0);

		if(len <= 0 || len > sizeof(quint64) * 2)
		{
			Q_ASSERT(false);
			*ok = false;
			return 0;
		}

		quint64 result = 0;

		for(int i = 0; i < len; i++)
		{
			unsigned char ch = ptr[i];

			ch = asciiDecodeX(ch, ok);

			if (*ok == false)
			{
				return 0;
			}

			result <<= 4;
			result += ch;
		}

		*ok = true;

		return result;
	}

	quint8 TcpSlaveThread::Connection::asciiDecodeX(quint8 ch, bool* ok) const
	{
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
					*ok = false;
					return 0;
				}
			}
		}

		return ch;
	}

	quint8* TcpSlaveThread::Connection::asciiEncodeXX(quint8 v8, quint8* ptr)
	{
		*ptr = asciiEncodeX((v8 >> 4) & 0x0F);
		ptr++;

		*ptr = asciiEncodeX(v8 & 0x0F);
		ptr++;

		return ptr;
	}

	quint8* TcpSlaveThread::Connection::asciiEncodeXXXX(quint16 v16, quint8* ptr)
	{
		ptr = asciiEncodeXX((v16 >> 8) & 0xFF, ptr);
		ptr = asciiEncodeXX(v16 & 0xFF, ptr);

		return ptr;
	}

	quint8 TcpSlaveThread::Connection::asciiEncodeX(quint8 ch)
	{
		if (ch <= 9)
		{
			return ch + '0';
		}

		if (ch <= 0x0F)
		{
			return ch - 0x0A + 'A';
		}

		Q_ASSERT(false);

		return '0';
	}

	quint8 TcpSlaveThread::Connection::nonStandardModbusCrcCalculation(const quint8* ptr, int lenInChars)
	{
		// Non-standart modbus request CRC calculation used on AEC Kozloduy in UIK system.
		//
		// Reverse ingeneered from request making code:
		//
		// static void makeRequest(array<unsigned char>^ request, int chan, bool Ust)
		// {
		// 	request[0] = ':'; //header
		// 	request[1] = '0'; //slave address
		// 	request[2] = '0' + chan;
		// 	request[3] = '0'; //function
		// 	request[4] = '3';
		// 	request[5] = '0'; //start address Hi
		// 	request[6] = '0';
		// 	request[7] = '0'; //start Lo
		// 	request[8] = '0';
		// 	request[9] = '0'; //Number Hi
		// 	request[10]= '0';
		// 	request[11]= '6'; //Number Lo
		// 	request[12]= '6';
		// 	request[13]= '0'; //CRC
		// 	request[14]= '0';
		// 	request[15]= 0x0D;
		// 	request[16]= 0x0A;
		//
		// 	if(Ust)
		// 	{
		// 		request[7] = '6'; //start Lo
		// 		request[8] = '6';
		// 		request[11]= '4'; //Number Lo
		// 		request[12]= '6';
		// 	}
		//
		// ---------------- Non-standard CRC calculation! -----------------------
		//
		//	1. ASCII decoding is used before summing CRC
		//  2. Result is not two's complementing
		//
		//	Standard Modbus LRC calculation see in ModbusProtocol.cpp
		//
		//  //
		//
		// 	unsigned int CRC=0;
		// 	for(int i=1;i<13;i+=2)
		// 		CRC+=uncodeASCII(request[i])*16 + uncodeASCII(request[i+1]);
		// 	CRC = CRC & 0xFF;
		// ----------------------------------------------------------------------
		//
		// 	request[13] = codeASCII(CRC>>4);
		// 	request[14] = codeASCII(CRC&0xF);
		// }

		quint8 crc = 0;
		bool ok = true;

		for(int i = 0; i < lenInChars; i++)
		{
			quint8 ch = asciiDecodeX(ptr[0], &ok);

			Q_ASSERT(ok == true);

			crc += ch;
		}

		return crc;
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
