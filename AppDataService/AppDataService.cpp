#include "../OnlineLib/CfgLoader.h"

#include "AppDataService.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"
#include "RtTrendsServer.h"
#include "AppDataReceiver.h"
#include "ApertureFile.h"
#include "AppDataSrvTools.h"

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
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger, "AppDataServiceWorker")
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

void AppDataServiceWorker::processGetServiceInfoRequest(const Network::GetServiceInfoRequest& rq)
{
	int aperturesSize = rq.aperturerecords().size();

	if (aperturesSize == 0)
	{
		return;
	}

	QString logMsg;

	for(int i = 0; i < aperturesSize; i++)
	{
		ApertureRecord ar;

		ar.readFromProto(rq.aperturerecords(i));

		m_appSignalStates.overrideAperture(ar, logMsg);

		if (logMsg.isEmpty() == false)
		{
			DEBUG_LOG_MSG(logger(), logMsg);
		}

		m_apertureFile.updateAperture(ar);
	}

	m_apertureFile.save();

	m_updateArchSignalsAnyway = true;

	emit restartArchSignalsTimer();
}

void AppDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	QMutexLocker l(&m_startStopMutex);

	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::AppDataService, m_curSettingsProfile);

	serviceInfo.set_settingsxml(xmlString.toStdString());

	serviceInfo.set_cfgserviceip1(cfgServiceIP1().address32());
	serviceInfo.set_cfgserviceport1(cfgServiceIP1().port());

	serviceInfo.set_cfgserviceip2(cfgServiceIP2().address32());
	serviceInfo.set_cfgserviceport2(cfgServiceIP2().port());

	if (m_tcpAppDataServerThread != nullptr)
	{
		m_tcpAppDataServerThread->getClientsList(&serviceInfo);
	}

	if (m_rtTrendsServerThread != nullptr)
	{
		m_rtTrendsServerThread->getClientsList(&serviceInfo);
	}

	for(const AppDataSource* ads : m_appDataSources)
	{
		TEST_PTR_CONTINUE(ads);

		Network::AppDataSourceState* state = serviceInfo.add_appdatasourcesstates();

		ads->getState(state);
	}

	copyArchSignalsInfo(serviceInfo);
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

