#include <QXmlStreamReader>
#include <QMetaProperty>

#include "../lib/DeviceObject.h"
#include "../lib/CfgServerLoader.h"

#include "AppDataService.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"

#include "version.h"


// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

const char* const AppDataServiceWorker::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const AppDataServiceWorker::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
const char* const AppDataServiceWorker::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";

AppDataServiceWorker::AppDataServiceWorker(const QString& serviceName,
										   int& argc,
										   char** argv,
										   const VersionInfo &versionInfo,
										   std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(ServiceType::AppDataService, serviceName, argc, argv, versionInfo, logger),
	m_logger(logger),
	m_timer(this),
	m_signalStatesQueue(1)			// shoud be resized after cfg loading according to signals count
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		m_appDataChannelThread[channel] = nullptr;
		m_tcpArchiveClientThreads[channel] = nullptr;
	}
}


AppDataServiceWorker::~AppDataServiceWorker()
{
}


ServiceWorker* AppDataServiceWorker::createInstance() const
{
	AppDataServiceWorker* newInstance = new AppDataServiceWorker(serviceName(), argc(), argv(), versionInfo(), m_logger);

	newInstance->init();

	return newInstance;
}


void AppDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	serviceInfo.set_clientrequestip(m_cfgSettings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_cfgSettings.clientRequestIP.port());
}


void AppDataServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");
}

void AppDataServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting(SETTING_EQUIPMENT_ID);

	m_cfgServiceIP1Str = getStrSetting(SETTING_CFG_SERVICE_IP1);

	m_cfgServiceIP1 = HostAddressPort(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_REQUEST);

	m_cfgServiceIP2Str = getStrSetting(SETTING_CFG_SERVICE_IP2);

	m_cfgServiceIP2 = HostAddressPort(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(m_equipmentID));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(m_cfgServiceIP1.addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(m_cfgServiceIP2.addressPortStr()));
}


void AppDataServiceWorker::runCfgLoaderThread()
{
	CfgLoader* cfgLoader = new CfgLoader(m_equipmentID, 1, m_cfgServiceIP1, m_cfgServiceIP2, false, m_logger, E::SoftwareType::AppDataService, 0, 1, USED_SERVER_COMMIT_NUMBER);

	m_cfgLoaderThread = new CfgLoaderThread(cfgLoader);

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &AppDataServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();

	m_cfgLoaderThread->enableDownloadConfiguration();
}


void AppDataServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(false);
		return;
	}

	m_cfgLoaderThread->quit();

	delete m_cfgLoaderThread;
}

void AppDataServiceWorker::runTcpAppDataServer()
{
	assert(m_tcpAppDataServerThread == nullptr);

	TcpAppDataServer* tcpAppDataSever = new TcpAppDataServer();

	m_tcpAppDataServerThread = new TcpAppDataServerThread(	m_cfgSettings.clientRequestIP,
															tcpAppDataSever,
															m_enabledAppDataSources,
															m_appSignals,
															m_signalStates,
															m_unitInfo,
															m_logger);
	m_tcpAppDataServerThread->start();
}

void AppDataServiceWorker::stopTcpAppDataServer()
{
	if (m_tcpAppDataServerThread != nullptr)
	{
		m_tcpAppDataServerThread->quitAndWait(10000);
		delete m_tcpAppDataServerThread;

		m_tcpAppDataServerThread = nullptr;
	}
}

void AppDataServiceWorker::runTcpArchiveClientThreads()
{
	for(int channel = AppDataServiceSettings::DATA_CHANNEL_1; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		assert(m_tcpArchiveClientThreads[channel] == nullptr);

		if (m_cfgSettings.appDataServiceChannel[channel].archServiceStrID.isEmpty() == true)
		{
			m_tcpArchiveClientThreads[channel] = nullptr;
			continue;
		}

		TcpArchiveClient* client = new TcpArchiveClient(channel,
														m_cfgSettings.appDataServiceChannel[channel].archServiceIP,
														E::SoftwareType::AppDataService,
														m_equipmentID,
														m_majorVersion,
														m_minorVersion,
														USED_SERVER_COMMIT_NUMBER,
														m_logger,
														m_signalStatesQueue);

		m_tcpArchiveClientThreads[channel] = new Tcp::Thread(client);

		m_tcpArchiveClientThreads[channel]->start();
	}
}

