#include "../UtilsLib/WUtils.h"

#include "AppDataSource.h"
#include "RtTrendsServer.h"

// -------------------------------------------------------------------------------
//
// AppSignals class implementation
//
// -------------------------------------------------------------------------------

AppSignals::~AppSignals()
{
	int TO_DO_refactor_AppSignals;

	clear();
}

void AppSignals::clear()
{
	m_hash2Signal.clear();

	for(AppSignal* signal : *this)
	{
		delete signal;
	}

	HashedVector<QString, AppSignal*>::clear();
}

void AppSignals::buildHash2Signal()
{
	m_hash2Signal.clear();

	m_hash2Signal.reserve(static_cast<int>(count() * 1.3));

	for(AppSignal* signal : *this)
	{
		Hash hash = calcHash(signal->appSignalID());

		if (m_hash2Signal.contains(hash))
		{
			AppSignal* s = m_hash2Signal[hash];

			qDebug() << "AppSignals::buildHash2Signal() hash collision" << QString::number(hash, 16) << signal->appSignalID() << "and" << s->appSignalID();

			assert(false);
			continue;
		}

		m_hash2Signal.insert(hash, signal);
	}
}

const AppSignal* AppSignals::getSignal(Hash hash) const
{
	if (m_hash2Signal.contains(hash))
	{
		return m_hash2Signal[hash];
	}

	return nullptr;
}

// -------------------------------------------------------------------------------
//
// AppDataSource class implementation
//
// -------------------------------------------------------------------------------
AppDataSource::AppDataSource(const DataSource& dataSource) :
	m_signalStatesQueue(3)
{
	// copy DataSource properties to THIS object
	//
	*static_cast<DataSource*>(this) = dataSource;

	m_cachedDataUID = appDataUID();

	initParsingBuffers(appDataFramesQuantity());
}

// Contructor for object NOT really used for packet receiving.
// This object used in SCM for AppDataSource state data displaying only.
//
AppDataSource::AppDataSource(const Network::DataSourceInfo& proto) :
	m_signalStatesQueue(3)
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
		if (appSignals.contains(signalID) == false)
		{
			assert(false);
			continue;
		}

		AppSignal* signal = appSignals.value(signalID, nullptr);

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

		TEST_PTR_CONTINUE(dynState);

		m_signalStates.append(dynState);
	}

	m_acquiredSignalsCount = static_cast<int>(m_signalStates.count());

	int queueSize = m_acquiredSignalsCount * 3;

	if (queueSize < 200)
	{
		queueSize = 200;
	}

	m_signalStatesQueue.resize(queueSize);
}


bool AppDataSource::getState(Network::AppDataSourceState* proto) const
{
	TEST_PTR_RETURN_FALSE(proto);

	proto->set_id(ID());
//	proto->set_state(TO_INT(state()));
	proto->set_receivesdata(receivesData());
	proto->set_uptime(uptime());
	proto->set_receiveddataid(receivedDataID());
	proto->set_rupframeplanttime(rupFramePlantTime());

	//

	int TO_DO_remove_this_fields;

//	proto->set_rupframesqueuesize(rupFramesQueueSize());
//	proto->set_rupframesqueuecursize(rupFramesQueueCurSize());
//	proto->set_rupframesqueuecurmaxsize(rupFramesQueueCurMaxSize());

	//

	proto->set_datareceivingrate(dataReceivingRate());
	proto->set_receiveddatasize(receivedDataSize());
	proto->set_receivedframescount(receivedFramesCount());
	proto->set_receivedpacketcount(receivedPacketCount());
	proto->set_lostpacketcount(lostPacketCount());
	proto->set_dataprocessingenabled(dataProcessingEnabled());
	proto->set_processedpacketcount(processedPacketCount());
	proto->set_lastpacketsystemtime(lastPacketSystemTime());
	proto->set_rupframeplanttime(rupFramePlantTime());
	proto->set_rupframenumerator(rupFrameNumerator());
	proto->set_signalstatesqueuesize(signalStatesQueueSize());
	proto->set_signalstatesqueuecursize(signalStatesQueueCurSize());
	proto->set_signalstatesqueuecurmaxsize(signalStatesQueueCurMaxSize());
	proto->set_acquiredsignalscount(acquiredSignalsCount());
	proto->set_errorprotocolversion(errorProtocolVersion());
	proto->set_errorframesquantity(errorFramesQuantity());
	proto->set_errorframeno(errorFrameNo());
	proto->set_errordataid(errorDataID());
	proto->set_errorframesize(errorFrameSize());
	proto->set_errorduplicateplanttime(errorDuplicatePlantTime());
	proto->set_errornonmonotonicplanttime(errorNonmonotonicPlantTime());
	proto->set_errorplanttimeformat(errorPlantTimeFormat());
	proto->set_lmequipmentid(moduleEquipmentID().toStdString());

	return true;
}

