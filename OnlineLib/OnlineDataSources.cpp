#include "OnlineDataSources.h"

// -------------------------------------------------------------------------------------------
//
// BaseOnlineDataSources class implementation
//
// -------------------------------------------------------------------------------------------

BaseOnlineDataSources::BaseOnlineDataSources(const HostAddressPort& dataReceivingIP,
											 E::SoftwareRunMode swRunMode,
											 int parsingThreadsCount,
											 CircularLoggerShared log) :
	m_rupFramesReceiver(dataReceivingIP, swRunMode, *this, log),
	m_parsingThreadsCount(parsingThreadsCount),
	m_log(log)
{
	int optimalThreadsCount = std::thread::hardware_concurrency();

	if (m_parsingThreadsCount <= 0 || m_parsingThreadsCount > optimalThreadsCount)
	{
		if (optimalThreadsCount != 0)
		{
			m_parsingThreadsCount = optimalThreadsCount;
		}
		else
		{
			m_parsingThreadsCount = 4;
		}
	}
}

BaseOnlineDataSources::~BaseOnlineDataSources()
{
	clear();
}

void BaseOnlineDataSources::clear()
{
	m_lanControllerToSource.clear();
	m_ipToSource.clear();
	m_signalToSource.clear();
	m_moduleToSource.clear();

	for(BaseOnlineDataSource* src : m_sources)
	{
		DELETE_IF_NOT_NULL(src);
	}

	m_sources.clear();
}

void BaseOnlineDataSources::run()
{
	startProcessingThreads();
	startRupFramesReceiver();
}

void BaseOnlineDataSources::stop()
{
	m_stopSource.request_stop();
}

CircularLoggerShared BaseOnlineDataSources::log()
{
	return m_log;
}

//void RupFramesReceiver::wakeupAllProcessingThreads()
//{
//	std::lock_guard lg(m_packetProcessigRequiredMutex);
//	m_packetProcessingRequiredCondition.notify_all();
//	m_statesProcessingRequiredCondition.notify_all();
//}


bool BaseOnlineDataSources::append(BaseOnlineDataSource* onlineSource,
							   CircularLoggerShared logger)
{
	TEST_PTR_RETURN_FALSE(onlineSource);

	bool result = true;

	E::LanControllerType lanType = onlineSource->sourceType();

	for(const LanControllerInfo& lci : onlineSource->lanControllersInfo()())
	{
		if (lci.isEnabled(lanType) == false ||
			lci.appDataFramesQuantity == 0)
		{
			continue;
		}

		if (m_lanControllerToSource.contains(lci.equipmentID) == true)
		{
			DEBUG_LOG_ERR(logger, QString("Duplicate OnlineDataSource adapter EquipmentID %1 (LAN type %2)").
											arg(lci.equipmentID).arg(E::valueToString(lanType)));
			result = false;
			continue;
		}

		if (m_ipToSource.contains(lci.appDataIP32()) == true)
		{
			DEBUG_LOG_ERR(logger, QString("Duplicate OnlineDataSource IP-address %1 (LAN type %2)").
										arg(lci.appDataIP).arg(E::valueToString(lanType)));
			continue;
		}

		m_sources.push_back(onlineSource);

		m_moduleToSource.insert({onlineSource->moduleEquipmentID(), onlineSource});

		const QStringList& sourceSignals = onlineSource->associatedSignals(lanType);

		for(const QString& signalID : sourceSignals)
		{
			Hash signalHash = calcHash(signalID);

			Q_ASSERT(m_signalToSource.contains(signalHash) == false);

			m_signalToSource.insert({signalHash, onlineSource});
		}

		m_lanControllerToSource.insert({lci.equipmentID, onlineSource});
		m_ipToSource.insert({lci.appDataIP32(), onlineSource});
	}

	return result;
}

bool BaseOnlineDataSources::pushRupFrame(quint32 sourceIP,
									 qint64 serverTime,
									 bool isSimFrame,
									 Rup::Frame& rupFrame,
									 const QThread* thread)
{
	auto it = m_ipToSource.find(sourceIP);

	if (it == m_ipToSource.end())
	{
		return false;
	}

	BaseOnlineDataSource* source = it->second;

	bool readyToParsing = source->pushRupFrame(sourceIP, serverTime, isSimFrame, rupFrame, thread);

	if (readyToParsing == true)
	{
		processingRequired(source, true);
	}

	return true;
}

