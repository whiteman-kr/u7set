#include <type_traits>
#include <cstddef>
#include <cstring>

#ifdef _WIN32
#include <Mstcpip.h>	// SIO_UDP_CONNRESET
#endif

#include <QThread>

#include "AppDataReceiver.h"

using asio::ip::udp;
using asio::io_context;
using asio::steady_timer;

StdThreadsGuard::~StdThreadsGuard()
{
	for (std::thread& t : m_threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
}

void StdThreadsGuard::append(std::thread&& thread)
{
	Q_ASSERT(thread.joinable() == true);
	m_threads.emplace_back(std::move(thread));
}

// -------------------------------------------------------------------------------
//
// AppDataReceiver class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiver::AppDataReceiver(const HostAddressPort& dataReceivingIP,
								 AppDataSources& appDataSources,
								 DynamicAppSignalStates& signalStates,
								 int processingThreadsCount,
								 E::SoftwareRunMode swRunMode,
								 CircularLoggerShared log) :
	m_dataReceivingIP(dataReceivingIP),
	m_appDataSources(appDataSources),
	m_processingThreadsCountFromSettings(processingThreadsCount),
	m_log(log),
	m_statesProcessingThread(signalStates, log)
{
	setObjectName("AppDataReceiver");

	m_isSimulationMode = (swRunMode == E::SoftwareRunMode::Simulation);

	m_appDataReceivingIP = udp::endpoint(
								asio::ip::make_address(dataReceivingIP.addressStr().toStdString()),
								dataReceivingIP.port());

	for(AppDataSource* appDataSource : appDataSources)
	{
		appDataSource->setStatesProcessingThreadWakeupParams(&m_statesProcessingRequiredMutex,
															 &m_statesProcessingRequiredCondition,
															 &m_statesProcessingRequired);
	}
}

AppDataReceiver::~AppDataReceiver()
{
}

void AppDataReceiver::fillAppDataReceiveState(Network::AppDataReceiveState* adrs)
{
	TEST_PTR_RETURN(adrs);

	adrs->set_receivingspeed(m_receivingSpeed);
	adrs->set_rupframesreceivingspeed(m_rupFramesReceivingSpeed);

	adrs->set_rupframescount(m_rupFramesCount);
	adrs->set_simframescount(m_simFramesCount);

	adrs->set_errdatagramsize(m_errDatagramSize);
	adrs->set_errsimversion(m_errSimVersion);
	adrs->set_errunknownappdatasourceip(m_errUnknownAppDataSourceIP);
	adrs->set_errrupframecrc(m_errRupFrameCRC);
	adrs->set_errnotexpectedsimpacket(m_errNotExpectedSimPacket);
	adrs->set_errnoappdata(m_errNoAppData);
}

void AppDataReceiver::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
													bool isArchivingQueue,
													const QString& description)
{
	m_statesProcessingThread.registerDestSignalStatesQueue(destQueue, isArchivingQueue, description);
}

void AppDataReceiver::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
{
	m_statesProcessingThread.unregisterDestSignalStatesQueue(destQueue);
}

void AppDataReceiver::registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
														const std::set<Hash>& hashes)
{
	m_statesProcessingThread.registerGatewaySignalStatesQueue(destQueue, hashes);
}

void AppDataReceiver::unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue)
{
	m_statesProcessingThread.unregisterGatewaySignalStatesQueue(destQueue);
}

void AppDataReceiver::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is started (receiving IP %1)").
							arg(appDataReceivingIPStr()));

	m_workGuard.emplace(asio::make_work_guard(m_ioContext));

	StdThreadsGuard stg;
	startProcessingThreads(stg);

	try
	{
		startTimer500ms();
		createAndBindSocket();

		m_ioContext.run();
	}

	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	cancelTimer();
	closeSocket();
	resetWorkGuard();

	wakeupAllProcessingThreads();

	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread finished (receiving IP %1)").
							arg(appDataReceivingIPStr()));
}

