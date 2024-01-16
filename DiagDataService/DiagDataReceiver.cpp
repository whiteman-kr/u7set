#include "DiagDataReceiver.h"

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
// DiagDataReceiver class implementation
//
// -------------------------------------------------------------------------------

DiagDataReceiver::DiagDataReceiver(const HostAddressPort& dataReceivingIP,
								 DiagDataSources& diagDataSources,
								 DynamicDiagSignalStates& signalStates,
								 int processingThreadsCount,
								 E::SoftwareRunMode swRunMode,
								 CircularLoggerShared log) :
	m_dataReceivingIP(dataReceivingIP),
	m_diagDataSources(diagDataSources),
	m_processingThreadsCountFromSettings(processingThreadsCount),
	m_log(log),
	m_statesProcessingThread(signalStates, log)
{
	setObjectName("DiagDataReceiver");

	m_isSimulationMode = (swRunMode == E::SoftwareRunMode::Simulation);

	m_diagDataReceivingIP = udp::endpoint(
								ip::address::from_string(dataReceivingIP.addressStr().toStdString()),
								dataReceivingIP.port());

	for(DiagDataSource* diagDataSource : diagDataSources)
	{
		diagDataSource->setStatesProcessingThreadWakeupParams(&m_statesProcessigRequiredMutex,
															 &m_statesProcessingRequiredCondition,
															 &m_statesProcessingRequired);
	}
}

DiagDataReceiver::~DiagDataReceiver()
{
}

//void DiagDataReceiver::fillAppDataReceiveState(Network::AppDataReceiveState* adrs)
//{
//	TEST_PTR_RETURN(adrs);

//	adrs->set_receivingspeed(m_receivingSpeed);
//	adrs->set_rupframesreceivingspeed(m_rupFramesReceivingSpeed);

//	adrs->set_rupframescount(m_rupFramesCount);
//	adrs->set_simframescount(m_simFramesCount);

//	adrs->set_errdatagramsize(m_errDatagramSize);
//	adrs->set_errsimversion(m_errSimVersion);
//	adrs->set_errunknownappdatasourceip(m_errUnknownAppDataSourceIP);
//	adrs->set_errrupframecrc(m_errRupFrameCRC);
//	adrs->set_errnotexpectedsimpacket(m_errNotExpectedSimPacket);
//}

//void DiagDataReceiver::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
//													bool isArchivingQueue,
//													const QString& description)
//{
//	m_statesProcessingThread.registerDestSignalStatesQueue(destQueue, isArchivingQueue, description);
//}

//void DiagDataReceiver::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
//{
//	m_statesProcessingThread.unregisterDestSignalStatesQueue(destQueue);
//}

//void DiagDataReceiver::registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
//														const std::set<Hash>& hashes)
//{
//	m_statesProcessingThread.registerGatewaySignalStatesQueue(destQueue, hashes);
//}

//void DiagDataReceiver::unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue)
//{
//	m_statesProcessingThread.unregisterGatewaySignalStatesQueue(destQueue);
//}

void DiagDataReceiver::run()
{
	DEBUG_LOG_MSG(m_log, QString("DiagDataReceiver thread is started (receiving IP %1)").
							arg(diagDataReceivingIPStr()));

	m_thisThread = QThread::currentThread();

	StdThreadsGuard stg;

	startProcessingThreads(stg);

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

	wakeupAllProcessingThreads();

	DELETE_IF_NOT_NULL(m_timer);

	closeSocket();

	DELETE_IF_NOT_NULL(m_ioContext);

	DEBUG_LOG_MSG(m_log, QString("DiagDataReceiver thread finished (receiving IP %1)").
							arg(diagDataReceivingIPStr()));
}

