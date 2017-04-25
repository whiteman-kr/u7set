#include "TuningSignalState.h"

#include<memory>


//
// TuningSignalState
//

float TuningSignalState::value() const
{
	return m_value;

}

float TuningSignalState::readLowLimit() const
{
	return m_readLowLimit;
}

float TuningSignalState::readHighLimit() const
{
	return m_readHighLimit;
}

bool TuningSignalState::underflow() const
{
	return m_flags.m_underflow;
}

bool TuningSignalState::overflow() const
{
	return m_flags.m_overflow;
}

bool TuningSignalState::valid() const
{
	//return true;
	return m_flags.m_valid;
}

bool TuningSignalState::writing() const
{
	return m_flags.m_writing;
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

// Copy function that updates the redraw flag
//
void TuningSignalState::copy(const TuningSignalState& source)
{
	if (m_readHighLimit != source.readHighLimit())
	{
		m_flags.m_needRedraw = true;
		m_readHighLimit = source.readHighLimit();
	}

	if (m_readLowLimit != source.readLowLimit())
	{
		m_flags.m_needRedraw = true;
		m_readLowLimit = source.readLowLimit();
	}

	if (m_value != source.value())
	{
		m_flags.m_needRedraw = true;

		m_value = source.value();
	}

	m_flags.m_underflow = m_value < m_readLowLimit;
	m_flags.m_overflow = m_value > m_readHighLimit;

	if (m_flags.m_valid != source.valid())
	{
		m_flags.m_needRedraw = true;
		m_flags.m_valid = source.valid();
	}

	if (m_flags.m_writing != source.writing())
	{
		m_flags.m_needRedraw = true;
		m_flags.m_writing = source.writing();
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

	m_flags.m_writing = true;
}

void TuningSignalState::invalidate()
{
	m_flags.m_valid = false;
}
