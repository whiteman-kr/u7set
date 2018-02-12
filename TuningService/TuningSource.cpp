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
		m_lmDataType = DataSource::DataType::Tuning;
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

	void TuningSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml)
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
		for(TuningSource* ds : *this)
		{
			delete ds;
		}

		QHash<QString, TuningSource*>::clear();
		m_ip2Source.clear();
	}

	void TuningSources::buildIP2DataSourceMap()
	{
		for(TuningSource* source : *this)
		{
			m_ip2Source.insert(source->lmAddress32(), source);
		}
	}

	const TuningSource* TuningSources::getSourceByIP(quint32 ip) const
	{
		return m_ip2Source.value(ip, nullptr);
	}

	const TuningSource* TuningSources::getSourceByID(const QString& sourceID) const
	{
		return value(sourceID, nullptr);
	}
}
