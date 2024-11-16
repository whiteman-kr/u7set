#include <cctype>

#include "ModbusUdpSlaveThread.h"
#include "ModbusSlaveGatewayHandler.h"

namespace Modbus
{

	UdpSlaveThread::UdpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusSlaveHandler& handler) :
		m_listeningIP(listeningIP),
		m_handler(handler),
		m_log(handler.log())
	{
		m_listeningEndpoint = udp::endpoint(ip::address::from_string(m_listeningIP.addressStr().toStdString()),
												m_listeningIP.port());
	}

	void UdpSlaveThread::start()
	{
		Q_ASSERT(m_thread == nullptr);

		m_thread = new std::jthread(&UdpSlaveThread::run, this);
	}

	void UdpSlaveThread::stop()
	{
		TEST_PTR_RETURN(m_thread);

		m_thread->request_stop();

		delete m_thread;

		m_thread = nullptr;
	}

	void UdpSlaveThread::run()
	{
		DEBUG_LOG_MSG(m_log, QString("Gateway %1 started").arg(m_handler.gatewayID()));

		m_stopToken = m_thread->get_stop_token();

		bool exit = false;

		while(!exit)
		{
			try
			{
				m_ioContext = new io_context;

				startTimer();
				createAndBindSocket();

				m_ioContext->run();

				exit = true;
			}

			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}

			DELETE_IF_NOT_NULL(m_timer);

			closeSocket();

			DELETE_IF_NOT_NULL(m_ioContext);
		}

		DEBUG_LOG_MSG(m_log, QString("Gateway %1 stoped").arg(m_handler.gatewayID()));
	}

	void UdpSlaveThread::startTimer()
	{
		if (m_timer == nullptr)
		{
			TEST_PTR_RETURN(m_ioContext);
			m_timer = new steady_timer(*m_ioContext);
		}

		m_timer->expires_after(asio::chrono::milliseconds(1000));
		m_timer->async_wait(bind(&UdpSlaveThread::onTimer, this,
								std::placeholders::_1));
	}

	void UdpSlaveThread::onTimer(const error_code& error)
	{
		Q_UNUSED(error);

		if (exitIfStopRequested() == true)
		{
			return;
		}

		if (m_socket == nullptr)
		{
			createAndBindSocket();
		}

		startTimer();
	}

	bool UdpSlaveThread::exitIfStopRequested()
	{
		if (m_stopToken.stop_requested() == false)
		{
			return false;
		}

		if (m_ioContext != nullptr)
		{
			m_ioContext->stop();
		}
		else
		{
			Q_ASSERT(false);
		}

		return true;
	}

	bool UdpSlaveThread::createAndBindSocket()
	{
		Q_ASSERT(isSocketWorkable() == false);

		TEST_PTR_RETURN_FALSE(m_ioContext);

		if (m_socket != nullptr)
		{
			closeSocket();
		}

		m_socket = new udp::socket(*m_ioContext);

		error_code error;

		m_socket->open(asio::ip::udp::v4(), error);

		if (!error)
		{
			m_socketBound = false;

			m_socket->bind(m_listeningEndpoint, error);

			if (!error)
			{
				m_socketBound = true;

				DEBUG_LOG_MSG(m_log, QString("Gateway %1 listening socket created and bound to %2").
											arg(m_handler.gatewayID(), m_listeningIP.addressPortStr()));

				asio::socket_base::receive_buffer_size rxBufferSize(1024);

				m_socket->set_option(rxBufferSize, error);

				if (error)
				{
					DEBUG_LOG_ERR(m_log, QString("Gateway %1 error changing listening socket receive buffer size").
											arg(m_handler.gatewayID()));
				}

				m_socket->get_option(rxBufferSize);

				DEBUG_LOG_MSG(m_log, QString("Gateway %1 listening socket receive buffer size %2 bytes").
											arg(m_handler.gatewayID()).
											arg(rxBufferSize.value()));
				restartScan();
				startReceive();
			}
			else
			{
				DEBUG_LOG_MSG(m_log, QString("Gateway %1 error binding listening socket to %2: %3").
											arg(m_handler.gatewayID()).
											arg(m_listeningIP.addressPortStr()).
											arg(QString::fromStdString(error.message())));

				closeSocket();
			}
		}
		else
		{
			DEBUG_LOG_MSG(m_log, QString("Gateway %1 listening socket opening error: %2").
											arg(m_handler.gatewayID()).
											arg(QString::fromStdString(error.message())));
			closeSocket();
		}

		return isSocketWorkable();
	}

	bool UdpSlaveThread::isSocketWorkable() const
	{
		return	m_socket != nullptr &&
				m_socket->is_open() &&
				m_socketBound == true;
	}

	void UdpSlaveThread::closeSocket()
	{
		if (m_socket != nullptr)
		{
			m_socket->close();
			delete m_socket;
			m_socket = nullptr;

			DEBUG_LOG_MSG(m_log, QString("Gateway %1 close listening socket").
								 arg(m_handler.gatewayID()));
		}

		m_socketBound = false;

		restartScan();
	}

	void UdpSlaveThread::startReceive()
	{
		if (isSocketWorkable() == false)
		{
			closeSocket();
			return;
		}

		m_socket->async_receive_from(asio::buffer(m_tempBuffer, TEMP_BUFFER_SIZE),
													m_recvFromEndpoint,
													bind(&UdpSlaveThread::onReceiveData, this,
															std::placeholders::_1, std::placeholders::_2));
	}

	void UdpSlaveThread::onReceiveData(const error_code& error, size_t bytesReceived)
	{
		if (exitIfStopRequested() == true)
		{
			return;
		}
		
		if (error)
		{
			DEBUG_LOG_MSG(m_log, QString("Gateway %1 data receive error '%2' (%3)").
								arg(m_handler.gatewayID(), QString::fromStdString(error.message())).
								 arg(error.value()));
			closeSocket();
			return;
		}

		QString peerAddr =QString("%1:%2").
								arg(QString::fromStdString(m_recvFromEndpoint.address().to_string())).
								arg(m_recvFromEndpoint.port());

		// QString recvStr;

		// if (bytesReceived < TEMP_BUFFER_SIZE - 1)
		// {
		// 	m_tempBuffer[bytesReceived] = 0;

		// 	recvStr = QString(reinterpret_cast<const char*>(m_tempBuffer));

		// 	qDebug() << C_STR(QString("receive %1 from %2: %3").
		// 					  arg(bytesReceived).arg(peerAddr).arg(recvStr));
		// }

		// General ASCII Modbus message format:
		//
		// ':XXXX ... XXXCRLF'
		//
		// where
		//	':' - ASCII_START_MARKER
		//  X  - hex digits 0123456789ABCDEFabcdef
		//  CR - ASCII_END_MARKER_1
		//  LF - ASCII_END_MARKER_2

		for(size_t i = 0; i < bytesReceived; i++)
		{
			if (i >= TEMP_BUFFER_SIZE ||
				m_recvBufferIndex >= RECV_BUFFER_SIZE)
			{
				restartScan();
				break;
			}

			quint8 ch = m_tempBuffer[i];

			bool skipChar = false;

			switch(ch)
			{
			case ASCII_START_MARKER:
				m_recvBufferIndex = 0;			// start scan and copy Modbus ASCII request into m_recvBuffer
				m_endMarkerCount = 0;
				break;

			case ASCII_END_MARKER_1:
				if (m_recvBufferIndex >= 0 && m_endMarkerCount == 0)
				{
					m_endMarkerCount = 1;
				}
				else
				{
					restartScan();
					skipChar = true;
				}
				break;

			case ASCII_END_MARKER_2:
				if (m_recvBufferIndex >= 0 && m_endMarkerCount == 1)
				{
					m_endMarkerCount = 2;
				}
				else
				{
					restartScan();
					skipChar = true;
				}
				break;

			default:
				// ch is not: ASCII_START_MARKER, ASCII_END_MARKER_1, ASCII_END_MARKER_2
				//
				if (std::isxdigit(ch) == false ||			// char is not hexdigit
					m_recvBufferIndex < 0 ||				// scan still not started
					m_endMarkerCount > 0)					// ASCII_END_MARKER_1 already detected
				{
					{
						restartScan();
						skipChar = true;
					}
				}
			}

			if (skipChar)
			{
				continue;
			}

			if (m_recvBufferIndex >= 0)
			{
				m_recvBuffer[m_recvBufferIndex] = ch;
				m_recvBufferIndex++;

				if (m_endMarkerCount == 2)
				{
					// MbshProcData structure filling

					m_mpd.connNo = 1;
					m_mpd.peerAddr = peerAddr;
					m_mpd.error = error;

					m_mpd.recvBuffer = m_recvBuffer;
					m_mpd.recvBufferSize = RECV_BUFFER_SIZE;
					m_mpd.bytesReceived = m_recvBufferIndex;

					m_mpd.sendBuffer = m_sendBuffer;
					m_mpd.sendBufferSize = SEND_BUFFER_SIZE;
					m_mpd.sendBytes = 0;

					//

					size_t sendBytes = m_handler.asciiRequestProcessing(m_mpd);

					if (sendBytes > 0)
					{
						m_socket->send_to(asio::buffer(m_sendBuffer, sendBytes), m_recvFromEndpoint);
					}

					restartScan();
				}
			}
		}

		//m_socket->send_to(asio::buffer(m_tempBuffer + m_tempBufferIndex, bytesReceived), m_recvFromEndpoint);

		startReceive();
	}

	void UdpSlaveThread::restartScan()
	{
		m_recvBufferIndex = -1;
		m_endMarkerCount = 0;
	}
}
