#pragma once

#include <QHash>
#include <QMutex>
#include <QUuid>


namespace Proto
{
	class ComparatorSignal;
	class Comparator;
	class ComparatorSet;
} // namespace Proto

/// <summary>
/// Class representing a signal used in a comparator. It can be either a constant value or an application signal.
/// </summary>
class ComparatorSignal
{
	Q_GADGET

	/// \brief isConst, true - constant signal, false - application signal
	Q_PROPERTY(bool isConst READ isConst)

	/// \brief Value of constant signal if isConst == true
	Q_PROPERTY(double constValue READ constValue)

	/// \brief AppSignalID, ID of application signal if isConst == false
	Q_PROPERTY(QString appSignalId READ appSignalID)

	/// \brief isAcquired, true - signal is acquired, false - signal is not acquired
	Q_PROPERTY(bool isAcquired READ isAcquired)

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
	double m_constValue = 0; // if m_isConst == true
	QString m_appSignalID;   // if m_isConst == false
	bool m_isAcquired = true;
};

/// <summary>
/// Class representing a comparator with input, comparison, hysteresis, and output signals.
/// </summary>
class Comparator
{
	Q_GADGET

	/// \brief cmpType, type of comparison
	Q_PROPERTY(E::CmpType cmpType READ cmpType)

	/// \brief inAnalogSignalFormat, format of input analog signal
	Q_PROPERTY(E::AnalogAppSignalFormat inAnalogSignalFormat READ inAnalogSignalFormat)

	/// \brief input, input signal
	Q_PROPERTY(ComparatorSignal input READ input)

	/// \brief compare, compare signal
	Q_PROPERTY(ComparatorSignal compare READ compare)

	/// \brief hysteresis, hysteresis signal
	Q_PROPERTY(ComparatorSignal hysteresis READ hysteresis)

	/// \brief output, output signal
	Q_PROPERTY(ComparatorSignal output READ output)

	/// \brief label, comparator label
	Q_PROPERTY(QString label READ label)

	/// \brief precision, number of decimal places for constant values
	Q_PROPERTY(int precision READ precision)

	/// \brief schemaId, comparator schemaID
	Q_PROPERTY(QString schemaId READ schemaID)

	/// \brief schemaItemUuid, comparator schema item UUID
	Q_PROPERTY(QUuid schemaItemUuid READ schemaItemUuid)

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
	LmComparatorSet() = default;
	LmComparatorSet(const QString& lmID, std::shared_ptr<Comparator> omparator);

	void clear();
	void append(std::shared_ptr<Comparator> comparator);

	QString lmID() const;
	void setLmID(const QString& lmID);

	const std::vector<std::shared_ptr<Comparator>>& comparators() const;

private:
	QString m_lmID;
	std::vector<std::shared_ptr<Comparator>> m_comparatorList;
};

// ------------------------------------------------------------------------------------------------
//
//	ComparatorSet class declaration
//
// ------------------------------------------------------------------------------------------------

class ComparatorSet
{
public:
	ComparatorSet() = default;
	ComparatorSet(const ComparatorSet& src);
	ComparatorSet(ComparatorSet&& src) noexcept;

	ComparatorSet& operator=(const ComparatorSet& src);
	ComparatorSet& operator=(ComparatorSet&& src) noexcept;

public:
	void dump() const;

	void clear();
	void insert(const QString& lmID, std::shared_ptr<Comparator> comparator); // insert comparator

	// get comparators of signal
	//
	QStringList inputSignalIDs() const;    // return list all AppSignalID of signals that contains comparators
	std::vector<std::shared_ptr<Comparator>> getByInputSignalID(
		const QString& appSignalID) const; // return vector all comparators by AppSignalID of signal

	std::shared_ptr<Comparator> getByOutputSignalID(
		const QString& appSignalID) const; // return vector all comparators by AppSignalID of signal

	// get comparators of LM
	//
	QStringList lmIDs() const;             // return list all EquipmentID of LMs that contains comparators
	std::vector<std::shared_ptr<Comparator>> getByLmID(
		const QString& equipmentID) const; // return vector all comparators by EquipmentID of LM

	// serialize
	//
	void serializeTo(Proto::ComparatorSet* set) const;
	bool serializeFrom(const Proto::ComparatorSet& set);
	bool serializeFrom(const QByteArray& fileData);

private:
	mutable QMutex m_mutex;

	QHash<QString, std::vector<std::shared_ptr<Comparator>>> m_byInputSignal;
	QHash<QString, std::shared_ptr<Comparator>> m_byOutputSignal;
	QHash<QString, std::shared_ptr<LmComparatorSet>> m_byLm;
};
