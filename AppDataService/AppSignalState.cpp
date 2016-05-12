#include "AppSignalState.h"


// -------------------------------------------------------------------------------
//
// AppSignalState class implementation
//
// -------------------------------------------------------------------------------

AppSignalState::AppSignalState()
{
	m_flags.reset();
}


void AppSignalState::setSignalParams(int index, Signal* signal)
{
	if (signal == nullptr)
	{
		assert(false);
		return;
	}

	m_index = index;
	m_signal = signal;

	m_aperture = signal->aperture();
	m_lowLimit = signal->lowLimit();
	m_highLimit = signal->highLimit();

	m_absAperture = fabs(m_highLimit - m_lowLimit) * (m_aperture / 100.0);
}


void AppSignalState::setState(Times time, AppSignalStateFlags flags, double value)
{
	bool writeStateChanges = false;

	if (m_initialized == false)
	{
		writeStateChanges = true;
		m_initialized = true;
	}
	else
	{
		if (m_flags.valid != flags.valid)
		{
			writeStateChanges = true;		// changes of signal validity always writing
		}

		if (flags.valid)
		{
			// current state is Valid
			//
			if (m_isDiscreteSignal)
			{
				// is discrete signal
				//
				if (static_cast<int>(value) != static_cast<int>(m_value))
				{
					writeStateChanges = true;
				}
			}
			else
			{
				// is analog signal
				//
				if (fabs(value - m_value) > m_absAperture)
				{
					writeStateChanges = true;
				}

				if (value > m_highLimit)
				{
					flags.overflow = 1;
				}

				if (value < m_lowLimit)
				{
					flags.underflow = 1;
				}

				if (flags.allFlags != m_flags.allFlags)
				{
					writeStateChanges = true;
				}
			}
		}
	}

	if (writeStateChanges)
	{
		m_time = time;

		m_flags = flags;
		m_value = value;
	}
}


AppSignalStates::~AppSignalStates()
{
	clear();
}


void AppSignalStates::clear()
{
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

	m_appSignalState = new AppSignalState[size];
	m_size = size;
}


AppSignalState* AppSignalStates::operator [] (int index)
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
	for(Signal* signal : *this)
	{
		delete signal;
	}

	HashedVector<QString, Signal*>::clear();
}
