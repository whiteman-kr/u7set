#include "ComparatorSet.h"

#include "../lib/ProtoSerialization.h"

// ------------------------------------------------------------------------------------------------
//
//	ComparatorSignal class implementation
//
// ------------------------------------------------------------------------------------------------

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

bool ComparatorSignal::serializeFrom(const Proto::ComparatorSignal& s)
{
	m_isConst = s.isconst();
	m_constValue = s.constvalue();
	m_appSignalID = QString::fromStdString(s.appsignalid());
	m_isAcquired = s.isacquired();

	return true;
}

// ------------------------------------------------------------------------------------------------
//
//	Comparator class implementation
//
// ------------------------------------------------------------------------------------------------
E::CmpType Comparator::cmpType() const
{
	return m_cmpType;
}

void Comparator::setCmpType(E::CmpType cmpType)
{
	m_cmpType = cmpType;
}

E::AnalogAppSignalFormat Comparator::inAnalogSignalFormat() const
{
	return m_inAnalogSignalFormat;
}

void Comparator::setInAnalogSignalFormat(E::AnalogAppSignalFormat inAnalogSignalFormat)
{
	m_inAnalogSignalFormat = inAnalogSignalFormat;
}

ComparatorSignal& Comparator::input()
{
	return m_inputSignal;
}

ComparatorSignal& Comparator::compare()
{
	return m_compareSignal;
}

ComparatorSignal& Comparator::hysteresis()
{
	return m_hysteresisSignal;
}

ComparatorSignal& Comparator::output()
{
	return m_outputSignal;
}

const ComparatorSignal& Comparator::input() const
{
	return m_inputSignal;
}

const ComparatorSignal& Comparator::compare() const
{
	return m_compareSignal;
}

const ComparatorSignal& Comparator::hysteresis() const
{
	return m_hysteresisSignal;
}

const ComparatorSignal& Comparator::output() const
{
	return m_outputSignal;
}

QString Comparator::label() const
{
	return m_label;
}

void Comparator::setLabel(const QString& label)
{
	m_label = label;
}

int Comparator::precision() const
{
	return m_precision;
}

void Comparator::setPrecision(int precision)
{
	m_precision = precision;
}

QString Comparator::schemaID() const
{
	return m_schemaID;
}

void Comparator::setSchemaID(const QString& schemaID)
{
	m_schemaID = schemaID;
}

QUuid Comparator::schemaItemUuid() const
{
	return m_schemaItemUuid;
}

void Comparator::setSchemaItemUuid(QUuid schemaItemUuid)
{
	m_schemaItemUuid = schemaItemUuid;
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

	m_inputSignal.serializeTo(c->mutable_input());
	m_compareSignal.serializeTo(c->mutable_compare());
	m_hysteresisSignal.serializeTo(c->mutable_hysteresis());
	m_outputSignal.serializeTo(c->mutable_output());

	c->set_label(m_label.toStdString());
	c->set_precision(m_precision);
	c->set_schemaid(m_schemaID.toStdString());
	Proto::Write(c->mutable_schemaitemuuid(), m_schemaItemUuid);
}

bool Comparator::serializeFrom(const Proto::Comparator& c)
{
	m_cmpType = static_cast<E::CmpType>(c.cmptype());
	m_inAnalogSignalFormat = static_cast<E::AnalogAppSignalFormat>(c.inanalogsignalformat());

	m_inputSignal.serializeFrom(c.input());
	m_compareSignal.serializeFrom(c.compare());
	m_hysteresisSignal.serializeFrom(c.hysteresis());
	m_outputSignal.serializeFrom(c.output());

	m_label = QString::fromStdString(c.label());
	m_precision = c.precision();
	m_schemaID = QString::fromStdString(c.schemaid());
	m_schemaItemUuid = Proto::Read(c.schemaitemuuid());

	return true;
}

// ------------------------------------------------------------------------------------------------
//
//	LmComparatorSet class implementation
//
// ------------------------------------------------------------------------------------------------

LmComparatorSet::LmComparatorSet()
{
}

LmComparatorSet::LmComparatorSet(const QString& lmID, std::shared_ptr<Comparator> comparator)
{
	if (lmID.isEmpty() == true || comparator == nullptr)
	{
		return;
	}

	m_lmID = lmID;
	m_comparatorList.append(comparator);
}

LmComparatorSet::~LmComparatorSet()
{
}

void LmComparatorSet::clear()
{
	m_lmID.clear();
	m_comparatorList.clear();
}

void LmComparatorSet::append(std::shared_ptr<Comparator> comparator)
{
	if (comparator == nullptr)
	{
		assert(false);
		return;
	}

	m_comparatorList.append(comparator);
}

QString	LmComparatorSet::lmID() const
{
	return m_lmID;
}

void LmComparatorSet::setLmID(const QString& lmEquipmentID)
{
	m_lmID = lmEquipmentID;
}

const QVector<std::shared_ptr<Comparator>>& LmComparatorSet::comparators() const
{
	return m_comparatorList;
}

// ------------------------------------------------------------------------------------------------
//
//	ComparatorSet class implementation
//
// ------------------------------------------------------------------------------------------------

ComparatorSet::ComparatorSet()
{
}

ComparatorSet::ComparatorSet(const ComparatorSet& src)
{
	*this = src;
	return;
}

ComparatorSet::ComparatorSet(ComparatorSet&& src)
{
	*this = std::move(src);
	return;
}