void AppDataServiceWorker::stopTcpArchiveClientThreads()
{
	for(int channel = AppDataServiceSettings::DATA_CHANNEL_1; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		if (m_tcpArchiveClientThreads[channel] == nullptr)
		{
			continue;
		}

		m_tcpArchiveClientThreads[channel]->quitAndWait();

		delete m_tcpArchiveClientThreads[channel];

		m_tcpArchiveClientThreads[channel] = nullptr;
	}
}

void AppDataServiceWorker::runTimer()
{
	connect(&m_timer, &QTimer::timeout, this, &AppDataServiceWorker::onTimer);

	m_timer.setInterval(1000);
	m_timer.start();
}


void AppDataServiceWorker::stopTimer()
{
	m_timer.stop();
}


void AppDataServiceWorker::initialize()
{
	// Service Main Function initialization
	//
	runCfgLoaderThread();
	runTimer();
	qDebug() << "DataServiceMainFunctionWorker initialized";
}


void AppDataServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//
	clearConfiguration();

	stopTimer();

	stopTcpAppDataServer();
	stopCfgLoaderThread();

	qDebug() << "DataServiceWorker stoped";
}


void AppDataServiceWorker::onTimer()
{
}


void AppDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	qDebug() << "Configuration Ready!";

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	// stop all AppDataChannelThreads and
	// free all allocated resources
	//
	clearConfiguration();

	bool result = readConfiguration(configurationXmlData);

	if (result == false)
	{
		return;
	}

	for(Builder::BuildFileInfo bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			continue;
		}

		result = true;

		if (bfi.ID == CFG_FILE_ID_DATA_SOURCES)
		{
			result &= readDataSources(fileData);			// fill m_appDataSources
		}

		if (bfi.ID == CFG_FILE_ID_APP_SIGNAL_SET)
		{
			result &= readAppSignals(fileData);				// fill m_unitInfo and m_appSignals
		}

		if (result == true)
		{
			qDebug() << "Read file " << bfi.pathFileName << " OK";
		}
		else
		{
			qDebug() << "Read file " << bfi.pathFileName << " ERROR";
			break;
		}
	}

	if (result == true)
	{
		applyNewConfiguration();
	}
}


bool AppDataServiceWorker::readConfiguration(const QByteArray& fileData)
{
	XmlReadHelper xml(fileData);

	bool result = m_cfgSettings.readFromXml(xml);

	if (result == true)
	{
		qDebug() << "Reading settings - OK";
	}
	else
	{
		qDebug() << "Settings read ERROR!";
	}

	return result;
}


bool AppDataServiceWorker::readDataSources(QByteArray& fileData)
{
	XmlReadHelper xml(fileData);

	bool result = true;

	m_appDataSources.clear();
	m_enabledAppDataSources.clear();

	while (1)
	{
		bool find = xml.findElement(DataSource::ELEMENT_DATA_SOURCE);

		if (find == false)
		{
			break;
		}

		AppDataSource* dataSource = new AppDataSource();

		result &= dataSource->readFromXml(xml);

		if (result == false)
		{
			assert(false);
			qDebug() << "DataSource error reading from xml";
			delete dataSource;
			continue;
		}

		int channel = dataSource->lmChannel();

		if (channel < 0 || channel >= AppDataServiceSettings::DATA_CHANNEL_COUNT)
		{
			assert(false);
			qDebug() << "DataSource wrong channel";
			delete dataSource;
			continue;
		}

		// dataSource->lmAdapterID() is unique for all data sources
		//
		m_appDataSources.insert(dataSource->lmAdapterID(), dataSource);

		qDebug() << "DataSource: " << dataSource->lmAdapterID();

		if (dataSource->lmDataEnable() == true)
		{
			// dataSource->lmAddress32() is unique for enabled datasources
			//
			m_enabledAppDataSources.insert(dataSource->lmAddress32(), dataSource);
		}
	}

	return result;
}


bool AppDataServiceWorker::readAppSignals(QByteArray& fileData)
{
	::Proto::AppSignalSet signalSet;

	bool result = signalSet.ParseFromArray(fileData.constData(), fileData.size());

	if (result == false)
	{
		return false;
	}

	int signalCount = signalSet.appsignal_size();

	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& appSignal = signalSet.appsignal(i);

		if (m_appSignals.contains(QString::fromStdString(appSignal.appsignalid())) == true)
		{
			assert(false);
			continue;
		}

		Signal* s = new Signal;

		s->serializeFrom(appSignal);

		m_appSignals.insert(s->appSignalID(), s);
	}

	return true;


