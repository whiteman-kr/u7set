#pragma once

#include <QHash>
#include <memory>

#include "..\lib\Types.h"


// ------------------------------------------------------------------------------------------------
//
//	Comparator class declaration
//
// ------------------------------------------------------------------------------------------------

class Comparator
{
public:
	enum CmpType
	{
		Equ,
		Greate,
		Less,
		NotEqu
	};

	Comparator();

	QString	inSignalID() const;
	void setInSignalID(const QString& inSignalID);

	CmpType cmpType() const;
	void setCmpType(CmpType cmpType);

	E::AnalogAppSignalFormat analogSignalFormat() const;
	void setAnalogSignalFormat(E::AnalogAppSignalFormat analogSignalFormat);

	bool cmpValueIsConst() const;
	void setCmpValueIsConst(bool cmpValueIsConst);

	double cmpConstValue() const;
	void setCmpConstValue(double cmpConstValue);

	QString cmpSignalID() const;
	void setCmpSignalID(const QString& cmpSignalID);

	bool hysteresisIsConst() const;
	void setHysteresisIsConst(bool hysteresisIsConst);

	double hysteresisConstValue() const;
	void sethysteresisConstValue(double gysteresisConstValue);

	QString hysteresisSignalID() const;
	void setHysteresisSignalID(const QString& hysteresisSignalID);

	QString outSignalID() const;
	void setOutSignalID(const QString& outSignalID);

	QString schemaID() const;
	void setSchemaID(const QString& schemaID);

	QUuid uuid() const;
	void setUuid(QUuid uuid);

private:
	QString	m_inSignalID;

	CmpType m_cmpType= CmpType::Equ;
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::SignedInt32;

	bool m_cmpValueIsConst = true;
	double m_cmpConstValue = 0;					// if m_cmpValueIsConst == true
	QString m_cmpSignalID;						// if m_cmpValueIsConst == false

	bool m_hysteresisIsConst = true;
	double m_hysteresisConstValue = 0;			// if m_hysteresisIsConst == true
	QString m_hysteresisSignalID;				// if m_hysteresisIsConst == false

	QString m_outSignalID;

	QString m_schemaID;
	QUuid m_uuid;
};


// ------------------------------------------------------------------------------------------------
//
//	ComparatorStorage class declaration
//
// ------------------------------------------------------------------------------------------------

class ComparatorStorage
{
public:
	ComparatorStorage();

	void insert(const QString& lmID, std::shared_ptr<Comparator> comparator);

	QList<std::shared_ptr<Comparator>> getByLmID(const QString& lmID);
	QList<std::shared_ptr<Comparator>> getByInputSignalID(const QString& appSignalID);

private:
	QHash<QString, std::shared_ptr<Comparator>> m_byLm;
	QHash<QString, std::shared_ptr<Comparator>> m_bySignal;
};

