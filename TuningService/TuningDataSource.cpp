#include "TuningDataSource.h"


// -------------------------------------------------------------------------------
//
// TuningDataSource class implementation
//
// -------------------------------------------------------------------------------

TuningDataSource::TuningDataSource()
{
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
		TuningData td(lmStrID(), 0, 0);
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

	QHash<quint32, TuningDataSource*>::clear();
}
