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

    double readLowLimit() const;
    void setReadLowLimit(double value);

    double readHighLimit() const;
    void setReadHighLimit(double value);

    int decimalPlaces() const;
	void setDecimalPlaces(int value);

	bool valid() const;
	void setValid(bool value);

	bool underflow() const;

	bool overflow() const;

    Hash appSignalHash() const;

    bool redraw();

    bool userModified() const;
    void clearUserModified();

    bool limitsUnbalance() const;

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

    double m_readLowLimit = 0;
    double m_readHighLimit = 0;

    int m_decimalPlaces = 0;

    bool m_valid = false;
    bool m_underflow = false;
    bool m_overflow = false;

	Hash m_appSignalHash = 0;

    bool m_redraw = false;
    bool m_userModified = false;
};


#endif // TUNINGOBJECT_H
