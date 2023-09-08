#include "../UtilsLib/WUtils.h"

#include "AppDataSource.h"
#include "RtTrendsServer.h"

// -------------------------------------------------------------------------------
//
// AppDataSource class implementation
//
// -------------------------------------------------------------------------------

AppDataSource::AppDataSource(const DataSource& dataSource) :
	m_signalStatesQueue(3),
	m_gatewaySignalStatesQueue(3)
{
	// copy DataSource properties to THIS object
	//
	*static_cast<DataSource*>(this) = dataSource;

	m_cachedAppDataUID = appDataUID();

	initParsingBuffers(appDataFramesQuantity());

	m_acquiredSignalsCount = static_cast<int>(m_appSignals.size());

	m_workcycle_ms = moduleWorkcycle_ms();
}

// Contructor for object NOT really used for packet receiving.
// This object used in SCM for AppDataSource state data displaying only.
//
AppDataSource::AppDataSource(const Network::DataSourceInfo& proto) :
	m_signalStatesQueue(3),
	m_gatewaySignalStatesQueue(3)
{
	loadFromProto(proto);
}

AppDataSource::~AppDataSource()
{
}

void AppDataSource::prepare(const AppSignals& appSignals,
							DynamicAppSignalStates* signalStates,
							int autoArchivingGroupsCount,
							CircularLoggerShared timeErrLog)
{
	if (signalStates == nullptr)
	{
		assert(false);
		return;
	}

	setTimeErrLog(timeErrLog);

	m_autoArchivingGroupsCount = autoArchivingGroupsCount;

	const QStringList& sourceAssociatedSignals = associatedSignals(E::LanControllerType::AppData);

	m_signalStates.clear();

	for(const QString& signalID : sourceAssociatedSignals)
	{
		if (appSignals.containsID(signalID) == false)
		{
			assert(false);
			continue;
		}

		const AppSignal* signal = appSignals.getSignalByID(signalID);

		TEST_PTR_CONTINUE(signal);

		if (signal->regValueAddr().isValid() == false)
		{
			continue;
		}

		if (signal->isBus() == true)
		{
			continue;
		}

		DynamicAppSignalState* dynState = signalStates->getStateByID(signal->appSignalID());

		dynState->setQueues(&m_signalStatesQueue, &m_gatewaySignalStatesQueue);

/*		if (dynState->appSignalID() == "#LM1_MEANDR_10MS_2")
		{
			dynState->m_debug_replace_time = true;
		}*/

		TEST_PTR_CONTINUE(dynState);

		m_signalStates.append(dynState);
	}

	m_acquiredSignalsCount = static_cast<int>(m_signalStates.count());

	int queueSize = std::max(m_acquiredSignalsCount * 3, 200);

	m_signalStatesQueue.resize(queueSize);

	queueSize = std::max(m_acquiredSignalsCount / 2, 1000);

	m_gatewaySignalStatesQueue.resize(queueSize);
}

void AppDataSource::setStatesProcessingThreadWakeupParams(std::mutex* statesProcessigRequiredMutex,
										  std::condition_variable* statesProcessingRequiredCondition,
										  std::queue<AppDataSource*>* statesProcessingRequired)
{
	m_statesProcessigRequiredMutex = statesProcessigRequiredMutex;
	m_statesProcessingRequiredCondition = statesProcessingRequiredCondition;
	m_statesProcessingRequired = statesProcessingRequired;
}

