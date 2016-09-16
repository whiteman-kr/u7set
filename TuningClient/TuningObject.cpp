#include "TuningObject.h"

TuningObject::TuningObject()
	:
	  m_value(0.0),
	  m_lowLimit(0.0),
	  m_highLimit(0.0),
	  m_decimalPlaces(0),
	  m_valid(false),
	  m_underflow(false),
	  m_overflow(false),
	  m_analog(false)
{


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

double TuningObject::lowLimit() const
{
	return m_lowLimit;
}

void TuningObject::setLowLimit(double value)
{
	m_lowLimit = value;
}

double TuningObject::highLimit() const
{
	return m_highLimit;
}

void TuningObject::setHighLimit(double value)
{
	m_highLimit = value;
}

int TuningObject::decimalPlaces() const
{
	return m_decimalPlaces;
}

void TuningObject::setDecimalPlaces(int value)
{
	m_decimalPlaces = value;
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
