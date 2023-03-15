#include "AppDataReceiver.h"

StdThreadsGuard::StdThreadsGuard()
{
}

StdThreadsGuard::~StdThreadsGuard()
{
	for(auto& p : m_threads)
	{
		p.second.join();
	}
}

void StdThreadsGuard::append(std::thread& thread)
{
	Q_ASSERT(thread.joinable() == true);

	auto thread_id = std::hash<std::thread::id>{}(thread.get_id());

	if (m_threads.contains(thread_id))
	{
		Q_ASSERT(false);
		return;
	}

	auto p = m_threads.insert({thread_id, std::move(thread)});

	Q_ASSERT(p.first->second.joinable() == true);

	Q_ASSERT(thread.joinable() == false);
}

// -------------------------------------------------------------------------------
//
// AppDataReceiver class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiver::AppDataReceiver(const HostAddressPort& dataReceivingIP,
								 AppDataSources& appDataSources,
								 int processingThreadsCount,
								 E::SoftwareRunMode swRunMode,
								 CircularLoggerShared log) :
	m_appDataSources(appDataSources),
	m_processingThreadsCountFromSettings(processingThreadsCount),
	m_log(log)
{
	m_isSimulationMode = (swRunMode == E::SoftwareRunMode::Simulation);

	m_appDataReceivingIP = udp::endpoint(
								ip::address::from_string(dataReceivingIP.addressStr().toStdString()),
								dataReceivingIP.port());

	setObjectName("AppDataReceiver");
}

AppDataReceiver::~AppDataReceiver()
{
}

void AppDataReceiver::fillAppDataReceiveState(Network::AppDataReceiveState* adrs)
{
	adrs->set_receivingspeed(m_receivingSpeed);
	adrs->set_rupframesreceivingspeed(m_rupFramesReceivingSpeed);

	adrs->set_rupframescount(m_rupFramesCount);
	adrs->set_simframescount(m_simFramesCount);

	adrs->set_errdatagramsize(m_errDatagramSize);
	adrs->set_errsimversion(m_errSimVersion);
	adrs->set_errunknownappdatasourceip(m_errUnknownAppDataSourceIP);
	adrs->set_errrupframecrc(m_errRupFrameCRC);

	adrs->set_errnotexpectedsimpacket(m_errNotExpectedSimPacket);
}

void AppDataReceiver::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is started (receiving IP %1)").
							arg(appDataReceivingIPStr()));

	m_thisThread = QThread::currentThread();

	StdThreadsGuard stg;

	startProcessingThreads(stg);

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

	wakeupAllProcessingThreads();

	DELETE_IF_NOT_NULL(m_timer);

	closeSocket();

	DELETE_IF_NOT_NULL(m_ioContext);

	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is finished (receiving IP %1)").
							arg(appDataReceivingIPStr()));
}

void AppDataReceiver::startTimer1s()
{
	TEST_PTR_RETURN(m_ioContext);

	if (m_timer == nullptr)
	{
		m_timer = new steady_timer(*m_ioContext);
	}

	m_timer->expires_after(asio::chrono::seconds(1));
	m_timer->async_wait(bind(&AppDataReceiver::onTimer1s, this,
						   std::placeholders::_1));
}

void AppDataReceiver::onTimer1s(const error_code& error)
{
	if (stopIfQuitRequested() == true)
	{
		return;
	}

	if (!error)
	{
		if (isSocketWorkable() == false)
		{
			createAndBindSocket();
			clearReceiverStatistics();
		}
		else
		{
			// socket is workable
			//
			if (m_rupFramesReceivingSpeed == 0)
			{
				m_noReceiveCtr++;

				if (m_noReceiveCtr == 3)
				{
					m_noReceiveCtr = 0;

					qDebug() << "No RUP frames received in 3 seconds";

					closeSocket();
					clearReceiverStatistics();
					createAndBindSocket();
				}
			}
		}

		updateReceiverStatistics();
		updateDataSourcesStatistics();
	}
	else
	{
		DELETE_IF_NOT_NULL(m_timer);
	}

	startTimer1s();
}

void AppDataReceiver::clearReceiverStatistics()
{
	m_receivingSpeed = 0;
	m_rupFramesReceivingSpeed = 0;
	m_rupFramesCount = 0;
	m_simFramesCount = 0;

	m_errDatagramSize = 0;
	m_errSimVersion = 0;
	m_errUnknownAppDataSourceIP = 0;
	m_errRupFrameCRC = 0;
	m_errNotExpectedSimPacket = 0;

	m_receivedPerSecond = 0;
	m_rupFramesReceivedPerSecond = 0;
}

void AppDataReceiver::updateReceiverStatistics()
{
	m_receivingSpeed = m_receivedPerSecond;
	m_receivedPerSecond = 0;

	m_rupFramesReceivingSpeed = m_rupFramesReceivedPerSecond;
	m_rupFramesReceivedPerSecond = 0;

	qDebug() << C_STR(QString("Receive RUP frames %1/s").arg(m_rupFramesReceivingSpeed));
}

