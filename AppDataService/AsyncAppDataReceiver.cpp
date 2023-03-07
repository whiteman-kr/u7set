#include "AsyncAppDataReceiver.h"

// -------------------------------------------------------------------------------
//
// AsyncAppDataReceiver class implementation
//
// -------------------------------------------------------------------------------

AsyncAppDataReceiver::AsyncAppDataReceiver(const HostAddressPort& dataReceivingIP,
								 const AppDataSourcesIP& appDataSourcesIP,
								 E::SoftwareRunMode swRunMode,
								 CircularLoggerShared log) :
	m_appDataSourcesIP(appDataSourcesIP),
	m_log(log)
{
	m_isSimulationMode = (swRunMode == E::SoftwareRunMode::Simulation);

	m_appDataReceivingIP = udp::endpoint(
								ip::address::from_string(dataReceivingIP.addressStr().toStdString()),
								dataReceivingIP.port());
}

AsyncAppDataReceiver::~AsyncAppDataReceiver()
{
}

void AsyncAppDataReceiver::fillAppDataReceiveState(Network::AppDataReceiveState* adrs)
{
	adrs->set_receivingrate(m_receivingRate);
	adrs->set_udpreceivingrate(m_udpReceivingRate);
	adrs->set_rupframesreceivingrate(m_rupFramesReceivingRate);

	adrs->set_rupframescount(m_rupFramesCount);
	adrs->set_simframescount(m_simFramesCount);

	adrs->set_errdatagramsize(m_errDatagramSize);
	adrs->set_errsimversion(m_errSimVersion);
	adrs->set_errunknownappdatasourceip(m_errUnknownAppDataSourceIP);
	adrs->set_errrupframecrc(m_errRupFrameCRC);

	adrs->set_errnotexpectedsimpacket(m_errNotExpectedSimPacket);
}

void AsyncAppDataReceiver::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is started (receiving IP %1)").
							arg(appDataReceivingIPStr()));

	m_thisThread = QThread::currentThread();

	try
	{
		m_ioContext = new io_context;
		startTimer1s();
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

	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is finished (receiving IP %1)").
							arg(appDataReceivingIPStr()));
}

void AsyncAppDataReceiver::startTimer1s()
{
	TEST_PTR_RETURN(m_ioContext);

	if (m_timer == nullptr)
	{
		m_timer = new steady_timer(*m_ioContext);
	}

	m_timer->expires_after(asio::chrono::seconds(1));
	m_timer->async_wait(bind(&AsyncAppDataReceiver::onTimer1s, this,
						   std::placeholders::_1));
}

void AsyncAppDataReceiver::onTimer1s(const error_code& error)
{
	if (isQuitRequested() == true)
	{
		if (m_ioContext != nullptr)
		{
			m_ioContext->stop();
		}
		else
		{
			Q_ASSERT(false);
		}

		return;
	}

	if (!error)
	{
		if (isSocketWorkable() == false)
		{
			bool res = createAndBindSocket();

			clearStatistics();

			if (res == false)
			{
				return;
			}

			startReceive();
		}
		else
		{
			// socket is workable
			//
			if (m_rupFramesReceivingRate == 0)
			{
				m_noReceiveCtr++;

				if (m_noReceiveCtr == 3)
				{
					qDebug() << "No RUP frames received in 3 seconds";
					closeSocket();
					createAndBindSocket();
				}
			}
		}

		updateStatistics();
	}
	else
	{
		DELETE_IF_NOT_NULL(m_timer);
	}

	startTimer1s();
}

void AsyncAppDataReceiver::clearStatistics()
{
	m_receivingRate = 0;
	m_receivedPerSecond = 0;

	m_udpReceivingRate = 0;
	m_udpReceivedPerSecond = 0;

	m_rupFramesReceivingRate = 0;
	m_rupFramesReceivedPerSecond = 0;
}

void AsyncAppDataReceiver::updateStatistics()
{
	m_receivingRate.store(m_receivedPerSecond);
	m_receivedPerSecond = 0;

	m_udpReceivingRate.store(m_udpReceivedPerSecond);
	m_udpReceivedPerSecond = 0;

	m_rupFramesReceivingRate.store(m_rupFramesReceivedPerSecond);
	m_rupFramesReceivedPerSecond = 0;

	qDebug() << C_STR(QString("Receive RUP frames %1").arg(m_rupFramesReceivingRate));
}

bool AsyncAppDataReceiver::createAndBindSocket()
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

		m_socket->bind(m_appDataReceivingIP, error);

		if (!error)
		{
			m_socketBound = true;

			DEBUG_LOG_MSG(m_log, QString("AsyncAppDataReceiver listening socket is created and bound to %1").
							arg(appDataReceivingIPStr()));
			startReceive();
		}
	}
	else
	{
		DEBUG_LOG_MSG(m_log, QString("AsyncAppDataReceiver listening socket opening error: %1").
						arg(QString::fromStdString(error.message())));

		closeSocket();
	}

	return isSocketWorkable();