void AppDataReceiver::startTimer500ms()
{
	if (m_timer == nullptr)
	{
		m_timer = std::make_unique<steady_timer>(m_ioContext);
	}

	m_timer->expires_after(asio::chrono::milliseconds(500));

	m_timer->async_wait([this](const asio::error_code& error)
						{
							onTimer500ms(error);
						});
}

void AppDataReceiver::cancelTimer()
{
	if (m_timer != nullptr)
	{
		asio::error_code ec;
		m_timer->cancel(ec);
		m_timer.reset();
	}
}

void AppDataReceiver::onTimer500ms(const asio::error_code& error)
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
		m_timer.reset();
	}

	m_1second ^= 1;

	startTimer500ms();
}

void AppDataReceiver::clearReceiverStatistics()
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

void AppDataReceiver::updateReceiverStatistics()
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

void AppDataReceiver::updateDataSourcesStatistics()
{
	for(AppDataSource* source : m_appDataSources)
	{
		TEST_PTR_CONTINUE(source);

		QString logStr;

		bool invalidateSignals = source->updateStatistics_500ms(m_1second, logStr);

		if (logStr.isEmpty() == false)
		{
			DEBUG_LOG_WRN(m_log, logStr);
		}

		if (invalidateSignals == true)
		{
			requireSignalsInvalidation(source);
		}
	}
}

bool AppDataReceiver::createAndBindSocket()
{
	Q_ASSERT(isSocketWorkable() == false);

	if (m_socket != nullptr)
	{
		closeSocket();
	}

	m_socket = std::make_unique<udp::socket>(m_ioContext);

	asio::error_code error;

	m_socket->open(udp::v4(), error);

	if (!error)
	{
#ifdef _WIN32
		DWORD bytes = 0; BOOL b = FALSE;
		::WSAIoctl(m_socket->native_handle(), SIO_UDP_CONNRESET,
				   &b, sizeof(b), nullptr, 0, &bytes, nullptr, nullptr);
#endif
		m_socketBound = false;

		m_socket->set_option(asio::socket_base::reuse_address(true), error);

		asio::socket_base::receive_buffer_size rxBufferSize(10 * 1024 * 1024);

		m_socket->set_option(rxBufferSize, error);

		if (error)
		{
			DEBUG_LOG_ERR(m_log, "AppDataReceiver error changing udp socket receive buffer size");
		}

		m_socket->get_option(rxBufferSize);

		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver udp socket receive buffer size %1 bytes").
							 arg(rxBufferSize.value()));

		m_socket->bind(m_appDataReceivingIP, error);

		if (!error)
		{
			m_socketErrorCtr = 0;
			m_socketBound = true;

			DEBUG_LOG_MSG(m_log, QString("AppDataReceiver socket created and bound to %1").
							arg(appDataReceivingIPStr()));

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
		asio::error_code error;
		m_socket->close(error);
		m_socket.reset();
	}

	m_socketBound = false;
}

void AppDataReceiver::startReceive()
{
	if (isSocketWorkable() == false)
	{
		closeSocket();
		return;
	}

	m_writeIndex ^= 1;

	m_socket->async_receive_from(
		asio::buffer(m_receiveBuffer[m_writeIndex].data(), RECV_BUFFER_SIZE),
		m_receiveFromIP[m_writeIndex],
		[this](const asio::error_code& error, std::size_t n)
		{
			receivePackets(error, n);
		});
}

static_assert(std::is_standard_layout_v<Rup::Frame>);
static_assert(std::is_standard_layout_v<Rup::SimFrame>);
static_assert(offsetof(Rup::SimFrame, rupFrame) == 0);

