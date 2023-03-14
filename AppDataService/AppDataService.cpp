#include <QXmlStreamReader>
#include <QMetaProperty>

#include "../OnlineLib/CfgServerLoader.h"

#include "AppDataService.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"
#include "RtTrendsServer.h"
#include "AsyncAppDataReceiver.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataServiceWorker::AppDataServiceWorker(const SoftwareInfo& softwareInfo,
										   const QString& serviceName,
										   int& argc,
										   char** argv,
										   CircularLoggerShared logger,
										   E::ServiceRunMode runMode) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger, runMode),
	m_timer(this)
{
}

AppDataServiceWorker::~AppDataServiceWorker()
{
}

ServiceWorker* AppDataServiceWorker::createInstance() const
{
	AppDataServiceWorker* newInstance = new AppDataServiceWorker(softwareInfo(),
																 serviceName(),
																 argc(), argv(),
																 logger(),
																 serviceRunMode());
	newInstance->init();

	return newInstance;
}

void AppDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::AppDataService, m_curSettingsProfile);

	serviceInfo.set_settingsxml(xmlString.toStdString());
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

E::SecurityLevel AppDataServiceWorker::securityLevel() const
{
	return m_curSettingsProfile.securityLevel;
}

void AppDataServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption(CmdLineOption::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption(CmdLineOption::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1, "IP address of first Configuration Service.", "IPv4:Port");
	cp.addSingleValueOption(CmdLineOption::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2, "IP address of second Configuration Service.", "IPv4:Port");
	cp.addSingleValueOption("ptc", SoftwareSetting::PROCESSING_THREADS_COUNT, "App data processing threads count", "N");
	cp.addSingleValueOption("recvip", SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP, "Override AppDataReceivingIP", "IPv4:Port");
	cp.addSimpleOption(CmdLineOption::LOG_RUP_TIME_ERR, "Log RUP frames time errors");
}