void AppDataServiceWorker::ackDiscretesLog(const Network::AckDiscretesLogRequest& ackLogRequest)
{
	if (m_discretesLogWriter)
	{
		m_discretesLogWriter->ackDiscretesLog(ackLogRequest);
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

void AppDataServiceWorker::initialize()
{
	DEBUG_LOG_MSG(logger(), "AppDataServiceWorker is started");

	m_discretesLogWriter = std::make_shared<DiscretesLogWriter>();

	runCfgLoaderThread();

	connect(this, &AppDataServiceWorker::restartArchSignalsTimer, this, &AppDataServiceWorker::onRestartArchSignalsTimer);
}

void AppDataServiceWorker::shutdown()
{
	clearConfiguration();

	stopCfgLoaderThread();

	m_discretesLogWriter.reset();

	DEBUG_LOG_MSG(logger(), "AppDataServiceWorker finished");
}

void AppDataServiceWorker::runCfgLoaderThread()
{
	// assert(m_cfgLoaderThread == nullptr);			// once should be runned

	// m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	// connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &AppDataServiceWorker::onConfigurationReady);

	// m_cfgLoaderThread->start();

	// m_cfgLoaderThread->enableDownloadConfiguration();

	assert(m_grpcCfgLoaderThread == nullptr);			// once should be runned

	m_grpcCfgLoaderThread = new GrpcCfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	connect(m_grpcCfgLoaderThread, &GrpcCfgLoaderThread::signal_configurationReady, this, &AppDataServiceWorker::onConfigurationReady);

	m_grpcCfgLoaderThread->start();

	m_grpcCfgLoaderThread->enableDownloadConfiguration();

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

	bool res = readBuildInfo(configurationXmlData);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error reading BuildInfo from configurationXmlData"));
		return;
	}

	const AppDataServiceSettings* typedSettingsPtr = dynamic_cast<const AppDataServiceSettings*>(currentSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		DEBUG_LOG_MSG(logger(), "Settings casting error!");
		return;
	}

	setCfgServiceID1(typedSettingsPtr->cfgServiceID1);
	setCfgServiceID2(typedSettingsPtr->cfgServiceID2);

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
	return AppDataSrvTools::readAppDataSources(fileData, profile, m_appDataSources, logger());
}

bool AppDataServiceWorker::readAppSignals(const QByteArray& fileData)
{
	return AppDataSrvTools::readAppSignals(fileData, m_appSignals);
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
	AppDataSrvTools::createAndInitSignalStates(m_appSignals, m_appSignalStates, m_autoArchivingGroupsCount);

	//

	m_apertureFile.clear();

	if (m_apertureFile.load(cmdLineArg(0)) == false)
	{
		DEBUG_LOG_WRN(logger(), "Aperture.csv file NOT loded!");
	}

	QString logMsg;

	for(const auto& [signalID, ar] : m_apertureFile.apertures())
	{

		m_appSignalStates.overrideAperture(ar, logMsg);

		if (logMsg.isEmpty() == false)
		{
			DEBUG_LOG_MSG(logger(), logMsg);
		}
	}

	//

	m_recordsPerMin.clear();
}

void AppDataServiceWorker::buildAcuiredAppSignalIDs()
{
	m_acquiredAppSignalIDs.clear();
	m_acquiredAppSignalIDs.reserve(m_appSignals.count());

	for(const AppSignal& signal : m_appSignals)
	{
		if (signal.isAcquired() == true)
		{
			m_acquiredAppSignalIDs.push_back(signal.appSignalID());
		}
	}
}

void AppDataServiceWorker::prepareAppDataSources()
{
	for(AppDataSource* appDataSource : m_appDataSources)
	{
		TEST_PTR_CONTINUE(appDataSource);

		appDataSource->prepare(m_appSignals, &m_appSignalStates, m_discretesLogWriter,
							   m_autoArchivingGroupsCount, m_timeErrLog);
	}
}

void AppDataServiceWorker::applyNewConfiguration()
{
	QMutexLocker l(&m_startStopMutex);

	m_autoArchivingGroupsCount = m_curSettingsProfile.autoArchiveInterval * 60;

	createTimeErrLog();
	createAndInitSignalStates();
	buildAcuiredAppSignalIDs();

	m_discretesLogWriter->start(buildInfo().project, equipmentID(), m_curSettingsProfile.discretesLogHours, logger());

	prepareAppDataSources();

	runAppDataReceiverThread();
	runTcpArchiveClientThread();
//	runTcpAppDataServer();
	runRtTrendsServerThread();
	runGrpcAppDataSrv();

	onRestartArchSignalsTimer();

	connect(m_archSignalsUpdateTimer, &QTimer::timeout, this, &AppDataServiceWorker::onArchSignalsTimer);
}

void AppDataServiceWorker::clearConfiguration()
{
	QMutexLocker l(&m_startStopMutex);

	if (m_archSignalsUpdateTimer != nullptr)
	{
		m_archSignalsUpdateTimer->stop();
		delete m_archSignalsUpdateTimer;
		m_archSignalsUpdateTimer = nullptr;
	}

	// free all resources allocated in onConfigurationReady
	//
	m_discretesLogWriter->stop();

	stopGrpcAppDataSrv();
	stopRtTrendsServerThread();
//	stopTcpAppDataServer();
	stopTcpArchiveClientThread();
	stopAppDataReceiverThread();

	shutdownTimeErrLog();

	m_appSignals.clear();
	m_appDataSources.clear();
	m_appSignalStates.clear();
	m_acquiredAppSignalIDs.clear();
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

	std::vector<Tcp::ListenAddress> listenAddrs;

	for(const RqCtrlSettings& rcs :  m_curSettingsProfile.rcSettings)
	{
		if (rcs.enable() == true)
		{
			listenAddrs.emplace_back(rcs.equipmentID(), rcs.clientRequestIP(), rcs.securityLevel());
		}
	 };

	m_tcpAppDataServerThread = new TcpAppDataServerThread(softwareInfo(), listenAddrs, *this);
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
					if (rcs.enable() == true)
					{
						listenAddresses.emplace_back(rcs.equipmentID(), rcs.rtTrendsRequestIP(), rcs.securityLevel());
					}
				  });

	m_rtTrendsServerThread = new RtTrends::ServerThread(listenAddresses,
														softwareInfo(),
														m_appDataSources,
														m_appSignalStates,
														logger());

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

void AppDataServiceWorker::runGrpcAppDataSrv()
{
	m_grpcAppDataSrvs.clear();
	m_grpcAppDataSrvs.reserve(m_curSettingsProfile.rcSettings.size());

	for(const RqCtrlSettings& rcs : m_curSettingsProfile.rcSettings)
	{
		if (rcs.enable() == false)
		{
			continue;
		}

		HostAddressPort listenIP = rcs.clientRequestIP();

		m_grpcAppDataSrvs.emplace_back(std::make_unique<GrpcAppDataSrv>(softwareInfo(),
															true, m_clients, false,
															listenIP, m_appDataSources,
															m_appDataReceiver, m_appSignals,
															m_appSignalStates, m_discretesLogWriter,
															logger()));
	}
}

void AppDataServiceWorker::stopGrpcAppDataSrv()
{
	m_grpcAppDataSrvs.clear();
}

