#include <QXmlStreamReader>
#include <QMetaProperty>

#include "../OnlineLib/CfgServerLoader.h"
#include "GatewayDescriptionParser.h"

#include "GatewayService.h"

// -------------------------------------------------------------------------------
//
// GatewayServiceWorker class implementation
//
// -------------------------------------------------------------------------------

GatewayServiceWorker::GatewayServiceWorker(const SoftwareInfo& softwareInfo,
										   const QString& serviceName,
										   int& argc,
										   char** argv,
										   CircularLoggerShared logger,
										   E::ServiceRunMode runMode) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger, runMode),
	m_timer(this)
{
}

GatewayServiceWorker::~GatewayServiceWorker()
{
}

ServiceWorker* GatewayServiceWorker::createInstance() const
{
	GatewayServiceWorker* newInstance = new GatewayServiceWorker(softwareInfo(),
																 serviceName(),
																 argc(), argv(),
																 logger(),
																 serviceRunMode());
	newInstance->init();

	return newInstance;
}

void GatewayServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::AppDataService, m_curSettingsProfile);

	serviceInfo.set_settingsxml(xmlString.toStdString());
}

bool GatewayServiceWorker::isConnectedToConfigurationService(quint32& ip, quint16& port) const
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

void GatewayServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption(CmdLineOption::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption(CmdLineOption::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1, "IP address of first Configuration Service.", "IPv4:Port");
	cp.addSingleValueOption(CmdLineOption::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2, "IP address of second Configuration Service.", "IPv4:Port");

	cp.addSimpleOption(CmdLineOption::CFG_PARSE, "Parse gateway description file.");

	cp.addSingleValueOption(CmdLineOption::CFG_FILE,
							SoftwareSetting::GATEWAY_DESCRIPTION_FILE, "Gateway description file name.", "file");
}