void AppDataServiceWorker::loadSettings()
{
	m_appDataProcessingThreadCount = QString(getStrSetting(SoftwareSetting::PROCESSING_THREADS_COUNT)).toInt();

	m_strCmdLineAppDataReceivingIP = getStrSetting(SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP);
	m_cmdLineAppDataReceivingIP.setAddressPortStr(m_strCmdLineAppDataReceivingIP, PORT_APP_DATA_SERVICE_DATA);

	m_logRupTimeErrors = cmdLineParser().optionIsSet(CmdLineOption::LOG_RUP_TIME_ERR);

	DEBUG_LOG_MSG(logger(), QString(tr("Settings from command line or registry:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::PROCESSING_THREADS_COUNT).arg(m_appDataProcessingThreadCount));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP).arg(m_cmdLineAppDataReceivingIP.addressPortStrIfSet()));
}

void AppDataServiceWorker::runAppDataReceiverThread()
{
	if (m_asyncAppDataReceiver != nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_asyncAppDataReceiver = new AsyncAppDataReceiver(m_curSettingsProfile.appDataReceivingIP,
													  m_appDataSources,
													  m_appDataProcessingThreadCount,
													  sessionParams().softwareRunMode,
													  logger());

	m_asyncAppDataReceiver->start();
}

void AppDataServiceWorker::stopAppDataReceiverlThread()
{
	if (m_asyncAppDataReceiver != nullptr)
	{
		m_asyncAppDataReceiver->quitAndWait();
		delete m_asyncAppDataReceiver;
		m_asyncAppDataReceiver = nullptr;
	}
}

void AppDataServiceWorker::runSignalStatesProcessingThread()
{
	if (m_signalStatesProcessingThread != nullptr)
	{
		assert(false);
		return;
	}

	m_signalStatesProcessingThread = new SignalStatesProcessingThread(m_appDataSources, logger());

	m_signalStatesProcessingThread->start();
}

void AppDataServiceWorker::stopSignalStatesProcessingThread()
{
	if (m_signalStatesProcessingThread != nullptr)
	{
		m_signalStatesProcessingThread->quitAndWait();
		delete m_signalStatesProcessingThread;
		m_signalStatesProcessingThread = nullptr;
	}
}

void AppDataServiceWorker::runAppDataProcessingThreads()
{
//	assert(m_appDataReceiverThread != nullptr);

//	m_appDataProcessingThreadsPool.startProcessingThreads(m_appDataProcessingThreadCount,
//														  m_appDataSourcesIP,
//														  m_appDataReceiverThread,
//														  logger());
/*	if (m_asyncAppDataReceiver != nullptr)
	{
		m_appDataProcessingThreadsPool.startProcessingThreads(m_appDataProcessingThreadCount,
															  *m_asyncAppDataReceiver,
															  logger());
	}
	else
	{
		Q_ASSERT(false);
	}*/
}

void AppDataServiceWorker::stopAppDataProcessingThreads()
{
//	m_appDataProcessingThreadsPool.stopProcessingThreads();
}

void AppDataServiceWorker::runTcpAppDataServer()
{
	assert(m_tcpAppDataServerThread == nullptr);

	TcpAppDataServer* tcpAppDataSever = new TcpAppDataServer(softwareInfo(),
															 m_curSettingsProfile.securityLevel,
															 m_asyncAppDataReceiver,
															 m_signalStatesProcessingThread);

	m_tcpAppDataServerThread = new TcpAppDataServerThread(	m_curSettingsProfile.clientRequestIP,
															tcpAppDataSever,
															m_appDataSources,
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

void AppDataServiceWorker::runTcpArchiveClientThread()
{
	assert(m_tcpArchiveClientThread == nullptr);

	if (m_curSettingsProfile.archServiceID.isEmpty() == true)
	{
		DEBUG_LOG_WRN(logger(), "ArchiveService is not assigned");
		return;
	}

	TcpArchiveClient* tcpArchiveClient = new TcpArchiveClient(softwareInfo(),
												m_curSettingsProfile.archServiceIP,
												m_signalStatesProcessingThread,
												logger());

	m_tcpArchiveClientThread = new TcpArchiveClientThread(tcpArchiveClient);

	m_tcpArchiveClientThread->start();
}

void AppDataServiceWorker::stopTcpArchiveClientThread()
{
	if (m_tcpArchiveClientThread == nullptr)
	{
		return;
	}

	m_tcpArchiveClientThread->quitAndWait();

	delete m_tcpArchiveClientThread;

	m_tcpArchiveClientThread = nullptr;
}

void AppDataServiceWorker::runRtTrendsServerThread()
{
	assert(m_rtTrendsServerThread == nullptr);

	m_rtTrendsServerThread = new RtTrends::ServerThread(m_curSettingsProfile.rtTrendsRequestIP,
														*this,
														m_curSettingsProfile.securityLevel);

	m_rtTrendsServerThread->start();
}

void AppDataServiceWorker::stopRtTrendsServerThread()
{
	if (m_rtTrendsServerThread != nullptr)
	{
		m_rtTrendsServerThread->quitAndWait(10000);
		delete m_rtTrendsServerThread;

		m_rtTrendsServerThread = nullptr;
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

	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

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

void AppDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData,
												const BuildFileInfoArray buildFileInfoArray,
												SessionParams sessionParams,
												std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
{
	setSessionParams(sessionParams);

	DEBUG_LOG_MSG(logger(), "Configuration is ready");

	// stop all threads and free all allocated resources
	//
	clearConfiguration();

	const AppDataServiceSettings* typedSettingsPtr = dynamic_cast<const AppDataServiceSettings*>(currentSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		DEBUG_LOG_MSG(logger(), "Settings casting error!");
		return;
	}

	// making modificable local copy of settings
	//
	m_curSettingsProfile = *typedSettingsPtr;

	// replace some cfg settings by command line arguments
	//
	if (m_strCmdLineAppDataReceivingIP.isEmpty() == false)
	{
		m_curSettingsProfile.appDataReceivingIP = m_cmdLineAppDataReceivingIP;
	}

	bool result = true;

	for(const Builder::BuildFileInfo& bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			result = false;
			continue;
		}

		result = true;

		if (bfi.ID == CfgFileId::APP_DATA_SOURCES)
		{
			result &= readAppDataSources(fileData, sessionParams.currentSettingsProfile);			// fill m_appDataSources
		}

		if (bfi.ID == CfgFileId::APP_SIGNAL_SET)
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

bool AppDataServiceWorker::readAppDataSources(const QByteArray& fileData, const QString& profile)
{
	QVector<DataSource> dataSources;

	bool result = DataSourcesXML<DataSource>::readFromXml(fileData, &dataSources);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error reading AppDataSources from XML-file"));
		return false;
	}

	result = m_appDataSources.init(profile, dataSources, logger());

	if (result == true)
	{
		DEBUG_LOG_MSG(logger(), QString("AppDataSources successfully loaded"));
	}
	else
	{
		DEBUG_LOG_ERR(logger(), QString("AppDataSources loading error!"));
	}

	return result;
}


bool AppDataServiceWorker::readAppSignals(const QByteArray& fileData)
{
	::Proto::AppSignalSet signalSet;

	bool result = signalSet.ParseFromArray(fileData.constData(), static_cast<int>(fileData.size()));

	if (result == false)
	{
		return false;
	}

	int signalCount = signalSet.appsignal_size();

	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& appSignal = signalSet.appsignal(i);

		m_appSignals.insert(appSignal);
	}

	return true;
}


void AppDataServiceWorker::createTimeErrLog()
{
	Q_ASSERT(m_timeErrLog == nullptr);

	if (m_logRupTimeErrors == true)
	{
		m_timeErrLog = std::make_shared<CircularLogger>();

		LOGGER_INIT(m_timeErrLog, QString("RupTimeErr"), QString());

		m_timeErrLog->setLogCodeInfo(false);
	}
}

void AppDataServiceWorker::shutdownTimeErrLog()
{
	if (m_timeErrLog != nullptr)
	{
		LOGGER_SHUTDOWN(m_timeErrLog);
		m_timeErrLog = nullptr;
	}
}

void AppDataServiceWorker::createAndInitSignalStates()
{
	m_signalStates.clear();

	if (m_appSignals.isEmpty())
	{
		return;
	}

	int signalCount = 0;

	for(AppSignal* signal : m_appSignals)
	{
		TEST_PTR_CONTINUE(signal);

		if (signal->isBus() == true)
		{
			continue;
		}

		signalCount++;
	}

	m_signalStates.setSize(signalCount);

	int index = 0;

	for(AppSignal* signal : m_appSignals)
	{
		TEST_PTR_CONTINUE(signal);

		if (signal->isBus() == true)
		{
			continue;
		}

		DynamicAppSignalState* signalState = m_signalStates[index];

		signalState->setSignalParams(signal, m_appSignals);

		index++;
	}

	m_signalStates.buidlHash2State();

	m_signalStates.setAutoArchivingGroups(m_autoArchivingGroupsCount);
}

void AppDataServiceWorker::prepareAppDataSources()
{
	for(AppDataSource* appDataSource : m_appDataSources)
	{
		TEST_PTR_CONTINUE(appDataSource);

		appDataSource->prepare(m_appSignals, &m_signalStates, m_autoArchivingGroupsCount, m_timeErrLog);
	}
}

void AppDataServiceWorker::applyNewConfiguration()
{
	m_autoArchivingGroupsCount = m_curSettingsProfile.autoArchiveInterval * 60;

	createTimeErrLog();
	createAndInitSignalStates();
	prepareAppDataSources();

	runSignalStatesProcessingThread();
	runTcpArchiveClientThread();
	runAppDataReceiverThread();
	runTcpAppDataServer();
	runAppDataProcessingThreads();
	runRtTrendsServerThread();
}

void AppDataServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
	stopRtTrendsServerThread();
	stopAppDataProcessingThreads();
	stopTcpAppDataServer();
	stopAppDataReceiverlThread();
	stopTcpArchiveClientThread();
	stopSignalStatesProcessingThread();
	shutdownTimeErrLog();

	m_appSignals.clear();
	m_appDataSources.clear();
	m_signalStates.clear();
}