/*	qint64 prevServerTime = -1;

	while(isQuitRequested() == false)
	{
		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		if (prevServerTime != -1 && serverTime - prevServerTime < 1000)
		{
			msleep(200);
			continue;
		}

		prevServerTime = serverTime;

		qDebug() << C_STR(QString("Try create AsyncAppDataReceiver listening socket on %1").arg(m_dataReceivingIP.addressPortStr()));

		m_socket = new QUdpSocket();

		bool result = m_socket->bind(m_dataReceivingIP.address(), m_dataReceivingIP.port());

		if (result == false)
		{
			qDebug() << C_STR(QString("AsyncAppDataReceiver listening socket binding error to %1").arg(m_dataReceivingIP.addressPortStr()));

			closeSocket();

			msleep(200);

			continue;
		}

		// bind Ok

		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket is created and bound to %1").arg(m_dataReceivingIP.addressPortStr()));

		QVariant osRecvBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver: OS defined receive buffer size - %1 bytes").arg(osRecvBufSize.toInt()));

		QVariant newRecvBufSize(static_cast<int>(2 * 1024 * 1024));

		m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

		QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

		DEBUG_LOG_MSG(m_log, (QString("AppDataReceiver: new receive buffer size is set - %1 bytes").arg(currentBufSize.toInt())));

		if (newRecvBufSize.toInt() != currentBufSize.toInt())
		{
			qDebug() << "";
			DEBUG_LOG_WRN(m_log, QString("WARNING!!! Receive buffer size is not changed to required size."));
			DEBUG_LOG_MSG(m_log, QString("Try change value of registry key (create if key is not exist)"));
			DEBUG_LOG_MSG(m_log, QString("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters\\DefaultReceiveWindow"));
			qDebug() << "";
		}

		break;
	}

	return m_socket != nullptr; */
}

bool AsyncAppDataReceiver::isSocketWorkable() const
{
	return	m_socket != nullptr &&
			m_socket->is_open() &&
			m_socketBound == true;
}

void AsyncAppDataReceiver::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
		m_socketBound = false;
	}

	qDebug() << "AsyncAppDataReceiver listening socket closed";
}

void AsyncAppDataReceiver::startReceive()
{
	if (isSocketWorkable() == false)
	{
		closeSocket();
		return;
	}

	m_writeIndex ^= 1;

	m_socket->async_receive_from(asio::buffer(m_receiveBuffer[m_writeIndex], RECV_BUFFER_SIZE),
									m_receiveFromIP[m_writeIndex],
									bind(&AsyncAppDataReceiver::receivePackets, this,
										std::placeholders::_1,
										std::placeholders::_2));
}

void AsyncAppDataReceiver::receivePackets(const error_code& error, size_t bytesReceived)
{
	if (error)
	{
		closeSocket();
		return;
	}

	udp::endpoint receiveFromIP = m_receiveFromIP[m_writeIndex];
	Rup::SimFrame& simFrame = *reinterpret_cast<Rup::SimFrame*>(m_receiveBuffer[m_writeIndex]);

	startReceive();

	m_udpReceivedPerSecond++;
	m_receivedPerSecond += static_cast<int>(bytesReceived);

	bool isSimFrame = false;
	bool isValidFrame = false;
	quint32 sourceIP = 0;

	do
	{
		if (bytesReceived == sizeof(Rup::Frame))
		{
			sourceIP = receiveFromIP.address().to_v4().to_ulong();
			isValidFrame = true;
		}
		else
		{
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
			}
			else
			{
				// received datagram  has unknown size, skip this datagram
				//
				m_errDatagramSize++;
				break;
			}
		}

		if (isValidFrame == true && simFrame.rupFrame.checkCRC64() == false)
		{
			m_errRupFrameCRC++;
			isValidFrame = false;
			break;
		}
	} while(false);		// Its Ok!

	if (isValidFrame == true)
	{
		m_rupFramesReceivedPerSecond++;
		m_rupFramesCount++;

		AppDataSourceShared dataSource = m_appDataSourcesIP.value(sourceIP, nullptr);

		if (dataSource != nullptr)
		{
			qint64 serverTime = QDateTime::currentMSecsSinceEpoch();
			dataSource->pushRupFrame(sourceIP, serverTime, isSimFrame, simFrame.rupFrame, m_thisThread);
		}
		else
		{
			m_errUnknownAppDataSourceIP++;

			if (m_unknownAppDataSourcesIP.contains(sourceIP) == false && m_unknownAppDataSourcesIP.size() < 500)
			{
				m_unknownAppDataSourcesIP.insert(sourceIP);
			}
		}
	}
}

QString AsyncAppDataReceiver::appDataReceivingIPStr() const
{
	return QString::fromStdString(m_appDataReceivingIP.address().to_string());
}