void AppDataSource::setState(const Network::AppDataSourceState& proto)
{
	setID(proto.id());
	setReceivesData(proto.receivesdata());
	setUptime(proto.uptime());
	setReceivedDataID(proto.receiveddataid());
	setRupFramePlantTime(proto.rupframeplanttime());

	//

	int TO_DO_remove_this_fields;

//	setRupFramesQueueSize(proto.rupframesqueuesize());
//	setRupFramesQueueCurSize(proto.rupframesqueuecursize());
//	setRupFramesQueueCurMaxSize(proto.rupframesqueuecurmaxsize());

	//

	setDataReceivingRate(proto.datareceivingrate());
	setReceivedDataSize(proto.receiveddatasize());
	setReceivedFramesCount(proto.receivedframescount());
	setReceivedPacketCount(proto.receivedpacketcount());
	setLostPacketCount(proto.lostpacketcount());
	setDataProcessingEnabled(proto.dataprocessingenabled());
	setProcessedPacketCount(proto.processedpacketcount());
	setLastPacketSystemTime(proto.lastpacketsystemtime());
	setRupFramePlantTime(proto.rupframeplanttime());
	setRupFrameNumerator(static_cast<quint16>(proto.rupframenumerator()));
	setSignalStatesQueueSize(proto.signalstatesqueuesize());
	setSignalStatesQueueCurSize(proto.signalstatesqueuecursize());
	setSignalStatesQueueCurMaxSize(proto.signalstatesqueuecurmaxsize());
	setAcquiredSignalsCount(proto.acquiredsignalscount());
	setErrorProtocolVersion(proto.errorprotocolversion());
	setErrorFramesQuantity(proto.errorframesquantity());
	setErrorFrameNo(proto.errorframeno());
	setErrorDataID(proto.errordataid());
	setErrorFrameSize(proto.errorframesize());
	setErrorDuplicatePlantTime(proto.errorduplicateplanttime());
	setErrorNonmonotonicPlantTime(proto.errornonmonotonicplanttime());
	setErrorPlantTimeFormat(proto.errorplanttimeformat());
}

bool AppDataSource::getSignalState(SimpleAppSignalStateArchiveFlag* state, const QThread* thread)
{
	TEST_PTR_RETURN_FALSE(state);

	bool result = m_signalStatesQueue.pop(state, thread);

	m_signalStatesQueueCurSize = m_signalStatesQueue.size(thread);

	return result;
}

void AppDataSource::invalidateSignals(const QThread* thread)
{
	for(DynamicAppSignalState* signalState : m_signalStates)
	{
		TEST_PTR_CONTINUE(signalState);

		signalState->setUnavailable(m_rupTimes, m_signalStatesQueue, thread);
	}

	qDebug() << "Invalidate";
}

bool AppDataSource::parseBuffer(ParsingBuffer& readBuffer, const QThread* thread)
{
	if (readBuffer.readyToParsing == false)
	{
		Q_ASSERT(false);
		return false;
	}

	m_receivedPacketCount++;

	m_lastPacketServerTime = readBuffer.frame0ServerTime;

	if (m_firstPacketServerTime == 0)
	{
		m_firstPacketServerTime = m_lastPacketServerTime;
	}

	const Rup::Header& header = readBuffer.frame0Header();

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
	}

	m_rupFrameNumerator = header.numerator;

	//

	QDateTime plantTime;

	const Rup::TimeStamp& timeStamp = header.timeStamp;

	// don't delete this to prevent plantTime conversion from Local to UTC time during call plantTime.toMSecsSinceEpoch()!!!
	//
	plantTime.setTimeSpec(Qt::UTC);

	plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
	plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

	QDateTime localTime = QDateTime::fromMSecsSinceEpoch(readBuffer.frame0ServerTime);

	// don't delete this to prevent localTime conversion from Local to UTC time during call localTime.toMSecsSinceEpoch()!!!
	//
	localTime.setTimeSpec(Qt::UTC);

	//

	m_rupTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
	m_rupTimes.system.timeStamp = readBuffer.frame0ServerTime;
	m_rupTimes.local.timeStamp = localTime.toMSecsSinceEpoch();

	checkPlantTime(header.timeStamp);

	m_lastRupTimes = m_rupTimes;

	//

	quint16 packetNo = header.numerator;
	bool isSimPacket = readBuffer.isSimPacket;
	const char* rupData = readBuffer.rupData();
	int rupDataSize = readBuffer.rupDataSize();

	int autoArchivingGroup = getAutoArchivingGroup(m_rupTimes.system.timeStamp);

	for(DynamicAppSignalState* signalState : m_signalStates)
	{
		TEST_PTR_CONTINUE(signalState);

		signalState->setState(m_rupTimes, isSimPacket, packetNo, rupData, rupDataSize,
							  autoArchivingGroup, m_signalStatesQueue, thread);
	}

	m_signalStatesQueue.getSizes(&m_signalStatesQueueCurSize, &m_signalStatesQueueCurMaxSize, &m_signalStatesQueueSize, thread);

	readBuffer.prepareToWriting();

	return true;
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
			if (lci.isProvideAppData() == false ||
				lci.appDataEnable == false ||
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