void DiagDataReceiver::startTimer500ms()
{
	TEST_PTR_RETURN(m_ioContext);

	if (m_timer == nullptr)
	{
		m_timer = new steady_timer(*m_ioContext);
	}

	m_timer->expires_after(asio::chrono::milliseconds(500));
	m_timer->async_wait(bind(&DiagDataReceiver::onTimer500ms, this,
						   std::placeholders::_1));
}

void DiagDataReceiver::onTimer500ms(const error_code& error)
{
	if (stopIfQuitRequested() == true)
	{
		return;
	}

	if (!error)
	{
		if (m_receivedPerSecond == 0 && m_1second)
		{
			m_noReceiveCtr++;

			if (m_noReceiveCtr >= NO_RUP_FRAMES_TIMEOUT)
			{
				qDebug() << C_STR(QString("No RUP frames received in %1 seconds").
									arg(NO_RUP_FRAMES_TIMEOUT));
			}
		}

		if (isSocketWorkable() == false ||
			m_noReceiveCtr >= NO_RUP_FRAMES_TIMEOUT ||
			m_socketErrorCtr >= MAX_SOCKET_ERROR_COUNT)
		{
			closeSocket();
			clearReceiverStatistics();
			createAndBindSocket();
		}

		updateReceiverStatistics();
		updateDataSourcesStatistics();
	}
	else
	{
		DELETE_IF_NOT_NULL(m_timer);
	}

	m_1second ^= 1;

	startTimer500ms();
}

void DiagDataReceiver::clearReceiverStatistics()
{
	m_noReceiveCtr = 0;
	m_socketErrorCtr = 0;

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

void DiagDataReceiver::updateReceiverStatistics()
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

			qDebug() << C_STR(QString("Receive RUP frames %1/s").arg(m_rupFramesReceivingSpeed));

			m_lastUpdateTime = now;
		}
	}
}

void DiagDataReceiver::updateDataSourcesStatistics()
{
	for(DiagDataSource* source : m_diagDataSources)
	{
		TEST_PTR_CONTINUE(source);

		bool invalidateSignals = source->updateStatistics_500ms(m_1second);

		if (invalidateSignals == true)
		{
			requireSignalsInvalidation(source);
		}
	}
}

bool DiagDataReceiver::createAndBindSocket()
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

		m_socket->bind(m_diagDataReceivingIP, error);

		if (!error)
		{
			m_socketBound = true;

			DEBUG_LOG_MSG(m_log, QString("DiagDataReceiver socket created and bound to %1").
							arg(diagDataReceivingIPStr()));

			asio::socket_base::receive_buffer_size rxBufferSize(10 * 1024 * 1024);

			m_socket->set_option(rxBufferSize, error);

			if (error)
			{
				DEBUG_LOG_ERR(m_log, "DiagDataReceiver error changing udp socket receive buffer size");
			}

			m_socket->get_option(rxBufferSize);

			DEBUG_LOG_MSG(m_log, QString("DiagDataReceiver udp socket receive buffer size %1 bytes").
										arg(rxBufferSize.value()));

			startReceive();
		}
		else
		{
			DEBUG_LOG_MSG(m_log, QString("DiagDataReceiver error binding listening socket to %1: %2").
							arg(m_dataReceivingIP.addressPortStr()).
							arg(QString::fromStdString(error.message())));

			closeSocket();
		}
	}
	else
	{
		DEBUG_LOG_MSG(m_log, QString("DiagDataReceiver listening socket opening error: %1").
						arg(QString::fromStdString(error.message())));

		closeSocket();
	}

	return isSocketWorkable();
}

bool DiagDataReceiver::isSocketWorkable() const
{
	return	m_socket != nullptr &&
			m_socket->is_open() &&
			m_socketBound == true;
}

void DiagDataReceiver::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
		m_socketBound = false;
	}
}

