#include "RupFramesReceiver.h"

// -------------------------------------------------------------------------------
//
// RupFramesReceiver class implementation
//
// -------------------------------------------------------------------------------

RupFramesReceiver::RupFramesReceiver(const QString& threadName,
							const HostAddressPort& dataReceivingIP,
							OnlineDataSources& onlineDataSources,
							E::SoftwareRunMode swRunMode,
							CircularLoggerShared log) :
	m_threadName(threadName),
	m_dataReceivingIP(dataReceivingIP),
	m_onlineDataSources(onlineDataSources),
	m_log(log)
{
	m_isSimulationMode = (swRunMode == E::SoftwareRunMode::Simulation);
}

RupFramesReceiver::~RupFramesReceiver()
{
}

void RupFramesReceiver::run()
{
	DEBUG_LOG_MSG(m_log, getLogStr(QString("thread started (receiving IP %1)").
													arg(dataReceivingIPStr())));

	m_thisThread = QThread::currentThread();

	try
	{
		m_ioContext = new io_context;
		startTimer500ms();
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

	DEBUG_LOG_MSG(m_log, getLogStr(QString("thread finished (receiving IP %1)").
													arg(dataReceivingIPStr())));
}

void RupFramesReceiver::startTimer500ms()
{
	TEST_PTR_RETURN(m_ioContext);

	if (m_timer == nullptr)
	{
		m_timer = new steady_timer(*m_ioContext);
	}

	m_timer->expires_after(asio::chrono::milliseconds(TIMER_PEROIOD_MS));
	m_timer->async_wait(bind(&RupFramesReceiver::onTimer500ms, this,
						   std::placeholders::_1));
}

void RupFramesReceiver::onTimer500ms(const error_code& error)
{
	if (stopIfQuitRequested() == true)
	{
		return;
	}

	if (!error)
	{
		if (m_receivedPerSecond == 0)
		{
			m_noReceiveCtr++;

			if (m_noReceiveCtr >= MAX_NO_FRAMES_CTR)
			{
				qDebug() << C_STR(getLogStr(QString("no RUP frames received in %1 milliseconds").
									arg(NO_RUP_FRAMES_TIMEOUT_MS)));
			}
		}

		if (isSocketWorkable() == false ||
			m_noReceiveCtr >= MAX_NO_FRAMES_CTR ||
			m_socketErrorCtr >= MAX_SOCKET_ERROR_COUNT)
		{
			closeSocket();
			clearReceiverStatistics();
			createAndBindSocket();
		}

		updateReceiverStatistics();
		m_onlineDataSources.updateDataSourcesStatistics500ms(m_1second);
	}
	else
	{
		DELETE_IF_NOT_NULL(m_timer);
	}

	m_1second ^= true;

	startTimer500ms();
}

void RupFramesReceiver::clearReceiverStatistics()
{
	m_noReceiveCtr = 0;
	m_socketErrorCtr = 0;

	m_receivingSpeed = 0;
	m_rupFramesReceivingSpeed = 0;
	m_rupFramesCount = 0;
	m_simFramesCount = 0;

	m_errDatagramSize = 0;
	m_errSimVersion = 0;
	m_errUnknownDataSourceIP = 0;
	m_errRupFrameCRC = 0;
	m_errNotExpectedSimPacket = 0;

	m_receivedPerSecond = 0;
	m_rupFramesReceivedPerSecond = 0;
}

void RupFramesReceiver::updateReceiverStatistics()
{
	qint64 now = QDateTime::currentMSecsSinceEpoch();

	if (m_lastUpdateTime == 0)
	{
		m_lastUpdateTime = now;
	}
	else
	{
		qint64 dt = now - m_lastUpdateTime;

		if (dt > 900)
		{
			m_receivingSpeed = static_cast<int>((m_receivedPerSecond * 1000.0) / dt);
			m_receivedPerSecond = 0;

			m_rupFramesReceivingSpeed = static_cast<int>((m_rupFramesReceivedPerSecond * 1000.0) / dt);
			m_rupFramesReceivedPerSecond = 0;

			qDebug() << C_STR(getLogStr(QString("receive RUP frames %1/s").arg(m_rupFramesReceivingSpeed)));

			m_lastUpdateTime = now;
		}
	}
}

bool RupFramesReceiver::createAndBindSocket()
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

		udp::endpoint recvEndpoint(ip::address::from_string(m_dataReceivingIP.addressStr().toStdString()),
									m_dataReceivingIP.port());

		m_socket->bind(recvEndpoint, error);

		if (!error)
		{
			m_socketBound = true;

			DEBUG_LOG_MSG(m_log, getLogStr(QString("socket created and bound to %1").
														arg(dataReceivingIPStr())));

			asio::socket_base::receive_buffer_size rxBufferSize(10 * 1024 * 1024);

			m_socket->set_option(rxBufferSize, error);

			if (error)
			{
				DEBUG_LOG_ERR(m_log, getLogStr(QString("error changing socket receive buffer size").
														arg(m_threadName)));
			}

			m_socket->get_option(rxBufferSize);

			DEBUG_LOG_MSG(m_log, getLogStr(QString("socket receive buffer size %1 bytes").
														arg(rxBufferSize.value())));

			startReceive();
		}
		else
		{
			DEBUG_LOG_MSG(m_log, getLogStr(QString("error binding listening socket to %1: %2").
														arg(m_dataReceivingIP.addressPortStr()).
														arg(QString::fromStdString(error.message()))));

			closeSocket();
		}
	}
	else
	{
		DEBUG_LOG_MSG(m_log, getLogStr(QString("listening socket opening error: %1").
						arg(QString::fromStdString(error.message()))));

		closeSocket();
	}

	return isSocketWorkable();
}