void GatewayServiceWorker::loadSettings()
{
/*	m_appDataProcessingThreadCount = QString(getStrSetting(SoftwareSetting::PROCESSING_THREADS_COUNT)).toInt();

	m_strCmdLineAppDataReceivingIP = getStrSetting(SoftwareSetting::OVERRIDE_APP_DATA_RECEIVING_IP);
	m_cmdLineAppDataReceivingIP.setAddressPortStr(m_strCmdLineAppDataReceivingIP, PORT_APP_DATA_SERVICE_DATA);

	m_logRupTimeErrors = cmdLineParser().optionIsSet(CmdLineOption::LOG_RUP_TIME_ERR);*/

	DEBUG_LOG_MSG(logger(), QString(tr("Settings from command line or registry:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));
}

bool GatewayServiceWorker::processCustomCmdLineSettings()
{
	const CommandLineParser& clp = cmdLineParser();

	if (clp.optionIsSet(CmdLineOption::CFG_PARSE) == false)
	{
		return true;
	}

	if (clp.optionIsSet(CmdLineOption::CFG_FILE) == false)
	{
		DEBUG_LOG_ERR(logger(), "To parse gateway description file cmd line option -f=fileName should be set!");
		return false;
	}

	QString fileName = clp.optionValue(CmdLineOption::CFG_FILE);

	QFile file(fileName);

	if (file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text) == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Can't open file %1!").arg(fileName));
		return false;
	}

	parseGatewayDescription(fileName, file.readAll());

	return false;
}

void GatewayServiceWorker::initialize()
{
	DEBUG_LOG_MSG(logger(), "GatewayServiceWorker is started");

	runCfgLoaderThread();
	runTimer();
}

void GatewayServiceWorker::shutdown()
{
	clearConfiguration();

	stopTimer();

	stopCfgLoaderThread();

	DEBUG_LOG_MSG(logger(), "GatewayServiceWorker finished");
}

void GatewayServiceWorker::runCfgLoaderThread()
{
	assert(m_cfgLoaderThread == nullptr);			// once should be runned

	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &GatewayServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();

	m_cfgLoaderThread->enableDownloadConfiguration();
}

void GatewayServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	m_cfgLoaderThread->quitAndWait();

	delete m_cfgLoaderThread;

	m_cfgLoaderThread = nullptr;
}

void GatewayServiceWorker::onConfigurationReady(const QByteArray configurationXmlData,
												const BuildFileInfoArray buildFileInfoArray,
												SessionParams sessionParams,
												std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
{
	Q_UNUSED(configurationXmlData);

	setSessionParams(sessionParams);

	DEBUG_LOG_MSG(logger(), "Configuration is ready");

	DEBUG_LOG_MSG(logger(), "");

	DEBUG_LOG_MSG(logger(), "Settings profile: " + currentSettingsProfile->profile);

	DEBUG_LOG_MSG(logger(), "");

	// stop all threads and free all allocated resources
	//
	clearConfiguration();

	const GatewayServiceSettings* typedSettingsPtr = dynamic_cast<const GatewayServiceSettings*>(currentSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		DEBUG_LOG_MSG(logger(), "Settings casting error!");
		return;
	}

	// making modificable local copy of settings
	//
	m_curSettingsProfile = *typedSettingsPtr;

	bool result = true;
	QString errStr;

	//

	QByteArray gatewayDescriptionFileData;

	m_cfgLoaderThread->getFileBlockedByID(CfgFileId::GATEWAY_DESCRIPTION,
										  &gatewayDescriptionFileData,
										  &errStr);
	result &= errStr.isEmpty();

	// reads gateway description and fills m_acquiredSignals
	//
	result &= readGatewayDescription(gatewayDescriptionFileData);

	if (result != false)
	{
		m_acquiredSignals.clear();
		m_gateways.fillAcquiredSignalsSet(&m_acquiredSignals);
	}

	//

	QByteArray appSignalSetFileData;

	m_cfgLoaderThread->getFileBlockedByID(CfgFileId::APP_SIGNAL_SET,
										  &appSignalSetFileData,
										  &errStr);
	result &= errStr.isEmpty();
	result &= readAppSignals(appSignalSetFileData);

	//

	if (result == true)
	{
		applyNewConfiguration();
	}
}

bool GatewayServiceWorker::readAppSignals(const QByteArray& fileData)
{
	::Proto::AppSignalSet signalSet;

	bool result = signalSet.ParseFromArray(fileData.constData(), static_cast<int>(fileData.size()));

	if (result == false)
	{
		DEBUG_LOG_ERR(logger(), "AppSignalSet file reading - ERROR");
		return false;
	}

	DEBUG_LOG_MSG(logger(), "AppSignalSet file reading - OK");

	int signalCount = signalSet.appsignal_size();
	int aquiredSignalsCount = 0;

	m_appSignals.clear();

	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& appSignal = signalSet.appsignal(i);

		const QString&& appSignalID = QString::fromStdString(appSignal.appsignalid());

		if (m_acquiredSignals.contains(calcHash(appSignalID)) == true)
		{
			m_appSignals.insert(appSignal);
			aquiredSignalsCount++;
		}
	}

	DEBUG_LOG_MSG(logger(), QString("All gatreways acquire %1 signal(s)").arg(aquiredSignalsCount));

	return true;
}

bool GatewayServiceWorker::readGatewayDescription(const QByteArray& fileData)
{
	XmlReadHelper xml(fileData);

	bool result = m_gateways.readFromXml(xml);

	return result;
}

void GatewayServiceWorker::createAndInitSignalStates()
{/*
//	m_appSignalStates.clear();

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

	//m_appSignalStates.setSize(signalCount);

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

	m_appSignalStates.setAutoArchivingGroups(m_autoArchivingGroupsCount);*/
}

void GatewayServiceWorker::applyNewConfiguration()
{
	// already filled:
	//
	//		m_appSignals
	//		m_gateways

	bool result = true;

	result &= m_handlers.init(m_gateways, m_appSignals);

	if (result == false)
	{
		clearConfiguration();
		return;
	}

	m_handlers.run();

}

void GatewayServiceWorker::clearConfiguration()
{
	m_handlers.shutdown();
	m_handlers.clear();
	m_appSignals.clear();
	m_gateways.clear();
}

void GatewayServiceWorker::runTimer()
{
	connect(&m_timer, &QTimer::timeout, this, &GatewayServiceWorker::onTimer);

	m_timer.setInterval(1000);
	m_timer.start();
}

void GatewayServiceWorker::stopTimer()
{
	m_timer.stop();
}

void GatewayServiceWorker::onTimer()
{
}

void GatewayServiceWorker::parseGatewayDescription(const QString& filePathName, const QString& gwDesc)
{
	DEBUG_LOG_MSG(logger(), "");
	DEBUG_LOG_MSG(logger(), QString("Parsing gateway description file: %1").arg(filePathName));

	AppSignals appSignals;

	Gateway::Parser gdp(appSignals);

	gdp.parse(gwDesc);

	int errCount = 0;
	int wrnCount = 0;

	const Gateway::ParserLog& parserLog = gdp.log();

	for(const auto& r : parserLog)
	{
		switch(r.msgType)
		{
		case Gateway::LogMsgType::Message:
			{
				QString msg = r.msg;
				msg = msg.mid(0, 1).toUpper() + msg.mid(1);
				DEBUG_LOG_MSG(logger(), msg);
			}
			break;

		case Gateway::LogMsgType::Warning:
			DEBUG_LOG_WRN(logger(), "Warning: " + r.msg);
			wrnCount++;
			break;

		case Gateway::LogMsgType::Error:
			DEBUG_LOG_ERR(logger(), "Error: " + r.msg);
			errCount++;
			break;

		default:
			Q_ASSERT(false);
		}
	}

	DEBUG_LOG_MSG(logger(), QString("Parsing finished with %1 errors, %2 warnings")
										.arg(errCount).arg(wrnCount));
	DEBUG_LOG_MSG(logger(), "");
}



