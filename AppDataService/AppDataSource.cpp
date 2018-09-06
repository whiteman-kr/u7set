#include "../lib/WUtils.h"

#include "AppDataSource.h"
#include "RtTrendsServer.h"

// -------------------------------------------------------------------------------------------------
//
// AppSignalStatesQueue class implementation
//
// -------------------------------------------------------------------------------------------------

SimpleAppSignalStatesQueue::SimpleAppSignalStatesQueue(int queueSize) :
	LockFreeQueue<SimpleAppSignalState>(queueSize)
{
}

bool SimpleAppSignalStatesQueue::pushAutoPoint(SimpleAppSignalState state)
{
	// state is a copy!
	//
	state.flags.autoPoint = 1;

	return push(&state);
}

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
		m_absCoarseAperture = fabs(m_highLimit - m_lowLimit) * (m_coarseAperture / 100.0);
		m_absFineAperture = fabs(m_highLimit - m_lowLimit) * (m_fineAperture / 100.0);
	}

	m_current[0].hash = m_current[1].hash = m_stored.hash = calcHash(signal->appSignalID());
}


bool AppSignalStateEx::setState(const Times& time, quint32 validity, double value, int autoArchivingGroup, SimpleAppSignalStatesQueue& statesQueue)
{
	SimpleAppSignalState prevState = current();			// prevState is a COPY of current()!
	SimpleAppSignalState curState = prevState;

	// check time to set !!!!
	//

	// curState's time and value should be updated always
	//
	curState.time = time;
	curState.value = value;

	curState.flags.clearReasonsFlags();

	if (validity == 0)
	{
		// new state is invalid
		//
		if (prevState.flags.valid == 1)
		{
			// prevState is valid and not stored, archive it
			//

			if (m_prevStateIsStored == false)
			{
				statesQueue.pushAutoPoint(prevState);

				m_prevStateIsStored = true;
			}

			//			logState(prevState);

			curState.flags.valid = 0;
			curState.flags.validityChange = 1;
		}
		else
		{
			// validity is not changed, nothing to do
		}
	}
	else
	{
		// new state is valid
		//
		if (prevState.flags.valid == 0)
		{
			// prevState is invalid, archive invalid autopoint
			//
			SimpleAppSignalState tmpState = curState;

			tmpState.time += -1;		// current time offset back on 1 ms
			tmpState.flags.valid = 0;
			tmpState.value = 0;

			statesQueue.pushAutoPoint(tmpState);

//			logState(autoPointState);

			curState.flags.valid = 1;
			curState.flags.validityChange = 1;
		}
		else
		{
			//  prevState also is valid, check signal's value
			//
			if (m_isDiscreteSignal == true)
			{
				if (curState.value != prevState.value)
				{
					curState.flags.fineAperture = 0;		// its important!
					curState.flags.coarseAperture = 1;		//
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
						curState.flags.fineAperture = 1;
					}

					if (absAperture > m_coarseAperture)
					{
						curState.flags.coarseAperture = 1;
					}
				}
				else
				{
					double absValueChange = fabs(m_stored.value - curState.value);

					if (absValueChange > m_absFineAperture)
					{
						curState.flags.fineAperture = 1;
					}

					if (absValueChange > m_absCoarseAperture)
					{
						curState.flags.coarseAperture = 1;
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

		statesQueue.push(&m_stored);

		m_prevStateIsStored = true;

//		logState(m_stored);
	}
	else
	{
		m_prevStateIsStored = false;
	}

	// curState should be update always
	//
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

void AppSignalStateEx::addRtSession(const QThread* rtProcessingOwner, std::shared_ptr<RtTrends::Session> newSession, Hash signalHash)
{
	TEST_PTR_RETURN(newSession);

	if (signalHash != calcHash(m_signal->appSignalID()))
	{
		assert(false);
		return;
	}

	takeRtProcessingOwnership(rtProcessingOwner);

	int newSessionID = newSession->id();

	if (m_rtSessions.contains(newSessionID) == false)
	{
		RtSession rtSession;

		rtSession.session = newSession;
		rtSession.samplePeriod = newSession->samplePeriod();

		m_rtSessions.insert(newSessionID, rtSession);
	}
	else
	{
		assert(false);
	}

	releaseRtProcessingOwnership(rtProcessingOwner);
}

void AppSignalStateEx::takeRtProcessingOwnership(const QThread* newProcessingOwner)
{
	bool result = false;

	do
	{
		const QThread* expectedOwner = nullptr;
		result = m_rtProcessingOwner.compare_exchange_strong(expectedOwner, newProcessingOwner);
	}
	while(result == false);
}

void AppSignalStateEx::releaseRtProcessingOwnership(const QThread* currentProcessingOwner)
{
	bool result = m_rtProcessingOwner.compare_exchange_strong(currentProcessingOwner, nullptr);

	assert(result == true);

	Q_UNUSED(result);
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

		if (signal->acquire() == false ||
			signal->signalType() == E::SignalType::Bus)
		{
			continue;
		}

		SignalParseInfo parceInfo;

		parceInfo.setSignalParams(index, *signal);

		m_signalsParseInfo.append(parceInfo);
	}

	m_acquiredSignalsCount = m_signalsParseInfo.count();

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
	quint32 rupDataSize = 0;
	bool dataReceivingTimeout = false;

	bool result = getDataToParsing(&times, &rupData, &rupDataSize, &dataReceivingTimeout);

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
		/*if (parseInfo.appSignalID != "#TEST_R01_CH01_MD07_CTRLIN_INH01A")
		{
			continue;
		}*/

		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			m_badSignalStateIndexCount++;
			continue;
		}

		if (dataReceivingTimeout == true)
		{
			value = 0;
			validity = 0;
		}
		else
		{
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
		}

		signalState->setState(times, validity, value, autoArchivingGroup, m_signalStatesQueue);
	}

	m_signalStatesQueueSize = m_signalStatesQueue.size();
	m_signalStatesQueueMaxSize = m_signalStatesQueue.maxSize();

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
bool AppDataSource::setRtTrendsSamplePeriod(int sessionID, E::RtTrendsSamplePeriod samplePeriod)
{
	return true;
}

bool AppDataSource::appendRtTrendsSignal(int sessionID, Hash appendSignalHash, E::RtTrendsSamplePeriod samplePeriod)
{
	// function return TRUE if appendSignalHash is present in this Source and Source required tracking
	// if appendSignalHash is not in this Source, function return FALSE
	//

	const Signal* s = m_appSignals->getSignal(appendSignalHash);

	if (s == nullptr)
	{
		return false;			// it is ok, appendSignalHash is not in current AppDataSource
	}

	RtTrends::Session* session = m_rtTrendsSessions.value(sessionID, nullptr);

	if (session == nullptr)
	{
		session = new RtTrends::Session;

		session->ID = sessionID;
		session->samplePeriod = samplePeriod;

		m_rtTrendsSessions.insert(sessionID, session);
	}
	else
	{
		assert(session->samplePeriod == samplePeriod);		// changes of samplePeriod required processing here!!!
	}

	assert(session->trackedSignals.contains(appendSignalHash) == false);

	session->trackedSignals.insert(appendSignalHash, true);

	return true;
}

bool AppDataSource::deleteRtTrendsSignal(int sessionID, Hash deleteSignalHash, E::RtTrendsSamplePeriod samplePeriod)
{
	// function return TRUE if this Source is not longer needs traking
	// otherwise function return FALSE
	//

	RtTrends::Session* session = m_rtTrendsSessions.value(sessionID, nullptr);

	if (session == nullptr)
	{
		assert(false);
		return true;
	}

	int removedItems = session->trackedSignals.remove(deleteSignalHash);

	assert(removedItems == 1);

	if (session->trackedSignals.size() == 0)
	{
		m_rtTrendsSessions.remove(sessionID);
		delete session;
		return true;
	}

	assert(session->samplePeriod == samplePeriod);	// changes of samplePeriod required processing here!!

	return false;
}
*/
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
		qDebug() << "Signal index (" << parseInfo.index << ") has unknown E::SignalType " << parseInfo.type;
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


