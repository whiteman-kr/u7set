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

AppDataServiceWorker::AppDataServiceWorker(const SoftwareInfo& softwareInfo,
										   const QString& serviceName,
										   int& argc,
										   char** argv,
										   CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
	m_timer(this),
	m_signalStatesQueue(1)			// shoud be resized after cfg loading according to signals count
{
}

AppDataServiceWorker::~AppDataServiceWorker()
{
}

ServiceWorker* AppDataServiceWorker::createInstance() const
{
	AppDataServiceWorker* newInstance = new AppDataServiceWorker(softwareInfo(), serviceName(), argc(), argv(), logger());

	newInstance->init();

	return newInstance;
}

void AppDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	serviceInfo.set_clientrequestip(m_cfgSettings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_cfgSettings.clientRequestIP.port());
}

bool AppDataServiceWorker::isConnectedToConfigurationService(quint32& ip, quint16& port) const
{
	if (m_cfgLoaderThread == nullptr)
	{
		return false;
	}

	Tcp::ConnectionState&& state = m_cfgLoaderThread->getConnectionState();

	if (state.isConnected)
	{
		ip = state.peerAddr.address32();
		port = state.peerAddr.port();

		return true;
	}

	return false;
}

bool AppDataServiceWorker::isConnectedToArchiveService(quint32 &ip, quint16 &port) const
{
	if (m_tcpArchiveClientThread == nullptr)
	{
		return false;
	}

	Tcp::ConnectionState&& state = m_tcpArchiveClientThread->getConnectionState();

	if (state.isConnected == true)
	{
		ip = state.peerAddr.address32();
		port = state.peerAddr.port();

		return true;
	}

	return false;
}

void AppDataServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");
	cp.addSingleValueOption("ptc", SETTING_PROCESSING_THREADS_COUNT, "App data processing threads count", "N");
}

