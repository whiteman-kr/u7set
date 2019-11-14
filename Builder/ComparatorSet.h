#pragma once

#include "../lib/Types.h"
#include "../Proto/serialization.pb.h"

#include <QUuid>
#include <QMutex>

namespace Builder
{
	// ------------------------------------------------------------------------------------------------
	//
	//	Comparator class declaration
	//
	// ------------------------------------------------------------------------------------------------

	class ComparatorSignal
	{
	public:

		ComparatorSignal();

		bool isConst() const;
		void setIsConst(bool isConst);

		double constValue() const;
		void setConstValue(double constValue);

		QString appSignalID() const;
		void setAppSignalID(const QString& appSignalID);

		bool isAcquired() const;
		void setIsAcquired(bool isAcquired);

		void serializeTo(Proto::ComparatorSignal* s) const;
		void serializeFrom(const Proto::ComparatorSignal& s);

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
		Comparator();
		virtual ~Comparator();

		E::CmpType cmpType() const;
		void setCmpType(E::CmpType cmpType);

		E::AnalogAppSignalFormat intAnalogSignalFormat() const;
		void setInAnalogSignalFormat(E::AnalogAppSignalFormat intAnalogSignalFormat);

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
		void serializeFrom(const Proto::Comparator& c);

	private:
		E::CmpType m_cmpType = E::CmpType::Equ;
		E::AnalogAppSignalFormat m_inAnalogSignalFormat = E::AnalogAppSignalFormat::SignedInt32;

		ComparatorSignal m_inputSignal;
		ComparatorSignal m_compareSignal;
		ComparatorSignal m_hysteresisSignal;
		ComparatorSignal m_outputSignal;

		int m_precision = 2;
		QString m_label;
		QString m_schemaID;
		QUuid m_schemaItemUuid;
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
		virtual ~LmComparatorSet();

		void clear();

		QString	lmID() const;
		void setLmID(const QString& lmID);

		int comparatorCount() const;
		std::shared_ptr<Comparator> comparator(int index) const;

		void insert(std::shared_ptr<Comparator> comparator);

	private:
		mutable QMutex m_mutex;

		QString m_lmID;
		QList<std::shared_ptr<Comparator>> m_comparatorList;
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
		virtual ~ComparatorSet();

		void clear();

		int lmCount() const;
		std::shared_ptr<LmComparatorSet> lmComparatorSet(int index) const;
		std::shared_ptr<LmComparatorSet> lmComparatorSet(const QString& lmID) const;

		int comparatorCount() const;
		QList<std::shared_ptr<Comparator>> getByInputSignalID(const QString& appSignalID) const;

		void insert(const QString& lmID, std::shared_ptr<Comparator> comparator);	// insert Comparator

		void serializeTo(Proto::ComparatorSet* set);
		void serializeFrom(const Proto::ComparatorSet& set);

	private:
		mutable QMutex m_mutex;

		QMap<QString, std::shared_ptr<LmComparatorSet>> m_setMap;
		QList<std::shared_ptr<LmComparatorSet>> m_lmList;

		QHash<QString, std::shared_ptr<Comparator>> m_bySignal;
	};
}
