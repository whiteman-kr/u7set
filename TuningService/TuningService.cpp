#include "TuningService.h"


TuningServiceWorker::TuningServiceWorker(const QString& serviceStrID,
										 const QString& cfgServiceIP1,
										 const QString& cfgServiceIP2) :
	ServiceWorker(ServiceType::TuningService, serviceStrID, cfgServiceIP1, cfgServiceIP2)
{
}


TuningServiceWorker* TuningServiceWorker::createInstance()
{
	return new TuningServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2());
}


bool TuningServiceWorker::loadConfigurationFromFile(const QString& fileName)
{
	QByteArray cfgXmlData;

	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	cfgXmlData = file.readAll();

	XmlReadHelper xml(cfgXmlData);

	bool result = true;

	result &= m_tuningSettings.readFromXml(xml);
	result &= readTuningDataSources(xml);

	return result;
}


bool TuningServiceWorker::readTuningDataSources(XmlReadHelper& xml)
{
	bool result = true;

	m_dataSources.clear();

	result = xml.findElement("TuningLMs");

	if (result == false)
	{
		return false;
	}

	int sourceCount = 0;

	result &= xml.readIntAttribute("Count", &sourceCount);

	for(int i = 0; i < sourceCount; i++)
	{
		result = xml.findElement(DataSource::ELEMENT_DATA_SOURCE);

		if (result == false)
		{
			return false;
		}

		TuningDataSource* ds = new TuningDataSource();

		result &= ds->readFromXml(xml);

		if (result == false)
		{
			delete ds;
			break;
		}

		m_dataSources.insert(ds->lmAddress32(), ds);
	}

	return result;
}


void TuningServiceWorker::initialize()
{
	int a = 0;
}

void TuningServiceWorker::shutdown()
{
}