bool RupFramesReceiver::isSocketWorkable() const
{
	return	m_socket != nullptr &&
			m_socket->is_open() &&
			m_socketBound == true;
}

void RupFramesReceiver::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
		m_socketBound = false;
	}
}

void RupFramesReceiver::startReceive()
{
	if (isSocketWorkable() == false)
	{
		closeSocket();
		return;
	}

	udp::endpoint* receiveIntoFromIP = nullptr;
	char* receiveIntoBuf = nullptr;

	m_receiveBufIndex ^= 1;

	if (m_receiveBufIndex == 0)
	{
		receiveIntoFromIP = &m_receiveFromIP0;
		receiveIntoBuf = m_receiveBuffer0;
	}
	else
	{
		receiveIntoFromIP = &m_receiveFromIP1;
		receiveIntoBuf = m_receiveBuffer1;
	}

	m_socket->async_receive_from(asio::buffer(	receiveIntoBuf, RECV_BUFFER_SIZE),
												*receiveIntoFromIP,
												bind(&RupFramesReceiver::receivePackets, this,
												std::placeholders::_1,
												std::placeholders::_2));
}

void RupFramesReceiver::receivePackets(const error_code& error, size_t bytesReceived)
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

	udp::endpoint* receiveFromIP = nullptr;
	char* receiveBuf = nullptr;

	if (m_receiveBufIndex == 0)
	{
		receiveFromIP = &m_receiveFromIP0;
		receiveBuf = m_receiveBuffer0;
	}
	else
	{
		receiveFromIP = &m_receiveFromIP1;
		receiveBuf = m_receiveBuffer1;
	}

	Rup::SimFrame& simFrame = *reinterpret_cast<Rup::SimFrame*>(receiveBuf);

	startReceive();

	m_receivedPerSecond += static_cast<int>(bytesReceived);

	bool isSimFrame = false;
	bool isValidFrame = false;
	quint32 sourceIP = 0;

	do
	{
		if (bytesReceived == sizeof(Rup::Frame))
		{
			sourceIP = receiveFromIP->address().to_v4().to_ulong();
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
					qDebug() << C_STR(getLogStr(QString("software is NOT in SIMULATION mode, %1 sim packets has been ignored.").
									  arg(m_errNotExpectedSimPacket)));
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

		bool res = m_onlineDataSources.pushRupFrame(sourceIP, serverTime, isSimFrame, simFrame.rupFrame, m_thisThread);

		if (res == false)
		{
			// unknown sourceIP
			//
			m_errUnknownDataSourceIP++;

			if (m_unknownDataSourcesIP.size() < 100)
			{
				m_unknownDataSourcesIP.insert(sourceIP);
			}
		}
	}
}

bool RupFramesReceiver::stopIfQuitRequested()
{
	if (isQuitRequested() == false)
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

QString RupFramesReceiver::dataReceivingIPStr() const
{
	return m_dataReceivingIP.addressPortStr();
}

QString RupFramesReceiver::getLogStr(const QString& str)
{
	return m_threadName + ": " + str;
}

//void RupFramesReceiver::trace_dt(const QString& portID)
//{
//	if (portID.isEmpty() || portID == "SYSTEMID_RACK01_FSCC01_MD00_ETHERNET02")
//	{
//		qint64 curTime = QDateTime::currentMSecsSinceEpoch();

//		if (m_prevPacketTime != 0 )
//		{
//			qint64 dt = curTime - m_prevPacketTime;

//			if (dt < 4 || dt > 6)
//			{
//				qDebug() << "dt =" << dt;
//			}
//		}

//		m_prevPacketTime = curTime;
//	}
//}
