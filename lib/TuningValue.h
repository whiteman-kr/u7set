#pragma once

#include <QObject>

#include "Types.h"
#include "../Proto/network.pb.h"

enum class TuningValueType
{
	Discrete,
	SignedInt32,
	SignedInt64,
	Float,
	Double
};

class TuningValue
{
	Q_GADGET

public:
	TuningValue() = default;
	explicit TuningValue(TuningValueType valueType);
	explicit TuningValue(QVariant value);
	explicit TuningValue(TuningValueType valueType, double value);
	explicit TuningValue(const Proto::TuningValue& message);

	TuningValueType type() const;
	void setType(TuningValueType valueType);

	QString typeStr() const;

	qint32 discreteValue() const;
	void setDiscreteValue(qint32 discreteValue);

	qint32 int32Value() const;
	void setInt32Value(qint32 int32Value);

	qint64 int64Value() const;
	void setInt64Value(qint64 int32Value);

	float floatValue() const;
	void setFloatValue(float floatValue);

	double doubleValue() const;
	void setDoubleValue(double doubleValue);

	void setValue(TuningValueType valueType, qint64 int32Value, double doubleValue);
	void setValue(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, qint64 intValue, double doubleValue);
	void setValue(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, const QVariant& value);

	QVariant toVariant() const;
	void fromVariant(QVariant value);

	double toDouble() const;
	void fromDouble(double value);

	float toFloat() const;
	void fromFloat(float value);

	QString toString() const;
	QString toString(E::AnalogFormat analogFormat, int precision) const;

	void fromString(QString value, bool* ok = nullptr);

	bool save(Proto::TuningValue* message) const;
	bool load(const Proto::TuningValue& message);

	static TuningValueType getTuningValueType(E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat);
	static TuningValue createFromDouble(E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat, double value);
	static TuningValueType typeFromStr(const QString& typeStr);

	friend bool operator < (const TuningValue& l, const TuningValue& r);
	friend bool operator <= (const TuningValue& l, const TuningValue& r);
	friend bool operator > (const TuningValue& l, const TuningValue& r);
	friend bool operator >= (const TuningValue& l, const TuningValue& r);
	friend bool operator == (const TuningValue& l, const TuningValue& r);
	friend bool operator != (const TuningValue& l, const TuningValue& r);

	static int tuningValueTypeId();
	QString tuningValueTypeString() const;

	qint64 rawInt64() const;
	double rawDouble() const;

private:
	TuningValueType m_type = TuningValueType::Discrete;		// If type is Discrete or SignedInteger, then value is kept in intValue
	qint64 m_int64 = 0;
	double m_double = 0.0;

	static const char* TYPE_STR_DISCRETE;
	static const char* TYPE_STR_SIGNED_INT32;
	static const char* TYPE_STR_SIGNED_INT64;
	static const char* TYPE_STR_FLOAT;
	static const char* TYPE_STR_DOUBLE;
	static const char* TYPE_STR_UNKNOWN;
};

Q_DECLARE_METATYPE(TuningValue)

extern bool operator < (const TuningValue& l, const TuningValue& r);
extern bool operator <= (const TuningValue& l, const TuningValue& r);
extern bool operator > (const TuningValue& l, const TuningValue& r);
extern bool operator >= (const TuningValue& l, const TuningValue& r);
extern bool operator == (const TuningValue& l, const TuningValue& r);
extern bool operator != (const TuningValue& l, const TuningValue& r);
