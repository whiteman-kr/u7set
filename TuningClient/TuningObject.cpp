#include "TuningObject.h"

TuningObject::TuningObject()
{
	m_value = 0;
	m_valid = false;
	m_underflow = false;
	m_overflow = false;

}

QString TuningObject::customAppSignalID() const
{
	return m_customAppSignalID;
}

void TuningObject::setCustomAppSignalID(const QString& value)
{
	m_customAppSignalID = value;
}

QString TuningObject::equipmentID() const
{
	return m_equipmentID;
}

void TuningObject::setEquipmentID(const QString& value)
{
	m_equipmentID = value;
}

QString TuningObject::appSignalID() const
{
	return m_appSignalID;
}

void TuningObject::setAppSignalID(const QString& value)
{
	m_appSignalID = value;
}

QString TuningObject::caption() const
{
	return m_caption;
}

void TuningObject::setCaption(const QString& value)
{
	m_caption = value;
}

QString TuningObject::units() const
{
	return m_units;
}

void TuningObject::setUnits(const QString& value)
{
	m_units = value;
}

bool TuningObject::analog() const
{
	return m_analog;
}

void TuningObject::setAnalog(bool value)
{
	m_analog = value;
}

QVariant TuningObject::value() const
{
	return m_value;

}

void TuningObject::setValue(const QVariant& value)
{
	m_value = value;
}

bool TuningObject::valid() const
{
	return m_valid;
}

void TuningObject::setValid(bool value)
{
	m_valid = value;
}

bool TuningObject::underflow() const
{
	return m_underflow;
}

void TuningObject::setUnderflow(bool value)
{
	m_underflow = value;
}

bool TuningObject::overflow() const
{
	return m_overflow;
}

void TuningObject::setOverflow(bool value)
{
	m_overflow = value;
}
