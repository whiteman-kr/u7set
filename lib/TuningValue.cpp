#include "TuningValue.h"
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

TuningValue::TuningValue(const Proto::TuningValue& message)
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
	return m_intValue == 0 ? 0 : 1;
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

void TuningValue::fromDouble(double value)
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		m_intValue = static_cast<qint32>(value);
		break;

	case TuningValueType::SignedInteger:
		m_intValue = static_cast<qint32>(value);
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
}

double TuningValue::toFloat() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		return static_cast<float>(m_intValue);

	case TuningValueType::SignedInteger:
		return static_cast<float>(m_intValue);

	case TuningValueType::Float:
		return m_floatValue;

	case TuningValueType::Double:
		return static_cast<float>(m_doubleValue);

	default:
		assert(false);
		return 0;
	}
}

void TuningValue::fromFloat(float value)
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		m_intValue = static_cast<qint32>(value);
		break;

	case TuningValueType::SignedInteger:
		m_intValue = static_cast<qint32>(value);
		break;

	case TuningValueType::Float:
		m_floatValue = value;
		break;

	case TuningValueType::Double:
		m_doubleValue = static_cast<double>(value);
		break;

	default:
		assert(false);
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

void TuningValue::fromString(QString value, bool* ok)
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		m_intValue = static_cast<qint32>(value.toInt(ok));
		break;

	case TuningValueType::SignedInteger:
		m_intValue = static_cast<qint32>(value.toInt(ok));
		break;

	case TuningValueType::Float:
		m_floatValue = value.toFloat(ok);
		break;

	case TuningValueType::Double:
		m_doubleValue = value.toDouble(ok);
		break;

	default:
		assert(false);
	}
}

bool TuningValue::save(Proto::TuningValue* message) const
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

bool TuningValue::load(const Proto::TuningValue& message)
{
	m_type = static_cast<TuningValueType>(message.type());
	m_intValue = message.intvalue();
	m_floatValue = message.floatvalue();
	m_doubleValue = message.doublevalue();

	return true;
}

TuningValueType TuningValue::getTuningValueType(E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat)
{
	switch(signalType)
	{
	case E::SignalType::Discrete:
		return TuningValueType::Discrete;

	case E::SignalType::Analog:

		switch(analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::SignedInt32:
			return TuningValueType::SignedInteger;

		case E::AnalogAppSignalFormat::Float32:
			return TuningValueType::Float;

		default:
			assert(false);
		}

		break;

	case E::SignalType::Bus:
		return TuningValueType::Discrete;

	default:
		assert(false);
	}

	return TuningValueType::Discrete;
}

TuningValue TuningValue::createFromDouble(E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat, double value)
{
	TuningValue tv;

	tv.setType(getTuningValueType(signalType, analogSignalFormat));
	tv.fromDouble(value);

	return tv;
}

int TuningValue::tuningValueTypeId()
{
	return qMetaTypeId<TuningValue>();
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
