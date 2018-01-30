#pragma once

#include <QObject>

#include "Types.h"
#include "../Proto/network.pb.h"

enum class TuningValueType
{
	Discrete,
	SignedInteger,
	Float,
	Double
};

class TuningValue
{
	Q_GADGET

public:
	TuningValue() = default;
	TuningValue(TuningValueType valueType);
	explicit TuningValue(TuningValueType valueType, double value);
	TuningValue(const Proto::TuningValue& message);

	TuningValueType type() const;
	void setType(TuningValueType valueType);

	qint32 discreteValue() const;
	void setDiscreteValue(qint32 discreteValue);

	qint32 intValue() const;
	void setIntValue(qint32 intValue);

	float floatValue() const;
	void setFloatValue(float floatValue);

	double doubleValue() const;
	void setDoubleValue(double doubleValue);

	void setValue(TuningValueType valueType, qint32 intValue, float floatValue, double doubleValue);
	void setValue(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, qint32 intValue, float floatValue, double doubleValue);

	double toDouble() const;
	void fromDouble(double value);

	double toFloat() const;
	void fromFloat(float value);

	QString toString(int precision = -1) const;
	void fromString(QString value, bool* ok = nullptr);

	bool save(Proto::TuningValue* message) const;
	bool load(const Proto::TuningValue& message);

	static TuningValueType getTuningValueType(E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat);
	static TuningValue createFromDouble(E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat, double value);

	friend bool operator < (const TuningValue& l, const TuningValue& r);
	friend bool operator <= (const TuningValue& l, const TuningValue& r);
	friend bool operator > (const TuningValue& l, const TuningValue& r);
	friend bool operator >= (const TuningValue& l, const TuningValue& r);
	friend bool operator == (const TuningValue& l, const TuningValue& r);
	friend bool operator != (const TuningValue& l, const TuningValue& r);

	static int tuningValueTypeId();

private:
	TuningValueType m_type = TuningValueType::Discrete;		// If type is Discrete or SignedInteger, then value is kept in intValue
	qint32 m_intValue = 0;
	float m_floatValue = 0.0;
	double m_doubleValue = 0.0;
};

extern bool operator < (const TuningValue& l, const TuningValue& r);
extern bool operator <= (const TuningValue& l, const TuningValue& r);
extern bool operator > (const TuningValue& l, const TuningValue& r);
extern bool operator >= (const TuningValue& l, const TuningValue& r);
extern bool operator == (const TuningValue& l, const TuningValue& r);
extern bool operator != (const TuningValue& l, const TuningValue& r);
