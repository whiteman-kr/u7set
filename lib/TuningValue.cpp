#include "TuningValue.h"
#include <cmath>

const char* TuningValue::TYPE_STR_DISCRETE = "Discrete";
const char* TuningValue::TYPE_STR_SIGNED_INT32 = "SignedInt32";
const char* TuningValue::TYPE_STR_SIGNED_INT64 = "SignedInt64";
const char* TuningValue::TYPE_STR_FLOAT = "Float";
const char* TuningValue::TYPE_STR_DOUBLE = "Double";
const char* TuningValue::TYPE_STR_UNKNOWN = "?";

TuningValue::TuningValue(TuningValueType valueType)
{
	m_type = valueType;
}

TuningValue::TuningValue(QVariant value)
{
	fromVariant(value);
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

QString TuningValue::typeStr() const
{
	switch(m_type)
	{
	case TuningValueType::Discrete:
		return TYPE_STR_DISCRETE;

	case TuningValueType::SignedInt32:
		return TYPE_STR_SIGNED_INT32;

	case TuningValueType::SignedInt64:
		return TYPE_STR_SIGNED_INT64;

	case TuningValueType::Float:
		return TYPE_STR_FLOAT;

	case TuningValueType::Double:
		return TYPE_STR_DOUBLE;

	default:
		assert(false);
		return TYPE_STR_UNKNOWN;
	}
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
	assert(false);		// remove when tunable int64 will exists

	assert(m_type == TuningValueType::SignedInt64);
	return m_int64;
}

void TuningValue::setInt64Value(qint64 intValue)
{
	assert(false);		// remove when tunable int64 will exists

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
	assert(false);		// remove when tunable Double will exists

	assert(m_type == TuningValueType::Double);
	return m_double;
}

void TuningValue::setDoubleValue(double doubleValue)
{
	assert(false);		// remove when tunable Double will exists

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

void TuningValue::setValue(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, const QVariant& value)
{
	setValue(getTuningValueType(signalType, analogFormat), value.toLongLong(), value.toDouble());
}

QVariant TuningValue::toVariant() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:

		return QVariant(discreteValue());

	case TuningValueType::SignedInt32:

		return QVariant(int32Value());

	case TuningValueType::SignedInt64:

		assert(false);		// remove when tunable int64 will exist
		return QVariant(int64Value());

	case TuningValueType::Float:

		return QVariant(floatValue());

	case TuningValueType::Double:

		assert(false);		// remove when tunable double will exist
		return QVariant(doubleValue());

	default:
		assert(false);
	}

	return QVariant();
}

void TuningValue::fromVariant(QVariant value)
{
	switch (static_cast<QMetaType::Type>(value.type()))	// From help: Although this function is declared as returning QVariant::Type, the return value
														// should be interpreted as QMetaType::Type. In particular, QVariant::UserType is returned here only
														// if the value is equal or greater than QMetaType::User.
	{
	case QMetaType::Bool:

		m_type = TuningValueType::Discrete;
		m_int64 = value.toBool() == true ? 1 : 0;

		break;

	case QMetaType::Int:

		m_type = TuningValueType::SignedInt32;
		m_int64 = value.toInt();

		break;

	case QMetaType::LongLong:

		assert(false);		// remove when tunable int64 will exist
		m_type = TuningValueType::SignedInt64;
		m_int64 = value.toLongLong();

		break;

	case QMetaType::Float:

		m_type = TuningValueType::Float;
		m_double = value.toFloat();

		break;

	case QMetaType::Double:

		assert(false);		// remove when tunable double will exist
		m_type = TuningValueType::Double;
		m_double = value.toDouble();

		break;

	default:
		assert(false);
	}

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

QString TuningValue::toString() const
{
	return toString(E::AnalogFormat::g_9_or_9e, -1);
}

QString TuningValue::toString(E::AnalogFormat analogFormat, int precision) const
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
		if (analogFormat == E::AnalogFormat::g_9_or_9e || analogFormat == E::AnalogFormat::G_9_or_9E)
		{
			return QString::number(floatValue(), static_cast<char>(analogFormat));
		}

		if (precision < 0)
		{
			precision = 7;
		}

		return QString::number(floatValue(), static_cast<char>(analogFormat), precision);

	case TuningValueType::Double:
		if (analogFormat == E::AnalogFormat::g_9_or_9e || analogFormat == E::AnalogFormat::G_9_or_9E)
		{
			return QString::number(doubleValue(), static_cast<char>(analogFormat));
		}

		if (precision < 0)
		{
			precision = 15;
		}

		return QString::number(doubleValue(), static_cast<char>(analogFormat), precision);

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
		m_double = static_cast<double>(value.toFloat(ok));
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

TuningValueType TuningValue::typeFromStr(const QString& typeStr)
{
	if (typeStr == QString(TYPE_STR_DISCRETE))
	{
		return TuningValueType::Discrete;
	}

	if (typeStr == QString(TYPE_STR_SIGNED_INT32))
	{
		return TuningValueType::SignedInt32;
	}

	if (typeStr == QString(TYPE_STR_SIGNED_INT64))
	{
		return TuningValueType::SignedInt64;
	}

	if (typeStr == QString(TYPE_STR_FLOAT))
	{
		return TuningValueType::Float;
	}

	if (typeStr == QString(TYPE_STR_DOUBLE))
	{
		return TuningValueType::Double;
	}

	assert(false);

	return TuningValueType::Discrete;
}

int TuningValue::tuningValueTypeId()
{
	return qMetaTypeId<TuningValue>();
}

QString TuningValue::tuningValueTypeString() const
{
	switch (m_type)
	{
	case TuningValueType::Discrete:		return QObject::tr(TYPE_STR_DISCRETE);
	case TuningValueType::SignedInt32:	return QObject::tr(TYPE_STR_SIGNED_INT32);
	case TuningValueType::SignedInt64:	return QObject::tr(TYPE_STR_SIGNED_INT64);
	case TuningValueType::Float:		return QObject::tr(TYPE_STR_FLOAT);
	case TuningValueType::Double:		return QObject::tr(TYPE_STR_DOUBLE);
	default:
		assert(false);
		return TYPE_STR_UNKNOWN;
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
