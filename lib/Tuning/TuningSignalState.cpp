#include "TuningSignalState.h"
#include <cmath>

TuningValue::TuningValue(double value, TuningValueType valueType)
{
	type = valueType;

	switch (type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		intValue = static_cast<int>(value);
		break;

	case TuningValueType::Float:
		floatValue = static_cast<float>(value);
		break;

	case TuningValueType::Double:
		doubleValue = value;
		break;

	default:
		assert(false);
	}
	return;
}

TuningValue::TuningValue(const Network::TuningValue& message)
{
	load(message);
	return;
}

double TuningValue::toDouble() const
{
	switch (type)
	{
	case TuningValueType::Discrete:
		return intValue;

	case TuningValueType::SignedInteger:
		return intValue;

	case TuningValueType::Float:
		return floatValue;

	case TuningValueType::Double:
		return doubleValue;

	default:
		assert(false);
		return 0;
	}
}

QString TuningValue::toString(int precision) const
{
	switch (type)
	{
	case TuningValueType::Discrete:
		return intValue == 1 ? "1" : "0";

	case TuningValueType::SignedInteger:
		return QString::number(intValue);

	case TuningValueType::Float:
		if (precision < 0)
		{
			precision = 6;
		}
		return QString::number(floatValue, 'f', precision);

	case TuningValueType::Double:
		if (precision < 0)
		{
			precision = 12;
		}
		return QString::number(doubleValue, 'f', precision);

	default:
		assert(false);
		return 0;
	}
}

bool TuningValue::save(Network::TuningValue* message) const
{
	if (message == nullptr)
	{
		assert(message);
		return false;
	}

	message->set_type(static_cast<int>(type));
	message->set_intvalue(intValue);
	message->set_floatvalue(floatValue);
	message->set_doublevalue(doubleValue);

	return true;
}

bool TuningValue::load(const Network::TuningValue& message)
{
	type = static_cast<TuningValueType>(message.type());
	intValue = message.intvalue();
	floatValue = message.floatvalue();
	doubleValue = message.doublevalue();

	return true;
}

bool operator < (const TuningValue& l, const TuningValue& r)
{
	assert(l.type == r.type);

	switch (l.type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		return l.intValue < r.intValue;

	case TuningValueType::Float:
		return l.floatValue < r.floatValue;

	case TuningValueType::Double:
		return l.doubleValue < r.doubleValue;

	default:
		assert(false);
		return false;
	}
}

bool operator > (const TuningValue& l, const TuningValue& r)
{
	assert(l.type == r.type);

	switch (l.type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		return l.intValue > r.intValue;

	case TuningValueType::Float:
		return l.floatValue > r.floatValue;

	case TuningValueType::Double:
		return l.doubleValue > r.doubleValue;

	default:
		assert(false);
		return false;
	}
}

bool operator == (const TuningValue& l, const TuningValue& r)
{
	assert(l.type == r.type);

	switch (l.type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		return l.intValue == r.intValue;

	case TuningValueType::Float:
		return  std::nextafter(l.floatValue, std::numeric_limits<float>::lowest()) <= r.floatValue &&
				std::nextafter(l.floatValue, std::numeric_limits<float>::max()) >= r.floatValue;

	case TuningValueType::Double:
		return  std::nextafter(l.doubleValue, std::numeric_limits<double>::lowest()) <= r.doubleValue &&
				std::nextafter(l.doubleValue, std::numeric_limits<double>::max()) >= r.doubleValue;

	default:
		assert(false);
		return false;
	}
}

bool operator != (const TuningValue& l, const TuningValue& r)
{
	return !operator==(l, r);
}

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

	return true;
}

void TuningSignalState::invalidate()
{
	m_flags.valid = false;
}

