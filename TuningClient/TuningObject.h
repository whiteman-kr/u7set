#ifndef TUNINGOBJECT_H
#define TUNINGOBJECT_H

#include "Stable.h"
#include "../lib/Hash.h"

class TuningObject
{
public:

	TuningObject();

	QString customAppSignalID() const;
	void setCustomAppSignalID(const QString& value);

	QString equipmentID() const;
	void setEquipmentID(const QString& value);

	QString appSignalID() const;
	void setAppSignalID(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	QString units() const;
	void setUnits(const QString& value);

	bool analog() const;
	void setAnalog(bool value);

	double value() const;
	void setValue(double value);

	double editValue() const;
	void setEditValue(double value);

	double defaultValue() const;
	void setDefaultValue(double value);

	double lowLimit() const;
	void setLowLimit(double value);

	double highLimit() const;
	void setHighLimit(double value);

	int decimalPlaces() const;
	void setDecimalPlaces(int value);

	bool valid() const;
	void setValid(bool value);

	bool underflow() const;
	void setUnderflow(bool value);

	bool overflow() const;
	void setOverflow(bool value);

	Hash appSignalHash() const;

private:

	QString m_customAppSignalID;
	QString m_equipmentID;
	QString m_appSignalID;
	QString m_caption;
	QString m_units;

	bool m_analog = false;

	double m_defaultValue = 0;
	double m_value = 0;
	double m_editValue = 0;

	double m_lowLimit = 0;
	double m_highLimit = 0;

	int m_decimalPlaces = 0;

	bool m_valid = 0;
	bool m_underflow = 0;
	bool m_overflow = 0;

	Hash m_appSignalHash = 0;
};


#endif // TUNINGOBJECT_H
