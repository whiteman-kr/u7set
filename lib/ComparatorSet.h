#pragma once

#include "../lib/Types.h"
#include "../Proto/serialization.pb.h"

#include <QUuid>
#include <QMutex>
#include <QVector>

// ------------------------------------------------------------------------------------------------
//
//	ComparatorSignal class declaration
//
// ------------------------------------------------------------------------------------------------

class ComparatorSignal
{
public:

	void setSignalParams(const QString& appSignalID, bool isAcquired, bool isConst, double constValue);
	void setConstValue(double constValue);

	bool isConst() const;
	double constValue() const;
	QString appSignalID() const;
	bool isAcquired() const;

	void serializeTo(Proto::ComparatorSignal* s) const;
	bool serializeFrom(const Proto::ComparatorSignal& s);

	void dump() const;

private:

	bool m_isConst = true;
	double m_constValue = 0;						// if m_isConst == true
	QString m_appSignalID;							// if m_isConst == false
	bool m_isAcquired = true;
};

// ------------------------------------------------------------------------------------------------
//
//	Comparator class declaration
//
// ------------------------------------------------------------------------------------------------

class Comparator
{
public:

	E::CmpType cmpType() const;
	void setCmpType(E::CmpType cmpType);

	E::AnalogAppSignalFormat inAnalogSignalFormat() const;
	void setInAnalogSignalFormat(E::AnalogAppSignalFormat inAnalogSignalFormat);

	ComparatorSignal& input();
	ComparatorSignal& compare();
	ComparatorSignal& hysteresis();
	ComparatorSignal& output();

	const ComparatorSignal& input() const;
	const ComparatorSignal& compare() const;
	const ComparatorSignal& hysteresis() const;
	const ComparatorSignal& output() const;

	QString label() const;
	void setLabel(const QString& label);

	int precision() const;
	void setPrecision(int precision);

	QString schemaID() const;
	void setSchemaID(const QString& schemaID);

	QUuid schemaItemUuid() const;
	void setSchemaItemUuid(QUuid schemaItemUuid);

	void serializeTo(Proto::Comparator* c) const;
	bool serializeFrom(const Proto::Comparator& c);

	void setHysteresisPinCaption(const QString& pinCaption) { m_hysteresisPinCaption = pinCaption; }
	QString hysteresisPinCaption() const { return m_hysteresisPinCaption; }

	void setHysteresisIsConstSignal(bool isConstSignal) { m_hysteresisIsConstSignal = isConstSignal; }
	bool hysteresisIsConstSignal() const { return m_hysteresisIsConstSignal; }

	void dump() const;

private:

	E::CmpType m_cmpType = E::CmpType::Equal;
	E::AnalogAppSignalFormat m_inAnalogSignalFormat = E::AnalogAppSignalFormat::SignedInt32;

	ComparatorSignal m_inputSignal;
	ComparatorSignal m_compareSignal;
	ComparatorSignal m_hysteresisSignal;
	ComparatorSignal m_outputSignal;

	int m_precision = 2;
	QString m_label;
	QString m_schemaID;
	QUuid m_schemaItemUuid;

	// fields not for serialization
	//
	bool m_hysteresisIsConstSignal = false;
	QString m_hysteresisPinCaption;
};


// ------------------------------------------------------------------------------------------------
//
//	LmComparatorSet class declaration
//
// ------------------------------------------------------------------------------------------------

class LmComparatorSet
{
public:

	LmComparatorSet();
	LmComparatorSet(const QString& lmID, std::shared_ptr<Comparator> omparator);
	~LmComparatorSet();

	void clear();
	void append(std::shared_ptr<Comparator> comparator);

	QString	lmID() const;
	void setLmID(const QString& lmID);

	const QVector<std::shared_ptr<Comparator>>& comparators() const;

private:

	QString m_lmID;
	QVector<std::shared_ptr<Comparator>> m_comparatorList;
};

// ------------------------------------------------------------------------------------------------
//
//	ComparatorSet class declaration
//
// ------------------------------------------------------------------------------------------------

class ComparatorSet
{
public:
	ComparatorSet();
	ComparatorSet(const ComparatorSet& src);
	ComparatorSet(ComparatorSet&& src);
	virtual ~ComparatorSet();

	ComparatorSet& operator= (const ComparatorSet& src);
	ComparatorSet& operator= (ComparatorSet&& src);

public:
	void dump() const;

	void clear();
	void insert(const QString& lmID, std::shared_ptr<Comparator> comparator);					// insert comparator

	// get comparators of signal
	//
	QStringList inputSignalIDs() const;															// return list all AppSignalID of signals that contains compartors
	QVector<std::shared_ptr<Comparator>> getByInputSignalID(const QString& appSignalID) const;	// return vector all comparators by AppSignalID of signal

	// get comparators of LM
	//
	QStringList lmIDs() const;																	// return list all EquipmentID of LMs that contains compartors
	QVector<std::shared_ptr<Comparator>> getByLmID(const QString& equipmentID) const;			// return vector all comparators by EquipmentID of LM

	// serialize
	//
	void serializeTo(Proto::ComparatorSet* set) const;
	bool serializeFrom(const Proto::ComparatorSet& set);
	bool serializeFrom(const QByteArray& fileData);

private:
	mutable QMutex m_mutex;

	QHash<QString, QVector<std::shared_ptr<Comparator>>> m_bySignal;
	QHash<QString, std::shared_ptr<LmComparatorSet>> m_byLm;
};
