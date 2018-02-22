#include "TuningValue.h"
#include <cmath>

TuningValue::TuningValue(TuningValueType valueType)
{
	m_type = valueType;
}

TuningValue::TuningValue(TuningValueType valueType, double value)
{
	m_type = valueType;

	fromDouble(value);
}

TuningValue::TuningValue(const Proto::TuningValue& message)
{
	load(message);
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
	return m_int64 == 0 ? 0 : 1;
}

void TuningValue::setDiscreteValue(qint32 discreteValue)
{
	assert(m_type == TuningValueType::Discrete);
	m_int64 = discreteValue == 0 ? 0 : 1;
}

qint32 TuningValue::int32Value() const
{
	assert(m_type == TuningValueType::SignedInt32);
	return static_cast<qint32>(m_int64);
}

void TuningValue::setInt32Value(qint32 intValue)
{
	assert(m_type == TuningValueType::SignedInt32);
	m_int64 = intValue;
}

qint64 TuningValue::int64Value() const
{
	assert(false);		// remove when tuningable int64 will exists

	assert(m_type == TuningValueType::SignedInt64);
	return m_int64;
}

void TuningValue::setInt64Value(qint64 intValue)
{
	assert(false);		// remove when tuningable int64 will exists

	assert(m_type == TuningValueType::SignedInt64);
	m_int64 = intValue;
}

float TuningValue::floatValue() const
{
	assert(m_type == TuningValueType::Float);
	return static_cast<float>(m_double);
}

void TuningValue::setFloatValue(float floatValue)
{
	assert(m_type == TuningValueType::Float);
	m_double = static_cast<double>(floatValue);
}

double TuningValue::doubleValue() const
{
	assert(false);		// remove when tuningable Double will exists

	assert(m_type == TuningValueType::Double);
	return m_double;
}

void TuningValue::setDoubleValue(double doubleValue)
{
	assert(false);		// remove when tuningable Double will exists

	assert(m_type == TuningValueType::Double);
	m_double = doubleValue;
}

void TuningValue::setValue(TuningValueType valueType, qint64 intValue, double doubleValue)
{
	assert(valueType == TuningValueType::Discrete ||
		   valueType == TuningValueType::SignedInt32 ||
		   valueType == TuningValueType::Float);			// append others types when will exists

	m_type = valueType;
	m_int64 = intValue;
	m_double = doubleValue;
}

void TuningValue::setValue(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, qint64 intValue, double doubleValue)
{
	setValue(getTuningValueType(signalType, analogFormat), intValue, doubleValue);
}

double TuningValue::toDouble() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInt32:
	case TuningValueType::SignedInt64:

		return static_cast<double>(m_int64);

	case TuningValueType::Float:
	case TuningValueType::Double:

		return m_double;

	default:
		assert(false);
	}

	return 0;
}

void TuningValue::fromDouble(double value)
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		m_int64 = value == 0.0 ? 0 : 1;
		break;

	case TuningValueType::SignedInt32:
		m_int64 = static_cast<qint32>(value);
		break;

	case TuningValueType::Float:
		m_double = static_cast<float>(value);
		break;

	case TuningValueType::Double:
		m_double = value;
		break;

	default:
		assert(false);
	}
}

float TuningValue::toFloat() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInt32:
	case TuningValueType::SignedInt64:

		return static_cast<float>(m_int64);

	case TuningValueType::Float:
	case TuningValueType::Double:
		return static_cast<float>(m_double);

	default:
		assert(false);
	}

	return 0;
}

void TuningValue::fromFloat(float value)
{
	switch (m_type)
	{
	case TuningValueType::Discrete:
		m_int64 = value == 0.0 ? 0 : 1;
		break;

	case TuningValueType::SignedInt32:
		m_int64 = static_cast<qint32>(value);
		break;

	case TuningValueType::SignedInt64:
		m_int64 = static_cast<qint64>(value);
		break;

	case TuningValueType::Float:
	case TuningValueType::Double:
		m_double = static_cast<double>(value);
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
		return discreteValue() == 0 ? "0" : "1";

	case TuningValueType::SignedInt32:
		return QString::number(int32Value());

	case TuningValueType::SignedInt64:
		return QString::number(int64Value());

	case TuningValueType::Float:
		if (precision < 0)
		{
			precision = 7;
		}

		return QString::number(floatValue(), 'f', precision);

	case TuningValueType::Double:
		if (precision < 0)
		{
			precision = 15;
		}
		return QString::number(doubleValue(), 'f', precision);

	default:
		assert(false);
	}

	return "";
}

