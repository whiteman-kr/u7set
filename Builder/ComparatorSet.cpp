#include "ComparatorSet.h"

#include "../lib/ProtoSerialization.h"

namespace Builder
{
	// ------------------------------------------------------------------------------------------------
	//
	//	ComparatorSignal class implementation
	//
	// ------------------------------------------------------------------------------------------------

	ComparatorSignal::ComparatorSignal()
	{
	}

	bool ComparatorSignal::isConst() const
	{
		return m_isConst;
	}

	void ComparatorSignal::setIsConst(bool isConst)
	{
		m_isConst = isConst;
	}

	double ComparatorSignal::constValue() const
	{
		assert(m_isConst == true);

		return m_constValue;
	}

	void ComparatorSignal::setConstValue(double constValue)
	{
		assert(m_isConst == true);

		m_constValue = constValue;
	}

	QString ComparatorSignal::appSignalID() const
	{
		return m_appSignalID;
	}

	void ComparatorSignal::setAppSignalID(const QString& appSignalID)
	{
		m_appSignalID = appSignalID;
	}

	bool ComparatorSignal::isAcquired() const
	{
		return m_isAcquired;
	}

	void ComparatorSignal::setIsAcquired(bool isAcquired)
	{
		m_isAcquired = isAcquired;
	}

	void ComparatorSignal::serializeTo(Proto::ComparatorSignal* s) const
	{
		if (s == nullptr)
		{
			assert(false);
			return;
		}

		s->set_isconst(m_isConst);
		s->set_constvalue(m_constValue);
		s->set_appsignalid(m_appSignalID.toStdString());
		s->set_isacquired(m_isAcquired);
	}

	void ComparatorSignal::serializeFrom(const Proto::ComparatorSignal& s)
	{
		m_isConst = s.isconst();
		m_constValue = s.constvalue();
		m_appSignalID = QString::fromStdString(s.appsignalid());
		m_isAcquired = s.isacquired();
	}

	// ------------------------------------------------------------------------------------------------
	//
	//	Comparator class implementation
	//
	// ------------------------------------------------------------------------------------------------

	Comparator::Comparator()
	{
	}

	Comparator::CmpType Comparator::cmpType() const
	{
		return m_cmpType;
	}

	void Comparator::setCmpType(Comparator::CmpType cmpType)
	{
		m_cmpType = cmpType;
	}

	E::AnalogAppSignalFormat Comparator::intAnalogSignalFormat() const
	{
		return m_inAnalogSignalFormat;
	}

	void Comparator::setInAnalogSignalFormat(E::AnalogAppSignalFormat analogSignalFormat)
	{
		m_inAnalogSignalFormat = analogSignalFormat;
	}

	ComparatorSignal& Comparator::input()
	{
		return m_signal[SignalType::Input];
	}

	ComparatorSignal& Comparator::compare()
	{
		return m_signal[SignalType::Compare];
	}

	ComparatorSignal& Comparator::hysteresis()
	{
		return m_signal[SignalType::Hysteresis];
	}

	ComparatorSignal& Comparator::output()
	{
		return m_signal[SignalType::Output];
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

	void Comparator::serializeTo(Proto::Comparator* c) const
	{
		if (c == nullptr)
		{
			assert(false);
			return;
		}

		c->set_cmptype(TO_INT(m_cmpType));
		c->set_inanalogsignalformat(TO_INT(m_inAnalogSignalFormat));

		m_signal[SignalType::Input].serializeTo(c->mutable_input());
		m_signal[SignalType::Compare].serializeTo(c->mutable_compare());
		m_signal[SignalType::Hysteresis].serializeTo(c->mutable_hysteresis());
		m_signal[SignalType::Output].serializeTo(c->mutable_output());

		c->set_schemaid(m_schemaID.toStdString());
		Proto::Write(c->mutable_schemaitemuuid(), m_uuid);
	}

	void Comparator::serializeFrom(const Proto::Comparator& c)
	{
		m_cmpType = static_cast<Comparator::CmpType>(c.cmptype());
		m_inAnalogSignalFormat = static_cast<E::AnalogAppSignalFormat>(c.inanalogsignalformat());

		m_signal[SignalType::Input].serializeFrom(c.input());
		m_signal[SignalType::Compare].serializeFrom(c.compare());
		m_signal[SignalType::Hysteresis].serializeFrom(c.hysteresis());
		m_signal[SignalType::Output].serializeFrom(c.output());

		m_schemaID = QString::fromStdString(c.schemaid());
		m_uuid = Proto::Read(c.schemaitemuuid());
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

	int ComparatorSet::comparatorCount() const
	{
		QMutexLocker l(&m_mutex);

		return m_bySignal.count();
	}

	QList<std::shared_ptr<Comparator>> ComparatorSet::getByInputSignalID(const QString& appSignalID) const
	{
		QMutexLocker l(&m_mutex);

		return m_bySignal.values(appSignalID);
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

		if (comparator->input().appSignalID().isEmpty() == true || comparator->input().isAcquired() == false)
		{
			return;
		}

		if (comparator->output().appSignalID().isEmpty() == true)
		//if (comparator->output().appSignalID().isEmpty() == true || comparator->output().isAcquired() == false)
		{
			return;
		}

		m_bySignal.insertMulti(comparator->input().appSignalID(), comparator);

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

	void ComparatorSet::serializeTo(Proto::ComparatorSet* set)
	{
		if (set == nullptr)
		{
			assert(false);
			return;
		}

		int lmcount = lmCount();
		for(int lm = 0; lm < lmcount; lm++)
		{
			std::shared_ptr<LmComparatorSet> pLmComparatorSet = lmComparatorSet(lm);
			if (pLmComparatorSet == nullptr)
			{
				continue;
			}

			::Proto::LmComparatorSet* protoLmComparatorSet = set->add_lmcomparatorset();

			if (pLmComparatorSet->lmID().isEmpty() == true)
			{
				continue;
			}

			// set equipmentID of LM in proto message
			//
			protoLmComparatorSet->set_lmequipmentid(pLmComparatorSet->lmID().toStdString());

			// get all comparator of LM
			//
			int cmpcount = pLmComparatorSet->comparatorCount();
			for(int c = 0; c < cmpcount; c++)
			{
				std::shared_ptr<Comparator> pComparator = pLmComparatorSet->comparator(c);
				if (pComparator == nullptr)
				{
					continue;
				}

				// set every comporator in proto message
				//
				Proto::Comparator* protoComparator = protoLmComparatorSet->add_comparator();
				pComparator->serializeTo(protoComparator);
			}
		}
	}

	void ComparatorSet::serializeFrom(const Proto::ComparatorSet& set)
	{
		int lmcount = set.lmcomparatorset_size();
		for(int lm = 0; lm < lmcount; lm++)
		{
			const Proto::LmComparatorSet& protoLmComparatorSet = set.lmcomparatorset(lm);

			// get equipmentID of LM
			//
			QString lmID = QString::fromStdString(protoLmComparatorSet.lmequipmentid());
			if (lmID.isEmpty() == true)
			{
				return;
			}

			// get all comparator of LM
			//
			int cmpcount = protoLmComparatorSet.comparator_size();
			for(int c = 0; c < cmpcount; c++)
			{
				const Proto::Comparator& protoComparator = protoLmComparatorSet.comparator(c);

				std::shared_ptr<Comparator> pComparator = std::make_shared<Comparator>();
				if (pComparator == nullptr)
				{
					continue;
				}

				pComparator->serializeFrom(protoComparator);

				// insert comparator
				//
				insert(lmID, pComparator);
			}
		}
	}
}














