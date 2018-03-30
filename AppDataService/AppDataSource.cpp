#include "../lib/WUtils.h"

#include "AppDataSource.h"

// -------------------------------------------------------------------------------
//
// AppSignalState class implementation
//
// -------------------------------------------------------------------------------

AppSignalStateEx::AppSignalStateEx()
{
	m_current[0].flags.all = 0;
	m_current[1].flags.all = 0;
	m_stored.flags.all = 0;
}


void AppSignalStateEx::setSignalParams(int index, Signal* signal)
{
	if (signal == nullptr)
	{
		assert(false);
		return;
	}

	m_index = index;
	m_signal = signal;

	m_isDiscreteSignal = signal->isDiscrete();

	m_archive = signal->archive();

	m_coarseAperture = signal->coarseAperture();
	m_fineAperture = signal->fineAperture();

	m_lowLimit = signal->lowEngeneeringUnits();
	m_highLimit = signal->highEngeneeringUnits();
	m_adaptiveAperture = signal->adaptiveAperture();

	if (m_adaptiveAperture == false)
	{
		m_absRoughAperture = fabs(m_highLimit - m_lowLimit) * (m_coarseAperture / 100.0);
		m_absSmoothAperture = fabs(m_highLimit - m_lowLimit) * (m_fineAperture / 100.0);
	}

	m_current[0].hash = m_current[1].hash = m_stored.hash = calcHash(signal->appSignalID());
}


bool AppSignalStateEx::setState(const Times& time, quint32 validity, double value, int autoArchivingGroup)
{
	// update current state
	//

	SimpleAppSignalState curState = current();			// curState is a COPY of current()!

	// check time to set !!!!
	//
	curState.flags.clearReasonsFlags();

	curState.time = time;

	curState.value = value;

	if (m_initialized == false)
	{
		// initialize state
		//
		m_initialized = true;

		curState.flags.valid = validity;
		curState.flags.validityChange = 1;
	}
	else
	{
		// state already initialized
		// check validity changes
		//
		if (validity != curState.flags.valid)
		{
			// validity has been changed
			//
			curState.flags.valid = validity;
			curState.flags.validityChange = 1;
		}
		else
		{
			// no validity changes, check value if new state is valid
			//
			if (validity == AppSignalState::VALID)
			{
				if (m_isDiscreteSignal == true)
				{
					if (curState.value != m_stored.value)
					{
						curState.flags.smoothAperture = 0;		// its important!
						curState.flags.roughAperture = 1;		//
					}
				}
				else
				{
					// is analog signal, check aperture changes
					//
					if (m_adaptiveAperture == true)
					{
						double absAperture = fabs((fabs(curState.value - m_stored.value) * 100) / m_stored.value);

						if (absAperture > m_fineAperture)
						{
							curState.flags.smoothAperture = 1;
						}

						if (absAperture > m_coarseAperture)
						{
							curState.flags.roughAperture = 1;
						}
					}
					else
					{
						double absValueChange = fabs(m_stored.value - curState.value);

						if (absValueChange > m_absSmoothAperture)
						{
							curState.flags.smoothAperture = 1;
						}

						if (absValueChange > m_absRoughAperture)
						{
							curState.flags.roughAperture = 1;
						}
					}
				}
			}
		}
	}

	if (m_autoArchivingGroup == autoArchivingGroup)
	{
		curState.flags.autoPoint = 1;

//		qDebug() << "Auto " << m_signal->appSignalID();
	}

	bool hasArchivingReason = curState.flags.hasArchivingReason();

	if (hasArchivingReason == true)
	{
		// update stored state
		//
		m_stored = curState;
	}

	setNewCurState(curState);

	return hasArchivingReason;
}


Hash AppSignalStateEx::hash() const
{
	assert(m_current[0].hash != 0);
	assert(m_current[0].hash == m_stored.hash);
	assert(m_current[1].hash == m_stored.hash);

	return m_current[0].hash;
}


QString AppSignalStateEx::appSignalID() const
{
	if (m_signal == nullptr)
	{
		assert(false);
		return QString();
	}

	return m_signal->appSignalID();
}


void AppSignalStateEx::setAutoArchivingGroup(int archivingGroup)
{
	m_autoArchivingGroup = archivingGroup;
}