/*	XmlReadHelper xml(fileData);

	if (xml.findElement("Units") == false)
	{
		return false;
	}

	int unitCount = 0;

	result &= xml.readIntAttribute("Count", &unitCount);

	for(int count = 0; count < unitCount; count++)
	{
		if(xml.findElement("Unit") == false)
		{
			result = false;
			break;
		}

		int unitID = 0;
		QString unitCaption;

		result &= xml.readIntAttribute("ID", &unitID);
		result &= xml.readStringAttribute("Caption", &unitCaption);

		m_unitInfo.append(unitID, unitCaption);
	}

	if (m_unitInfo.count() != unitCount)
	{
		qDebug() << "Units loading error";
		assert(false);
		return false;
	}

	if (xml.findElement("Signals") == false)
	{
		return false;
	}

	int signalCount = 0;

	result &= xml.readIntAttribute("Count", &signalCount);

	//quint64 time1 = QDateTime::currentMSecsSinceEpoch();

	for(int count = 0; count < signalCount; count++)
	{
		if (xml.findElement("Signal") == false)
		{
			result = false;
			break;
		}

		Signal* signal = new Signal();

		bool res = signal->readFromXml(xml);	// time-expensive function !!!

		if (res == true)
		{
			if (m_appSignals.contains(signal->appSignalID()) == false)
			{
				m_appSignals.insert(signal->appSignalID(), signal);
			}
			else
			{
				res = false;
			}
		}

		if (res == false)
		{
			delete signal;
		}

		result &= res;
	}

	//quint64 time2 = QDateTime::currentMSecsSinceEpoch();
	//qDebug() << "time " << (time2 - time1) << " per 1 " << (time2 - time1)/289;

	m_appSignals.buildHash2Signal();*/

	return result;
}


void AppDataServiceWorker::createAndInitSignalStates()
{
	m_signalStates.clear();

	if (m_appSignals.isEmpty())
	{
		return;
	}

	int signalCount = m_appSignals.count();

	m_signalStates.setSize(signalCount);

	int index = 0;

	for(Signal* signal : m_appSignals)
	{
		AppSignalStateEx* signalState = m_signalStates[index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setSignalParams(index, signal);

		index++;
	}

	m_signalStates.buidlHash2State();

	m_signalStates.setAutoArchivingGroups(m_autoArchivingGroupsCount);
}


void AppDataServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
	stopTcpAppDataServer();
	stopDataChannelThreads();
	stopTcpArchiveClientThreads();

	m_unitInfo.clear();

	m_appSignals.clear();
	m_enabledAppDataSources.clear();
	m_signalStates.clear();
}

void AppDataServiceWorker::applyNewConfiguration()
{
	m_autoArchivingGroupsCount = m_cfgSettings.autoArchiveInterval * 60;

	resizeAppSignalEventsQueue();

	createAndInitSignalStates();

	initDataChannelThreads();

	runTcpArchiveClientThreads();

	runDataChannelThreads();

	runTcpAppDataServer();
}

void AppDataServiceWorker::resizeAppSignalEventsQueue()
{
	int queueSize = m_appSignals.count() * 2;

	if (queueSize == 0)
	{
		queueSize = 1;
	}

	if (queueSize > APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE)
	{
		DEBUG_LOG_WRN(m_logger,"AppSignalEvents queue is reached max size");

		queueSize = APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE;
	}

	m_signalStatesQueue.resize(queueSize);
}

void AppDataServiceWorker::initDataChannelThreads()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		// create AppDataChannelThread
		//
		m_appDataChannelThread[channel] = new AppDataChannelThread(channel,
						m_cfgSettings.appDataServiceChannel[channel].appDataReceivingIP,
						m_signalStatesQueue,
						m_autoArchivingGroupsCount);

		// add AppDataSources to channel
		//
		for(AppDataSource* appDataSource : m_enabledAppDataSources)
		{
			if (appDataSource->lmDataType() == DataSource::DataType::App &&
				appDataSource->lmChannel() == channel)
			{
				m_appDataChannelThread[channel]->addDataSource(appDataSource);
			}
		}

		// build required data structures inside AppDataChannel
		//
		m_appDataChannelThread[channel]->prepare(m_appSignals, &m_signalStates);
	}
}


void AppDataServiceWorker::runDataChannelThreads()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		if (m_appDataChannelThread[channel] != nullptr)
		{
			m_appDataChannelThread[channel]->start();
		}
		else
		{
			assert(false);
		}
	}
}


void AppDataServiceWorker::stopDataChannelThreads()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		if (m_appDataChannelThread[channel] != nullptr)
		{
			m_appDataChannelThread[channel]->quitAndWait();
			delete m_appDataChannelThread[channel];
			m_appDataChannelThread[channel] = nullptr;
		}
	}
}

