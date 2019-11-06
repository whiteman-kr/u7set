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
		void setHysteresisConstValue(double hysteresisConstValue);

		QString hysteresisSignalID() const;
		void setHysteresisSignalID(const QString& hysteresisSignalID);

		QString outSignalID() const;
		void setOutSignalID(const QString& outSignalID);

		QString schemaID() const;
		void setSchemaID(const QString& schemaID);

		QString	lmID() const;
		void setLmID(const QString& lmID);

		QUuid uuid() const;
		void setUuid(QUuid uuid);

		void serializeTo(Proto::Comparator* c) const;
		void serializeFrom(const Proto::Comparator& c);

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
		QString m_lmID;
		QUuid m_uuid;
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

		void serializeTo(Proto::LmComparatorSet* set);
		void serializeFrom(const Proto::LmComparatorSet& set);

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

		void insert(const QString& lmID, std::shared_ptr<Comparator> comparator);	// insert Comparator
		void insert(std::shared_ptr<LmComparatorSet> lmComparatorSet);				// insert LmComparatorSet

		void serializeTo(Proto::ComparatorSet* set);
		void serializeFrom(const Proto::ComparatorSet& set);

	private:
		mutable QMutex m_mutex;

		QMap<QString, std::shared_ptr<LmComparatorSet>> m_setMap;
		QList<std::shared_ptr<LmComparatorSet>> m_lmList;
	};
}
