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
		setLmDataType(DataSource::DataType::Tuning);
	}

	TuningSource::~TuningSource()
	{
		if (m_deleteTuningData == true)
		{
			delete m_tuningData;
		}
	}

	void TuningSource::setTuningData(TuningData* tuningData)
	{
		if (tuningData == nullptr)
		{
			assert(false);
			return;
		}

		m_tuningData = tuningData;
	}

	const TuningData* TuningSource::tuningData() const
	{
		return m_tuningData;
	}

	void TuningSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml) const
	{
		if (m_tuningData == nullptr)
		{
			TuningData td(lmEquipmentID());
			td.writeToXml(xml);
			return;
		}

		m_tuningData->writeToXml(xml);
	}

	bool TuningSource::readAdditionalSectionsFromXml(XmlReadHelper& xml)
	{
		assert(m_tuningData == nullptr);

		m_tuningData = new TuningData();

		m_deleteTuningData = true;

		bool result = m_tuningData->readFromXml(xml);

		return result;
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
		m_ip2Source.clear();
		m_id2Source.clear();

		QVector<TuningSource>::clear();
	}

	void TuningSources::buildMaps()
	{
		int index = 0;

		for(const TuningSource& source : *this)
		{
			m_ip2Source.insert(source.lmAddress32(), index);
			m_id2Source.insert(source.lmEquipmentID(), index);

			index++;
		}
	}

	const TuningSource* TuningSources::getSourceByIP(quint32 ip) const
	{
		int index = m_ip2Source.value(ip, -1);

		if (index >= 0)
		{
			return &(*this)[index];
		}

		return nullptr;
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
}
