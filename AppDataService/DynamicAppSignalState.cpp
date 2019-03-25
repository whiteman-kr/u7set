#include "DynamicAppSignalState.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "RtTrendsServer.h"

// -------------------------------------------------------------------------------
//
// AppSignalStateEx class implementation
//
// -------------------------------------------------------------------------------

DynamicAppSignalState::DynamicAppSignalState()
{
	m_current[0].flags.all = 0;
	m_current[1].flags.all = 0;
}

void DynamicAppSignalState::setSignalParams(int index, Signal* signal)
{
	if (signal == nullptr)
	{
		assert(false);
		return;
	}

	m_index = index;
	m_signal = signal;
	m_signalHash = calcHash(signal->appSignalID());

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

	m_current[0].hash = m_current[1].hash = /*m_stored.hash =*/ calcHash(signal->appSignalID());
}

bool DynamicAppSignalState::setState(const Times& time,
								quint16 packetNo,
								quint32 validity,
								double value,
								int autoArchivingGroup,
								SimpleAppSignalStatesQueue& statesQueue,
								const QThread* thread)
{
	SimpleAppSignalState prevState = current();			// prevState is a COPY of current()!
	SimpleAppSignalState curState = prevState;

	// curState's fields should be updated always
	//
	curState.time = time;
	curState.value = value;
	curState.packetNo = packetNo;

	//

	if (validity == AppSignalState::INVALID)
	{
		if (prevState.flags.valid == AppSignalState::VALID)
		{
			// prevState is valid and not stored, archive it
			//
			if (m_prevStateIsStored == false)
			{
				statesQueue.pushAutoPoint(prevState, thread);

				rtSessionsProcessing(prevState, true, thread);

				m_prevStateIsStored = true;
			}

			//			logState(prevState);

			curState.flags.valid = AppSignalState::INVALID;
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
			SimpleAppSignalState tmpState = curState;

			tmpState.time += -1;		// current time offset back on 1 ms
			tmpState.flags.valid = AppSignalState::INVALID;
			tmpState.value = 0;

			statesQueue.pushAutoPoint(tmpState, thread);

			rtSessionsProcessing(tmpState, true, thread);

//			logState(autoPointState);

			curState.flags.valid = AppSignalState::VALID;
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
		statesQueue.push(curState, thread);

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

