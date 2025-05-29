#include "DynamicAppSignalState.h"
#include "RtTrendsServer.h"
#include "ApertureFile.h"

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
	m_swCalcFunction = signal->swCalcFunction();

	m_archive = signal->archive();

	m_lowLimit = signal->lowEngineeringUnits();
	m_highLimit = signal->highEngineeringUnits();

	m_reverseLimits = (m_lowLimit > m_highLimit);

	m_defaultApertureType = signal->apertureType();
	m_defaultCoarseAperture = signal->coarseAperture();
	m_defaultFineAperture =	signal->fineAperture();
	m_apertureOverrided = false;

	setAperture(m_defaultApertureType,
				m_defaultCoarseAperture,
				m_defaultFineAperture);

	m_enableTuning = signal->enableTuning();
	m_tuningDefaultValue = signal->tuningDefaultValue();

	if (signal->hasFlagsSignals() == true)
	{
		static const std::vector<E::AppSignalStateFlagType> flagsTypes = E::values<E::AppSignalStateFlagType>();

		for(E::AppSignalStateFlagType flagType : flagsTypes)
		{
			if (flagType == E::AppSignalStateFlagType::StateAvailable ||
				flagType == E::AppSignalStateFlagType::SwSimulated ||
				flagType == E::AppSignalStateFlagType::TuningDefault)
			{
				continue;			// this flags cant't be set by another app signal
			}

			QString flagSignalID = signal->getFlagSignalID(flagType);

			if (flagSignalID.isEmpty() == true)
			{
				continue;
			}

			const AppSignal* flagSignal = appSignals.getSignalByID(flagSignalID);

			if (flagSignal == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			if (flagSignal->regValueAddr().isValid() == false)
			{
				Q_ASSERT(false);
				continue;
			}

			FlagSignalParceInfo fspi;

			fspi.flagType = flagType;

#ifdef QT_DEBUG
			fspi.flagSignalID = flagSignal->appSignalID();				// required for debugging only
#endif

			fspi.flagSignalAddr = flagSignal->regValueAddr();

			if (fspi.flagSignalAddr.isValid() == false ||
				(fspi.flagSignalAddr.bit() < 0 || fspi.flagSignalAddr.bit() >= 16))
			{
				Q_ASSERT(false);		// flag signal reg addr is invalid
				continue;
			}

			if (flagType == E::AppSignalStateFlagType::AboveHighLimit)
			{
				m_overrideAboveHighLimitFlag = true;
			}

			if (flagType == E::AppSignalStateFlagType::BelowLowLimit)
			{
				m_overrideBelowLowLimitFlag = true;
			}

			m_flagsSignalsParceInfo.emplace_back(fspi);
		}
	}

	m_current[0].hash = m_current[1].hash = m_signalHash;
}

void DynamicAppSignalState::setQueues(SimpleAppSignalStatesArchiveFlagQueue* signalStatesQueue,
			   GatewayAppSignalStatesQueue* gatewaySignalStatesQueue)
{
	Q_ASSERT(signalStatesQueue != nullptr);
	Q_ASSERT(gatewaySignalStatesQueue != nullptr);

	Q_ASSERT(m_statesQueue == nullptr);
	Q_ASSERT(m_gwStatesQueue == nullptr);

	m_statesQueue = signalStatesQueue;
	m_gwStatesQueue = gatewaySignalStatesQueue;
}

#define PUSH_AUTO_POINT(state)	{																\
									if (m_archive == true)										\
									{															\
										m_statesQueue->pushAutoPoint(state, m_archive, thread); \
										pushedStatesCtr++;										\
										m_statesSaved++;										\
									}															\
									if (m_hasRtSessions == true)								\
									{															\
										rtSessionsProcessing(state, true, thread);				\
									}															\
								}

