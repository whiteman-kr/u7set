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

	m_aperture = signal->aperture();
	m_lowLimit = signal->lowEngeneeringUnits();
	m_highLimit = signal->highEngeneeringUnits();

	m_absAperture = fabs(m_highLimit - m_lowLimit) * (m_aperture / 100.0);
}


void AppSignalStateEx::setState(Times time, quint32 validity, double value)
{
	bool updateStoredState = false;

	// update current state
	//
	m_current.time = time;
	m_current.flags.valid = validity;
	m_current.value = value;

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
			if (m_stored.value != m_current.value)
			{
				updateStoredState = true;
			}
		}
		else
		{
			// is analog signal
			//
			if (fabs(m_stored.value - m_current.value) > m_absAperture)
			{
				updateStoredState = true;
			}

			if (value > m_highLimit)
			{
				m_current.flags.overflow = 1;
			}

			if (value < m_lowLimit)
			{
				m_current.flags.underflow = 1;
			}
		}
	}

	if (m_stored.flags.all != m_current.flags.all)
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


QString AppSignalStateEx::appSignalID()
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

		if (state.m_signal == nullptr)
		{
			assert(false);
			continue;
		}

		Hash hash = calcHash(state.m_signal->appSignalID());

		m_hash2State.insert(hash, &state);
	}
}


bool AppSignalStates::getState(Hash hash, AppSignalState& state) const
{
	if (m_hash2State.contains(hash))
	{
		const AppSignalStateEx* stateEx = m_hash2State[hash];

		state = stateEx->m_current;

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