void AppSignalStateEx::setNewCurState(const SimpleAppSignalState& newCurState)
{
	int writeStateIndex = m_curStateIndex.load() == 0 ? 1 : 0;

	m_current[writeStateIndex] = newCurState;				// safe atomic writing to not-now-reading struct

	m_curStateIndex.store(writeStateIndex);					// change now-reading struct to updated
}

AppSignalStates::~AppSignalStates()
{
	clear();
}


void AppSignalStates::clear()
{
	m_hash2State.clear();

	if (m_appSignalState != nullptr)
	{
		delete [] m_appSignalState;
		m_appSignalState = nullptr;
	}

	m_size = 0;
}


void AppSignalStates::setSize(int size)
{
	clear();

	if (size > 1000000)		// limit to 1 million of signals
	{
		assert(false);
		return;
	}

	m_appSignalState = new AppSignalStateEx[size];
	m_size = size;

	for(int i = 0; i < m_size; i++)
	{
		m_appSignalState[i].invalidate();
	}
}


AppSignalStateEx* AppSignalStates::operator [] (int index)
{
#ifdef QT_DEBUG

	if (m_appSignalState == nullptr ||
		index < 0  || index >= m_size)
	{
		assert(false);
		return nullptr;
	}

#endif

	return m_appSignalState + index;
}


void AppSignalStates::buidlHash2State()
{
	m_hash2State.clear();

	m_hash2State.reserve(m_size * 1.3);

	for(int i = 0; i < m_size; i++)
	{
		AppSignalStateEx& state = m_appSignalState[i];

		Hash hash = state.hash();

		if (m_hash2State.contains(hash) == true)
		{
			assert(false);			// collision !
		}
		else
		{
			m_hash2State.insert(hash, &state);
		}
	}
}


bool AppSignalStates::getCurrentState(Hash hash, AppSignalState& state) const
{
	if (m_hash2State.contains(hash))
	{
		const AppSignalStateEx* stateEx = m_hash2State[hash];

		state = stateEx->current();

		assert(state.m_hash == hash);

		return true;
	}

	return false;
}


bool AppSignalStates::getStoredState(Hash hash, AppSignalState& state) const
{
	if (m_hash2State.contains(hash))
	{
		const AppSignalStateEx* stateEx = m_hash2State[hash];

		state = stateEx->stored();

		assert(state.m_hash == hash);

		return true;
	}

	return false;
}


void AppSignalStates::setAutoArchivingGroups(int autoArchivingGroupsCount)
{
	int count = 0;

	for(int i = 0; i < m_size; i++)
	{
		if (m_appSignalState->archive() == true)
		{
			m_appSignalState[i].setAutoArchivingGroup(count % autoArchivingGroupsCount);
			count++;
		}
		else
		{
			m_appSignalState[i].setAutoArchivingGroup(AppSignalStateEx::NO_AUTOARCHIVING_GROUP);
		}
	}
}

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

	m_hash2Signal.reserve(count() * 1.3);

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
// SignalParseInfo struct implementation
//
// -------------------------------------------------------------------------------

void AppDataSource::SignalParseInfo::setSignalParams(int i, const Signal& s)
{
	appSignalID = s.appSignalID();

	index = i;

	valueAddr = s.regValueAddr();
	validityAddr = s.regValidityAddr();

	type = s.signalType();
	analogSignalFormat = s.analogSignalFormat();
	byteOrder = s.byteOrder();
	dataSize = s.dataSize();
}

// -------------------------------------------------------------------------------
//
// AppDataSource class implementation
//
// -------------------------------------------------------------------------------

AppDataSource::AppDataSource() :
	m_signalStatesQueue(10)
{
}

AppDataSource::AppDataSource(const DataSource& dataSource) :
	m_signalStatesQueue(10)
{
	*(reinterpret_cast<DataSource*>(this)) = dataSource;
}


