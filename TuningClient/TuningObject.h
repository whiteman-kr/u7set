#ifndef TUNINGOBJECT_H
#define TUNINGOBJECT_H

#include "Stable.h"

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

	QVariant value() const;
	void setValue(const QVariant& value);

	bool valid() const;
	void setValid(bool value);

	bool underflow() const;
	void setUnderflow(bool value);

	bool overflow() const;
	void setOverflow(bool value);


private:

	QString m_customAppSignalID;
	QString m_equipmentID;
	QString m_appSignalID;
	QString m_caption;
	QString m_units;

	bool m_analog;

	QVariant m_value;

	bool m_valid;
	bool m_underflow;
	bool m_overflow;
};


#endif // TUNINGOBJECT_H
