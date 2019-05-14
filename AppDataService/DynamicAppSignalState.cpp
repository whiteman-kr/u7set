#include "DynamicAppSignalState.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "RtTrendsServer.h"
#include "AppDataSource.h"

// -------------------------------------------------------------------------------
//
// DynamicAppSignalState class implementation
//
// -------------------------------------------------------------------------------

DynamicAppSignalState::DynamicAppSignalState()
{
	m_current[0].flags.all = 0;
	m_current[1].flags.all = 0;
}

void DynamicAppSignalState::setSignalParams(const Signal* signal, const AppSignals& appSignals)
{
	TEST_PTR_RETURN(signal);

	m_signal = signal;
	m_signalHash = calcHash(signal->appSignalID());

	m_valueAddr = signal->regValueAddr();

	m_signalType = signal->signalType();
	m_analogSignalFormat = signal->analogSignalFormat();
	m_byteOrder = signal->byteOrder();
	m_dataSize = signal->dataSize();

	if (signal->hasStateFlagsSignals() == true)
	{
		static const std::vector<E::AppSignalStateFlagType> flagsTypes = E::values<E::AppSignalStateFlagType>();

		for(E::AppSignalStateFlagType flagType : flagsTypes)
		{
			QString flagSignalID = signal->stateFlagSignal(flagType);

			if (flagSignalID.isEmpty() == true)
			{
				continue;
			}

			const Signal* flagSignal = appSignals.value(flagSignalID, nullptr);

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

#ifdef Q_DEBUG
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

	m_lowLimit = signal->lowEngeneeringUnits();
	m_highLimit = signal->highEngeneeringUnits();
	m_adaptiveAperture = signal->adaptiveAperture();

	if (m_adaptiveAperture == false)
	{
		m_absCoarseAperture = fabs(m_highLimit - m_lowLimit) * (m_coarseAperture / 100.0);
		m_absFineAperture = fabs(m_highLimit - m_lowLimit) * (m_fineAperture / 100.0);
	}

	m_current[0].hash = m_current[1].hash = m_signalHash;
}

bool DynamicAppSignalState::setState(const Times& time,
								quint16 packetNo,
								const char* rupData,
								int rupDataSize,
								int autoArchivingGroup,
								SimpleAppSignalStatesQueue& statesQueue,
								const QThread* thread)
{
	SimpleAppSignalState prevState = current();			// prevState is a COPY of current()!
	SimpleAppSignalState curState;

	// curState's fields should be updated always
	//
	curState.hash = m_signalHash;

	curState.time = time;

	curState.packetNo = packetNo;

	curState.flags.stateAvailable = 1;

	// update validity flag

	bool result = false;

	quint32 validity = AppSignalState::VALID;

	if (m_validityAddr.isValid() == true)
	{
		result = getBit(rupData, rupDataSize, m_validityAddr, validity);

		RETURN_IF_FALSE(result);
	}

	curState.flags.valid = validity;

	// update signal value

	double value = 0;

	result = getValue(rupData, rupDataSize, value);

	RETURN_IF_FALSE(result);

	curState.value = value;

	//

	if (validity == AppSignalState::INVALID)
	{
		if (prevState.flags.valid == AppSignalState::VALID)
		{
			// prevState is valid and not stored, archive it
			//
			if (m_prevStateIsStored == false)
			{
				if (m_archive == true)
				{
					statesQueue.pushAutoPoint(prevState, thread);
				}

				rtSessionsProcessing(prevState, true, thread);

				m_prevStateIsStored = true;
			}
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
		if (prevState.flags.valid == AppSignalState::INVALID)
		{
			// prevState is invalid, archive invalid autopoint
			//
			SimpleAppSignalState tmpState = prevState;

			tmpState.time = curState.time;
			tmpState.time += -1;						// current time offset back on 1 ms

			if (m_archive == true)
			{
				statesQueue.pushAutoPoint(tmpState, thread);
			}

			rtSessionsProcessing(tmpState, true, thread);
		}
		else
		{
			//  prevState also is valid, check signal's value
			//
			switch(m_signalType)
			{
			case E::SignalType::Discrete:

				if (curState.value != prevState.value)
				{
					curState.flags.fineAperture = 0;		// its important!
					curState.flags.coarseAperture = 1;		//
				}
				break;

			case E::SignalType::Analog:

				// is analog signal, check aperture changes
				//
				if (m_adaptiveAperture == true)
				{
					if (m_fineStoredValue != 0)
					{
						double fineAbsAperture = fabs((fabs(value - m_fineStoredValue) * 100) / m_fineStoredValue);

						if (fineAbsAperture > m_fineAperture)
						{
							curState.flags.fineAperture = 1;
						}
					}
					else
					{
						m_fineStoredValue = curState.value;
					}

					if (m_coarseStoredValue != 0)
					{
						double coarseAbsAperture = fabs((fabs(value - m_coarseStoredValue) * 100) / m_coarseStoredValue);

						if (coarseAbsAperture > m_coarseAperture)
						{
							curState.flags.coarseAperture = 1;
						}
					}
					else
					{
						m_coarseStoredValue = curState.value;
					}
				}
				else
				{
					if (fabs(m_fineStoredValue - curState.value) > m_absFineAperture)
					{
						curState.flags.fineAperture = 1;
					}

					if (fabs(m_coarseStoredValue - curState.value) > m_absCoarseAperture)
					{
						curState.flags.coarseAperture = 1;
					}
				}

				curState.flags.aboveHighLimit = (curState.value > m_signal->highEngeneeringUnits() ? 1 : 0);
				curState.flags.belowLowLimit = (curState.value < m_signal->lowEngeneeringUnits() ? 1 : 0);

				break;

			case E::SignalType::Bus:
				assert(false);					// bus signals should not be parsed here
				break;
			}
		}

		// update other signal flags
		//
		for(const FlagSignalParceInfo& fspi : m_flagsSignalsParceInfo)
		{
			quint32 bit = 0;

			result = getBit(rupData, rupDataSize, fspi.flagSignalAddr, bit);

			if (result == false)
			{
				continue;
			}

			switch(fspi.flagType)
			{
			case E::AppSignalStateFlagType::Validity:
			case E::AppSignalStateFlagType::StateAvailable:
				assert(false);								// this flags should not be in m_flagsSignalsParceInfo array!
				break;

			case E::AppSignalStateFlagType::Simulated:
				curState.flags.simulated = bit;
				break;

			case E::AppSignalStateFlagType::Blocked:
				curState.flags.blocked = bit;
				break;

			case E::AppSignalStateFlagType::Unbalanced:
				curState.flags.unbalanced = bit;
				break;

			case E::AppSignalStateFlagType::AboveHighLimit:
				curState.flags.aboveHighLimit = bit;
				break;

			case E::AppSignalStateFlagType::BelowLowLimit:
				curState.flags.belowLowLimit = bit;
				break;

			default:
				assert(false);								// unknown flagType
			}
		}
	}

	if (m_autoArchivingGroup == autoArchivingGroup)
	{
		curState.flags.autoPoint = 1;
	}

	curState.flags.updateArchivingReasonFlags(prevState.flags);

	bool hasArchivingReason = curState.flags.hasArchivingReason();

	if (hasArchivingReason == true)
	{
/*		if (curState.flags.hasAutoPointReasonOnly() == false && m_signalType == E::SignalType::Discrete)
		{
			qDebug() << C_STR(QString("%1 %2").arg(m_signal->appSignalID()).arg(curState.flags.print()));
		}*/

		if (m_archive == true)
		{
			statesQueue.push(curState, thread);
		}

		// update stored states
		//
		if (curState.flags.hasShortTermArchivingReasonOnly() == true)
		{
			m_fineStoredValue = curState.value;
		}
		else
		{
			m_fineStoredValue = m_coarseStoredValue = curState.value;
		}

		m_prevStateIsStored = true;
	}
	else
	{
		m_prevStateIsStored = false;
	}

	// curState should be update always
	//
	setNewCurState(curState);

	if (m_hasRtSessions == true)
	{
		rtSessionsProcessing(curState, hasArchivingReason, thread);
	}

	return hasArchivingReason;
}

void DynamicAppSignalState::setUnavailable(const Times& time,
			  SimpleAppSignalStatesQueue& statesQueue,
			  const QThread* thread)
{
	SimpleAppSignalState prevState = current();			// prevState is a COPY of current()!

	if (prevState.flags.stateAvailable == 0)
	{
		return;
	}

	// prevState.flags.stateAvailable == 1
	//
	if (m_prevStateIsStored == false)
	{
		// prevState is not stored, archive it
		//

		statesQueue.pushAutoPoint(prevState, thread);

		rtSessionsProcessing(prevState, true, thread);

		m_prevStateIsStored = true;
	}

	SimpleAppSignalState curState;

	curState.hash = prevState.hash;
	curState.time = time;

	// curState.flags set to 0 in constructor

	curState.flags.updateArchivingReasonFlags(prevState.flags);

	statesQueue.push(curState, thread);

	m_prevStateIsStored = true;

	setNewCurState(curState);

	if (m_hasRtSessions == true)
	{
		rtSessionsProcessing(curState, true, thread);
	}
}

Hash DynamicAppSignalState::hash() const
{
	assert(m_current[0].hash != 0);
/*	assert(m_current[0].hash == m_stored.hash);
	assert(m_current[1].hash == m_stored.hash);*/

	return m_current[0].hash;
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
	if (m_hasRtSessions == false)
	{
		return;
	}

//	QThread* thread = QThread::currentThread();

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

DynamicAppSignalState* DynamicAppSignalStates::getStateByHash(Hash signalHash)
{
	return m_hash2State.value(signalHash, nullptr);
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

		state = stateEx->current();

		assert(state.m_hash == hash);

		return true;
	}

	return false;
}

void DynamicAppSignalStates::setAutoArchivingGroups(int autoArchivingGroupsCount)
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
			m_appSignalState[i].setAutoArchivingGroup(DynamicAppSignalState::NO_AUTOARCHIVING_GROUP);
		}
	}
}