void DiagDataReceiver::startReceive()
{
	if (isSocketWorkable() == false)
	{
		closeSocket();
		return;
	}

	m_writeIndex ^= 1;

	m_socket->async_receive_from(asio::buffer(m_receiveBuffer[m_writeIndex], RECV_BUFFER_SIZE),
									m_receiveFromIP[m_writeIndex],
									bind(&DiagDataReceiver::receivePackets, this,
										std::placeholders::_1,
										std::placeholders::_2));
}

void DiagDataReceiver::receivePackets(const error_code& error, size_t bytesReceived)
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

		DiagDataSource* source = m_diagDataSources.getSourceByIP(sourceIP);

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

			if (m_unknownDiagDataSourcesIP.contains(sourceIP) == false &&
				m_unknownDiagDataSourcesIP.size() < 500)
			{
				m_unknownDiagDataSourcesIP.insert(sourceIP);
			}
		}
	}
}

void DiagDataReceiver::requireBufferProcessing(DiagDataSource *source)
{
	std::lock_guard lg(m_packetProcessigRequiredMutex);
	m_packetProcessingRequired.insert({source, true});
	m_packetProcessingRequiredCondition.notify_one();
}

void DiagDataReceiver::requireSignalsInvalidation(DiagDataSource *source)
{
	std::lock_guard lg(m_packetProcessigRequiredMutex);
	m_packetProcessingRequired.insert({source, false});
	m_packetProcessingRequiredCondition.notify_one();
}

void DiagDataReceiver::startProcessingThreads(StdThreadsGuard& stg)
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

	std::thread t(&SignalStatesProcessingThread::processStates,
				  &m_statesProcessingThread, std::ref(*this));

	stg.append(t);
}

void DiagDataReceiver::wakeupAllProcessingThreads()
{
	std::lock_guard lg(m_packetProcessigRequiredMutex);
	m_packetProcessingRequiredCondition.notify_all();
	m_statesProcessingRequiredCondition.notify_all();
}

bool DiagDataReceiver::stopIfQuitRequested()
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

QString DiagDataReceiver::diagDataReceivingIPStr() const
{
	return QString("%1:%2").
				arg(QString::fromStdString(m_diagDataReceivingIP.address().to_string())).
				arg(m_diagDataReceivingIP.port());
}

void DiagDataReceiver::trace_dt(const QString& portID)
{
	if (portID.isEmpty() || portID == "SYSTEMID_RACK01_FSCC01_MD00_ETHERNET02")
	{
		qint64 curTime = QDateTime::currentMSecsSinceEpoch();

		if (m_prevPacketTime != 0 )
		{
			qint64 dt = curTime - m_prevPacketTime;

			if (dt < 4 || dt > 6)
			{
				qDebug() << "dt =" << dt;
			}
		}

		m_prevPacketTime = curTime;
	}
}


void processPackets(DiagDataReceiver& receiver, int threadNumber)
{
	CircularLoggerShared log = receiver.log();

	DEBUG_LOG_MSG(log, QString("DiagDataProcessingThread #%1 is started").arg(threadNumber));

	QThread* thisThread = QThread::currentThread();

	auto& waitConditionMutex = receiver.m_packetProcessigRequiredMutex;
	auto& waitCondition = receiver.m_packetProcessingRequiredCondition;
	auto& requireProcessing = receiver.m_packetProcessingRequired;

	std::unique_lock ul(waitConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		waitCondition.wait(ul, [&receiver, &requireProcessing]() -> bool
								{
									return	receiver.isQuitRequested() ||
											!requireProcessing.empty();
								});

		// here ul is LOCKED!

		if (receiver.isQuitRequested() == true)
		{
			ul.unlock();
			break;
		}

		auto it = requireProcessing.begin();

		if (it == requireProcessing.end())
		{
			ul.unlock();
			continue;
		}

		DiagDataSource* source = it->first;;
		bool requireBufferProcessing = it->second;

		requireProcessing.erase(it);

		ul.unlock();

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
	}

	DEBUG_LOG_MSG(log, QString("DiagDataProcessingThread #%1 finished").arg(threadNumber));
}

