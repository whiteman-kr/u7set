#include "../OnlineLib/CfgServerLoader.h"

#include "AppDataService.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"
#include "RtTrendsServer.h"
#include "AppDataReceiver.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataServiceWorker::AppDataServiceWorker(const SoftwareInfo& softwareInfo,
										   const QString& serviceName,
										   int argc,
										   char** argv,
										   CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}

AppDataServiceWorker::AppDataServiceWorker(const AppDataServiceWorker* worker) :
	ServiceWorker(worker)
{
}

AppDataServiceWorker::~AppDataServiceWorker()
{
}

ServiceWorker* AppDataServiceWorker::createInstance() const
{
	AppDataServiceWorker* newInstance = new AppDataServiceWorker(this);

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

void AppDataServiceWorker::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
														 bool isArchivingQueue,
														 const QString& description)
{
	TEST_PTR_RETURN(m_appDataReceiver);

	m_appDataReceiver->registerDestSignalStatesQueue(destQueue, isArchivingQueue, description);
}

void AppDataServiceWorker::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
{
	TEST_PTR_RETURN(m_appDataReceiver);

	m_appDataReceiver->unregisterDestSignalStatesQueue(destQueue);
}

void AppDataServiceWorker::registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
															const std::set<Hash>& hashes)
{
	TEST_PTR_RETURN(m_appDataReceiver);

	m_appDataReceiver->registerGatewaySignalStatesQueue(destQueue, hashes);
}

void AppDataServiceWorker::unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue)
{
	TEST_PTR_RETURN(m_appDataReceiver);

	m_appDataReceiver->unregisterGatewaySignalStatesQueue(destQueue);
}

void AppDataServiceWorker::fillAppDataReceiveState(Network::AppDataReceiveState* adrs)
{
	if (m_appDataReceiver != nullptr)
	{
		m_appDataReceiver->fillAppDataReceiveState(adrs);
	}
	else
	{
		Q_ASSERT(false);
	}
}

void AppDataServiceWorker::initServiceSpecificCmdLineArgs()
{
	addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	addValueCmdLineArg(CmdLineArg::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1, "IP address of first Configuration Service.", "IPv4:Port");
	addValueCmdLineArg(CmdLineArg::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2, "IP address of second Configuration Service.", "IPv4:Port");
	addValueCmdLineArg(CmdLineArg::PTC, SoftwareSetting::PROCESSING_THREADS_COUNT, "App data processing threads count", "N");
	addValueCmdLineArg(CmdLineArg::RECVIP, SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP, "Override AppDataReceivingIP", "IPv4:Port");
	addSimpleNoWritableCmdLineArg(CmdLineArg::LOG_RUP_TIME_ERR, "Log RUP frames time errors");
}