int DynamicAppSignalState::setStateRaw(AppDataSource& source,
									const Times& time,
									bool isSimPacket,
									quint16 packetNo,
									const char* rupData,
									int rupDataSize,
									int autoArchivingGroup,
									const QThread* thread)
{
	double value = 0;
	AppSignalStateFlags flags;

	flags.stateAvailable = 1;
	flags.valid = 1;			// if flag signal for validity is assigned, this value will override in cycle below
	flags.swSimulated = isSimPacket ? 1 : 0;

	// get signal flags
	//
	for(const FlagSignalParceInfo& fspi : m_flagsSignalsParceInfo)
	{
		quint32 flagState = 0;

		if(getBit(rupData, rupDataSize, fspi.flagSignalAddr, flagState) == false)
		{	Q_ASSERT(false);
			continue;
		}

		switch(fspi.flagType)
		{
		case E::AppSignalStateFlagType::StateAvailable:
		case E::AppSignalStateFlagType::SwSimulated:
		case E::AppSignalStateFlagType::TuningDefault:
			Q_ASSERT(false);						// this flags should NOT be in m_flagsSignalsParceInfo array!
			break;

		case E::AppSignalStateFlagType::Validity:
			flags.valid = flagState;
			break;

		case E::AppSignalStateFlagType::Simulated:
			flags.simulated = flagState;
			if (flagState)
			{
				source.incSimFlagsCount();
			}
			break;

		case E::AppSignalStateFlagType::Blocked:
			flags.blocked = flagState;
			if (flagState)
			{
				source.incBlockFlagsCount();
			}
			break;

		case E::AppSignalStateFlagType::Mismatch:
			flags.mismatch = flagState;
			if (flagState)
			{
				source.incMismatchFlagsCount();
			}
			break;

		case E::AppSignalStateFlagType::AboveHighLimit:
			flags.aboveHighLimit = flagState;
			break;

		case E::AppSignalStateFlagType::BelowLowLimit:
			flags.belowLowLimit = flagState;
			break;

		default:
			Q_ASSERT(false);								// unknown flagType
		}
	}

	if (getValue(rupData, rupDataSize, value) == false)
	{
		return 0;
	}

	return setStateParsed(time, packetNo, value, flags, autoArchivingGroup, thread);
}