void AppDataReceiver::receivePackets(const asio::error_code& error, size_t bytesReceived)
{
	qint64 serverTime = currentMSecsSinceEpoch();

	if (stopIfQuitRequested() == true)
	{
		return;
	}

	if (error)
	{
		m_socketErrorCtr++;
		startReceive();
		return;
	}

	m_noReceiveCtr = 0;

	const udp::endpoint receiveFromIP = m_receiveFromIP[m_writeIndex];
	const std::byte* const rawData = m_receiveBuffer[m_writeIndex].data();

	startReceive();

	m_receivedPerSecond += static_cast<int>(bytesReceived);

	bool crcOk = false;
	quint32 sourceIP = 0;

	if (bytesReceived == sizeof(Rup::Frame))
	{
		Rup::Frame rupFrame;
		std::memcpy(&rupFrame, rawData, sizeof(rupFrame));

		if ((reverseUint16(rupFrame.header.flags.all) ^ Rup::APP_DATA) != 0)
		{
			m_errNoAppData++;
			return;
		}

		sourceIP = receiveFromIP.address().to_v4().to_ulong();

		AppDataSource* source = m_appDataSources.getSourceByIP(sourceIP);

		if (source == nullptr)
		{
			collectUnknownSourcesIP(sourceIP);
			return;
		}

		m_rupFramesReceivedPerSecond++;
		m_rupFramesCount++;

		crcOk = rupFrame.checkCRC64();

		if (crcOk == true)
		{
			source->pushRupFrame(sourceIP, serverTime,
								 false, rupFrame,
								 source->cachedAppDataUID());

			requireBufferProcessing(source);
		}
		else
		{
			m_errRupFrameCRC++;
			source->incErrorFrameCRC();
		}

		return;
	}

	if (bytesReceived == sizeof(Rup::SimFrame))
	{
		if (m_isSimulationMode == false)
		{
			m_errNotExpectedSimPacket++;

			if ((m_errNotExpectedSimPacket % 1000) == 0)
			{
				qDebug() << C_STR(QString("Software is NOT in SIMULATION mode, %1 sim packets has been ignored.").
								  arg(m_errNotExpectedSimPacket));
			}

			return;
		}

		Rup::SimFrame simFrame;
		std::memcpy(&simFrame, rawData, sizeof(simFrame));

		if ((reverseUint16(simFrame.rupFrame.header.flags.all) ^ Rup::APP_DATA) != 0)
		{
			m_errNoAppData++;
			return;
		}

		if (reverseUint16(simFrame.simVersion) != 1)
		{
			m_errSimVersion++;
			return;
		}

		sourceIP = reverseUint32(simFrame.sourceIP);

		AppDataSource* source = m_appDataSources.getSourceByIP(sourceIP);

		if (source == nullptr)
		{
			collectUnknownSourcesIP(sourceIP);
			return;
		}

		m_simFramesCount++;
		m_rupFramesReceivedPerSecond++;
		m_rupFramesCount++;

		crcOk = simFrame.rupFrame.checkCRC64();

		if (crcOk == true)
		{
			source->pushRupFrame(sourceIP, serverTime,
								 true, simFrame.rupFrame,
								 source->cachedAppDataUID());

			requireBufferProcessing(source);
		}
		else
		{
			m_errRupFrameCRC++;
			source->incErrorFrameCRC();
		}

		return;
	}

	// received datagram  has unknown size, skip this datagram
	//
	m_errDatagramSize++;
}

void AppDataReceiver::collectUnknownSourcesIP(quint32 sourceIP)
{
	m_errUnknownAppDataSourceIP++;

	if (m_unknownAppDataSourcesIP.contains(sourceIP) == false &&
		m_unknownAppDataSourcesIP.size() < 500)
	{
		m_unknownAppDataSourcesIP.insert(sourceIP);
	}
}

void AppDataReceiver::requireBufferProcessing(AppDataSource* source)
{
	{
		std::lock_guard lg(m_packetProcessingRequiredMutex);

		TaskFlags& f = m_sourceTaskFlags[source];

		f |= TaskFlags::Parse;

		if (m_enqueuedSources.insert(source).second == true)
		{
			m_sourcesQueue.push(source);
		}
	}

	m_packetProcessingRequiredCondition.notify_one();
}

