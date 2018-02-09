#include "TuningSignalState.h"
#include <cmath>
#include "../AppSignal.h"

//
// TuningSignalState
//
TuningSignalState::TuningSignalState(const ::Network::TuningSignalState& message)
{
	setState(message);
}

Hash TuningSignalState::hash() const
{
	return m_hash;
}

TuningValue TuningSignalState::value() const
{
	return m_value;
}

double TuningSignalState::valueToDouble() const
{
	return m_value.toDouble();
}

TuningValue TuningSignalState::newValue() const
{
	return m_newValue;
}

void TuningSignalState::setNewValue(const TuningValue& value)
{
	m_newValue = value;

	if (m_value == m_newValue)
	{
		m_flags.userModified = false;
	}
	else
	{
		m_flags.userModified = true;
	}
}

TuningValue TuningSignalState::lowBound() const
{
	return m_lowBound;
}

TuningValue TuningSignalState::highBound() const
{
	return m_highBound;
}

bool TuningSignalState::valid() const
{
	return m_flags.valid;
}

bool TuningSignalState::outOfRange() const
{
	return m_flags.outOfRange;
}

bool TuningSignalState::writeInProgress() const
{
	return m_flags.writeInProgress;
}

bool TuningSignalState::controlIsEnabled() const
{
	return m_flags.controlIsEnabled;
}

int TuningSignalState::writeErrorCode() const
{
	return m_writeErrorCode;
}

bool TuningSignalState::userModified() const
{
	return m_flags.userModified;
}

void TuningSignalState::clearUserModified()
{
	m_flags.userModified = false;
}

bool TuningSignalState::setState(const ::Network::TuningSignalState& message)
{
	m_hash = message.signalhash();
	m_flags.valid = message.valid();

	m_value.load(message.value());
	m_lowBound.load(message.readlowbound());
	m_highBound.load(message.readhighbound());

	m_flags.outOfRange = m_value < m_lowBound || m_value > m_highBound;

	m_flags.writeInProgress = message.writeinprogress();
	m_writeErrorCode = message.writeerrorcode();

	//writeClient
	//successfulReadTime
	//writeRequestTime
	//successfulWriteTime
	//unsuccessfulWriteTime

	return true;
}

void TuningSignalState::invalidate()
{
	m_flags.valid = false;
}

bool TuningSignalState::limitsUnbalance(const AppSignalParam& asp) const
{
	if (valid() == true && asp.isAnalog() == true)
	{
		if (lowBound() != asp.tuningLowBound() || highBound() != asp.tuningHighBound())
		{
			return true;
		}
	}
	return false;
}

