#include "AppSignalStateEx.h"


// -------------------------------------------------------------------------------
//
// AppSignalState class implementation
//
// -------------------------------------------------------------------------------

AppSignalStateEx::AppSignalStateEx()
{
	m_current.m_flags.all = 0;
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

	m_aperture = signal->aperture();
	m_lowLimit = signal->lowEngeneeringUnits();
	m_highLimit = signal->highEngeneeringUnits();

	m_absAperture = fabs(m_highLimit - m_lowLimit) * (m_aperture / 100.0);

	Hash hash = calcHash(signal->appSignalID());

	m_current.m_hash = hash;
	m_stored.m_hash = hash;
}


void AppSignalStateEx::setState(Times time, quint32 validity, double value)
{
	bool updateStoredState = false;

	// update current state
	//
	m_current.m_time = time;
	m_current.m_flags.valid = validity;
	m_current.m_value = value;

	if (m_initialized == false)
	{
		m_initialized = true;
		m_stored = m_current;			// also init stored state before value testings
		updateStoredState = true;
	}

	if (validity == VALID_STATE)
	{
		// current state is Valid
		//
		if (m_isDiscreteSignal == true)
		{
			if (m_stored.m_value != m_current.m_value)
			{
				updateStoredState = true;
			}
		}
		else
		{
			// is analog signal
			//
			if (fabs(m_stored.m_value - m_current.m_value) > m_absAperture)
			{
				updateStoredState = true;
			}

//			if (value > m_highLimit)
//			{
//				m_current.flags.overflow = 1;
//			}

//			if (value < m_lowLimit)
//			{
//				m_current.flags.underflow = 1;
//			}
		}
	}

	if (m_stored.m_flags.all != m_current.m_flags.all)
	{
		updateStoredState = true;		// changes of signal flags always write
	}

	if (updateStoredState)
	{
		// update stored state
		//
		m_stored = m_current;

		//qDebug() << "State changes " << m_signal->appSignalID() << " val = " << m_current.value  << " flags = " << m_current.flags.all;
	}
}


Hash AppSignalStateEx::hash() const
{
	assert(m_current.m_hash == m_stored.m_hash);

	return m_current.m_hash;
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

		state = stateEx->m_current;

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

		state = stateEx->m_stored;

		assert(state.m_hash == hash);

		return true;
	}

	return false;
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