// returns count of states pushed in statesQueue
//
int DynamicAppSignalState::setStateParsed(const Times& time,
									quint16 packetNo,
									double value,
									AppSignalStateFlags flags,
									int autoArchivingGroup,
									const QThread* thread)
{
	SimpleAppSignalState prevState = current();			// prevState is a COPY of current()!
	SimpleAppSignalState curState;

	// curState's fields should be updated always
	//
	curState.hash = m_signalHash;
	curState.time = time;
	curState.packetNo = packetNo;
	curState.flags = flags;
	curState.value = value;

	int pushedStatesCtr = 0;

	{ 	// --- Signal state Validity processing start ---

		if (curState.flags.valid == AppSignalState::INVALID)
		{
			// new state is NOT valid
			//
			if (prevState.flags.valid == AppSignalState::VALID)
			{
				// prevState is valid and not stored, archive it
				//
				if (m_prevStateIsStored == false)
				{
					PUSH_AUTO_POINT(prevState)
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
				// prevState is invalid, archive invalid autopoint with time (curState.time - 1)
				//
				SimpleAppSignalState tmpState = prevState;

				tmpState.time = curState.time;
				tmpState.time += -1;						// current time offset back on 1 ms

				PUSH_AUTO_POINT(tmpState)
			}
			else
			{
				// validity is not changed, nothing to do
			}
		}
	} // --- Signal state Validity processing end ---

	//

	{ // --- Signal state Value processing start ---

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
		{
			AnalogValueStatus curValueStatus = analogValueStatus(curState.value);
			AnalogValueStatus prevValueStatus = analogValueStatus(prevState.value);

			bool checkApertures = true;

			if (curValueStatus == AnalogValueStatus::Normal)
			{
				if (prevValueStatus != AnalogValueStatus::Normal && !m_prevStateIsStored)
				{
					PUSH_AUTO_POINT(prevState)

					curState.flags.fineAperture = 1;
					curState.flags.coarseAperture = 1;
					checkApertures = false;
				}
			}
			else
			{
				// curValue is NaN or Inf
				//
				if (prevValueStatus != curValueStatus && !m_prevStateIsStored)
				{
					PUSH_AUTO_POINT(prevState)

					curState.flags.fineAperture = 1;
					curState.flags.coarseAperture = 1;
				}

				checkApertures = false;
			}

			// check aperture changes
			//
			if (checkApertures == true)
			{
				switch(m_apertureType)
				{
				case E::ApertureType::ValuePercent:

					if (m_fineStoredValue != 0)
					{
						double fineAbsAperture = fabs(((value - m_fineStoredValue) * 100) / m_fineStoredValue);

						if (fineAbsAperture > m_absFineAperture)
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
						double coarseAbsAperture = fabs(((value - m_coarseStoredValue) * 100) / m_coarseStoredValue);

						if (coarseAbsAperture > m_absCoarseAperture)
						{
							curState.flags.coarseAperture = 1;
						}
					}
					else
					{
						m_coarseStoredValue = curState.value;
					}

					break;

				case E::ApertureType::RangePercent:
				case E::ApertureType::AbsValue:

					if (fabs(m_fineStoredValue - curState.value) > m_absFineAperture)
					{
						curState.flags.fineAperture = 1;
					}

					if (fabs(m_coarseStoredValue - curState.value) > m_absCoarseAperture)
					{
						curState.flags.coarseAperture = 1;
					}

					break;

				default:
					Q_ASSERT(false);
				}

				if (m_reverseLimits == false)
				{
					if (m_overrideAboveHighLimitFlag == false)
					{
						curState.flags.aboveHighLimit = (curState.value > m_highLimit ? 1 : 0);
					}
					if (m_overrideBelowLowLimitFlag == false)
					{
						curState.flags.belowLowLimit = (curState.value < m_lowLimit ? 1 : 0);
					}
				}
				else
				{
					if (m_overrideAboveHighLimitFlag == false)
					{
						curState.flags.aboveHighLimit = (curState.value < m_highLimit ? 1 : 0);
					}
					if (m_overrideBelowLowLimitFlag == false)
					{
						curState.flags.belowLowLimit = (curState.value > m_lowLimit ? 1 : 0);
					}
				}
			}
		}

		break;

		case E::SignalType::Bus:
			assert(false);					// bus signals should not be parsed here
			break;
		}

		// update tuningDefault flag
		//
		if (m_enableTuning == true)
		{
			TuningValue currTuningValue;
			currTuningValue.setValue(m_tuningDefaultValue.type(),
									 static_cast<quint64>(value),
									 value);

			curState.flags.tuningDefault = (currTuningValue == m_tuningDefaultValue ? 1 : 0);
		}
		else
		{
			// curState.flags.tuningDefault sets to 0 in constructor of curState
		}
	} // // --- Signal state Value processing end ---

	if (m_autoArchivingGroup == autoArchivingGroup)
	{
		curState.flags.autoPoint = 1;
	}

	curState.flags.updateArchivingReasonFlags(prevState.flags);

	bool hasArchivingReason = curState.flags.hasArchivingReason();

	if (hasArchivingReason == true)
	{
		m_statesQueue->push(curState, m_archive, thread);
		pushedStatesCtr++;

		if (m_archive == true)
		{
			m_statesSaved++;
		}

		// update apertures stored states
		//
		if (curState.flags.fineAperture == 1)
		{
			m_fineStoredValue = curState.value;
		}

		if (curState.flags.coarseAperture == 1)
		{
			m_coarseStoredValue = curState.value;
		}

		m_prevStateIsStored = true;
	}
	else
	{
		m_prevStateIsStored = false;
	}

	if (m_gatewayQueueMask != 0 && hasGatewaySendReasone(curState.flags) == true)
	{
		sendAppSignalStateChangeToGateway(prevState, curState, thread);
		pushedStatesCtr++;
	}

	// curState should be update always
	//
	setNewCurState(curState);

	if (m_hasRtSessions == true)
	{
		rtSessionsProcessing(curState, hasArchivingReason, thread);
	}

	return pushedStatesCtr;
}

