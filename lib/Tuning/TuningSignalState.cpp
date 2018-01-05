#include "TuningSignalState.h"
#include <cmath>

TuningValue::TuningValue(double value, TuningValueType valueType)
{
	m_type = valueType;

	switch (m_type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		m_intValue = static_cast<int>(value);
		break;

	case TuningValueType::Float:
		m_floatValue = static_cast<float>(value);
		break;

	case TuningValueType::Double:
		m_doubleValue = value;
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

TuningValueType TuningValue::type() const
{
	return m_type;
}

void TuningValue::setType(TuningValueType valueType)
{
	m_type = valueType;
}

qint32 TuningValue::discreteValue() const
{
	assert(m_type == TuningValueType::Discrete);
	return m_intValue;
}

void TuningValue::setDiscreteValue(qint32 discreteValue)
{
	assert(m_type == TuningValueType::Discrete);
	m_intValue = discreteValue;
}

qint32 TuningValue::intValue() const
{
	assert(m_type == TuningValueType::SignedInteger);
	return m_intValue;
}

void TuningValue::setIntValue(qint32 intValue)
{
	assert(m_type == TuningValueType::SignedInteger);
	m_intValue = intValue;
}

float TuningValue::floatValue() const
{
	assert(m_type == TuningValueType::Float);
	return m_floatValue;
}

void TuningValue::setFloatValue(float floatValue)
{
	assert(m_type == TuningValueType::Float);
	m_floatValue = floatValue;
}

double TuningValue::doubleValue() const
{
	assert(m_type == TuningValueType::Double);
	return m_doubleValue;
}

void TuningValue::setDoubleValue(double doubleValue)
{
	assert(m_type == TuningValueType::Double);
	m_doubleValue = doubleValue;
}

double TuningValue::toDouble() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		return static_cast<double>(m_intValue);

	case TuningValueType::SignedInteger:
		return static_cast<double>(m_intValue);

	case TuningValueType::Float:
		return static_cast<double>(m_floatValue);

	case TuningValueType::Double:
		return m_doubleValue;

	default:
		assert(false);
		return 0;
	}
}

QString TuningValue::toString(int precision) const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		return m_intValue == 1 ? "1" : "0";

	case TuningValueType::SignedInteger:
		return QString::number(m_intValue);

	case TuningValueType::Float:
		if (precision < 0)
		{
			precision = 6;
		}
		return QString::number(m_floatValue, 'f', precision);

	case TuningValueType::Double:
		if (precision < 0)
		{
			precision = 12;
		}
		return QString::number(m_doubleValue, 'f', precision);

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

	message->set_type(static_cast<int>(m_type));
	message->set_intvalue(m_intValue);
	message->set_floatvalue(m_floatValue);
	message->set_doublevalue(m_doubleValue);

	return true;
}

bool TuningValue::load(const Network::TuningValue& message)
{
	m_type = static_cast<TuningValueType>(message.type());
	m_intValue = message.intvalue();
	m_floatValue = message.floatvalue();
	m_doubleValue = message.doublevalue();

	return true;
}

bool operator < (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		return l.m_intValue < r.m_intValue;

	case TuningValueType::Float:
		return l.m_floatValue < r.m_floatValue;

	case TuningValueType::Double:
		return l.m_doubleValue < r.m_doubleValue;

	default:
		assert(false);
		return false;
	}
}

bool operator > (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		return l.m_intValue > r.m_intValue;

	case TuningValueType::Float:
		return l.m_floatValue > r.m_floatValue;

	case TuningValueType::Double:
		return l.m_doubleValue > r.m_doubleValue;

	default:
		assert(false);
		return false;
	}
}

bool operator == (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		return l.m_intValue == r.m_intValue;

	case TuningValueType::Float:
		return  std::nextafter(l.m_floatValue, std::numeric_limits<float>::lowest()) <= r.m_floatValue &&
				std::nextafter(l.m_floatValue, std::numeric_limits<float>::max()) >= r.m_floatValue;

	case TuningValueType::Double:
		return  std::nextafter(l.m_doubleValue, std::numeric_limits<double>::lowest()) <= r.m_doubleValue &&
				std::nextafter(l.m_doubleValue, std::numeric_limits<double>::max()) >= r.m_doubleValue;

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

