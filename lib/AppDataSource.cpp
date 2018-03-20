#include "AppDataSource.h"



// -------------------------------------------------------------------------------
//
// AppSignalState class implementation
//
// -------------------------------------------------------------------------------

AppSignalStateEx::AppSignalStateEx()
{
	m_current.flags.all = 0;
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

	m_current.hash = m_stored.hash = calcHash(signal->appSignalID());
}


bool AppSignalStateEx::setState(Times time, quint32 validity, double value, int autoArchivingGroup)
{
	// update current state
	//

	// check time to set !!!!
	//
	m_current.flags.clearReasonsFlags();

	m_current.time = time;

	m_current.value = value;

	if (m_initialized == false)
	{
		// initialize state
		//
		m_initialized = true;

		m_current.flags.valid = validity;
		m_current.flags.validityChange = 1;
	}
	else
	{
		// state already initialized
		// check validity changes
		//
		if (validity != m_current.flags.valid)
		{
			// validity has been changed
			//
			m_current.flags.valid = validity;
			m_current.flags.validityChange = 1;
		}
		else
		{
			// no validity changes, check value if new state is valid
			//
			if (validity == AppSignalState::VALID)
			{
				if (m_isDiscreteSignal == true)
				{
					if (m_current.value != m_stored.value)
					{
						m_current.flags.smoothAperture = 0;		// its important!
						m_current.flags.roughAperture = 1;		//
					}
				}
				else
				{
					// is analog signal, check aperture changes
					//
					if (m_adaptiveAperture == true)
					{
						double absAperture = fabs((fabs(m_current.value - m_stored.value) * 100) / m_stored.value);

						if (absAperture > m_fineAperture)
						{
							m_current.flags.smoothAperture = 1;
						}

						if (absAperture > m_coarseAperture)
						{
							m_current.flags.roughAperture = 1;
						}
					}
					else
					{
						double absValueChange = fabs(m_stored.value - m_current.value);

						if (absValueChange > m_absSmoothAperture)
						{
							m_current.flags.smoothAperture = 1;
						}

						if (absValueChange > m_absRoughAperture)
						{
							m_current.flags.roughAperture = 1;
						}
					}
				}
			}
		}
	}

	if (m_autoArchivingGroup == autoArchivingGroup)
	{
		m_current.flags.autoPoint = 1;

//		qDebug() << "Auto " << m_signal->appSignalID();
	}

	bool hasArchivingReason = m_current.flags.hasArchivingReason();

	if (hasArchivingReason == true)
	{
		// update stored state
		//
		m_stored = m_current;
	}

	return hasArchivingReason;
}


