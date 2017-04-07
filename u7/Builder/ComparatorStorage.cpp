#include "ComparatorStorage.h"

// ------------------------------------------------------------------------------------------------
//
//	Comparator class implementation
//
// ------------------------------------------------------------------------------------------------

QString	Comparator::inSignalID() const
{
	return m_inSignalID;
}

void Comparator::setInSignalID(const QString& inSignalID)
{
	m_inSignalID = inSignalID;
}

Comparator::CmpType Comparator::cmpType() const
{
	return m_cmpType;
}

void Comparator::setCmpType(Comparator::CmpType cmpType)
{
	m_cmpType = cmpType;
}

E::AnalogAppSignalFormat Comparator::analogSignalFormat() const
{
	return m_analogSignalFormat;
}

void Comparator::setAnalogSignalFormat(E::AnalogAppSignalFormat analogSignalFormat)
{
	m_analogSignalFormat = analogSignalFormat;
}

bool Comparator::cmpValueIsConst() const
{
	return m_cmpValueIsConst;
}

void Comparator::setCmpValueIsConst(bool cmpValueIsConst)
{
	m_cmpValueIsConst = cmpValueIsConst;
}

double Comparator::cmpConstValue() const
{
	assert(m_cmpValueIsConst == true);

	return m_cmpConstValue;
}

void Comparator::setCmpConstValue(double cmpConstValue)
{
	assert(m_cmpValueIsConst == true);

	m_cmpConstValue = cmpConstValue;
}

QString Comparator::cmpSignalID() const
{
	assert(m_cmpValueIsConst == false);

	return m_cmpSignalID;
}

void Comparator::setCmpSignalID(const QString& cmpSignalID)
{
	assert(m_cmpValueIsConst == false);

	m_cmpSignalID = cmpSignalID;
}

bool Comparator::hysteresisIsConst() const
{
	return m_hysteresisIsConst;
}

void Comparator::setHysteresisIsConst(bool hysteresisIsConst)
{
	m_hysteresisIsConst = hysteresisIsConst;
}

double Comparator::hysteresisConstValue() const
{
	assert(m_hysteresisIsConst == true);

	return m_hysteresisConstValue;
}

void Comparator::sethysteresisConstValue(double gysteresisConstValue)
{
	assert(m_hysteresisIsConst == true);

	m_hysteresisConstValue = gysteresisConstValue;
}

QString Comparator::hysteresisSignalID() const
{
	assert(m_hysteresisIsConst == false);

	return m_hysteresisSignalID;
}

void Comparator::setHysteresisSignalID(const QString& hysteresisSignalID)
{
	assert(m_hysteresisIsConst == false);

	m_hysteresisSignalID = hysteresisSignalID;
}

QString Comparator::outSignalID() const
{
	return m_outSignalID;
}

void Comparator::setOutSignalID(const QString& outSignalID)
{
	m_outSignalID = outSignalID;
}

QString Comparator::schemaID() const
{
	return m_schemaID;
}

void Comparator::setSchemaID(const QString& schemaID)
{
	m_schemaID = schemaID;
}

QUuid Comparator::uuid() const
{
	return m_uuid;
}

void Comparator::setUuid(QUuid uuid)
{
	m_uuid = uuid;
}

// ------------------------------------------------------------------------------------------------
//
//	ComparatorStorage class implementation
//
// ------------------------------------------------------------------------------------------------


ComparatorStorage::ComparatorStorage()
{
}


void ComparatorStorage::insert(const QString& lmID, std::shared_ptr<Comparator> comparator)
{
	if (comparator->inSignalID().isEmpty() == true)
	{
		assert(false);
		return;
	}

	m_byLm.insertMulti(lmID, comparator);
	m_bySignal.insertMulti(comparator->inSignalID(), comparator);
}


QList<std::shared_ptr<Comparator>> ComparatorStorage::getByLmID(const QString& lmID)
{
	return m_byLm.values(lmID);
}


QList<std::shared_ptr<Comparator>> ComparatorStorage::getByInputSignalID(const QString& appSignalID)
{
	return m_bySignal.values(appSignalID);
}

