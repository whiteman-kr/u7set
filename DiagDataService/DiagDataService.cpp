#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../HardwareLib/DeviceObject.h"
#include "DiagDataService.h"

// -------------------------------------------------------------------------------
//
// DiagDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

DiagDataServiceWorker::DiagDataServiceWorker(const SoftwareInfo& softwareInfo,
											 const QString& serviceName,
											 int argc,
											 char** argv,
											 CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}

DiagDataServiceWorker::DiagDataServiceWorker(const DiagDataServiceWorker* worker) :
	ServiceWorker(worker)
{
}

DiagDataServiceWorker::~DiagDataServiceWorker()
{
}

ServiceWorker* DiagDataServiceWorker::createInstance() const
{
	DiagDataServiceWorker* diagDataServiceWorker = new DiagDataServiceWorker(this);
	return diagDataServiceWorker;
}

void DiagDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::DiagDataService, m_curSettingsProfile);

	serviceInfo.set_settingsxml(xmlString.toStdString());
}

void DiagDataServiceWorker::initServiceSpecificCmdLineArgs()
{
	addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	addValueCmdLineArg(CmdLineArg::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
	addValueCmdLineArg(CmdLineArg::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
	addValueCmdLineArg(CmdLineArg::PTC, SoftwareSetting::PROCESSING_THREADS_COUNT, "Diag data processing threads count", "N");
	addValueCmdLineArg(CmdLineArg::RECVIP, SoftwareSetting::OVERRIDE_DIAG_DATA_RECEIVING_IP, "Override DiagDataReceivingIP", "IPv4:Port");
}

void DiagDataServiceWorker::loadServiceSpecificSettings()
{
	m_diagDataProcessingThreadCount = getSettingValue(SoftwareSetting::PROCESSING_THREADS_COUNT).toInt();

	m_strCmdLineDiagDataReceivingIP = getSettingValue(SoftwareSetting::OVERRIDE_DIAG_DATA_RECEIVING_IP);
	m_cmdLineDiagDataReceivingIP.setAddressPortStr(m_strCmdLineDiagDataReceivingIP, PORT_DIAG_DATA_SERVICE_DATA);

	DEBUG_LOG_MSG(logger(), "");
	DEBUG_LOG_MSG(logger(), QString(tr("Service settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::PROCESSING_THREADS_COUNT).arg(m_diagDataProcessingThreadCount));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::OVERRIDE_DIAG_DATA_RECEIVING_IP).arg(m_cmdLineDiagDataReceivingIP.addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), "");
}

void DiagDataServiceWorker::initialize()
{
	DEBUG_LOG_MSG(logger(), "DiagDataServiceWorker is started");

	runCfgLoaderThread();
}

void DiagDataServiceWorker::shutdown()
{
	clearConfiguration();

	stopTcpDiagDataServer();
	stopCfgLoaderThread();

	DEBUG_LOG_MSG(logger(), "DiagDataServiceWorker finished");
}

void DiagDataServiceWorker::runCfgLoaderThread()
{
	assert(m_cfgLoaderThread == nullptr);			// once should be runned

	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &DiagDataServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();

	m_cfgLoaderThread->enableDownloadConfiguration();
}

void DiagDataServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	m_cfgLoaderThread->quitAndWait();

	delete m_cfgLoaderThread;

	m_cfgLoaderThread = nullptr;
}

void DiagDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData,
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

	const DiagDataServiceSettings* typedSettingsPtr = dynamic_cast<const DiagDataServiceSettings*>(currentSettingsProfile.get());

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
	if (m_strCmdLineDiagDataReceivingIP.isEmpty() == false)
	{
		m_curSettingsProfile.diagDataReceivingIP = m_cmdLineDiagDataReceivingIP;
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

		if (bfi.ID == CfgFileId::DIAG_DATA_SOURCES)
		{
			result &= readDiagDataSources(fileData, sessionParams.currentSettingsProfile);
		}

		if (bfi.ID == CfgFileId::ACQUIRED_DIAG_SIGNALS)
		{
			result &= readDiagSignalsAndObjects(fileData);
		}

		if (bfi.ID == CfgFileId::DIAG_SIGNAL_TYPES)
		{
			result &= readDiagSignalTypes(fileData);			// fills m_diagSignalTypes
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

bool DiagDataServiceWorker::readDiagDataSources(const QByteArray& fileData, const QString& profile)
{
	Q_UNUSED(profile);

	m_dataSources.clear();

	bool result = DataSourcesXML<DataSource>::readFromXml(fileData, &m_dataSources);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error reading DiagDataSources from XML-file"));
		return false;
	}

	if (result == true)
	{
		DEBUG_LOG_MSG(logger(), QString("DiagDataSources successfully loaded"));
	}
	else
	{
		DEBUG_LOG_ERR(logger(), QString("DiagDataSources loading error!"));
	}

	return result;
}

bool DiagDataServiceWorker::readDiagSignalsAndObjects(const QByteArray& fileData)
{
	m_diagSignalsAndObjects.Clear();

	bool result = m_diagSignalsAndObjects.ParseFromArray(fileData.constData(), static_cast<int>(fileData.size()));

	if (result == false)
	{
		return false;
	}

	return true;
}

bool DiagDataServiceWorker::readDiagSignalTypes(const QByteArray& fileData)
{
	m_diagSignalTypes.clear();

	Hardware::DiagSignalTypes diagSignalTypes;

	XmlReadHelper xml(fileData);

	bool res = diagSignalTypes.readFromXml(xml);

	RETURN_IF_FALSE(res);

	const std::vector<Hardware::DiagSignalType>& dTypes = *diagSignalTypes.mutableDiagSignalTypes();

	for(const Hardware::DiagSignalType& dt : dTypes)
	{
		m_diagSignalTypes.emplace(calcHash(dt.signalTypeId), dt);
	}

	return res;
}

void DiagDataServiceWorker::applyNewConfiguration()
{
	if (m_onlineSources != nullptr)
	{
		Q_ASSERT(false);
	}

	m_onlineSources = new OnlineDataSources<DiagDataSource, SimpleDiagSignalState>(
								m_dataSources,
								m_curSettingsProfile.diagDataReceivingIP,
								sessionParams().softwareRunMode,
								m_diagDataProcessingThreadCount, logger());

	int dataSourceCount = m_onlineSources->count();

	bool initRes = true;

	for(int i = 0; i < dataSourceCount; i++)
	{
		DiagDataSource* dds = m_onlineSources->getDataSource(i);

		TEST_PTR_CONTINUE(dds);

		dds->init(m_diagSignalTypes,
				  m_diagSignalsAndObjects);
	}

	if ((m_onlineSources->isWorkable() && initRes)== false)
	{
		DEBUG_LOG_ERR(logger(), "OnlineDataSources initialization ERROR!");
		return;
	}

	m_onlineSources->run();

//	createTimeErrLog();
//	createAndInitSignalStates();
//	buildAcuiredAppSignalIDs();
//	prepareAppDataSources();

	runDiagDataReceiverThread();
//	runTcpArchiveClientThread();
	runTcpDiagDataServer();
//	runRtTrendsServerThread();
}

void DiagDataServiceWorker::clearConfiguration()
{

	// free all resources allocated in onConfigurationReady
	//
//	stopRtTrendsServerThread();
//	stopTcpArchiveClientThread();
	stopTcpDiagDataServer();
	stopDiagDataReceiverThread();

	if (m_onlineSources != nullptr)
	{
		m_onlineSources->stop();
		delete m_onlineSources;
		m_onlineSources = nullptr;
	}

//	shutdownTimeErrLog();

//	m_appSignals.clear();
//	m_appDataSources.clear();
//	m_appSignalStates.clear();
//	m_acquiredAppSignalIDs.clear();
}

void DiagDataServiceWorker::runDiagDataReceiverThread()
{
/*	if (m_diagDataReceiver != nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_diagDataReceiver = new DiagDataReceiver(m_curSettingsProfile.diagDataReceivingIP,
											m_diagDataSources,
											m_diagSignalStates,
											m_diagDataProcessingThreadCount,
											sessionParams().softwareRunMode,
											logger());
	m_diagDataReceiver->start();*/
}

void DiagDataServiceWorker::stopDiagDataReceiverThread()
{
/*	if (m_diagDataReceiver != nullptr)
	{
		m_diagDataReceiver->quitAndWait();
		delete m_diagDataReceiver;
		m_diagDataReceiver = nullptr;
	}*/
}

void DiagDataServiceWorker::runTcpDiagDataServer()
{
//	assert(m_tcpAppDataServerThread == nullptr);

//	m_tcpAppDataServerThread = new TcpAppDataServerThread(	softwareInfo(),
//															m_curSettingsProfile.clientRequestIP,
//															m_curSettingsProfile.securityLevel,
//															*this);
//	m_tcpAppDataServerThread->start();
}

void DiagDataServiceWorker::stopTcpDiagDataServer()
{
//	if (m_tcpAppDataServerThread != nullptr)
//	{
//		m_tcpAppDataServerThread->quitAndWait(10000);
//		delete m_tcpAppDataServerThread;

//		m_tcpAppDataServerThread = nullptr;
//	}
}