void TuningValue::fromString(QString value, bool* ok)
{
	if (ok == nullptr)
	{
		assert(false);
		return;
	}

	switch (m_type)
	{
	case TuningValueType::Discrete:
		m_int64 = value.toInt(ok) == 0 ? 0 : 1;
		break;

	case TuningValueType::SignedInt32:
		m_int64 = static_cast<qint32>(value.toInt(ok));
		break;

	case TuningValueType::SignedInt64:
		m_int64 = static_cast<qint64>(value.toLongLong(ok));
		break;

	case TuningValueType::Float:
		m_double = value.toFloat(ok);
		break;

	case TuningValueType::Double:
		m_double = value.toDouble(ok);
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
	message->set_intvalue(m_int64);
	message->set_doublevalue(m_double);

	return true;
}

bool TuningValue::load(const Proto::TuningValue& message)
{
	m_type = static_cast<TuningValueType>(message.type());
	m_int64 = message.intvalue();
	m_double = message.doublevalue();

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
			return TuningValueType::SignedInt32;

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

QString TuningValue::tuningValueTypeString() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:		return QObject::tr("Discrete");
	case TuningValueType::SignedInt32:	return QObject::tr("SignedInt32");
	case TuningValueType::SignedInt64:	return QObject::tr("SignedInt64");
	case TuningValueType::Float:		return QObject::tr("Float");
	case TuningValueType::Double:		return QObject::tr("Double");
	default:
		assert(false);
		return "?";
	}
}

qint64 TuningValue::rawInt64() const
{
	return m_int64;
}

double TuningValue::rawDouble() const
{
	return m_double;
}

bool operator < (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
		return l.discreteValue() < r.discreteValue();

	case TuningValueType::SignedInt32:
		return l.int32Value() < r.int32Value();

	case TuningValueType::SignedInt64:
		return l.int64Value() < r.int64Value();

	case TuningValueType::Float:
		return l.floatValue() < r.floatValue();

	case TuningValueType::Double:
		return l.doubleValue() < r.doubleValue();

	default:
		assert(false);
	}

	return false;
}

bool operator <= (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
		return l.discreteValue() <= r.discreteValue();

	case TuningValueType::SignedInt32:
		return l.int32Value() <= r.int32Value();

	case TuningValueType::SignedInt64:
		return l.int64Value() <= r.int64Value();

	case TuningValueType::Float:
		return l.floatValue() <= r.floatValue();

	case TuningValueType::Double:
		return l.doubleValue() <= r.doubleValue();

	default:
		assert(false);
	}

	return false;
}

bool operator > (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
		return l.discreteValue() > r.discreteValue();

	case TuningValueType::SignedInt32:
		return l.int32Value() > r.int32Value();

	case TuningValueType::SignedInt64:
		return l.int64Value() > r.int64Value();

	case TuningValueType::Float:
		return l.floatValue() > r.floatValue();

	case TuningValueType::Double:
		return l.doubleValue() > r.doubleValue();

	default:
		assert(false);
	}

	return false;
}

bool operator >= (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
		return l.discreteValue() >= r.discreteValue();

	case TuningValueType::SignedInt32:
		return l.int32Value() >= r.int32Value();

	case TuningValueType::SignedInt64:
		return l.int64Value() >= r.int64Value();

	case TuningValueType::Float:
		return l.floatValue() >= r.floatValue();

	case TuningValueType::Double:
		return l.doubleValue() >= r.doubleValue();

	default:
		assert(false);
	}

	return false;
}

bool operator == (const TuningValue& l, const TuningValue& r)
{
	assert(l.m_type == r.m_type);

	switch (l.m_type)
	{
	case TuningValueType::Discrete:
		return l.discreteValue() == r.discreteValue();

	case TuningValueType::SignedInt32:
		return l.int32Value() == r.int32Value();

	case TuningValueType::SignedInt64:
		return l.int64Value() == r.int64Value();

	case TuningValueType::Float:
		{
			float lFloat = l.floatValue();
			float rFloat = r.floatValue();

			return std::nextafter(lFloat, std::numeric_limits<float>::lowest()) <= rFloat &&
					std::nextafter(lFloat, std::numeric_limits<float>::max()) >= rFloat;
		}

	case TuningValueType::Double:
		{
			double lDouble = l.doubleValue();
			double rDouble = r.doubleValue();

			return  std::nextafter(lDouble, std::numeric_limits<double>::lowest()) <= rDouble &&
					std::nextafter(lDouble, std::numeric_limits<double>::max()) >= rDouble;
		}

	default:
		assert(false);
	}

	return false;
}

bool operator != (const TuningValue& l, const TuningValue& r)
{
	return !operator==(l, r);
}