void AppDataReceiver::requireSignalsInvalidation(AppDataSource* source)
{
	{
		std::lock_guard lg(m_packetProcessingRequiredMutex);

		TaskFlags& f = m_sourceTaskFlags[source];

		f |= TaskFlags::Invalidate;

		if (m_enqueuedSources.insert(source).second == true)
		{
			m_sourcesQueue.push(source);
		}
	}

	m_packetProcessingRequiredCondition.notify_one();
}

void AppDataReceiver::processPackets(int threadNumber)
{
	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is started").arg(threadNumber));

	auto& waitConditionMutex = m_packetProcessingRequiredMutex;
	auto& waitCondition = m_packetProcessingRequiredCondition;

	std::unique_lock ul(waitConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		waitCondition.wait(ul, [this]() -> bool
						   {
							   return isQuitRequested() || !m_sourcesQueue.empty();
						   });

		// here ul is LOCKED!

		if (isQuitRequested() == true)
		{
			ul.unlock();
			break;
		}

		if (m_sourcesQueue.empty() == true)
		{
			ul.unlock();
			continue;
		}

		AppDataSource* source = m_sourcesQueue.front();
		m_sourcesQueue.pop();

		TaskFlags sourceTaskFlags = TaskFlags::None;

		auto it = m_sourceTaskFlags.find(source);

		if (it != m_sourceTaskFlags.end())
		{
			sourceTaskFlags = it->second;
			m_sourceTaskFlags.erase(it);
		}

		m_enqueuedSources.erase(source);

		ul.unlock();

		TEST_PTR_CONTINUE(source);

		try
		{
			if (source->takeProcessingOwnership() == true)
			{
				if (has(sourceTaskFlags, TaskFlags::Parse))
				{
					source->parseNextBuffer();
				}

				if (has(sourceTaskFlags, TaskFlags::Invalidate))
				{
					source->invalidateSignals();
				}

				source->releaseProcessingOwnership();
			}
			else
			{
				// another thread already processing this source
			}
		}
		catch (const std::exception& e)
		{
			DEBUG_LOG_ERR(m_log, QString("AppDataProcessingThread #%1: exception while processing source: %2").
									arg(threadNumber).arg(e.what()));
			continue;
		}
		catch (...)
		{
			DEBUG_LOG_ERR(m_log, QString("AppDataProcessingThread #%1: unknown exception while processing source").
									arg(threadNumber));
			continue;
		}
	}

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 finished").arg(threadNumber));
}

void AppDataReceiver::startProcessingThreads(StdThreadsGuard& stg)
{
	int idealThreadCount = std::max(QThread::idealThreadCount(), 1);

	int poolSize = m_processingThreadsCountFromSettings;

	if (poolSize <= 0 || poolSize > idealThreadCount)
	{
		poolSize = idealThreadCount;
	}

	for(int i = 0; i < poolSize; i++)
	{
		std::thread t(&AppDataReceiver::processPackets, this, i + 1);

		stg.append(std::move(t));
	}

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThreadsPool started. Running threads count %1%2").
							arg(poolSize).arg(poolSize == idealThreadCount ? " (ideal)" : ""));

	std::thread t(&SignalStatesProcessingThread::processStates,
				  &m_statesProcessingThread, std::ref(*this));

	stg.append(std::move(t));
}

void AppDataReceiver::wakeupAllProcessingThreads()
{
	m_packetProcessingRequiredCondition.notify_all();
	m_statesProcessingRequiredCondition.notify_all();
}

bool AppDataReceiver::stopIfQuitRequested()
{
	if (isQuitRequested() == true)
	{
		cancelTimer();
		closeSocket();

		resetWorkGuard();

		m_ioContext.stop();

		wakeupAllProcessingThreads();

		return true;
	}

	return false;
}

void AppDataReceiver::resetWorkGuard()
{
	if (m_workGuard.has_value())
	{
		m_workGuard->reset();
		m_workGuard.reset();
	}
}

QString AppDataReceiver::appDataReceivingIPStr() const
{
	return QString("%1:%2").
				arg(QString::fromStdString(m_appDataReceivingIP.address().to_string())).
				arg(m_appDataReceivingIP.port());
}

void AppDataReceiver::trace_dt(const QString& portID)
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