bool AppDataSource::getState(Network::AppDataSourceState* proto) const
{
	TEST_PTR_RETURN_FALSE(proto);

	proto->set_id(m_id);
	proto->set_lmequipmentid(m_moduleEquipmentID.toStdString());
	proto->set_dataprocessingenabled(m_dataProcessingEnabled);
	proto->set_receivesdata(m_receivesData);
	proto->set_uptime(m_uptime);
	proto->set_receiveddataid(m_receivedDataID);
	proto->set_lmtime(m_lmTime);
	proto->set_rupframenumerator(static_cast<quint32>(m_rupFrameNumerator));

	//

	proto->set_datareceivingspeed(m_dataReceivingSpeed);
	proto->set_receiveddatasize(m_receivedDataSize);
	proto->set_receivedframescount(m_receivedFramesCount);
	proto->set_receivedpacketcount(m_receivedPacketCount);
	proto->set_lostpacketcount(m_lostPacketCount);
	proto->set_signalstatesqueuecursize(m_signalStatesQueueCurSize);
	proto->set_signalstatesqueuecurmaxsize(m_signalStatesQueueCurMaxSize);

	proto->set_errorprotocolversion(m_errorProtocolVersion);
	proto->set_errorframesquantity(m_errorFramesQuantity);
	proto->set_errorframeno(m_errorFrameNo);
	proto->set_errorframecrc(m_errorFrameCRC);
	proto->set_errordataid(m_errorDataID);
	proto->set_errorduplicateplanttime(m_errorDuplicatePlantTime);
	proto->set_errornonmonotonicplanttime(m_errorNonmonotonicPlantTime);
	proto->set_errorplanttimeformat(m_errorPlantTimeFormat);

	return true;
}

void AppDataSource::setState(const Network::AppDataSourceState& proto)
{
	m_id = proto.id();
	m_dataProcessingEnabled = proto.dataprocessingenabled();
	m_receivesData = proto.receivesdata();
	m_uptime = proto.uptime();
	m_receivedDataID = proto.receiveddataid();
	m_lmTime = proto.lmtime();
	m_rupFrameNumerator = proto.rupframenumerator();

	m_dataReceivingSpeed = proto.datareceivingspeed();
	m_receivedDataSize = proto.receiveddatasize();
	m_receivedFramesCount = proto.receivedframescount();
	m_receivedPacketCount = proto.receivedpacketcount();
	m_lostPacketCount = proto.lostpacketcount();
	m_signalStatesQueueCurSize = proto.signalstatesqueuecursize();
	m_signalStatesQueueCurMaxSize = proto.signalstatesqueuecurmaxsize();

	m_errorProtocolVersion = proto.errorprotocolversion();
	m_errorFramesQuantity = proto.errorframesquantity();
	m_errorFrameNo = proto.errorframeno();
	m_errorFrameCRC = proto.errorframecrc();
	m_errorDataID = proto.errordataid();
	m_errorDuplicatePlantTime = proto.errorduplicateplanttime();
	m_errorNonmonotonicPlantTime = proto.errornonmonotonicplanttime();
	m_errorPlantTimeFormat = proto.errorplanttimeformat();
}

bool AppDataSource::getSignalState(SimpleAppSignalStateArchiveFlag* state, const QThread* thread)
{
	TEST_PTR_RETURN_FALSE(state);

	bool result = m_signalStatesQueue.pop(state, thread);

	m_signalStatesQueueCurSize = m_signalStatesQueue.size(thread);

	return result;
}

bool AppDataSource::getGatewaySignalState(GatewayAppSignalStateQueueMask* gwState, const QThread* thread)
{
	TEST_PTR_RETURN_FALSE(gwState);

	bool result = m_gatewaySignalStatesQueue.pop(gwState, thread);

	m_gatewaySignalStatesQueueCurSize = m_gatewaySignalStatesQueue.size(thread);

	return result;
}

void AppDataSource::invalidateSignals(const QThread* thread)
{
	int pushedStatesCount = 0;

	for(DynamicAppSignalState* signalState : m_signalStates)
	{
		TEST_PTR_CONTINUE(signalState);

		pushedStatesCount += signalState->setUnavailable(m_rupTimes, m_signalStatesQueue, thread);

		if (pushedStatesCount >= 20)
		{
			pushedStatesCount -= 20;
			wakeupStatesProcessingThread();
		}
	}

	wakeupStatesProcessingThread();

	qDebug() << "Invalidate";
}

bool AppDataSource::statesQueueIsEmpty(QThread* thread) const
{
	return m_signalStatesQueue.isEmpty(thread);
}

