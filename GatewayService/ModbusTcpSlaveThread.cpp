#include "ModbusTcpSlaveThread.h"
#include "ModbusSlaveGatewayHandler.h"

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
		m_connNo = m_connectionInstance;
	}

	tcp::socket& TcpSlaveThread::Connection::socket()
	{
		return m_socket;
	}

	QString TcpSlaveThread::Connection::peerAddress() const
	{
		return QString("%1:%2").
					arg(QString::fromStdString(m_socket.remote_endpoint().address().to_string())).
					arg(m_socket.remote_endpoint().port());
	}

	int TcpSlaveThread::Connection::connectionNo() const
	{
		return m_connNo;
	}

	void TcpSlaveThread::Connection::startReceive()
	{
		if (m_firstStartReceive)
		{
			m_peerAddr = peerAddress();
			asio::ip::tcp::no_delay option(true);
			m_socket.set_option(option);
			m_firstStartReceive = false;

			m_handler.logRequest(QString("Gateway %1 accept new connection #%2 from %3").
								 arg(m_handler.gatewayID()).arg(m_connNo).arg(m_peerAddr));
		}

		m_socket.async_receive(asio::buffer(m_handler.recvBuffer(), m_handler.recvBufferSize()),
							   bind(&TcpSlaveThread::Connection::onReceiveData, this,
									std::placeholders::_1,
									std::placeholders::_2));
	}

	void TcpSlaveThread::Connection::onReceiveData(const error_code& error, size_t bytesReceived)
	{
		if (error)
		{
			int ev = error.value();

			if (ev == 2)
			{
				DEBUG_LOG_ERR(m_listener.log(), QString("Gateway %1 remote %2 close connection #%3").
												arg(m_handler.gatewayID(), m_peerAddr).arg(m_connNo));
			}
			else
			{
				DEBUG_LOG_ERR(m_listener.log(), QString("Gateway %1 receive data error '%2' (%3) from %3 on connection  #%4").
												arg(m_handler.gatewayID(), QString::fromStdString(error.message())).
												arg(ev).arg(m_peerAddr).arg(m_connNo));
			}

			m_handler.logReply(QString("Gateway %1 close connection #%2 with %3").
								 arg(m_handler.gatewayID()).arg(m_connNo).arg(m_peerAddr));

			m_listener.removeConnection(m_connNo);
			return;
		}

		if (bytesReceived == 0)
		{
			startReceive();
			return;
		}

		size_t sendBytesCount = 0;

		sendBytesCount = m_handler.tcpRequestProcessing(m_connNo, m_peerAddr, error, bytesReceived);

		if (sendBytesCount > 0)
		{
			if (sendBytesCount <= m_handler.sendBufferSize())
			{
				m_socket.write_some(asio::buffer(m_handler.sendBuffer(), sendBytesCount));
			}
			else
			{
				Q_ASSERT(false);
			}
		}

		startReceive();
	}

   // --------------------------------------------------------------------------------------------------------
   //
   // Modbus::TcpSlaveThread::Listener class implementaion
   //
   // --------------------------------------------------------------------------------------------------------

	TcpSlaveThread::Listener::Listener(const HostAddressPort& listeningIP,
									   ::Gateway::ModbusSlaveHandler& handler,
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

	::Gateway::ModbusSlaveHandler& TcpSlaveThread::Listener::gatewayHandler()
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
			DEBUG_LOG_MSG(m_log, QString("Gateway %1 close connection #%2").
								arg(m_handler.gatewayID()).arg(connectionNo));
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

		DEBUG_LOG_MSG(m_log, QString("Gateway %1 wait conections on %2").
							 arg(m_handler.gatewayID(), m_listeningIP.addressPortStr()));
	}

	void TcpSlaveThread::Listener::onAcceptConnection(ConnectionShared newConnection,
													const error_code& error)
	{
		if (error)
		{
			DEBUG_LOG_ERR(m_log, QString("Gateway %1 accept connection error: %2").
								 arg(m_handler.gatewayID(), QString::fromStdString(error.message())));
			return;
		}

		Q_ASSERT(m_newConnection == newConnection);
		Q_ASSERT(m_acceptedConnections.contains(newConnection->connectionNo()) == false);

		DEBUG_LOG_MSG(m_log, QString("Gateway %1 on %2 accept new connection #%3 from %4").
							 arg(m_handler.gatewayID(), m_listeningIP.addressPortStr()).
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

	TcpSlaveThread::TcpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusSlaveHandler& handler) :
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
		DEBUG_LOG_MSG(m_log, QString("Gateway %1 started").arg(m_handler.gatewayID()));

		bool exit = false;

		while(!exit)
		{
			try
			{
				io_context ioContext;

				Listener listener(m_listeningIP, m_handler, ioContext, m_thread->get_stop_token());

				listener.run();

				exit = true;
			}

			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}

		DEBUG_LOG_MSG(m_log, QString("Gateway %1 stoped").arg(m_handler.gatewayID()));
	}
}
