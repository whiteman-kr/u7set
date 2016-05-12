#include "TuningService.h"


TuningServiceWorker::TuningServiceWorker(const QString& serviceStrID,
										 const QString& cfgServiceIP1,
										 const QString& cfgServiceIP2,
										 const QString& cfgFileName) :
	ServiceWorker(ServiceType::TuningService, serviceStrID, cfgServiceIP1, cfgServiceIP2),
	m_cfgFileName(cfgFileName)
{
}


TuningServiceWorker::~TuningServiceWorker()
{
	clear();
}


void TuningServiceWorker::clear()
{
}


TuningServiceWorker* TuningServiceWorker::createInstance()
{
	return new TuningServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2(), m_cfgFileName);
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


void TuningServiceWorker::getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info)
{
	m_dataSources.getTuningDataSourcesInfo(info);
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

		m_dataSources.insert(ds->lmEquipmentID(), ds);
	}

	return result;
}


void TuningServiceWorker::initialize()
{
	loadConfigurationFromFile(m_cfgFileName);
	allocateSignalsAndStates();
}

void TuningServiceWorker::shutdown()
{
}


void TuningServiceWorker::setSignalValue(QString appSignalID, double value)
{

}


void TuningServiceWorker::allocateSignalsAndStates()
{
	QVector<TuningDataSourceInfo> info;

	getTuningDataSourcesInfo(info);

	// allocate Signals
	//
	m_appSignals.clear();

	for(const TuningDataSourceInfo& source : info)
	{
		for(const Signal& signal : source.tuningSignals)
		{
			Signal* appSignal = new Signal(false);

			*appSignal = signal;

			m_appSignals.insert(appSignal->appSignalID(), appSignal);
		}
	}

	int signalCount = m_appSignals.count();

	// allocate Signal states
	//
	m_appSignalStates.clear();

	m_appSignalStates.setSize(signalCount);

	for(int i = 0; i < signalCount; i++)
	{
		Signal* appSignal = m_appSignals[i];
		AppSignalState* appSignalState = m_appSignalStates[i];

		appSignalState->setSignalParams(i, appSignal);
	}
}
