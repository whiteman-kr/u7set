#include "ModbusUdpSlaveThread.h"
#include "ModbusSlaveGatewayHandler.h"

namespace Modbus
{

	UdpSlaveThread::UdpSlaveThread(const HostAddressPort& listeningIP, ::Gateway::ModbusSlaveHandler& handler) :
		m_recvIP(listeningIP),
		m_handler(handler),
		m_log(handler.log())
	{
		m_recvEndpoint = udp::endpoint(ip::address::from_string(m_recvIP.addressStr().toStdString()),
												m_recvIP.port());
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

		startTimer();
	}

	bool UdpSlaveThread::exitIfStopRequested()
	{
		if (m_stopToken.stop_requested() == false)
		{
			return false;
		}

		m_ioContext->stop();

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

			m_socket->bind(m_recvEndpoint, error);

			if (!error)
			{
				m_socketBound = true;

				DEBUG_LOG_MSG(m_log, QString("AppDataReceiver socket created and bound to %1").
									 arg(m_recvIP.addressPortStr()));

				asio::socket_base::receive_buffer_size rxBufferSize(10 * 1024 * 1024);

				m_socket->set_option(rxBufferSize, error);

				if (error)
				{
					DEBUG_LOG_ERR(m_log, "AppDataReceiver error changing udp socket receive buffer size");
				}

				m_socket->get_option(rxBufferSize);

				DEBUG_LOG_MSG(m_log, QString("AppDataReceiver udp socket receive buffer size %1 bytes").
									 arg(rxBufferSize.value()));

				startReceive();
			}
			else
			{
				DEBUG_LOG_MSG(m_log, QString("AppDataReceiver error binding listening socket to %1: %2").
									 arg(m_dataReceivingIP.addressPortStr()).
									 arg(QString::fromStdString(error.message())));

				closeSocket();
			}
		}
		else
		{
			DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket opening error: %1").
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
			m_socketBound = false;
		}
	}

	void UdpSlaveThread::startReceive()
	{
		if (isSocketWorkable() == false)
		{
			closeSocket();
			return;
		}

		m_writeIndex ^= 1;

		m_socket->async_receive_from(asio::buffer(m_receiveBuffer[m_writeIndex], RECV_BUFFER_SIZE),
									 m_receiveFromIP[m_writeIndex],
									 bind(&AppDataReceiver::receivePackets, this,
										  std::placeholders::_1,
										  std::placeholders::_2));
	}

	void AppDataReceiver::receivePackets(const error_code& error, size_t bytesReceived)
	{
		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		if (stopIfQuitRequested() == true)
		{
			return;
		}

		if (error)
		{
			m_socketErrorCtr++;
			return;
		}

		m_noReceiveCtr = 0;

		udp::endpoint receiveFromIP = m_receiveFromIP[m_writeIndex];
		Rup::SimFrame& simFrame = *reinterpret_cast<Rup::SimFrame*>(m_receiveBuffer[m_writeIndex]);

		startReceive();

		m_receivedPerSecond += static_cast<int>(bytesReceived);

		bool isSimFrame = false;
		bool isValidFrame = false;
		bool crcOk = false;
		quint32 sourceIP = 0;

		do
		{
			if (bytesReceived == sizeof(Rup::Frame))
			{
				sourceIP = receiveFromIP.address().to_v4().to_ulong();
				isValidFrame = true;
				break;
			}

			//

			if (bytesReceived == sizeof(Rup::SimFrame))
			{
				if (m_isSimulationMode == false)
				{
					m_errNotExpectedSimPacket++;

					if ((m_errNotExpectedSimPacket % 1000) == 0)
					{
						qDebug() << C_STR(QString("Software is not in SIMULATION mode, %1 sim packets has been ignored.").
										  arg(m_errNotExpectedSimPacket));
					}

					break;
				}

				quint16 simVersion = reverseUint16(simFrame.simVersion);

				if (simVersion != 1)
				{
					m_errSimVersion++;
					break;
				}

				sourceIP = reverseUint32(simFrame.sourceIP);

				m_simFramesCount++;

				isSimFrame = true;
				isValidFrame = true;
				break;
			}

			// received datagram  has unknown size, skip this datagram
			//
			m_errDatagramSize++;
			break;
		}
		while(true);		// Its OK!

		if (isValidFrame == true)
		{
			m_rupFramesReceivedPerSecond++;
			m_rupFramesCount++;

			crcOk = simFrame.rupFrame.checkCRC64();

			if (crcOk == false)
			{
				m_errRupFrameCRC++;
			}

			AppDataSource* source = m_appDataSources.getSourceByIP(sourceIP);

			if (source != nullptr)
			{
				if (crcOk == true)
				{
					source->pushRupFrame(sourceIP, serverTime,
										 isSimFrame, simFrame.rupFrame,
										 source->cachedAppDataUID(), m_thisThread);

					requireBufferProcessing(source);
				}
				else
				{
					source->incErrorFrameCRC();
				}
			}
			else
			{
				m_errUnknownAppDataSourceIP++;

				//			qDebug() << "Unknown IP" << C_STR(HostAddressPort(sourceIP, 0).addressStr());

				if (m_unknownAppDataSourcesIP.contains(sourceIP) == false &&
					m_unknownAppDataSourcesIP.size() < 500)
				{
					m_unknownAppDataSourcesIP.insert(sourceIP);
				}
			}
		}
	}


}
