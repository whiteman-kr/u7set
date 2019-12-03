#include "AppSignalStateEx.h"


// -------------------------------------------------------------------------------
//
// AppSignalState class implementation
//
// -------------------------------------------------------------------------------

DynamicAppSignalState::DynamicAppSignalState()
{
	m_current.flags.all = 0;
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

	m_isDiscreteSignal = signal->isDiscrete();

	m_coarseAperture = signal->coarseAperture();
	m_fineAperture = signal->fineAperture();

	m_lowLimit = signal->lowEngineeringUnits();
	m_highLimit = signal->highEngineeringUnits();
	m_adaptiveAperture = signal->adaptiveAperture();

	if (m_adaptiveAperture == false)
	{
		m_absRoughAperture = fabs(m_highLimit - m_lowLimit) * (m_coarseAperture / 100.0);
		m_absSmoothAperture = fabs(m_highLimit - m_lowLimit) * (m_fineAperture / 100.0);
	}

	m_current.hash = m_stored.hash = calcHash(signal->appSignalID());
}


bool DynamicAppSignalState::setState(Times time, quint32 validity, double value, int autoArchivingGroup)
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
						m_current.flags.fineAperture = 0;		// its important!
						m_current.flags.coarseAperture = 1;		//
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
							m_current.flags.fineAperture = 1;
						}

						if (absAperture > m_coarseAperture)
						{
							m_current.flags.coarseAperture = 1;
						}
					}
					else
					{
						double absValueChange = fabs(m_stored.value - m_current.value);

						if (absValueChange > m_absSmoothAperture)
						{
							m_current.flags.fineAperture = 1;
						}

						if (absValueChange > m_absRoughAperture)
						{
							m_current.flags.coarseAperture = 1;
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


Hash DynamicAppSignalState::hash() const
{
	assert(m_current.hash == m_stored.hash);
	assert(m_current.hash != 0);

	return m_current.hash;
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


void DynamicAppSignalState::setAutoArchivingGroup(int groupsCount)
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

	for(int i = 0; i < m_size; i++)
	{
		m_appSignalState[i].invalidate();
	}
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


void DynamicAppSignalStates::buidlHash2State()
{
	m_hash2State.clear();

	m_hash2State.reserve(static_cast<int>(m_size * 1.3));

	for(int i = 0; i < m_size; i++)
	{
		DynamicAppSignalState& state = m_appSignalState[i];

		Hash hash = state.hash();

		if (m_hash2State.contains(hash))
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


bool DynamicAppSignalStates::getStoredState(Hash hash, AppSignalState& state) const
{
	if (m_hash2State.contains(hash))
	{
		const DynamicAppSignalState* stateEx = m_hash2State[hash];

		state = stateEx->stored();

		assert(state.m_hash == hash);

		return true;
	}

	return false;
}


void DynamicAppSignalStates::setAutoArchivingGroups(int autoArchivingGroupsCount)
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