void AppDataServiceWorker::loadServiceSpecificSettings()
{
	m_appDataProcessingThreadCount = getSettingValue(SoftwareSetting::PROCESSING_THREADS_COUNT).toInt();

	m_strCmdLineAppDataReceivingIP = getSettingValue(SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP);
	m_cmdLineAppDataReceivingIP.setAddressPortStr(m_strCmdLineAppDataReceivingIP, PORT_APP_DATA_SERVICE_DATA);

	m_logRupTimeErrors = cmdLineArgIsSet(CmdLineArg::LOG_RUP_TIME_ERR);

	DEBUG_LOG_MSG(logger(), "");
	DEBUG_LOG_MSG(logger(), QString(tr("Service settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::PROCESSING_THREADS_COUNT).arg(m_appDataProcessingThreadCount));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP).arg(m_cmdLineAppDataReceivingIP.addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), "");
}

void AppDataServiceWorker::runAppDataReceiverThread()
{
	if (m_appDataReceiver != nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_appDataReceiver = new AppDataReceiver(m_curSettingsProfile.appDataReceivingIP,
											m_appDataSources,
											m_appSignalStates,
											m_appDataProcessingThreadCount,
											sessionParams().softwareRunMode,
											logger());
	m_appDataReceiver->start();
}

void AppDataServiceWorker::stopAppDataReceiverThread()
{
	if (m_appDataReceiver != nullptr)
	{
		m_appDataReceiver->quitAndWait();
		delete m_appDataReceiver;
		m_appDataReceiver = nullptr;
	}
}

void AppDataServiceWorker::runTcpAppDataServer()
{
	assert(m_tcpAppDataServerThread == nullptr);

	std::vector<Tcp::ListenAddress> listenAddresses;

	std::for_each(m_curSettingsProfile.rcSettings.begin(),
				  m_curSettingsProfile.rcSettings.end(),
				  [&listenAddresses](const RqCtrlSettings& rcs)
					{
						if (rcs.enable == true)
						{
							listenAddresses.emplace_back(rcs.equipmentID, rcs.clientRequestIP, rcs.securityLevel);
						}
					 });

	m_tcpAppDataServerThread = new TcpAppDataServerThread(softwareInfo(), listenAddresses, *this);
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

	m_tcpArchiveClientThread = new TcpArchiveClientThread(softwareInfo(),
														  m_curSettingsProfile.archServiceIP,
														  *this);
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

	std::vector<Tcp::ListenAddress> listenAddresses;

	std::for_each(m_curSettingsProfile.rcSettings.begin(),
				  m_curSettingsProfile.rcSettings.end(),
				  [&listenAddresses](const RqCtrlSettings& rcs)
				  {
						if (rcs.enable == true)
						{
							listenAddresses.emplace_back(rcs.equipmentID, rcs.rtTrendsRequestIP, rcs.securityLevel);
						}
				  });

	m_rtTrendsServerThread = new RtTrends::ServerThread(listenAddresses,
														*this);

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

void AppDataServiceWorker::initialize()
{
	DEBUG_LOG_MSG(logger(), "AppDataServiceWorker is started");

	runCfgLoaderThread();
}

void AppDataServiceWorker::shutdown()
{
	clearConfiguration();

	stopCfgLoaderThread();

	DEBUG_LOG_MSG(logger(), "AppDataServiceWorker finished");
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

	DEBUG_LOG_MSG(logger(), "");

	DEBUG_LOG_MSG(logger(), "Settings profile: " + currentSettingsProfile->profile);

	DEBUG_LOG_MSG(logger(), "");

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

	for(const OnlineLib::BuildFileInfo& bfi : buildFileInfoArray)
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

		if (bfi.ID == CfgFileId::ACQUIRED_APP_SIGNALS)
		{
			result &= readAppSignals(fileData);				// fills m_appSignals
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

bool AppDataServiceWorker::readAppDataSources(const QByteArray& fileData, const QString& profile)
{
	QVector<OnlineLib::DataSource> dataSources;

	bool result = OnlineLib::DataSourcesXML<OnlineLib::DataSource>::readFromXml(fileData, &dataSources);

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
	m_appSignalStates.clear();

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

	m_appSignalStates.setSize(signalCount);

	int index = 0;

	for(AppSignal* signal : m_appSignals)
	{
		TEST_PTR_CONTINUE(signal);

		if (signal->isBus() == true)
		{
			continue;
		}

		DynamicAppSignalState* signalState = m_appSignalStates[index];

		signalState->setSignalParams(signal, m_appSignals);

		index++;
	}

	m_appSignalStates.buidlHash2State();

	m_appSignalStates.setAutoArchivingGroups(m_autoArchivingGroupsCount);
}

void AppDataServiceWorker::buildAcuiredAppSignalIDs()
{
	m_acquiredAppSignalIDs.clear();
	m_acquiredAppSignalIDs.reserve(m_appSignals.count());

	for(const AppSignal* signal : m_appSignals)
	{
		TEST_PTR_CONTINUE(signal);

		if (signal->isAcquired() == true)
		{
			m_acquiredAppSignalIDs.push_back(signal->appSignalID());
		}
	}
}

void AppDataServiceWorker::prepareAppDataSources()
{
	for(AppDataSource* appDataSource : m_appDataSources)
	{
		TEST_PTR_CONTINUE(appDataSource);

		appDataSource->prepare(m_appSignals, &m_appSignalStates, m_autoArchivingGroupsCount, m_timeErrLog);
	}
}

void AppDataServiceWorker::applyNewConfiguration()
{
	m_autoArchivingGroupsCount = m_curSettingsProfile.autoArchiveInterval * 60;

	createTimeErrLog();
	createAndInitSignalStates();
	buildAcuiredAppSignalIDs();
	prepareAppDataSources();

	runAppDataReceiverThread();
	runTcpArchiveClientThread();
	runTcpAppDataServer();
	runRtTrendsServerThread();
}

void AppDataServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
	stopRtTrendsServerThread();
	stopTcpArchiveClientThread();
	stopTcpAppDataServer();
	stopAppDataReceiverThread();

	shutdownTimeErrLog();

	m_appSignals.clear();
	m_appDataSources.clear();
	m_appSignalStates.clear();
	m_acquiredAppSignalIDs.clear();
}

