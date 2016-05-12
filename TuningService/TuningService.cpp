#include "TuningService.h"


// -------------------------------------------------------------------------------------
//
// TuningServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

TuningServiceWorker::TuningServiceWorker(const QString& serviceStrID,
										 const QString& cfgServiceIP1,
										 const QString& cfgServiceIP2,
										 const QString& cfgFileName) :
	ServiceWorker(ServiceType::TuningService, serviceStrID, cfgServiceIP1, cfgServiceIP2),
	m_cfgFileName(cfgFileName),
	m_timer(this)
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
	TuningServiceWorker* worker = new TuningServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2(), m_cfgFileName);

	worker->setTuningService(m_tuningService);

	m_tuningService->setTuningServiceWorker(worker);

	return worker;
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

	runTuningSocket();

	if (m_tuningService != nullptr)
	{
		connect(this, &TuningServiceWorker::tuningServiceReady, m_tuningService, &TuningService::tuningServiceReady);
	}

	connect(&m_timer, &QTimer::timeout, this, &TuningServiceWorker::onTimer);

	m_timer.setInterval(500);
	m_timer.start();

	emit tuningServiceReady();
}


void TuningServiceWorker::shutdown()
{
	m_timer.stop();

	stopTuningSocket();
}


void TuningServiceWorker::runTuningSocket()
{
	m_tuningSocket = new Tuning::TuningSocketWorker(m_tuningSettings.tuningDataIP);

	m_tuningSocketThread = new SimpleThread(m_tuningSocket);
	m_tuningSocketThread->start();
}


void TuningServiceWorker::stopTuningSocket()
{
	m_tuningSocketThread->quitAndWait();		// m_tuningSocket delete inside
	delete m_tuningSocketThread;
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
	m_signal2Source.clear();

	for(const TuningDataSourceInfo& source : info)
	{
		for(const Signal& signal : source.tuningSignals)
		{
			Signal* appSignal = new Signal(false);

			*appSignal = signal;

			m_appSignals.insert(appSignal->appSignalID(), appSignal);

			if (m_signal2Source.contains(appSignal->appSignalID()))
			{
				assert(false);
				qDebug() << "Duplicate AppSignalID" << appSignal->appSignalID();
			}
			else
			{
				m_signal2Source.insert(appSignal->appSignalID(), source.lmEquipmentID);
			}
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


void TuningServiceWorker::onTimer()
{
	sendPeriodicReadRequests();
}


void TuningServiceWorker::sendPeriodicReadRequests()
{
	for(TuningDataSource* source : m_dataSources)
	{
		Tuning::SocketRequest sr;

		sr.lmIP = source->lmAddress32();
		sr.lmPort = source->lmPort();
		sr.lmNumber = source->lmNumber();
		sr.numerator = source->numerator();
		sr.operation = Tuning::OperationCode::Read;
		sr.startAddressW = m_tuningSettings.tuningDataOffsetW;			// read first frame
		sr.frameSizeW = m_tuningSettings.tuningRomFrameSizeW;
		sr.dataType = Tuning::DataType::Discrete;						//
		sr.romSizeW = m_tuningSettings.tuningRomSizeW;

		source->incNumerator();

		if (m_tuningSocket != nullptr)
		{
			m_tuningSocket->sendRequest(sr);
		}
	}
}


// -------------------------------------------------------------------------------------
//
// TuningService class implementation
//
// -------------------------------------------------------------------------------------

TuningService::TuningService(TuningServiceWorker* worker) :
	Service(worker)
{
	worker->setTuningService(this);
}


void TuningService::getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info)
{
	if (m_tuningServiceWorker == nullptr)
	{
		assert(false);
		return;
	}

	m_tuningServiceWorker->getTuningDataSourcesInfo(info);
}


void TuningService::setSignalState(QString appSignalID, double value)
{
	emit signal_setSignalState(appSignalID, value);
}

void TuningService::getSignalState(QString appSignalID)
{
	emit signal_getSignalState(appSignalID);
}