int DynamicAppSignalState::setUnavailable(const Times& time,
			  SimpleAppSignalStatesArchiveFlagQueue& statesQueue,
			  const QThread* thread)
{
	int pushedStatesCount = 0;

	SimpleAppSignalState prevState = current();			// prevState is a COPY of current()!

	if (prevState.flags.stateAvailable == 0)
	{
		return pushedStatesCount;
	}

	// prevState.flags.stateAvailable == 1
	//
	if (m_prevStateIsStored == false)
	{
		// prevState is not stored, archive it
		//

		statesQueue.pushAutoPoint(prevState, m_archive, thread);
		pushedStatesCount++;

		if (m_hasRtSessions == true)
		{
			rtSessionsProcessing(prevState, true, thread);
		}

		m_prevStateIsStored = true;
	}

	SimpleAppSignalState curState;

	curState.hash = prevState.hash;
	curState.time = time;

	// curState.flags set to 0 in constructor

	curState.flags.updateArchivingReasonFlags(prevState.flags);

	statesQueue.push(curState, m_archive, thread);
	pushedStatesCount++;

	sendAppSignalStateChangeToGateway(prevState, curState, thread);

	m_prevStateIsStored = true;

	setNewCurState(curState);

	if (m_hasRtSessions == true)
	{
		rtSessionsProcessing(curState, true, thread);
	}

	return pushedStatesCount;
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

void DynamicAppSignalState::setGatewayQueueMask(quint32 mask)
{
	m_gatewayQueueMask |= mask;
}

void DynamicAppSignalState::resetGatewayQueueMask(quint32 mask)
{
	m_gatewayQueueMask &= !mask;
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

		m_rtSessions.emplace(newSessionID, rtSession);

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

	auto removedCount = m_rtSessions.erase(sessionToRemoveID);

	Q_ASSERT(removedCount == 1);

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

	auto it = m_rtSessions.find(sessionID);

	if (it != m_rtSessions.end())
	{
		it->second.samplePeriodCounter = newSamplePeriodCounter;
	}

	releaseRtProcessingOwnership(rtProcessingOwner);
}

void DynamicAppSignalState::rtSessionsProcessing(const SimpleAppSignalState& state, bool pushAnyway, const QThread* thread)
{
	Q_ASSERT(m_hasRtSessions == true);

	takeRtProcessingOwnership(thread);

	for(auto& [id, session] : m_rtSessions)
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

void DynamicAppSignalState::overrideAperture(const ApertureRecord& ar)
{
	Q_ASSERT(m_signalHash == calcHash(ar.signalID));

	if (m_signalType != E::SignalType::Analog)
	{
		Q_ASSERT(false);
		return;
	}

	if (ar.setDefault == true)
	{
		setAperture(m_defaultApertureType, m_defaultCoarseAperture, m_defaultFineAperture);
		m_apertureOverrided = false;
	}
	else
	{
		setAperture(ar.apertureType, ar.coarseAperture, ar.fineAperture);
		m_apertureOverrided = true;
	}
}

int DynamicAppSignalState::onArchSignalsTimer()
{
	int inMinuteSaved = m_statesSaved;

	m_statesSaved = 0;

	return inMinuteSaved;
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

void DynamicAppSignalState::sendAppSignalStateChangeToGateway(const SimpleAppSignalState& prevState,
															  const SimpleAppSignalState& newState,
															  const QThread* thread)
{
	GatewayAppSignalStateQueueMask state;

	state.gatewayQueueMask = m_gatewayQueueMask;
	state.gwState.prevState = prevState;
	state.gwState.curState = newState;

	m_gwStatesQueue->push(state, thread);
}

void DynamicAppSignalState::setAperture(E::ApertureType type, double coarseAperture, double fineAperture)
{
	m_apertureType = type;
	m_coarseAperture = coarseAperture;
	m_fineAperture = fineAperture;

	switch(m_apertureType)
	{
	case E::ApertureType::RangePercent:
		m_absCoarseAperture = fabs(((m_highLimit - m_lowLimit) * m_coarseAperture) / 100.0);
		m_absFineAperture = fabs(((m_highLimit - m_lowLimit) * m_fineAperture) / 100.0);
		break;

	case E::ApertureType::ValuePercent:								// ex AdaptiveAperture
		// no break - Ok!
	case E::ApertureType::AbsValue:
		m_absCoarseAperture = fabs(m_coarseAperture);
		m_absFineAperture = fabs(m_fineAperture);
		break;

	default:
		Q_ASSERT(false);
	}

	if (m_absFineAperture > m_absCoarseAperture)
	{
		std::swap(m_absFineAperture, m_absCoarseAperture);
	}
}

void DynamicAppSignalState::setNewCurState(const SimpleAppSignalState& newCurState)
{
	int writeStateIndex = m_curStateIndex.load() == 0 ? 1 : 0;

	m_current[writeStateIndex] = newCurState;				// safe atomic writing to not-now-reading struct

	m_curStateIndex.store(writeStateIndex);					// change now-reading struct to updated
}

bool DynamicAppSignalState::hasGatewaySendReasone(AppSignalStateFlags flags) const
{
	if (m_signalType == E::SignalType::Discrete)
	{
		return	flags.validityChange ||
				flags.coarseAperture;		// discrete state change
	}

	if (m_signalType == E::SignalType::Analog)
	{
		return flags.validityChange ||
			   flags.limitFlagsChange;
	}

	return false;
}

// -------------------------------------------------------------------------------
//
// DynamicAppSignalStates class implementation
//
// -------------------------------------------------------------------------------

DynamicAppSignalStates::~DynamicAppSignalStates()
{
	clear();
}

void DynamicAppSignalStates::clear()
{
	m_hash2State.clear();

	std::for_each(m_appSignalState.begin(), m_appSignalState.end(),
				  [](DynamicAppSignalState* s) { DELETE_IF_NOT_NULL(s)});

	m_appSignalState.clear();
}

void DynamicAppSignalStates::setSize(int size)
{
	clear();

	if (size > 1000000)		// limit to 1 million of signals
	{
		assert(false);
		return;
	}

	m_appSignalState.resize(size);

	for(int i = 0; i < size; i++)
	{
		m_appSignalState[i] = new DynamicAppSignalState;
	}
}

DynamicAppSignalState* DynamicAppSignalStates::operator [] (int index)
{
	return m_appSignalState[index];
}

const DynamicAppSignalState* DynamicAppSignalStates::operator [] (int index) const
{
	return m_appSignalState[index];
}

const DynamicAppSignalState* DynamicAppSignalStates::getStateByHash(Hash signalHash) const
{
	return getValueOrNullptr(m_hash2State, signalHash);
}

DynamicAppSignalState* DynamicAppSignalStates::getStateByHash(Hash signalHash)
{
	return getValueOrNullptr(m_hash2State, signalHash);
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

	int size = TO_INT(m_appSignalState.size());

	for(int i = 0; i < size; i++)
	{
		DynamicAppSignalState* state = m_appSignalState[i];

		Hash hash = state->hash();

		if (m_hash2State.contains(hash) == true)
		{
			assert(false);			// collision !
		}
		else
		{
			m_hash2State.emplace(hash, state);
		}
	}
}

bool DynamicAppSignalStates::getCurrentState(Hash hash, AppSignalState& state) const
{
	const DynamicAppSignalState* stateEx = getValueOrNullptr(m_hash2State, hash);

	if (stateEx == nullptr)
	{
		return false;
	}

	stateEx->current().copyTo(state);

	Q_ASSERT(state.m_hash == hash);

	return true;
}

void DynamicAppSignalStates::setAutoArchivingGroups(int autoArchivingGroupsCount)
{
	if (autoArchivingGroupsCount <= 0)
	{
		return;
	}

	int count = 0;

	int size = TO_INT(m_appSignalState.size());

	for(int i = 0; i < size; i++)
	{
		if (m_appSignalState[i]->archive() == true)
		{
			m_appSignalState[i]->setAutoArchivingGroup(count % autoArchivingGroupsCount);
			count++;
		}
	}
}

void DynamicAppSignalStates::setGatewayQueueMask(const std::set<Hash>& hashes, quint32 mask)
{
	for(Hash h : hashes)
	{
		DynamicAppSignalState* st = getStateByHash(h);

		if (st != nullptr)
		{
			st->setGatewayQueueMask(mask);
		}
	}
}

void DynamicAppSignalStates::resetGatewayQueueMask(const std::set<Hash>& hashes, quint32 mask)
{
	for(Hash h : hashes)
	{
		DynamicAppSignalState* st = getStateByHash(h);

		if (st != nullptr)
		{
			st->resetGatewayQueueMask(mask);
		}
	}
}

void DynamicAppSignalStates::overrideAperture(const ApertureRecord& ar, QString& logMsg)
{
	DynamicAppSignalState* state = getValueOrNullptr(m_hash2State, calcHash(ar.signalID));

	if (state == nullptr)
	{
		return;
	}

	if (state->signalType() != E::SignalType::Analog)
	{
		logMsg.clear();
		return;
	}

	state->overrideAperture(ar);

	if (ar.setDefault == true)
	{
		logMsg = QString("Set default aperture of %1: ").arg(state->appSignalID());
	}
	else
	{
		logMsg = QString("Aperture override of %1: ").arg(state->appSignalID());
	}

	logMsg += QString("type = %1, coarse = %2, fine = %3").
			 arg(E::valueToString(state->apertureType())).arg(state->coarseAperture()).arg(state->fineAperture());
}

void DynamicAppSignalStates::clearStatesSavedCounters()
{
	for(DynamicAppSignalState* ds : m_appSignalState)
	{
		TEST_PTR_CONTINUE(ds);
		ds->clearStatesSavedCounter();
	}
}


