#include "TuningSignalState.h"

#include<memory>


//
// TuningSignalState
//

float TuningSignalState::value() const
{
	return m_value;

}

void TuningSignalState::setValue(float value)
{
	if (m_value != value)
	{
		m_flags.m_needRedraw = true;

		m_value = value;

		m_flags.m_underflow = m_value < m_readLowLimit;

		m_flags.m_overflow = m_value > m_readHighLimit;
	}
}

float TuningSignalState::editValue() const
{
	return m_editValue;
}

void TuningSignalState::onEditValue(float value)
{
	m_editValue = value;

	if (m_value == m_editValue)
	{
		m_flags.m_userModified = false;
	}
	else
	{
		m_flags.m_userModified = true;
	}

	m_flags.m_needRedraw = true;
}

bool TuningSignalState::valid() const
{
	//return true;
	return m_flags.m_valid;
}

void TuningSignalState::setValid(bool value)
{
	if (m_flags.m_valid != value)
	{
		m_flags.m_needRedraw = true;

		m_flags.m_valid = value;
	}

}

float TuningSignalState::readLowLimit() const
{
	return m_readLowLimit;
}

void TuningSignalState::setReadLowLimit(float value)
{
	if (m_readLowLimit != value)
	{
		m_flags.m_needRedraw = true;

		m_readLowLimit = value;
	}
}

float TuningSignalState::readHighLimit() const
{
	return m_readHighLimit;
}

void TuningSignalState::setReadHighLimit(float value)
{
	if (m_readHighLimit != value)
	{
		m_flags.m_needRedraw = true;

		m_readHighLimit = value;
	}
}

bool TuningSignalState::underflow() const
{
	return m_flags.m_underflow;
}

bool TuningSignalState::overflow() const
{
	return m_flags.m_overflow;
}

bool TuningSignalState::needRedraw()
{
	bool result = m_flags.m_needRedraw;

	m_flags.m_needRedraw = false;

	return result;
}

bool TuningSignalState::userModified() const
{
	return m_flags.m_userModified;
}

void TuningSignalState::clearUserModified()
{
	m_flags.m_userModified = false;
}

bool TuningSignalState::writing() const
{
	return m_flags.m_writing;
}

void TuningSignalState::setWriting(bool value)
{
	if (m_flags.m_writing != value)
	{
		m_flags.m_needRedraw = true;

		m_flags.m_writing = value;

		m_writingCounter = 0;
	}
}

void TuningSignalState::onReceiveValue(float readLowLimit, float readHighLimit, bool valid, float value, bool& writingFailed)
{
	m_readLowLimit = readLowLimit;
	m_readHighLimit = readHighLimit;
	m_value = value;

	m_flags.m_valid = valid;
	m_flags.m_underflow = m_value < m_readLowLimit;
	m_flags.m_overflow = m_value > m_readHighLimit;

	writingFailed = false;

	if (m_flags.m_writing == true)
	{
		if (value == m_editValue)
		{
			// Reset the writing flag
			//
			m_flags.m_writing = false;

			m_writingCounter = 0;

			m_flags.m_needRedraw = true;
		}
		else
		{
			const int MAX_TRY_WRITE_COUNT = 10;

			// Increase the writing counter and reset it if value is wrong in MAX_TRY_WRITE_COUNT tries
			//
			m_writingCounter++;

			if (m_writingCounter > MAX_TRY_WRITE_COUNT)
			{
				m_flags.m_needRedraw = true;

				m_flags.m_writing = false;

				m_writingCounter = 0;

				writingFailed = true;
			}
		}
	}
}


void TuningSignalState::onSendValue(float value)
{
	m_editValue = value;
}
