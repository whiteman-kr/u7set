#include "ComparatorSet.h"

#include "../lib/ProtoSerialization.h"

namespace Builder
{
	// ------------------------------------------------------------------------------------------------
	//
	//	Comparator class implementation
	//
	// ------------------------------------------------------------------------------------------------

	Comparator::Comparator()
	{
	}

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

	void Comparator::setHysteresisConstValue(double hysteresisConstValue)
	{
		assert(m_hysteresisIsConst == true);

		m_hysteresisConstValue = hysteresisConstValue;
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

	QString	Comparator::lmID() const
	{
		return m_lmID;
	}

	void Comparator::setLmID(const QString& lmEquipmentID)
	{
		m_lmID = lmEquipmentID;
	}

	QUuid Comparator::uuid() const
	{
		return m_uuid;
	}

	void Comparator::setUuid(QUuid uuid)
	{
		m_uuid = uuid;
	}


	void Comparator::serializeTo(Proto::Comparator* c) const
	{
		if (c == nullptr)
		{
			assert(false);
			return;
		}

		c->set_insignalid(m_inSignalID.toStdString());

		c->set_cmptype(TO_INT(m_cmpType));
		c->set_analogsignalformat(TO_INT(m_analogSignalFormat));

		c->set_cmpvalueisconst(m_cmpValueIsConst);
		c->set_cmpconstvalue(m_cmpConstValue);
		c->set_cmpsignalid(m_cmpSignalID.toStdString());

		c->set_hysteresisisconst(m_hysteresisIsConst);
		c->set_hysteresisconstvalue(m_hysteresisConstValue);
		c->set_hysteresissignalid(m_hysteresisSignalID.toStdString());

		c->set_outsignalid(m_outSignalID.toStdString());

		c->set_schemaid(m_schemaID.toStdString());
		c->set_lmid(m_lmID.toStdString());
		Proto::Write(c->mutable_uuid(), m_uuid);
	}

	void Comparator::serializeFrom(const Proto::Comparator& c)
	{
		m_inSignalID = QString::fromStdString(c.insignalid());

		m_cmpType = static_cast<Comparator::CmpType>(c.cmptype());
		m_analogSignalFormat = static_cast<E::AnalogAppSignalFormat>(c.analogsignalformat());

		m_cmpValueIsConst = c.cmpvalueisconst();
		m_cmpConstValue = c.cmpconstvalue();
		m_cmpSignalID = QString::fromStdString(c.cmpsignalid());

		m_hysteresisIsConst = c.hysteresisisconst();
		m_hysteresisConstValue = c.hysteresisconstvalue();
		m_hysteresisSignalID = QString::fromStdString(c.hysteresissignalid());

		m_outSignalID = QString::fromStdString(c.outsignalid());

		m_schemaID = QString::fromStdString(c.schemaid());
		m_lmID = QString::fromStdString(c.lmid());
		m_uuid = Proto::Read(c.uuid());
	}

	// ------------------------------------------------------------------------------------------------
	//
	//	LmComparatorSet class implementation
	//
	// ------------------------------------------------------------------------------------------------

	LmComparatorSet::LmComparatorSet()
	{
	}

	LmComparatorSet::~LmComparatorSet()
	{
		clear();
	}

	void LmComparatorSet::clear()
	{
		QMutexLocker l(&m_mutex);

		m_lmID.clear();
		m_comparatorList.clear();
	}

	QString	LmComparatorSet::lmID() const
	{
		QMutexLocker l(&m_mutex);
		return m_lmID;
	}

	void LmComparatorSet::setLmID(const QString& lmEquipmentID)
	{
		QMutexLocker l(&m_mutex);

		m_lmID = lmEquipmentID;
	}

	int	LmComparatorSet::comparatorCount() const
	{
		QMutexLocker l(&m_mutex);

		return m_comparatorList.count();
	}

	std::shared_ptr<Comparator> LmComparatorSet::comparator(int index) const
	{
		QMutexLocker l(&m_mutex);

		if (index < 0 || index >= m_comparatorList.count())
		{
			return nullptr;
		}

		return m_comparatorList[index];
	}

	void LmComparatorSet::insert(std::shared_ptr<Comparator> comparator)
	{
		QMutexLocker l(&m_mutex);

		if (comparator == nullptr)
		{
			assert(false);
			return;
		}

		 m_comparatorList.append(comparator);
	}

	void LmComparatorSet::serializeTo(Proto::LmComparatorSet* set)
	{
		if (set == nullptr)
		{
			assert(false);
			return;
		}

		if (m_lmID.isEmpty() == true)
		{
			assert(false);
			return;
		}

		// set equipmentID of LM in proto message
		//
		set->set_lmequipmentid(m_lmID.toStdString());

		// get all comparator of LM
		//
		int count = comparatorCount();
		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<Comparator> pComparator = comparator(i);
			if (pComparator == nullptr)
			{
				continue;
			}

			// set every comporator in proto message
			//
			Proto::Comparator* protoComparator = set->add_comparator();
			pComparator->serializeTo(protoComparator);
		}
	}