void BaseOnlineDataSources::updateDataSourcesStatistics500ms(bool oneSecond)
{
	for(BaseOnlineDataSource* source : m_sources)
	{
		TEST_PTR_CONTINUE(source);

		bool invalidateSignals = source->updateStatistics_500ms(oneSecond);

		if (invalidateSignals == true)
		{
			processingRequired(source, false);
		}
	}
}

BaseOnlineDataSource* BaseOnlineDataSources::getSourceByIP(quint32 ip)
{
	auto it = m_ipToSource.find(ip);

	if (it == m_ipToSource.end())
	{
		return nullptr;
	}

	return it->second;
}

BaseOnlineDataSource* BaseOnlineDataSources::getSignalSource(const QString& signalID)
{
	return getSignalSource(calcHash(signalID));
}

BaseOnlineDataSource* BaseOnlineDataSources::getSignalSource(Hash signalHash)
{
	auto it = m_signalToSource.find(signalHash);

	if (it == m_signalToSource.end())
	{
		return nullptr;
	}

	return it->second;
}

std::vector<BaseOnlineDataSource*>::iterator BaseOnlineDataSources::begin()
{
	return m_sources.begin();
}

std::vector<BaseOnlineDataSource*>::const_iterator BaseOnlineDataSources::begin() const
{
	return m_sources.begin();
}

std::vector<BaseOnlineDataSource*>::iterator BaseOnlineDataSources::end()
{
	return m_sources.end();
}

std::vector<BaseOnlineDataSource*>::const_iterator BaseOnlineDataSources::end() const
{
	return m_sources.end();
}

void BaseOnlineDataSources::startProcessingThreads()
{
	for(int i = 0; i < m_parsingThreadsCount; i++)
	{
		m_processingThreads.emplace_back(&BaseOnlineDataSources::processPackets, this, i + 1);
	}
}

void BaseOnlineDataSources::startRupFramesReceiver()
{
	m_processingThreads.emplace_back(&RupFramesReceiver::run, &m_rupFramesReceiver, m_stopSource.get_token());
}

void BaseOnlineDataSources::processingRequired(BaseOnlineDataSource* source, bool parse)
{
	TEST_PTR_RETURN(source);

	// parse == true - signal states parsing required
	// parse == false - signal states invalidation required

	std::lock_guard lg(m_processigRequiredMutex);
	m_processingRequiredQueue.emplace(source, parse);
	m_processingRequiredCondition.notify_one();
}

void BaseOnlineDataSources::processPackets(int threadNumber)
{
	std::stop_token stopToken = m_stopSource.get_token();

	DEBUG_LOG_MSG(m_log, QString("Diag data parsing thread #%1 started").arg(threadNumber));

	QThread* thisThread = QThread::currentThread();

	std::unique_lock ul(m_processigRequiredMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		m_processingRequiredCondition.wait(ul, stopToken, [&]() -> bool
								{
									return	!m_processingRequiredQueue.empty();
								});

		// here ul is LOCKED!

		if (stopToken.stop_requested() == true)
		{
			ul.unlock();
			break;
		}

		if (m_processingRequiredQueue.empty())
		{
			Q_ASSERT(false);
			ul.unlock();
			continue;
		}

		auto& p = m_processingRequiredQueue.front();

		BaseOnlineDataSource* sourceToProcessing = p.first;;
		bool requireBufferProcessing = p.second;

		m_processingRequiredQueue.pop();

		ul.unlock();

		if (sourceToProcessing->takeProcessingOwnership(thisThread) == true)
		{
			if (requireBufferProcessing == true)
			{
				sourceToProcessing->parseNextBuffer(thisThread);
			}
			else
			{
				sourceToProcessing->invalidateAllSignals(thisThread);
			}

			sourceToProcessing->releaseProcessingOwnership(thisThread);
		}
		else
		{
			// another thread already processing this source
		}
	}

	DEBUG_LOG_MSG(m_log, QString("Diag data parsing thread #%1 finished").arg(threadNumber));
}


