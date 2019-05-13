#include "../lib/WUtils.h"

#include "AppDataSource.h"
#include "RtTrendsServer.h"

// -------------------------------------------------------------------------------
//
// AppSignals class implementation
//
// -------------------------------------------------------------------------------

AppSignals::~AppSignals()
{
	clear();
}

void AppSignals::clear()
{
	m_hash2Signal.clear();

	for(Signal* signal : *this)
	{
		delete signal;
	}

	HashedVector<QString, Signal*>::clear();
}

void AppSignals::buildHash2Signal()
{
	m_hash2Signal.clear();

	m_hash2Signal.reserve(static_cast<int>(count() * 1.3));

	for(Signal* signal : *this)
	{
		Hash hash = calcHash(signal->appSignalID());

		if (m_hash2Signal.contains(hash))
		{
			Signal* s = m_hash2Signal[hash];

			qDebug() << "AppSignals::buildHash2Signal() hash collision" << QString::number(hash, 16) << signal->appSignalID() << "and" << s->appSignalID();

			assert(false);
			continue;
		}

		m_hash2Signal.insert(hash, signal);
	}
}

const Signal* AppSignals::getSignal(Hash hash) const
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

AppDataSource::AppDataSource() :
	m_signalStatesQueue(10 * 1000)
{
}

AppDataSource::AppDataSource(const DataSource& dataSource) :
	m_signalStatesQueue(10)
{
	*(reinterpret_cast<DataSource*>(this)) = dataSource;
}

void AppDataSource::prepare(const AppSignals& appSignals, DynamicAppSignalStates* signalStates, int autoArchivingGroupsCount)
{
	if (signalStates == nullptr)
	{
		assert(false);
		return;
	}

	m_autoArchivingGroupsCount = autoArchivingGroupsCount;

	const QStringList& sourceAssociatedSignals = associatedSignals();

	m_signalStates.clear();

	for(const QString& signalID : sourceAssociatedSignals)
	{
		if (appSignals.contains(signalID) == false)
		{
			assert(false);
			continue;
		}

		Signal* signal = appSignals.value(signalID, nullptr);

		TEST_PTR_CONTINUE(signal);

		if (signal->regValueAddr().isValid() == false)
		{
			continue;
		}

		DynamicAppSignalState* dynState = signalStates->getStateByID(signal->appSignalID());

		TEST_PTR_CONTINUE(dynState);

		m_signalStates.append(dynState);
	}

	m_acquiredSignalsCount = m_signalStates.count();

	int queueSize = m_acquiredSignalsCount * 3;

	if (queueSize < 200)
	{
		queueSize = 200;
	}

	m_signalStatesQueue.resize(queueSize);
}

bool AppDataSource::parsePacket()
{
	Times times;
	const char* rupData = nullptr;
	int rupDataSize = 0;
	bool dataReceivingTimeout = false;
	quint16 packetNo = 0;

	bool result = getDataToParsing(&times, &packetNo, &rupData, &rupDataSize, &dataReceivingTimeout);

	if (result == false)
	{
		assert(false);
		return false;
	}

	int autoArchivingGroup = getAutoArchivingGroup(times.system.timeStamp);

	const QThread* thread = QThread::currentThread();

	for(DynamicAppSignalState* signalState : m_signalStates)
	{
		TEST_PTR_CONTINUE(signalState);

		if (dataReceivingTimeout == true)
		{
			signalState->setUnavailable(times, m_signalStatesQueue, thread);
		}
		else
		{
			signalState->setState(times, packetNo, rupData, rupDataSize, autoArchivingGroup, m_signalStatesQueue, thread);
		}
	}

	m_signalStatesQueue.getSizes(&m_signalStatesQueueSize, &m_signalStatesQueueMaxSize, nullptr, thread);

	return true;
}

bool AppDataSource::getState(Network::AppDataSourceState* proto) const
{
	TEST_PTR_RETURN_FALSE(proto);

	proto->set_id(ID());
	proto->set_datareceives(dataReceives());
	proto->set_uptime(uptime());
	proto->set_receiveddataid(receivedDataID());
	proto->set_rupframesqueuesize(rupFramesQueueSize());
	proto->set_rupframesqueuemaxsize(rupFramesQueueMaxSize());
	proto->set_datareceivingrate(dataReceivingRate());
	proto->set_receiveddatasize(receivedDataSize());
	proto->set_receivedframescount(receivedFramesCount());
	proto->set_receivedpacketcount(receivedPacketCount());
	proto->set_lostedpacketcount(lostedPacketCount());
	proto->set_dataprocessingenabled(dataProcessingEnabled());
	proto->set_processedpacketcount(processedPacketCount());
	proto->set_lastpacketsystemtime(lastPacketSystemTime());
	proto->set_rupframeplanttime(rupFramePlantTime());
	proto->set_rupframenumerator(rupFrameNumerator());
	proto->set_signalstatesqueuesize(signalStatesQueueSize());
	proto->set_signalstatesqueuemaxsize(signalStatesQueueMaxSize());
	proto->set_acquiredsignalscount(acquiredSignalsCount());
	proto->set_errorprotocolversion(errorProtocolVersion());
	proto->set_errorframesquantity(errorFramesQuantity());
	proto->set_errorframeno(errorFrameNo());
	proto->set_errordataid(errorDataID());
	proto->set_errorframesize(errorFrameSize());
	proto->set_errorduplicateplanttime(errorDuplicatePlantTime());
	proto->set_errornonmonotonicplanttime(errorDuplicatePlantTime());
	proto->set_lmequipmentid(lmEquipmentID().toStdString());

	return true;
}

void AppDataSource::setState(const Network::AppDataSourceState& proto)
{
	setID(proto.id());
	setDataReceives(proto.datareceives());
	setUptime(proto.uptime());
	setReceivedDataID(proto.receiveddataid());
	setRupFramesQueueSize(proto.rupframesqueuesize());
	setRupFramesQueueMaxSize(proto.rupframesqueuemaxsize());
	setDataReceivingRate(proto.datareceivingrate());
	setReceivedDataSize(proto.receiveddatasize());
	setReceivedFramesCount(proto.receivedframescount());
	setReceivedPacketCount(proto.receivedpacketcount());
	setLostedPacketCount(proto.lostedpacketcount());
	setDataProcessingEnabled(proto.dataprocessingenabled());
	setProcessedPacketCount(proto.processedpacketcount());
	setLastPacketSystemTime(proto.lastpacketsystemtime());
	setRupFramePlantTime(proto.rupframeplanttime());
	setRupFrameNumerator(proto.rupframenumerator());
	setSignalStatesQueueSize(proto.signalstatesqueuesize());
	setSignalStatesQueueMaxSize(proto.signalstatesqueuemaxsize());
	setAcquiredSignalsCount(proto.acquiredsignalscount());
	setErrorProtocolVersion(proto.errorprotocolversion());
	setErrorFramesQuantity(proto.errorframesquantity());
	setErrorFrameNo(proto.errorframeno());
	setErrorDataID(proto.errordataid());
	setErrorFrameSize(proto.errorframesize());
	setErrorDuplicatePlantTime(proto.errorduplicateplanttime());
	setErrorNonmonotonicPlantTime(proto.errornonmonotonicplanttime());
}

bool AppDataSource::getSignalState(SimpleAppSignalState* state, const QThread* thread)
{
	TEST_PTR_RETURN_FALSE(state);

	bool result = m_signalStatesQueue.pop(state, thread);

	m_signalStatesQueueSize = m_signalStatesQueue.size(thread);

	return result;
}

int AppDataSource::getAutoArchivingGroup(qint64 currentSysTime)
{
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