bool AppDataSource::parseBuffer(ParsingBuffer& readBuffer, const QThread* thread)
{
	if (readBuffer.readyToParsing == false)
	{
		Q_ASSERT(false);
		return false;
	}

	m_receivesData = true;

	m_receivedPacketCount++;

	const Rup::Header& header = readBuffer.frame0Header();

	bool disableTimeCorrection = false;

	if (m_rupFrameNumerator != -1 &&
		((m_rupFrameNumerator + 1) & 0xFFFF) != header.numerator)
	{
		if (header.numerator > m_rupFrameNumerator)
		{
			m_lostPacketCount += header.numerator - m_rupFrameNumerator - 1;
		}
		else
		{
			m_lostPacketCount += 0xFFFF - m_rupFrameNumerator + header.numerator - 1;
		}

		disableTimeCorrection = true;		// no sequential packets, disable time correction
	}

	m_rupFrameNumerator = header.numerator;

	qint64 timeWithoutCorrection = readBuffer.frame0ServerTime;
	qint64 dt = timeWithoutCorrection - m_lastPacketServerTime;

	if (dt == 0)
	{
		// always do correction
		//
		m_lastPacketServerTime += 1;
	}
	else
	{
		if (disableTimeCorrection == true ||
			dt > 50 ||
			dt <= (m_workcycle_ms + 1))
		{
			// NO time correction
			//
			m_lastPacketServerTime = timeWithoutCorrection;
		}
		else
		{
			// time correction
			//
			m_lastPacketServerTime += m_workcycle_ms;
		}
	}

	if (m_firstPacketServerTime == 0)
	{
		m_firstPacketServerTime = m_lastPacketServerTime;
	}

	//

	QDateTime plantTime;

	const Rup::TimeStamp& timeStamp = header.timeStamp;

	// don't delete this to prevent plantTime conversion from Local to UTC time during call plantTime.toMSecsSinceEpoch()!!!
	//
	plantTime.setTimeSpec(Qt::UTC);

	plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
	plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

	QDateTime localTime = QDateTime::fromMSecsSinceEpoch(m_lastPacketServerTime);

	// don't delete this to prevent localTime conversion from Local to UTC time during call localTime.toMSecsSinceEpoch()!!!
	//
	localTime.setTimeSpec(Qt::UTC);

	//

	m_rupTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
	m_rupTimes.system.timeStamp = m_lastPacketServerTime;
	m_rupTimes.local.timeStamp = localTime.toMSecsSinceEpoch();

	m_lmTime = m_rupTimes.plant.timeStamp;

	checkPlantTime(header.timeStamp);

	m_lastRupTimes = m_rupTimes;

	//

	quint16 packetNo = header.numerator;
	bool isSimPacket = readBuffer.isSimPacket;
	const char* rupData = readBuffer.rupData();
	int rupDataSize = readBuffer.rupDataSize();

	int autoArchivingGroup = getAutoArchivingGroup(m_rupTimes.system.timeStamp);

	int pushedStatesCtr = 0;

	for(DynamicAppSignalState* signalState : m_signalStates)
	{
		TEST_PTR_CONTINUE(signalState);

		pushedStatesCtr += signalState->setState(m_rupTimes, isSimPacket, packetNo, rupData, rupDataSize,
												autoArchivingGroup, thread);

		if (pushedStatesCtr > 20)
		{
			pushedStatesCtr = 0;
			wakeupStatesProcessingThread();
		}
	}

	if (pushedStatesCtr != 0)
	{
		wakeupStatesProcessingThread();
	}

	m_signalStatesQueue.getSizes(&m_signalStatesQueueCurSize, &m_signalStatesQueueCurMaxSize, &m_signalStatesQueueSize, thread);

	readBuffer.prepareToWriting();

	return true;
}

void AppDataSource::wakeupStatesProcessingThread()
{
	Q_ASSERT(m_statesProcessigRequiredMutex != nullptr);
	Q_ASSERT(m_statesProcessingRequired != nullptr);
	Q_ASSERT(m_statesProcessingRequiredCondition != nullptr);

	std::lock_guard lg(*m_statesProcessigRequiredMutex);
	m_statesProcessingRequired->push(this);
	m_statesProcessingRequiredCondition->notify_all();

	Q_UNUSED(lg);
}