void AppDataReceiver::updateDataSourcesStatistics()
{
	for(AppDataSource* source : m_appDataSources)
	{
		TEST_PTR_CONTINUE(source);

		bool invalidateSignals = source->updateStatistics_1s();

		if (invalidateSignals == true)
		{
			requireSignalsInvalidation(source);
		}
	}
}

bool AppDataReceiver::createAndBindSocket()
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

			DEBUG_LOG_MSG(m_log, QString("AsyncAppDataReceiver socket created and bound to %1").
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

bool AppDataReceiver::isSocketWorkable() const
{
	return	m_socket != nullptr &&
			m_socket->is_open() &&
			m_socketBound == true;
}

void AppDataReceiver::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
		m_socketBound = false;

		qDebug() << "AsyncAppDataReceiver socket closed";
	}
}

void AppDataReceiver::startReceive()
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
		closeSocket();
		return;
	}

	udp::endpoint receiveFromIP = m_receiveFromIP[m_writeIndex];
	Rup::SimFrame& simFrame = *reinterpret_cast<Rup::SimFrame*>(m_receiveBuffer[m_writeIndex]);

	startReceive();

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

		AppDataSource* source = m_appDataSources.getSourceByIP(sourceIP);

		if (source != nullptr)
		{
			source->pushRupFrame(sourceIP, serverTime,
								 isSimFrame, simFrame.rupFrame, m_thisThread);

			requireBufferProcessing(source);
		}
		else
		{
			m_errUnknownAppDataSourceIP++;

			if (m_unknownAppDataSourcesIP.contains(sourceIP) == false &&
				m_unknownAppDataSourcesIP.size() < 500)
			{
				m_unknownAppDataSourcesIP.insert(sourceIP);
			}
		}
	}
}

void AppDataReceiver::requireBufferProcessing(AppDataSource* source)
{
	std::lock_guard lg(m_waitConditionMutex);
	m_requireProcessing.insert({source, true});
	m_processingRequiredCondition.notify_one();
}

void AppDataReceiver::requireSignalsInvalidation(AppDataSource* source)
{
	std::lock_guard lg(m_waitConditionMutex);
	m_requireProcessing.insert({source, false});
	m_processingRequiredCondition.notify_one();
}

void AppDataReceiver::startProcessingThreads(StdThreadsGuard& stg)
{
	int poolSize = m_processingThreadsCountFromSettings;

	int idealThreadCount = QThread::idealThreadCount();

	if (poolSize <= 0 || poolSize > idealThreadCount)
	{
		poolSize = idealThreadCount;
	}

	for(int i = 0; i < poolSize; i++)
	{
		std::thread t(&processPackets, std::ref(*this), i + 1);

		stg.append(t);
	}

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThreadsPool started. Running threads count %1%2").
							arg(poolSize).arg(poolSize == idealThreadCount ? " (ideal)" : ""));
}

void AppDataReceiver::wakeupAllProcessingThreads()
{
	std::lock_guard lg(m_waitConditionMutex);
	m_processingRequiredCondition.notify_all();
}

bool AppDataReceiver::stopIfQuitRequested()
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

		wakeupAllProcessingThreads();

		return true;
	}

	return false;
}

QString AppDataReceiver::appDataReceivingIPStr() const
{
	return QString("%1:%2").
				arg(QString::fromStdString(m_appDataReceivingIP.address().to_string())).
				arg(m_appDataReceivingIP.port());
}

void processPackets(AppDataReceiver& receiver, int threadNumber)
{
	CircularLoggerShared log = receiver.log();

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThread #%1 is started").arg(threadNumber));

	QThread* thisThread = QThread::currentThread();

	auto& waitConditionMutex = receiver.m_waitConditionMutex;
	auto& processingRequiredCondition = receiver.m_processingRequiredCondition;
	auto& requireProcessing = receiver.m_requireProcessing;

	std::unique_lock ul(waitConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		processingRequiredCondition.wait(ul);

		if (receiver.isQuitRequested() == true)
		{
			ul.unlock();
			break;
		}

		// here ul is LOCKED!

		int processingCtr = 0;

		while(true)
		{
			auto it = requireProcessing.begin();

			if (it == requireProcessing.end() ||
				processingCtr >= 20)
			{
				ul.unlock();
				break;
			}

			AppDataSource* source = it->first;;
			bool requireBufferProcessing = it->second;

			requireProcessing.erase(it);

			ul.unlock();

			processingCtr++;

			if (source->takeProcessingOwnership(thisThread) == true)
			{
				if (requireBufferProcessing == true)
				{
					source->parseNextBuffer(thisThread);
				}
				else
				{
					source->invalidateSignals(thisThread);
				}

				source->releaseProcessingOwnership(thisThread);
			}
			else
			{
				// another thread already processing this source
			}

			ul.lock();
		}
	}

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThread #%1 is finished").arg(threadNumber));
}

