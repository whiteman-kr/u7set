#include "ModbusTcpSlaveThread.h"

namespace Modbus
{
	// --------------------------------------------------------------------------------------------------------
	//
	// Modbus::TcpSlaveThread::Connection class implementaion
	//
	// --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::Connection::Connection(Listener& listener) :
		m_socket(listener.ioContext()),
		m_listener(listener)
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
			//DEBUG_LOG_ERR(m_log, QString());
			return;
		}

		TcpFrame& request = getRequestRef();

		request.reverseBytes();

		Q_ASSERT(request.header.protocolID == 0);

		if (request.header.length + sizeof(request.header) != bytesReceived)
		{
			Q_ASSERT(false);
			return;
		}

		if (request.modbusDeviceID == m_listener.modbusDeviceID())
		{
			int sendBytesCount = 0;

			switch(request.functionCode)
			{
			case FC_READ_HOLDING_REGISTERS:
				sendBytesCount = onFnReadHoldingRegisters(request);
				break;

			default:
				Q_ASSERT(false);
			}

			if (sendBytesCount > 0)
			{
				Q_ASSERT(sendBytesCount <= SEND_BUFFER_SIZE);
				m_socket.write_some(asio::buffer(m_sendBuffer, sendBytesCount));
			}
		}

		startReceive();
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

		int bytesCount = fn03Request.regsCount * 2;

		TcpFrame& reply = getReplyRef();

		reply.header.transactionID = request.header.transactionID;
		reply.header.protocolID = request.header.protocolID;

		reply.header.length = bytesCount + 2 + 1;

		reply.modbusDeviceID = request.modbusDeviceID;
		reply.functionCode = request.functionCode;

		reply.fn03Reply.bytesCount = bytesCount;

		int sendBytesCount = reply.header.length + 3 * sizeof(quint16);

		reply.reverseBytes();

		return sendBytesCount;
	}

	TcpFrame& TcpSlaveThread::Connection::getRequestRef()
	{
		return *reinterpret_cast<TcpFrame*>(m_receiveBuffer);
	}

	TcpFrame& TcpSlaveThread::Connection::getReplyRef()
	{
		return *reinterpret_cast<TcpFrame*>(m_sendBuffer);
	}

   // --------------------------------------------------------------------------------------------------------
   //
   // Modbus::TcpSlaveThread::Listener class implementaion
   //
   // --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::Listener::Listener(io_context& ioContext,
										const HostAddressPort& listeningAddr,
										int modbusDeviceID,
										std::stop_token stopToken,
										CircularLoggerShared logger) :
		m_ioContext(ioContext),
		m_listeningAddr(listeningAddr),
		m_modbusDeviceID(modbusDeviceID),
		m_stopToken(stopToken),
		m_log(logger),
		m_timer(ioContext),
		m_acceptor(ioContext, tcp::endpoint(
								  ip::address::from_string(listeningAddr.addressStr().toStdString()),
								  listeningAddr.port()))
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

	io_context& TcpSlaveThread::Listener::ioContext()
	{
		return m_ioContext;
	}

	int TcpSlaveThread::Listener::modbusDeviceID() const
	{
		return m_modbusDeviceID;
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
		if (exitIfStopRequested() == true)
		{
			return;
		}

		if (!error)
		{
			//			DEBUG_LOG_MSG(m_log, "Timer");
			// if (m_receivedPerSecond == 0 && m_1second)
			// {
			// 	m_noReceiveCtr++;

				   // 	if (m_noReceiveCtr >= NO_RUP_FRAMES_TIMEOUT)
				   // 	{
				   // 		qDebug() << C_STR(QString("No RUP frames received in %1 seconds").
				   // 						  arg(NO_RUP_FRAMES_TIMEOUT));
				   // 	}
				   // }

				   // if (isSocketWorkable() == false ||
				   // 	m_noReceiveCtr >= NO_RUP_FRAMES_TIMEOUT ||
				   // 	m_socketErrorCtr >= MAX_SOCKET_ERROR_COUNT)
				   // {
				   // 	closeSocket();
				   // 	clearReceiverStatistics();
				   // 	createAndBindSocket();
				   // }

				   // updateReceiverStatistics();
				   // updateDataSourcesStatistics();
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
							 arg(m_listeningAddr.addressPortStr()));
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
		Q_ASSERT(m_acceptedConnections.contains(newConnection) == false);

		DEBUG_LOG_MSG(m_log, QString("Modbus::TcpSlaveThread accept new connection #%1 from %2").
							 arg(newConnection->connectionNo()).
							 arg(newConnection->peerAddress()));

		m_acceptedConnections.emplace(m_newConnection);

		m_newConnection->startReceive();

		m_newConnection.reset();

		startListening();
	}

   // --------------------------------------------------------------------------------------------------------
   //
   // Modbus::TcpSlaveThread class implementaion
   //
   // --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::TcpSlaveThread()
	{
	}

	void TcpSlaveThread::start(const HostAddressPort& listeningAddr,
							   int modbusDeviceID,
							   CircularLoggerShared logger)
	{
		Q_ASSERT(m_thread == nullptr);

		m_thread = new std::jthread(&TcpSlaveThread::run, this, listeningAddr, modbusDeviceID, logger);
	}

	void TcpSlaveThread::stop()
	{
		TEST_PTR_RETURN(m_thread);

		m_thread->request_stop();

		delete m_thread;

		m_thread = nullptr;
	}

	void TcpSlaveThread::run(const HostAddressPort& listeningAddr,
							 int modbusDeviceID,
							 CircularLoggerShared logger)
	{
		DEBUG_LOG_MSG(logger, "Modbus::TcpSlaveThread started");

		try
		{
			io_context ioContext;

			Listener listener(ioContext, listeningAddr, modbusDeviceID, m_thread->get_stop_token(), logger);

			listener.run();
		}

		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

		DEBUG_LOG_MSG(logger, "Modbus::TcpSlaveThread stoped");
	}
}