void AppDataServiceWorker::getRecordsPerMin(std::vector<RecordsPerMin>* recordsPerMin,
	int count, double* updateStatus) const
{
	TEST_PTR_RETURN(recordsPerMin);
	TEST_PTR_RETURN(updateStatus);

	if (m_archSignalsUpdateTimer == nullptr)
	{
		*updateStatus = 0;
	}
	else
	{
		qint64 dt = QDateTime::currentMSecsSinceEpoch() - m_archSignalsTimerStartMs;

		if (dt < 0)
		{
			dt = 0;
		}
		else
		{
			if (dt > ARCH_SIGNALS_UPDATE_INTERVAL)
			{
				dt = ARCH_SIGNALS_UPDATE_INTERVAL;
			}
		}

		*updateStatus = static_cast<double>(dt) / ARCH_SIGNALS_UPDATE_INTERVAL * 100.0;
	}

	QMutexLocker loker(&m_recordsPerMinMutex);

	int recordsPerMinSize = TO_INT(m_recordsPerMin.size());

	if (recordsPerMinSize == 0)
	{
		recordsPerMin->clear();
		return;
	}

	if (count >= recordsPerMinSize)
	{
		recordsPerMin->resize(recordsPerMinSize);

		std::copy(m_recordsPerMin.begin(), m_recordsPerMin.begin() + recordsPerMinSize,
				  recordsPerMin->begin());
	}
	else
	{
		std::vector<RecordsPerMin> addVector;

		for(int i = count; i < recordsPerMinSize; i++)
		{
			if (m_recordsPerMin[i].overrided)
			{
				addVector.push_back(m_recordsPerMin[i]);
			}
		}

		recordsPerMin->resize(count + addVector.size());

		std::copy(m_recordsPerMin.begin(), m_recordsPerMin.begin() + count,
				  recordsPerMin->begin());

		std::copy(addVector.begin(), addVector.end(),
				  recordsPerMin->begin() + count);
	}
}

void AppDataServiceWorker::onRestartArchSignalsTimer()
{
	if (m_archSignalsUpdateTimer == nullptr)
	{
		m_archSignalsUpdateTimer = new QTimer;
	}

	m_archSignalsUpdateTimer->start(ARCH_SIGNALS_UPDATE_INTERVAL);
	m_archSignalsTimerStartMs = QDateTime::currentMSecsSinceEpoch();

	m_appSignalStates.clearStatesSavedCounters();
}

void AppDataServiceWorker::onArchSignalsTimer()
{
	m_archSignalsTimerStartMs = QDateTime::currentMSecsSinceEpoch();

	int count = TO_INT(m_appSignalStates.size());

	std::vector<RecordsPerMin> recordsPerMin;

	recordsPerMin.reserve(count);

	RecordsPerMin r;

	for(int i = 0; i < count; i++)
	{
		r.recordsCount = m_appSignalStates[i]->onArchSignalsTimer();
		r.dynamicStateIndex = i;
		r.overrided = m_appSignalStates[i]->apertureOverrided();

		if (r.recordsCount > 1 || r.overrided)
		{
			recordsPerMin.push_back(r);
		}
	}

	std::sort(	recordsPerMin.begin(),
				recordsPerMin.end(),
				[](const RecordsPerMin& a, const RecordsPerMin& b)
				{
					return a.recordsCount > b.recordsCount;
				});

	QMutexLocker loker(&m_recordsPerMinMutex);

	m_recordsPerMin.swap(recordsPerMin);
}

void AppDataServiceWorker::copyArchSignalsInfo(Network::ServiceInfo& serviceInfo) const
{
	int count = 1000;

	std::vector<RecordsPerMin> recordsPerMin;
	double updateStatus = 0;

	getRecordsPerMin(&recordsPerMin, count, &updateStatus);

	count = TO_INT(recordsPerMin.size());

	serviceInfo.set_archsignalsupdateprogress(updateStatus);

	bool archSignalsUpdated = m_updateArchSignalsAnyway;

	m_updateArchSignalsAnyway = false;

	if (m_cachedRecordsPerMin != recordsPerMin)
	{
		archSignalsUpdated = true;
		m_cachedRecordsPerMin = recordsPerMin;
	}

	if ((m_archSignalsRequestCtr % 25) == 0)
	{
		archSignalsUpdated = true;
	}

	m_archSignalsRequestCtr++;

	serviceInfo.set_archsignalsupdated(archSignalsUpdated);

	if (archSignalsUpdated == true)
	{
		for(int i = 0; i < count; i++)
		{
			const DynamicAppSignalState* state = m_appSignalStates[recordsPerMin[i].dynamicStateIndex];

			TEST_PTR_CONTINUE(state);

			Network::ArchSignalInfo* asi = serviceInfo.add_archsignalsinfo();

			asi->set_appsignalid(state->appSignalID().toStdString());
			asi->set_aperturetype(TO_INT(state->apertureType()));
			asi->set_coarseaperture(state->coarseAperture());
			asi->set_fineaperture(state->fineAperture());
			asi->set_abscoarseaperture(state->absCoarseAperture());
			asi->set_absfineaperture(state->absFineAperture());
			asi->set_recordspermin(recordsPerMin[i].recordsCount);
			asi->set_apertureoverrided(state->apertureOverrided());
			asi->set_lowengineeringunits(state->lowEngineeringUnits());
			asi->set_highengineeringunits(state->highEngineeringUnits());
			asi->set_signaltype(TO_INT(state->signalType()));
		}
	}
}
