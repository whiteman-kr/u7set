#include "TuningDataSource.h"


// -------------------------------------------------------------------------------
//
// TuningDataSource class implementation
//
// -------------------------------------------------------------------------------

TuningDataSource::TuningDataSource()
{
	m_dataType = DataSource::DataType::Tuning;
}


TuningDataSource::~TuningDataSource()
{
	if (m_deleteTuningData == true)
	{
		delete m_tuningData;
	}
}


void TuningDataSource::setTuningData(TuningData* tuningData)
{
	if (tuningData == nullptr)
	{
		assert(false);
		return;
	}

	m_tuningData = tuningData;
}


void TuningDataSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml)
{
	if (m_tuningData == nullptr)
	{
		TuningData td(lmEquipmentID(), 0, 0);
		td.writeToXml(xml);
		return;
	}

	m_tuningData->writeToXml(xml);
}


bool TuningDataSource::readAdditionalSectionsFromXml(XmlReadHelper& xml)
{
	assert(m_tuningData == nullptr);

	m_tuningData = new TuningData();

	m_deleteTuningData = true;

	m_tuningData->readFromXml(xml);

	return true;
}


void TuningDataSource::getTuningDataSourceInfo(TuningDataSourceInfo& info)
{
	info.channel = m_channel;
	info.dataType = m_dataType;
	info.lmEquipmentID = m_lmEquipmentID;
	info.lmCaption = m_lmCaption;
	info.lmAdapterID = m_lmAdapterID;
	info.lmDataEnable = m_lmDataEnable;
	info.lmAddressPort = m_lmAddressPort;
	info.lmDataID = m_lmDataID;

	info.tuningSignals.clear();

	if (m_tuningData == nullptr)
	{
		return;
	}

	QList<Signal*> signalList;

	m_tuningData->getSignals(signalList);

	info.tuningSignals.resize(signalList.count());

	int index = 0;

	for(Signal* signal : signalList)
	{
		if (index >= info.tuningSignals.count())
		{
			assert(false);
			break;
		}

		info.tuningSignals[index] = *signal;

		index++;
	}
}


// -------------------------------------------------------------------------------
//
// TuningDataSources class implementation
//
// -------------------------------------------------------------------------------

TuningDataSources::~TuningDataSources()
{
	clear();
}


void TuningDataSources::clear()
{
	for(TuningDataSource* ds : *this)
	{
		delete ds;
	}

	QHash<QString, TuningDataSource*>::clear();
}


void TuningDataSources::getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info)
{
	info.clear();

	info.resize(count());

	int index = 0;

	for(TuningDataSource* source : (*this))
	{
		if (source == nullptr)
		{
			assert(false);
			continue;
		}

		if (index >= info.count())
		{
			assert(false);
			break;
		}

		source->getTuningDataSourceInfo(info[index]);

		index++;
	}
}