	void LmComparatorSet::serializeFrom(const Proto::LmComparatorSet& set)
	{
		// get equipmentID of LM
		//
		m_lmID = QString::fromStdString(set.lmequipmentid());
		if (m_lmID.isEmpty() == true)
		{
			return;
		}

		// get all comparator of LM
		//
		int count = set.comparator_size();
		for(int i = 0; i < count; i++)
		{
			const Proto::Comparator& protoComparator = set.comparator(i);

			std::shared_ptr<Comparator> pComparator = std::make_shared<Comparator>();
			if (pComparator == nullptr)
			{
				continue;
			}

			pComparator->serializeFrom(protoComparator);

			// insert comparator
			//
			insert(pComparator);
		}
	}

	// ------------------------------------------------------------------------------------------------
	//
	//	ComparatorSet class implementation
	//
	// ------------------------------------------------------------------------------------------------

	ComparatorSet::ComparatorSet()
	{
	}

	ComparatorSet::~ComparatorSet()
	{
		clear();
	}

	void ComparatorSet::clear()
	{
		QMutexLocker l(&m_mutex);

		m_setMap.clear();
		m_lmList.clear();
	}

	int ComparatorSet::lmCount() const
	{
		QMutexLocker l(&m_mutex);

		return m_lmList.count();
	}

	std::shared_ptr<LmComparatorSet> ComparatorSet::lmComparatorSet(int index) const
	{
		QMutexLocker l(&m_mutex);

		if (index < 0 || index >= m_lmList.count())
		{
			return nullptr;
		}

		return m_lmList[index];
	}

	std::shared_ptr<LmComparatorSet> ComparatorSet::lmComparatorSet(const QString& lmID) const
	{
		QMutexLocker l(&m_mutex);

		if (lmID.isEmpty() == true)
		{
			return nullptr;
		}

		return m_setMap[lmID];
	}

	void ComparatorSet::insert(const QString& lmID, std::shared_ptr<Comparator> comparator)
	{
		QMutexLocker l(&m_mutex);

		if (lmID.isEmpty() == true)
		{
			return;
		}

		if (comparator == nullptr)
		{
			return;
		}

		if (comparator->inSignalID().isEmpty() == true || comparator->outSignalID().isEmpty() == true)
		{
			return;
		}

		std::shared_ptr<LmComparatorSet> lmComparatorSet = nullptr;

		if (m_setMap.contains(lmID) == false)
		{
			lmComparatorSet = std::make_shared<LmComparatorSet>();

			lmComparatorSet->setLmID(lmID);
			lmComparatorSet->insert(comparator);

			m_setMap.insert(lmID, lmComparatorSet);
			m_lmList.append(lmComparatorSet);

			return;
		}

		lmComparatorSet = m_setMap[lmID];
		if (lmComparatorSet == nullptr)
		{
			return;
		}

		lmComparatorSet->insert(comparator);
	}

	void ComparatorSet::insert(std::shared_ptr<LmComparatorSet> lmComparatorSet)
	{
		QMutexLocker l(&m_mutex);

		if (lmComparatorSet == nullptr)
		{
			return;
		}

		if (m_setMap.contains(lmComparatorSet->lmID()) == true)
		{
			return;
		}

		m_setMap.insert(lmComparatorSet->lmID(), lmComparatorSet);
		m_lmList.append(lmComparatorSet);
	}

	void ComparatorSet::serializeTo(Proto::ComparatorSet* set)
	{
		if (set == nullptr)
		{
			assert(false);
			return;
		}

		int count = lmCount();
		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<LmComparatorSet> pLmComparatorSet = lmComparatorSet(i);
			if (pLmComparatorSet == nullptr)
			{
				continue;
			}

			::Proto::LmComparatorSet* protoLmComparatorSet = set->add_lmcomparatorset();
			pLmComparatorSet->serializeTo(protoLmComparatorSet);
		}
	}

	void ComparatorSet::serializeFrom(const Proto::ComparatorSet& set)
	{
		int count = set.lmcomparatorset_size();
		for(int i = 0; i < count; i++)
		{
			const Proto::LmComparatorSet& protoLmComparatorSet = set.lmcomparatorset(i);

			std::shared_ptr<LmComparatorSet> pLmComparatorSet = std::make_shared<LmComparatorSet>();
			if (pLmComparatorSet == nullptr)
			{
				continue;
			}

			pLmComparatorSet->serializeFrom(protoLmComparatorSet);

			insert(pLmComparatorSet);
		}
	}

}