int AppDataSource::getAutoArchivingGroup(qint64 currentSysTime)
{
	if (m_autoArchivingGroupsCount <= 0)
	{
		return DynamicAppSignalState::NO_AUTOARCHIVING_GROUP;
	}

	if (m_lastAutoArchivingTime == 0)
	{
		m_lastAutoArchivingTime = (currentSysTime / TIME_1S) * TIME_1S;		// rounds time to seconds
		m_lastAutoArchivingGroup = 0;

		return DynamicAppSignalState::NO_AUTOARCHIVING_GROUP;
	}

	if (abs(currentSysTime - m_lastAutoArchivingTime) < TIME_1S)
	{
		return DynamicAppSignalState::NO_AUTOARCHIVING_GROUP;
	}

	m_lastAutoArchivingTime = (currentSysTime / TIME_1S) * TIME_1S;		// rounds time to seconds

	int retGroup = m_lastAutoArchivingGroup;

	m_lastAutoArchivingGroup++;

	if (m_lastAutoArchivingGroup >= m_autoArchivingGroupsCount)
	{
		m_lastAutoArchivingGroup = 0;
	}

	return retGroup;
}

AppDataSources::AppDataSources()
{
}

AppDataSources::~AppDataSources()
{
	clear();
}

bool AppDataSources::init(const QString& profile,
						  const QVector<DataSource>& dataSources,
						  CircularLoggerShared logger)
{
	clear();

	bool result = true;

	m_sources.reserve(dataSources.size());

	for(const DataSource& dataSource : dataSources)
	{
		if (dataSource.profile() != profile)
		{
			continue;
		}

		AppDataSource* appDataSource = nullptr;

		for(const LanControllerInfo& lci : dataSource.lanControllersInfo()())
		{
			if (lci.isAppDataEnabled() == false ||
				lci.appDataFramesQuantity == 0)
			{
				continue;
			}

			if (m_lanControllerToSource.contains(lci.equipmentID) == true)
			{
				DEBUG_LOG_ERR(logger, QString("Duplicate AppDataSource adapter EquipmentID %1").
												arg(lci.equipmentID));
				result = false;
				continue;
			}

			if (m_ipToSource.contains(lci.appDataIP32()) == true)
			{
				DEBUG_LOG_ERR(logger, QString("Duplicate AppDataSource IP-address %1").
											arg(lci.appDataIP));
				continue;
			}

			if (appDataSource == nullptr)
			{
				appDataSource = new AppDataSource(dataSource);

				m_sources.push_back(appDataSource);

				m_moduleToSource.insert({appDataSource->moduleEquipmentID(), appDataSource});

				const QStringList& sourceSignals = appDataSource->associatedSignals(E::LanControllerType::AppData);

				for(const QString& signalID : sourceSignals)
				{
					Hash signalHash = calcHash(signalID);

					Q_ASSERT(m_signalToSource.contains(signalHash) == false);

					m_signalToSource.insert({signalHash, appDataSource});
				}
			}

			m_lanControllerToSource.insert({lci.equipmentID, appDataSource});
			m_ipToSource.insert({lci.appDataIP32(), appDataSource});
		}
	}

	return result;
}

void AppDataSources::clear()
{
	m_lanControllerToSource.clear();
	m_ipToSource.clear();
	m_signalToSource.clear();
	m_moduleToSource.clear();

	for(auto& source : m_sources)
	{
		delete source;
	}

	m_sources.clear();
}

AppDataSource* AppDataSources::getSourceByIP(quint32 ip)
{
	auto it = m_ipToSource.find(ip);

	if (it == m_ipToSource.end())
	{
		return nullptr;
	}

	return it->second;
}

AppDataSource* AppDataSources::getSignalSource(const QString& signalID)
{
	return getSignalSource(calcHash(signalID));
}

AppDataSource* AppDataSources::getSignalSource(Hash signalHash)
{
	auto it = m_signalToSource.find(signalHash);

	if (it == m_signalToSource.end())
	{
		return nullptr;
	}

	return it->second;
}

std::vector<AppDataSource*>::iterator AppDataSources::begin()
{
	return m_sources.begin();
}

std::vector<AppDataSource*>::const_iterator AppDataSources::begin() const
{
	return m_sources.begin();
}

std::vector<AppDataSource*>::iterator AppDataSources::end()
{
	return m_sources.end();
}

std::vector<AppDataSource*>::const_iterator AppDataSources::end() const
{
	return m_sources.end();
}
