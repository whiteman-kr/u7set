#include "TuningSource.h"


namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TuningSource class implementation
	//
	// -------------------------------------------------------------------------------

	TuningSource::TuningSource()
	{
	}

	TuningSource::~TuningSource()
	{
	}

	void TuningSource::setTuningData(TuningDataShared tuningData)
	{
		if (tuningData == nullptr)
		{
			assert(false);
			return;
		}

		m_tuningData = tuningData;

		QVector<AppSignal*> tunableSignals;

		tuningData->getSignals(&tunableSignals);

		for(const AppSignal* s : tunableSignals)
		{
			TEST_PTR_CONTINUE(s);
			appendAssociatedSignal(E::LanControllerType::Tuning, s->appSignalID());
		}
	}

	TuningDataSharedConst TuningSource::tuningData() const
	{
		return m_tuningData;
	}

	void TuningSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml) const
	{
		if (m_tuningData == nullptr)
		{
			TuningData td(moduleEquipmentID());
			td.writeToXml(xml);
			return;
		}

		m_tuningData->writeToXml(xml);
	}

	bool TuningSource::readAdditionalSectionsFromXml(XmlReadHelper& xml)
	{
		assert(m_tuningData == nullptr);

		m_tuningData = std::make_shared<TuningData>();

		bool result = m_tuningData->readFromXml(xml);

		return result;
	}

	bool TuningSource::hasTuningSignals() const
	{
		TEST_PTR_RETURN_FALSE(m_tuningData);

		return m_tuningData->getSignalsCount() != 0;
	}

	const QStringList& TuningSource::getEnabledLansProvidedTuning() const
	{
		if (m_enabledLansProvidedTuning.has_value() == false)
		{
			QStringList lans;

			for(const LanControllerInfo& lci : lanControllersInfo()())
			{
				if (lci.isTuningEnabled() == true)
				{
					lans.append(lci.equipmentID);
				}
			}

			m_enabledLansProvidedTuning = lans;
		}

		return m_enabledLansProvidedTuning.value();
	}

	int TuningSource::getSignalsCount() const
	{
		if (m_tuningData != nullptr)
		{
			return m_tuningData->getSignalsCount();
		}

		Q_ASSERT(false);
		return 0;
	}

	// -------------------------------------------------------------------------------
	//
	// TuningSources class implementation
	//
	// -------------------------------------------------------------------------------

	TuningSources::~TuningSources()
	{
		clear();
	}

	void TuningSources::clear()
	{
		m_id2Source.clear();

		QVector<TuningSource>::clear();
	}

	void TuningSources::buildMaps()
	{
		int index = 0;

		for(const TuningSource& source : *this)
		{
			m_id2Source.insert(source.moduleEquipmentID(), index);
			index++;
		}
	}

	const TuningSource* TuningSources::getSourceByID(const QString& sourceID) const
	{
		int index = m_id2Source.value(sourceID, -1);

		if (index >= 0)
		{
			return &(*this)[index];
		}

		return nullptr;
	}

	QStringList TuningSources::getAllSourcesIDs() const
	{
		QStringList ids;

		for(const TuningSource&  src : *this)
		{
			ids.append(src.moduleEquipmentID());
		}

		return ids;
	}

	int TuningSources::getSignalsCount() const
	{
		int count = 0;

		for(const TuningSource& ts : *this)
		{
			count += ts.getSignalsCount();
		}

		return count;
	}
}
