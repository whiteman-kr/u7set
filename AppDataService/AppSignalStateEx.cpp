#include "AppSignalStateEx.h"


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

	m_roughAperture = signal->roughAperture();
	m_smoothAperture = signal->smoothAperture();

	m_lowLimit = signal->lowEngeneeringUnits();
	m_highLimit = signal->highEngeneeringUnits();
	m_adaptiveAperture = signal->adaptiveAperture();

	if (m_adaptiveAperture == false)
	{
		m_absRoughAperture = fabs(m_highLimit - m_lowLimit) * (m_roughAperture / 100.0);
		m_absSmoothAperture = fabs(m_highLimit - m_lowLimit) * (m_smoothAperture / 100.0);
	}

	Hash hash = calcHash(signal->appSignalID());

	m_current.hash = hash;
	m_stored.hash = hash;
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
						double absAperture = (fabs(m_current.value - m_stored.value) * 100) / m_stored.value;

						if (absAperture > m_smoothAperture)
						{
							m_current.flags.smoothAperture = 1;
						}

						if (absAperture > m_roughAperture)
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