void AppDataSource::prepare(const AppSignals& appSignals, AppSignalStates* signalStates, int autoArchivingGroupsCount)
{
	if (signalStates == nullptr)
	{
		assert(false);
		return;
	}

	m_signalStates = signalStates;

	m_autoArchivingGroupsCount = autoArchivingGroupsCount;

	m_signalsParseInfo.clear();

	const QStringList& sourceAssociatedSignals = associatedSignals();

	QHash<int, int> archivingGroupsSignalsCount;

	for(const QString& signalID : sourceAssociatedSignals)
	{
		if (appSignals.contains(signalID) == false)
		{
			assert(false);
			continue;
		}

		Signal* signal = appSignals.value(signalID, nullptr);

		if (signal == nullptr)
		{
			assert(false);
			continue;
		}

		if (signal->regValueAddr().isValid() == false)
		{
			continue;
		}

		int index = appSignals.indexOf(signalID);

		if (index == -1)
		{
			assert(false);
			continue;
		}

		SignalParseInfo parceInfo;

		parceInfo.setSignalParams(index, *signal);

		m_signalsParseInfo.append(parceInfo);

		// count signals of each autoArchiving group

		AppSignalStateEx* state = (*signalStates)[index];

		if (state == nullptr)
		{
			assert(false);
			continue;
		}

		int group = state->autoArchiningGroup();

		if (group < 0)
		{
			continue;
		}

		int groupSignalsCount = archivingGroupsSignalsCount.value(group, 0);

		groupSignalsCount++;

		archivingGroupsSignalsCount.insert(group, groupSignalsCount);
	}

	m_acquiredSignalsCount = m_signalsParseInfo.count();

	int maxCount = 0;

	for(int count : archivingGroupsSignalsCount)
	{
		if (count > maxCount)
		{
			maxCount = count;
		}
	}

	if (maxCount < 100)
	{
		maxCount = 100;
	}

	m_signalStatesQueue.resize(maxCount * 3);
}

bool AppDataSource::parsePacket()
{
	Times times;
	const char* rupData = nullptr;
	quint32 rupDataSize = 0;

	bool result = getDataToParsing(&times, &rupData, &rupDataSize);

	if (result == false)
	{
		assert(false);
		return false;
	}

	int autoArchivingGroup = getAutoArchivingGroup(times.system.timeStamp);

	quint32 validity = 0;
	double value = 0;

	for(const SignalParseInfo& parseInfo : m_signalsParseInfo)
	{
		assert(parseInfo.valueAddr.isValid() == true);

		result = getDoubleValue(rupData, rupDataSize, parseInfo, value);

		if (result == false)
		{
			m_valueParsingErrorCount++;
			continue;
		}

		result = getValidity(rupData, rupDataSize, parseInfo, validity);

		if (result == false)
		{
			m_validityParsingErrorCount++;
			continue;
		}

		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			m_badSignalStateIndexCount++;
			continue;
		}

		bool hasArchivingReason = signalState->setState(times, validity, value, autoArchivingGroup);

		if (hasArchivingReason == true)
		{
			m_signalStatesQueue.push(&signalState->stored());

			m_signalStatesQueueSize = m_signalStatesQueue.size();
			m_signalStatesQueueMaxSize = m_signalStatesQueue.maxSize();
		}
	}

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

bool AppDataSource::getSignalState(SimpleAppSignalState* state)
{
	TEST_PTR_RETURN_FALSE(state);

	bool result = m_signalStatesQueue.pop(state);

	m_signalStatesQueueSize = m_signalStatesQueue.size();

	return result;
}


/*

void AppDataReceiver::checkDataSourcesDataReceiving()
{
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

	for(const AppDataSourceShared dataSource : m_appDataSourcesIP)
	{
		if (dataSource == nullptr)
		{
			assert(false);
			continue;
		}

		if (dataSource->state() == E::DataSourceState::ReceiveData && (currentTime - dataSource->lastPacketTime()) > PACKET_TIMEOUT)
		{
			dataSource->setState(E::DataSourceState::NoData);

			invalidateDataSourceSignals(dataSource->lmAddress32(), currentTime);
		}
	}

}


void AppDataReceiver::invalidateDataSourceSignals(quint32 dataSourceIP, qint64 currentTime)
{
	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(dataSourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		return;
	}

	Times time;

	time.system.timeStamp = currentTime;

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setState(time, AppSignalState::INVALID, 0, NO_AUTOARCHIVING_GROUP);
	}

	HostAddressPort addr(dataSourceIP, 0);
	qDebug() << "Invalidate signals of source" << addr.addressStr();
}
*/