ComparatorSet::~ComparatorSet()
{
}

ComparatorSet& ComparatorSet::operator= (const ComparatorSet& src)
{
	decltype(m_bySignal) bySignal;
	decltype(m_byLm) byLm;

	{
		QMutexLocker l(&src.m_mutex);
		bySignal = src.m_bySignal;
		byLm = src.m_byLm;
	}

	{
		QMutexLocker l(&m_mutex);
		m_bySignal = std::move(bySignal);
		m_byLm = std::move(byLm);
	}

	return *this;
}

ComparatorSet& ComparatorSet::operator= (ComparatorSet&& src)
{
	decltype(m_bySignal) bySignal;
	decltype(m_byLm) byLm;

	{
		QMutexLocker l(&src.m_mutex);
		bySignal = std::move(src.m_bySignal);
		byLm = std::move(src.m_byLm);
	}

	{
		QMutexLocker l(&m_mutex);
		m_bySignal = std::move(bySignal);
		m_byLm = std::move(byLm);
	}

	return *this;
}

void ComparatorSet::clear()
{
	QMutexLocker l(&m_mutex);

	m_byLm.clear();
	m_bySignal.clear();
}

void ComparatorSet::insert(const QString& lmID, std::shared_ptr<Comparator> comparator)
{
	QMutexLocker l(&m_mutex);

	if (lmID.isEmpty() == true || comparator == nullptr)
	{
		return;
	}

	if (comparator->input().appSignalID().isEmpty() == true || comparator->input().isAcquired() == false)
	{
		return;
	}

	if (comparator->output().appSignalID().isEmpty() == true || comparator->output().isAcquired() == false)
	{
		return;
	}

	// insert by appSignalID of input signal
	//
	if(m_bySignal.contains(comparator->input().appSignalID()) == false)
	{
		QVector<std::shared_ptr<Comparator>> comparatorVector;
		comparatorVector.reserve(4);
		comparatorVector.push_back(comparator);

		m_bySignal.insert(comparator->input().appSignalID(), comparatorVector);
	}
	else
	{
		m_bySignal[comparator->input().appSignalID()].append(comparator);
	}

	// insert by EquipmentID of LM
	//
	if(m_byLm.contains(lmID) == false)
	{
		std::shared_ptr<LmComparatorSet> lmComparatorSet = std::make_shared<LmComparatorSet>(lmID, comparator);

		m_byLm.insert(lmID, lmComparatorSet);
	}
	else
	{
		m_byLm[lmID]->append(comparator);
	}
}

QStringList ComparatorSet::inputSignalIDs() const
{
	QMutexLocker l(&m_mutex);

	return static_cast<QStringList>(m_bySignal.keys());
}

QVector<std::shared_ptr<Comparator>> ComparatorSet::getByInputSignalID(const QString& appSignalID) const
{
	QMutexLocker l(&m_mutex);

	return m_bySignal.value(appSignalID);
}

QStringList ComparatorSet::lmIDs() const
{
	QMutexLocker l(&m_mutex);

	return static_cast<QStringList>(m_byLm.keys());
}

QVector<std::shared_ptr<Comparator>> ComparatorSet::getByLmID(const QString& equipmentID) const
{
	QMutexLocker l(&m_mutex);

	std::shared_ptr<LmComparatorSet> set = m_byLm[equipmentID];
	if (set == nullptr)
	{
		return QVector<std::shared_ptr<Comparator>>();
	}

	return set->comparators();
}

void ComparatorSet::serializeTo(Proto::ComparatorSet* set) const
{
	if (set == nullptr)
	{
		assert(false);
		return;
	}

	QMutexLocker l(&m_mutex);

	foreach (std::shared_ptr<LmComparatorSet> lmComparatorSet, m_byLm.values())
	{
		if (lmComparatorSet == nullptr || lmComparatorSet->lmID().isEmpty() == true)
		{
			continue;
		}

		::Proto::LmComparatorSet* protoLmComparatorSet = set->add_lmcomparatorset();

		protoLmComparatorSet->set_lmequipmentid(lmComparatorSet->lmID().toStdString());			// set equipmentID of LM in proto message

		for (std::shared_ptr<Comparator> comparator : lmComparatorSet->comparators())			// get all comparator of LM
		{
			if (comparator == nullptr)
			{
				continue;
			}

			// set every comporator to proto message from ComparatorSet
			//
			Proto::Comparator* protoComparator = protoLmComparatorSet->add_comparator();
			comparator->serializeTo(protoComparator);
		}
	}
}

bool ComparatorSet::serializeFrom(const Proto::ComparatorSet& set)
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
			continue;
		}

		// get all comparator of LM
		//
		int cmpcount = protoLmComparatorSet.comparator_size();
		for(int c = 0; c < cmpcount; c++)
		{
			const Proto::Comparator& protoComparator = protoLmComparatorSet.comparator(c);

			std::shared_ptr<Comparator> pComparator = std::make_shared<Comparator>();

			if (bool ok = pComparator->serializeFrom(protoComparator);
				ok == true)
			{
				insert(lmID, pComparator);
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

bool ComparatorSet::serializeFrom(const QByteArray& fileData)
{
	::Proto::ComparatorSet protoComparatorSet;

	bool result = protoComparatorSet.ParseFromArray(fileData.constData(), fileData.size());
	if (result == false)
	{
		return false;
	}

	bool ok = serializeFrom(protoComparatorSet);
	return ok;
}

