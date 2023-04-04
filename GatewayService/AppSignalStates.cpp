#include "AppSignalStates.h"
/*
// -------------------------------------------------------------------------------
//
// AppSignalState class implementation
//
// -------------------------------------------------------------------------------

DynamicAppSignalState::DynamicAppSignalState()
{
	m_current[0].flags.all = 0;
	m_current[1].flags.all = 0;
}

void DynamicAppSignalState::setSignalParams(const AppSignal* signal, const AppSignals& appSignals)
{
	TEST_PTR_RETURN(signal);

	m_signal = signal;
	m_signalHash = calcHash(signal->appSignalID());

	m_valueAddr = signal->regValueAddr();

	m_signalType = signal->signalType();
	m_analogSignalFormat = signal->analogSignalFormat();
	m_byteOrder = signal->byteOrder();
	m_dataSize = signal->dataSize();

	m_enableTuning = signal->enableTuning();
	m_tuningDefaultValue = signal->tuningDefaultValue();

	if (signal->hasFlagsSignals() == true)
	{
		static const std::vector<E::AppSignalStateFlagType> flagsTypes = E::values<E::AppSignalStateFlagType>();

		for(E::AppSignalStateFlagType flagType : flagsTypes)
		{
			QString flagSignalID = signal->getFlagSignalID(flagType);

			if (flagSignalID.isEmpty() == true)
			{
				continue;
			}

			const AppSignal* flagSignal = appSignals.getSignalByID(flagSignalID);

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

#ifdef QT_DEBUG
			fspi.flagSignalID = flagSignal->appSignalID();				// required for debugging only
#endif

			fspi.flagSignalAddr = flagSignal->regValueAddr();

			Q_ASSERT(fspi.flagSignalAddr.bit() >= 0 && fspi.flagSignalAddr.bit() < 16);

			if (fspi.flagType == E::AppSignalStateFlagType::Validity)
			{
				m_validityAddr = fspi.flagSignalAddr;		// validity flag should not be append to m_flagsSignalsParceInfo, it is Ok
			}
			else
			{
				m_flagsSignalsParceInfo.append(fspi);
			}
		}
	}

	m_archive = signal->archive();

	m_coarseAperture = signal->coarseAperture();
	m_fineAperture = signal->fineAperture();

	m_lowLimit = signal->lowEngineeringUnits();
	m_highLimit = signal->highEngineeringUnits();
	m_adaptiveAperture = signal->adaptiveAperture();

	if (m_adaptiveAperture == false)
	{
		m_absCoarseAperture = fabs(m_highLimit - m_lowLimit) * (m_coarseAperture / 100.0);
		m_absFineAperture = fabs(m_highLimit - m_lowLimit) * (m_fineAperture / 100.0);
	}

	m_current[0].hash = m_current[1].hash = m_signalHash;
}

QString DynamicAppSignalState::appSignalID() const
{
	if (m_signal == nullptr)
	{
		assert(false);
		return QString();
	}

	return m_signal->appSignalID();
}


void DynamicAppSignalState::setAutoArchivingGroup(int archivingGroup)
{
	m_autoArchivingGroup = archivingGroup;
}

void DynamicAppSignalState::appendRtSession(Hash signalHash,
									const QThread* rtProcessingOwner,
									std::shared_ptr<RtTrends::Session> newSession,
									int samplePeriodCounter)
{
	TEST_PTR_RETURN(rtProcessingOwner);
	TEST_PTR_RETURN(newSession);

	if (signalHash != m_signalHash)
	{
		assert(false);
		return;
	}

	int newSessionID = newSession->id();

	takeRtProcessingOwnership(rtProcessingOwner);

	if (m_rtSessions.contains(newSessionID) == false)
	{
		RtSession rtSession;

		rtSession.session = newSession;
		rtSession.sessionID = newSession->id();
		rtSession.samplePeriodCounter = samplePeriodCounter;
		rtSession.sampleCounter = 1000000;					// big value for first point immediately sending

		m_rtSessions.insert(newSessionID, rtSession);

		m_hasRtSessions = true;
	}
	else
	{
		assert(false);
	}

	releaseRtProcessingOwnership(rtProcessingOwner);
}

void DynamicAppSignalState::removeRtSession(Hash signalHash,
									const QThread* rtProcessingOwner,
									std::shared_ptr<RtTrends::Session> sessionToRemove)
{
	TEST_PTR_RETURN(rtProcessingOwner);
	TEST_PTR_RETURN(sessionToRemove);

	if (signalHash != m_signalHash)
	{
		assert(false);
		return;
	}

	int sessionToRemoveID = sessionToRemove->id();

	takeRtProcessingOwnership(rtProcessingOwner);

	assert(m_rtSessions.contains(sessionToRemoveID) == true);

	m_rtSessions.remove(sessionToRemoveID);

	if (m_rtSessions.size() == 0)
	{
		m_hasRtSessions = false;
	}

	releaseRtProcessingOwnership(rtProcessingOwner);
}

void DynamicAppSignalState::setRtSessionSamplePeriodCounter(Hash signalHash,
					const QThread* rtProcessingOwner,
					int sessionID,
					int newSamplePeriodCounter)
{
	TEST_PTR_RETURN(rtProcessingOwner);

	if (signalHash != m_signalHash)
	{
		assert(false);
		return;
	}

	takeRtProcessingOwnership(rtProcessingOwner);

	if (m_rtSessions.contains(sessionID) == true)
	{
		m_rtSessions[sessionID].samplePeriodCounter = newSamplePeriodCounter;
	}

	releaseRtProcessingOwnership(rtProcessingOwner);
}

void DynamicAppSignalState::rtSessionsProcessing(const SimpleAppSignalState& state, bool pushAnyway, const QThread* thread)
{
	Q_ASSERT(m_hasRtSessions == true);

	takeRtProcessingOwnership(thread);

	for(RtSession& session : m_rtSessions)
	{
		if (pushAnyway == true)
		{
			session.session->pushSignalState(m_signalHash, state, thread);
			session.sampleCounter = 0;
			continue;
		}

		session.sampleCounter++;

		if (session.sampleCounter >= session.samplePeriodCounter)
		{
			session.session->pushSignalState(m_signalHash, state, thread);
			session.sampleCounter = 0;
		}
	}

	releaseRtProcessingOwnership(thread);
}

bool DynamicAppSignalState::getValue(const char* rupData, int rupDataSize, double& value)
{
	Q_UNUSED(rupDataSize);

	// get double signal value from rupData buffer using parseInfo
	//
	int valueOffset = m_valueAddr.offset() * 2;		// offset in Words => offset in Bytes
	int bitNo = m_valueAddr.bit();

#ifdef QT_DEBUG

	if (valueOffset < 0 ||
		valueOffset >= rupDataSize ||
		bitNo < 0 ||
		bitNo >= SIZE_16BIT)
	{
		assert(false);
		return false;
	}

#endif

	switch(m_signalType)
	{
	case E::SignalType::Discrete:
		{
			quint16 rawValue16 = 0;

			assert(m_dataSize == SIZE_1BIT);

			rawValue16 = *reinterpret_cast<const quint16*>(rupData + valueOffset);

			if (m_byteOrder == E::ByteOrder::BigEndian)
			{
				rawValue16 = reverseUint16(rawValue16);
			}

			value = static_cast<double>((rawValue16 >> bitNo) & 0x0001);
		}
		break;

	case E::SignalType::Analog:
		assert(m_dataSize == SIZE_32BIT);
		assert(bitNo == 0);

		switch (m_analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			{
				float rawValueFloat = *reinterpret_cast<const float*>(rupData + valueOffset);

				if (m_byteOrder == E::ByteOrder::BigEndian)
				{
					rawValueFloat = reverseFloat(rawValueFloat);
				}

				value = static_cast<double>(rawValueFloat);
			}
			break;

		case E::AnalogAppSignalFormat::SignedInt32:
			{
				qint32 rawValueInt32 = *reinterpret_cast<const qint32*>(rupData + valueOffset);

				if (m_byteOrder == E::ByteOrder::BigEndian)
				{
					rawValueInt32 = reverseInt32(rawValueInt32);
				}

				value = static_cast<double>(rawValueInt32);
			}
			break;

		default:
			assert(false);			// unknown m_analogSignalFormat
		}

		break;

	default:
		assert(false);				// unknown m_signalType
		return false;
	}

	return true;
}

bool DynamicAppSignalState::getBit(const char* rupData, int rupDataSize, const Address16& addr, quint32& bit)
{
	if (addr.isValid() == false)
	{
		return false;
	}

	// get signal validity from m_rupData.data buffer using parseInfo
	//
	int offset = addr.offset() * 2;	// offset in Words => offset in Bytes

	if (offset >= rupDataSize)
	{
		assert(false);
		return false;
	}

	quint16 rawValue = *reinterpret_cast<const quint16*>(rupData + offset);

	if (m_byteOrder == E::ByteOrder::BigEndian)
	{
		rawValue = (rawValue >> 8) | (rawValue << 8);			// swap bytes
	}

	bit = static_cast<quint32>((rawValue >> addr.bit()) & 0x0001);

	return true;
}

void DynamicAppSignalState::takeRtProcessingOwnership(const QThread* newProcessingOwner)
{
	bool result = false;

	do
	{
		const QThread* expectedOwner = nullptr;
		result = m_rtProcessingOwner.compare_exchange_strong(expectedOwner, newProcessingOwner);
	}
	while(result == false);
}

void DynamicAppSignalState::releaseRtProcessingOwnership(const QThread* currentProcessingOwner)
{
	bool result = m_rtProcessingOwner.compare_exchange_strong(currentProcessingOwner, nullptr);

	assert(result == true);

	Q_UNUSED(result);
}

void DynamicAppSignalState::setNewCurState(const SimpleAppSignalState& newCurState)
{
	int writeStateIndex = m_curStateIndex.load() == 0 ? 1 : 0;

	m_current[writeStateIndex] = newCurState;				// safe atomic writing to not-now-reading struct

	m_curStateIndex.store(writeStateIndex);					// change now-reading struct to updated
}

DynamicAppSignalStates::~DynamicAppSignalStates()
{
	clear();
}

void DynamicAppSignalStates::clear()
{
	m_hash2State.clear();

	if (m_appSignalState != nullptr)
	{
		delete [] m_appSignalState;
		m_appSignalState = nullptr;
	}

	m_size = 0;
}

void DynamicAppSignalStates::setSize(int size)
{
	clear();

	if (size > 1000000)		// limit to 1 million of signals
	{
		assert(false);
		return;
	}

	m_appSignalState = new DynamicAppSignalState[size];
	m_size = size;
}

DynamicAppSignalState* DynamicAppSignalStates::operator [] (int index)
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

const DynamicAppSignalState* DynamicAppSignalStates::getStateByHash(Hash signalHash) const
{
	return m_hash2State.value(signalHash, nullptr);
}

DynamicAppSignalState* DynamicAppSignalStates::getStateByHash(Hash signalHash)
{
	return m_hash2State.value(signalHash, nullptr);
}

const DynamicAppSignalState* DynamicAppSignalStates::getStateByID(const QString& signalID) const
{
	return getStateByHash(calcHash(signalID));
}

DynamicAppSignalState* DynamicAppSignalStates::getStateByID(const QString& signalID)
{
	return getStateByHash(calcHash(signalID));
}

void DynamicAppSignalStates::buidlHash2State()
{
	m_hash2State.clear();

	m_hash2State.reserve(static_cast<int>(m_size * 1.3));

	for(int i = 0; i < m_size; i++)
	{
		DynamicAppSignalState& state = m_appSignalState[i];

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

bool DynamicAppSignalStates::getCurrentState(Hash hash, AppSignalState& state) const
{
	if (m_hash2State.contains(hash))
	{
		const DynamicAppSignalState* stateEx = m_hash2State[hash];

		stateEx->current().copyTo(state);

		Q_ASSERT(state.m_hash == hash);

		return true;
	}

	return false;
}

void DynamicAppSignalStates::setAutoArchivingGroups(int autoArchivingGroupsCount)
{
	if (autoArchivingGroupsCount <= 0)
	{
		return;
	}

	int count = 0;

	for(int i = 0; i < m_size; i++)
	{
		if (m_appSignalState->archive() == true)
		{
			m_appSignalState[i].setAutoArchivingGroup(count % autoArchivingGroupsCount);
			count++;
		}
	}
}

*/