/*
 *
 *

	qDebug() << "Ideal thread count:" << QThread::idealThreadCount();


void AppDataReceiver::checkDataSourcesDataReceiving()
{
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

	for(const AppDataSourceShared dataSource : m_appDataSourcesIP)
	{
		if (dataSource == nullptr)
		{
			assert(false);
			continue;
		}

		if (dataSource->state() == E::DataSourceState::ReceiveData && (currentTime - dataSource->lastPacketTime()) > PACKET_TIMEOUT)
		{
			dataSource->setState(E::DataSourceState::NoData);

			invalidateDataSourceSignals(dataSource->lmAddress32(), currentTime);
		}
	}

}


void AppDataReceiver::invalidateDataSourceSignals(quint32 dataSourceIP, qint64 currentTime)
{
	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(dataSourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		return;
	}

	Times time;

	time.system.timeStamp = currentTime;

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setState(time, AppSignalState::INVALID, 0, NO_AUTOARCHIVING_GROUP);
	}

	HostAddressPort addr(dataSourceIP, 0);
	qDebug() << "Invalidate signals of source" << addr.addressStr();
}*/


int AppDataSource::getAutoArchivingGroup(qint64 currentSysTime)
{
	if (m_lastAutoArchivingTime == 0)
	{
		m_lastAutoArchivingTime = (currentSysTime / TIME_1S) * TIME_1S;		// rounds time to seconds
		m_lastAutoArchivingGroup = 0;

		return AppSignalStateEx::NO_AUTOARCHIVING_GROUP;
	}

	if (abs(currentSysTime - m_lastAutoArchivingTime) < TIME_1S)
	{
		return AppSignalStateEx::NO_AUTOARCHIVING_GROUP;
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


bool AppDataSource::getDoubleValue(const char* rupData, int rupDataSize, const SignalParseInfo& parseInfo, double& value)
{
	// get double signal value from rupData buffer using parseInfo
	//
	int valueOffset = parseInfo.valueAddr.offset() * 2;		// offset in Words => offset in Bytes
	int bitNo = parseInfo.valueAddr.bit();

#ifdef QT_DEBUG

	if (valueOffset < 0 ||
		valueOffset >= rupDataSize ||
		bitNo <0 ||
		bitNo >= SIZE_16BIT)
	{
		assert(false);
		return false;
	}

#endif

	quint16 rawValue16 = 0;
	quint32 rawValue32 = 0;

	switch(parseInfo.type)
	{
	case E::SignalType::Discrete:

		assert(parseInfo.dataSize == SIZE_1BIT);

		rawValue16 = *reinterpret_cast<const quint16*>(rupData + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue16 = reverseUint16(rawValue16);
		}

		value = static_cast<double>((rawValue16 >> bitNo) & 0x0001);

		break;

	case E::SignalType::Analog:

		assert(parseInfo.dataSize == SIZE_32BIT);
		assert(bitNo == 0);

		rawValue32 = *reinterpret_cast<const quint32*>(rupData + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue32 = reverseUint32(rawValue32);
		}

		switch (parseInfo.analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			value = static_cast<double>(*reinterpret_cast<float*>(&rawValue32));
			break;

		case E::AnalogAppSignalFormat::SignedInt32:
			value = static_cast<double>(*reinterpret_cast<qint32*>(&rawValue32));
			break;

		default:
			assert(false);
		}
		break;

	default:
		qDebug() << "Signal index (" << parseInfo.index << ") has unknown E::SignalType " << parseInfo.dataSize;
		return false;
	}

	return true;
}


bool AppDataSource::getValidity(const char* rupData, int rupDataSize, const SignalParseInfo& parseInfo, quint32& validity)
{
	// get signal validity from m_rupData.data buffer using parseInfo
	//
	int validityOffset = parseInfo.validityAddr.offset();

	if (validityOffset == BAD_ADDRESS)
	{
		validity = AppSignalState::VALID;				// no validity flags in reg buffer
		return true;
	}

	validityOffset *= 2;					// offset in Words => offset in Bytes

	if (validityOffset >= rupDataSize)
	{
		assert(false);
		validity = AppSignalState::INVALID;
		return false;
	}

	quint16 rawValue = *reinterpret_cast<const quint16*>(rupData + validityOffset);

	if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
	{
		rawValue = reverseUint16(rawValue);
	}

	validity = static_cast<quint32>((rawValue >> parseInfo.validityAddr.bit()) & 0x0001);

	return true;
}




// -------------------------------------------------------------------------------
//
// SourceSignalsParseInfo class implementation
//
// -------------------------------------------------------------------------------