void AppDataServiceWorker::loadSettings()
{
	m_appDataProcessingThreadCount = QString(getStrSetting(SETTING_PROCESSING_THREADS_COUNT)).toInt();

	DEBUG_LOG_MSG(logger(), QString(tr("Load settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_PROCESSING_THREADS_COUNT).arg(m_appDataProcessingThreadCount));
}


void AppDataServiceWorker::runAppDataReceiverThread()
{
	if (m_appDataReceiverThread != nullptr)
	{
		assert(false);
		return;
	}

	m_appDataReceiverThread = new AppDataReceiverThread(m_cfgSettings.appDataReceivingIP, m_appDataSourcesIP, logger());

	m_appDataReceiverThread->start();
}

void AppDataServiceWorker::stopAppDataReceiverlThread()
{
	if (m_appDataReceiverThread != nullptr)
	{
		m_appDataReceiverThread->quitAndWait();
		delete m_appDataReceiverThread;
		m_appDataReceiverThread = nullptr;
	}
}

void AppDataServiceWorker::runAppDataProcessingThreads()
{
	assert(m_appDataReceiverThread != nullptr);

	m_appDataProcessingThreadsPool.startProcessingThreads(m_appDataProcessingThreadCount,
														  m_appDataSourcesIP,
														  m_appDataReceiverThread->appDataReceiver(),
														  logger());
}

void AppDataServiceWorker::stopAppDataProcessingThreads()
{
	m_appDataProcessingThreadsPool.stopProcessingThreads();
}

void AppDataServiceWorker::runTcpAppDataServer()
{
	assert(m_tcpAppDataServerThread == nullptr);

	TcpAppDataServer* tcpAppDataSever = new TcpAppDataServer(softwareInfo());

	m_tcpAppDataServerThread = new TcpAppDataServerThread(	m_cfgSettings.clientRequestIP,
															tcpAppDataSever,
															m_appDataSourcesIP,
															m_appSignals,
															m_signalStates,
															*this,
															logger());
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
	assert(m_tcpArchiveClientThread == nullptr);

	if (m_cfgSettings.archServiceID.isEmpty() == true)
	{
		return;
	}

	TcpArchiveClient* tcpArchiveClient = new TcpArchiveClient(softwareInfo(),
												m_cfgSettings.archServiceIP,
												logger(),
												m_signalStatesQueue);

	m_tcpArchiveClientThread = new TcpArchiveClientThread(tcpArchiveClient);

	m_tcpArchiveClientThread->start();
}

void AppDataServiceWorker::stopTcpArchiveClientThreads()
{
	if (m_tcpArchiveClientThread == nullptr)
	{
		return;
	}

	m_tcpArchiveClientThread->quitAndWait();

	delete m_tcpArchiveClientThread;

	m_tcpArchiveClientThread = nullptr;
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
	DEBUG_LOG_MSG(logger(), "AppDataServiceWorker is started");

	runCfgLoaderThread();
	runTimer();
}

void AppDataServiceWorker::shutdown()
{
	clearConfiguration();

	stopTimer();

	stopTcpAppDataServer();
	stopCfgLoaderThread();

	DEBUG_LOG_MSG(logger(), "AppDataServiceWorker is finished");
}

void AppDataServiceWorker::runCfgLoaderThread()
{
	assert(m_cfgLoaderThread == nullptr);			// once should be runned

	CfgLoader* cfgLoader = new CfgLoader(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	m_cfgLoaderThread = new CfgLoaderThread(cfgLoader);

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &AppDataServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();

	m_cfgLoaderThread->enableDownloadConfiguration();
}

void AppDataServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	m_cfgLoaderThread->quitAndWait();

	delete m_cfgLoaderThread;

	m_cfgLoaderThread = nullptr;
}

void AppDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	DEBUG_LOG_MSG(logger(), "Configuration is ready");

	// stop all threads and free all allocated resources
	//
	clearConfiguration();

	bool result = readConfigurationSettings(configurationXmlData);

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

void AppDataServiceWorker::onTimer()
{
}


bool AppDataServiceWorker::readConfigurationSettings(const QByteArray& fileData)
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


bool AppDataServiceWorker::readDataSources(const QByteArray& fileData)
{
	m_appDataSources.clear();
	m_appDataSourcesIP.clear();

	QVector<DataSource> dataSources;

	bool result = DataSourcesXML::readFromXml(fileData, &dataSources);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error reading AppDataSources from XML-file"));
		return false;
	}

	for(int i = 0; i < dataSources.count(); i++)
	{
		AppDataSourceShared appDataSource = std::make_shared<AppDataSource>(dataSources[i]);

		if (m_appDataSources.contains(appDataSource->lmAdapterID()) == true)
		{
			DEBUG_LOG_ERR(logger(), QString("Duplicate AppDataSource ID %1").arg(appDataSource->lmAdapterID()));
			continue;
		}

		if (m_appDataSourcesIP.contains(appDataSource->lmAddress32()) == true)
		{
			DEBUG_LOG_ERR(logger(), QString("Duplicate AppDataSource IP-address %1").arg(appDataSource->lmAddressPort().addressPortStr()));
			continue;
		}

		m_appDataSources.insert(appDataSource->lmAdapterID(), appDataSource);
		m_appDataSourcesIP.insert(appDataSource->lmAddress32(), appDataSource);
	}

	DEBUG_LOG_MSG(logger(), QString("AppDataSources successfully loaded"));

	return true;
}


bool AppDataServiceWorker::readAppSignals(const QByteArray& fileData)
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

	m_appSignals.buildHash2Signal();

	return true;
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

void AppDataServiceWorker::applyNewConfiguration()
{
	m_autoArchivingGroupsCount = m_cfgSettings.autoArchiveInterval * 60;

	resizeAppSignalEventsQueue();

	createAndInitSignalStates();

	runTcpArchiveClientThreads();
	runTcpAppDataServer();
	runAppDataReceiverThread();
	runAppDataProcessingThreads();
}

void AppDataServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
	stopAppDataProcessingThreads();
	stopAppDataReceiverlThread();
	stopTcpAppDataServer();
	stopTcpArchiveClientThreads();

	m_appSignals.clear();
	m_appDataSources.clear();
	m_appDataSourcesIP.clear();
	m_signalStates.clear();
}

void AppDataServiceWorker::resizeAppSignalEventsQueue()
{
	int queueSize = m_appSignals.count() * 2;

	if (queueSize == 0)
	{
		queueSize = 100;
	}

	if (queueSize > APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE)
	{
		DEBUG_LOG_WRN(logger(),"AppSignalEvents queue is reached max size");

		queueSize = APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE;
	}

	m_signalStatesQueue.resize(queueSize);
}