Hash AppSignalStateEx::hash() const
{
	assert(m_current.hash == m_stored.hash);
	assert(m_current.hash != 0);

	return m_current.hash;
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


void AppSignalStateEx::setAutoArchivingGroup(int groupsCount)
{
	if (groupsCount == 0)
	{
		m_autoArchivingGroup = -2;
	}
	else
	{
		m_autoArchivingGroup = static_cast<int>(hash() % groupsCount);
	}
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
	for(int i = 0; i < m_size; i++)
	{
		m_appSignalState[i].setAutoArchivingGroup(autoArchivingGroupsCount);
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

AppDataSource::AppDataSource()
{
}

AppDataSource::AppDataSource(AppSignalStates* signalStates, AppSignalStatesQueue* signalStatesQueue) :
	m_signalStates(signalStates),
	m_signalStatesQueue(signalStatesQueue)
{
}

void AppDataSource::prepare(const AppSignals& appSignals)
{
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

		int index = appSignals.indexOf(signalID);

		if (index == -1)
		{
			assert(false);
			continue;
		}

		SignalParseInfo parceInfo;

		parceInfo.setSignalParams(index, *signal);

		m_signalsParseInfo.append(parceInfo);
	}
}

bool AppDataSource::parsePacket()
{
	Times times;
	const char* data = nullptr;
	quint32 dataSize = 0;

	bool result = getDataToParsing(&times, &data, &dataSize);

	if (result == false)
	{
		return false;
	}

	return true;
}

bool AppDataSource::getState(Network::AppDataSourceState* protoState) const
{
	if (protoState == nullptr)
	{
		assert(false);
		return false;
	}

	protoState->set_id(ID());
	protoState->set_uptime(uptime());
	protoState->set_receiveddatasize(receivedDataSize());
	protoState->set_datareceivingrate(dataReceivingRate());
	protoState->set_receivedframescount(receivedFramesCount());
	protoState->set_processingenabled(dataProcessingEnabled());
	protoState->set_processedpacketcount(receivedPacketCount());
	protoState->set_errorprotocolversion(errorProtocolVersion());
	protoState->set_errorframesquantity(errorFramesQuantity());
	protoState->set_errorframeno(errorFrameNo());
	protoState->set_lostedpackets(lostedFramesCount());
	protoState->set_errorbadframesize(errorBadFrameSize());
	protoState->set_haserrors(hasErrors());

	return true;
}

bool AppDataSource::setState(const Network::AppDataSourceState& protoState)
{
	setID(protoState.id());
	setUptime(protoState.uptime());
	setReceivedDataSize(protoState.receiveddatasize());
	setDataReceivingRate(protoState.datareceivingrate());
	setReceivedFramesCount(protoState.receivedframescount());
	setDataProcessingEnabled(protoState.processingenabled());
	setReceivedPacketCount(protoState.processedpacketcount());
	setErrorProtocolVersion(protoState.errorprotocolversion());
	setErrorFramesQuantity(protoState.errorframesquantity());
	setErrorFrameNo(protoState.errorframeno());
	setLostedFramesCount(protoState.lostedpackets());
	setErrorBadFrameSize(protoState.errorbadframesize());
	setHasErrors(protoState.haserrors());

	return true;
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


/*
 *
 * void AppDataProcessingWorker::parseRupData()
{
	m_parsedRupDataCount++;

	if ((m_parsedRupDataCount % 500) == 0)
	{
		qDebug() << "Parced" << m_parsedRupDataCount;
	}

	// parse data from m_rupData
	//
	quint32 sourceIP = m_rupData.sourceIP;

	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(sourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		m_notFoundIPCount++;
		return;
	}

	int autoArchivingGroup = sourceParseInfo->getAutoArchivingGroup(m_rupData.time.system.timeStamp);

	quint32 validity = 0;
	double value = 0;
	bool result = true;

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		if (parseInfo.valueAddr.offset() == -1)
		{
			continue;
		}

		result = getDoubleValue(parseInfo, value);

		if (result == false)
		{
			m_valueParsingErrorCount++;
			continue;
		}

		result = getValidity(parseInfo, validity);

		if (result == false)
		{
			m_validityParsingErrorCount++;
			continue;
		}

		AppSignalStateEx* signalState = m_signalStates[parseInfo.index];

		if (signalState == nullptr)
		{
			m_badSignalStateIndexCount++;
			continue;
		}

		bool hasArchivingReason = signalState->setState(m_rupData.time, validity, value, autoArchivingGroup);

		if (hasArchivingReason == true)
		{
			m_signalStatesQueue.push(&signalState->stored());
		}
	}
}


bool AppDataProcessingWorker::getDoubleValue(const SignalParseInfo& parseInfo, double& value)
{
	// get double signal value from m_rupData.data buffer using parseInfo
	//
	int valueOffset = parseInfo.valueAddr.offset() * 2;		// offset in Words => offset in Bytes
	int bitNo = parseInfo.valueAddr.bit();

	if (m_rupData.dataSize > (Rup::FRAME_DATA_SIZE * Rup::MAX_FRAME_COUNT) ||
		valueOffset < 0 ||
		valueOffset >= m_rupData.dataSize ||
		bitNo <0 ||
		bitNo >= SIZE_16BIT)
	{
		return false;
	}

	quint16 rawValue16 = 0;
	quint32 rawValue32 = 0;

	switch(parseInfo.type)
	{
	case E::SignalType::Discrete:

		assert(parseInfo.dataSize == SIZE_1BIT);

		rawValue16 = *reinterpret_cast<quint16*>(m_rupData.data + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue16 = reverseUint16(rawValue16);
		}

		value = static_cast<double>((rawValue16 >> bitNo) & 0x0001);

		break;

	case E::SignalType::Analog:

		assert(parseInfo.dataSize == SIZE_32BIT);
		assert(bitNo == 0);

		rawValue32 = *reinterpret_cast<quint32*>(m_rupData.data + valueOffset);

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


bool AppDataProcessingWorker::getValidity(const SignalParseInfo& parseInfo, quint32& validity)
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

	if (validityOffset >= m_rupData.dataSize)
	{
		assert(false);
		validity = AppSignalState::INVALID;
		return false;
	}

	quint16 rawValue = *reinterpret_cast<quint16*>(m_rupData.data + validityOffset);

	if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
	{
		rawValue = qFromBigEndian<quint16>(rawValue);
	}

	validity = static_cast<quint32>((rawValue >> parseInfo.validityAddr.bit()) & 0x0001);

	return true;
}
*/





// -------------------------------------------------------------------------------
//
// SourceSignalsParseInfo class implementation
//
// -------------------------------------------------------------------------------


int AppDataSource::getAutoArchivingGroup(qint64 currentSysTime)
{
	if (m_lastAutoArchivingTime == 0)
	{
		m_lastAutoArchivingTime = (currentSysTime / TIME_1S) * TIME_1S;		// rounds time to seconds
		m_lastAutoArchivingGroup = 0;

		return NO_AUTOARCHIVING_GROUP;
	}

	if (abs(currentSysTime - m_lastAutoArchivingTime) < TIME_1S)
	{
		return NO_AUTOARCHIVING_GROUP;
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
