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
// SignalParseInfo struct implementation
//
// -------------------------------------------------------------------------------

void AppDataSource::SignalParseInfo::setSignalParams(int i, const Signal& s, const AppSignals& appSignals)
{
	appSignalID = s.appSignalID();

	index = i;

	valueAddr = s.regValueAddr();
	validityAddr = s.regValidityAddr();

	type = s.signalType();
	analogSignalFormat = s.analogSignalFormat();
	byteOrder = s.byteOrder();
	dataSize = s.dataSize();

	if (s.hasStateFlagsSignals() == true)
	{
		static const std::vector<E::AppSignalStateFlagType> flagsTypes = E::values<E::AppSignalStateFlagType>();

		for(E::AppSignalStateFlagType flagType : flagsTypes)
		{
			QString flagSignalID = s.stateFlagSignal(flagType);

			if (flagSignalID.isEmpty() == true)
			{
				continue;
			}

			Signal* flagSignal = appSignals.value(flagSignalID, nullptr);

			if (flagSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (flagSignal->regValueAddr().isValid() == false)
			{
				assert(false);
				continue;
			}

			FlagSignalParceInfo fspi;

			fspi.flagType = flagType;
			fspi.flagSignalID = flagSignal->appSignalID();
			fspi.flagSignalAddr = flagSignal->regValueAddr();

			flagsSignalsParceInfo.append(fspi);
		}
	}
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

		parceInfo.setSignalParams(index, *signal, appSignals);

		m_signalsParseInfo.append(parceInfo);
	}

	assert(false);		// to do: sort m_signalsParseInfo by flag signals dependency

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
	quint16 packetNo = 0;

	bool result = getDataToParsing(&times, &packetNo, &rupData, &rupDataSize, &dataReceivingTimeout);

	if (result == false)
	{
		assert(false);
		return false;
	}

	int autoArchivingGroup = getAutoArchivingGroup(times.system.timeStamp);

	quint32 validity = 0;
	double value = 0;

	const QThread* thread = QThread::currentThread();

	for(const SignalParseInfo& parseInfo : m_signalsParseInfo)
	{
		DynamicAppSignalState* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			m_badSignalStateIndexCount++;
			continue;
		}

		if (dataReceivingTimeout == true)
		{
			signalState->setUnavailable(times, m_signalStatesQueue, thread);
		}
		else
		{
			result = getDoubleValue(rupData, rupDataSize, parseInfo, value);

			if (result == false)
			{
				m_valueParsingErrorCount++;
				continue;
			}

			if (parseInfo.validityAddr.offset() != BAD_ADDRESS)
			{
				result = getValidity(rupData, rupDataSize, parseInfo, validity);

				if (result == false)
				{
					m_validityParsingErrorCount++;
					continue;
				}
			}
			else
			{
				validity = AppSignalState::VALID;
			}
		}

		signalState->setState(times, packetNo, validity, value, autoArchivingGroup, m_signalStatesQueue, thread);
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

	assert(validityOffset != BAD_ADDRESS);

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
