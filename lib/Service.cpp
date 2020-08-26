#include "../lib/Service.h"
#include "../lib/WUtils.h"

ServiceInfo::ServiceInfo()
{
}

ServiceInfo::ServiceInfo(E::SoftwareType _softwareType, quint16 _port, QString _name, QString _shortName) :
	softwareType(_softwareType),
	port(_port),
	name(_name),
	shortName(_shortName)
{
}

ServicesInfo::ServicesInfo()
{
	const ServiceInfo serviceInfo[] =
	{
		ServiceInfo(E::SoftwareType::BaseService, PORT_BASE_SERVICE, "Base Service", "BaseSrv"),
		ServiceInfo(E::SoftwareType::ConfigurationService, PORT_CONFIGURATION_SERVICE, "Configuration Service", "CfgSrv"),
		ServiceInfo(E::SoftwareType::AppDataService, PORT_APP_DATA_SERVICE, "Application Data Service", "AppDataSrv"),
		ServiceInfo(E::SoftwareType::TuningService, PORT_TUNING_SERVICE, "Tuning Service", "TuningSrv"),
		ServiceInfo(E::SoftwareType::ArchiveService, PORT_ARCHIVING_SERVICE, "Data Archiving Service", "DataArchSrv"),
		ServiceInfo(E::SoftwareType::DiagDataService, PORT_DIAG_DATA_SERVICE, "Diagnostics Data Service", "DiagDataSrv"),
	};

	for(const ServiceInfo& sInfo : serviceInfo)
	{
		insert(sInfo.softwareType, sInfo);
	}
}

// -------------------------------------------------------------------------------------
//
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

const char* const ServiceWorker::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const ServiceWorker::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
const char* const ServiceWorker::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";

int ServiceWorker::m_instanceNo = 0;

ServiceWorker::ServiceWorker(const SoftwareInfo& softwareInfo,
							 const QString& serviceName,
							 int& argc,
							 char** argv,
							 CircularLoggerShared logger) :
	m_softwareInfo(softwareInfo),
	m_argc(argc),
	m_argv(argv),
	m_logger(logger),
	m_initialServiceName(serviceName),
	m_serviceInstanceName(Service::getServiceInstanceName(serviceName, argc, argv)),
	m_settings(QSettings::SystemScope, RADIY_ORG, m_serviceInstanceName, this),
	m_cmdLineParser(argc, argv)
{
	TEST_PTR_RETURN(argv);

	m_instanceNo++;
}

ServiceWorker::~ServiceWorker()
{
}

int& ServiceWorker::argc() const
{
	return m_argc;
}

char** ServiceWorker::argv() const
{
	return m_argv;
}

QString ServiceWorker::appPath() const
{
	if (m_argv == nullptr ||
		m_argc == 0)
	{
		assert(false);
		return QString();
	}

	return QString(m_argv[0]);
}

QString ServiceWorker::cmdLine() const
{
	if (m_argv == nullptr ||
		m_argc == 0)
	{
		assert(false);
		return QString();
	}

	QString cl;

	for(int i = 0; i < m_argc; i++)
	{
		if (m_argv[i] == nullptr)
		{
			assert(false);
			continue;
		}

		cl += QString("%1 ").arg(m_argv[i]);
	}

	return cl.trimmed();
}

QString ServiceWorker::initialServiceName() const
{
	return m_initialServiceName;
}

QString ServiceWorker::serviceInstanceName() const
{
	return m_serviceInstanceName;
}

const SoftwareInfo& ServiceWorker::softwareInfo() const
{
	return m_softwareInfo;
}

E::SoftwareType ServiceWorker::softwareType() const
{
	return m_softwareInfo.softwareType();
}

void ServiceWorker::initAndProcessCmdLineSettings()
{
	if (m_instanceNo > 1)
	{
		assert(false);			// call initAndProcessCmdLineSettings() for first ServiceWorker instance only!
		return;
	}

	init();

	processCmdLineSettings();
}

void ServiceWorker::setService(Service* service)
{
	m_service = service;
}

Service* ServiceWorker::service()
{
	assert(m_service != nullptr);
	return m_service;
}

CommandLineParser& ServiceWorker::cmdLineParser()
{
	return m_cmdLineParser;
}

void ServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	Q_UNUSED(serviceInfo);
}

bool ServiceWorker::clearSettings()
{
	m_settings.clear();

	m_settings.sync();

	return CommandLineParser::checkSettingWriteStatus(m_settings, "", nullptr);
}

QString ServiceWorker::getStrSetting(const QString& settingName)
{
	QString cmdLineValue = m_cmdLineParser.settingValue(settingName);

	if (cmdLineValue.isEmpty() == true)
	{
		return m_settings.value(settingName).toString();
	}

	return cmdLineValue;
}

QString ServiceWorker::getCmdLineSetting(const QString& settingName)
{
	return  m_cmdLineParser.settingValue(settingName);
}

void ServiceWorker::init()
{
	m_cmdLineParser.addSimpleOption("h", "Print this help.");
	m_cmdLineParser.addSimpleOption("v", "Display version of service.");
	m_cmdLineParser.addSimpleOption("e", "Run service as a regular application.");
	m_cmdLineParser.addSimpleOption("i", "Install the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("u", "Uninstall the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("t", "Terminate (stop) the service.");
	m_cmdLineParser.addSimpleOption("inst", "Set service instance ID.");
	m_cmdLineParser.addSimpleOption("clr", "Clear all service settings.");

	initCmdLineParser();

	m_cmdLineParser.parse();
}

void ServiceWorker::processCmdLineSettings()
{
	m_cmdLineParser.processSettings(m_settings, m_logger);
}

void ServiceWorker::onThreadStarted()
{
	// loading common settings of services

	m_equipmentID = getStrSetting(SETTING_EQUIPMENT_ID);

	m_softwareInfo.setEquipmentID(m_equipmentID);		// !

	m_cfgServiceIP1Str = getStrSetting(SETTING_CFG_SERVICE_IP1);

	m_cfgServiceIP1.setAddressPortStr(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	m_cfgServiceIP2Str = getStrSetting(SETTING_CFG_SERVICE_IP2);

	m_cfgServiceIP2.setAddressPortStr(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	//

	loadSettings();

	initialize();

	emit work();
}

void ServiceWorker::onThreadFinished()
{
	shutdown();
	emit stopped();
}

// -------------------------------------------------------------------------------------
//
// Service class implementation
//
// -------------------------------------------------------------------------------------

Service::Service(ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger):
	m_logger(logger),
	m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
	m_serviceWorkerFactory(serviceWorker),
	m_timer500ms(this)
{
}

Service::~Service()
{
}

void Service::start()
{
	startServiceWorkerThread();

	startBaseRequestSocketThread();
}

void Service::stop()
{
	stopBaseRequestSocketThread();

	stopServiceWorkerThread();
}

QString Service::getInstanceID(int argc, char* argv[])
{
	for(int i = 0; i < argc; i++)
	{
		TEST_PTR_CONTINUE(argv[i]);

		QString arg(argv[i]);

		if (arg.trimmed().toLower().startsWith("-inst") == false)
		{
			continue;
		}

		// parse: -inst=InstanceID

		QStringList vl = arg.split("=");

		if (vl.count() != 2)
		{
			continue;
		}

		return vl[1].trimmed();
	}

	return QString();
}

QString Service::getServiceInstanceName(const QString& serviceName, int argc, char* argv[])
{
	QString instanceID = Service::getInstanceID(argc, argv);

	if (instanceID.isEmpty() == true)
	{
		return serviceName;
	}

	return QString("%1 (%2)").arg(serviceName).arg(instanceID);
}

void Service::onServiceWork()
{
	m_state = ServiceState::Work;
}

void Service::onServiceStopped()
{
	m_state = ServiceState::Stopped;
}

void Service::onBaseRequest(UdpRequest request)
{
	UdpRequest ack;

	ack.initAck(request);

	HostAddressPort ha(request.address().toIPv4Address(), request.port());

	switch(request.ID())
	{
		case RQID_SERVICE_GET_INFO:
		{
			Network::ServiceInfo si;
			getServiceInfo(si);
			ack.writeData(si);
			break;
		}

		case RQID_SERVICE_START:
			LOG_MSG(m_logger, QString("Service START request from SCM (%1).").arg(ha.addressStr()));
			startServiceWorkerThread();
			break;

		case RQID_SERVICE_STOP:
			LOG_MSG(m_logger, QString("Service STOP request from SCM (%1).").arg(ha.addressStr()));
			stopServiceWorkerThread();
			break;

		case RQID_SERVICE_RESTART:
			LOG_MSG(m_logger, QString("Service RESTART request from SCM (%1).").arg(ha.addressStr()));
			stopServiceWorkerThread();
			startServiceWorkerThread();
			break;

		default:
			assert(false);
			ack.setErrorCode(RQERROR_UNKNOWN_REQUEST);
			break;
	}

	emit ackBaseRequest(ack);
}

void Service::startServiceWorkerThread()
{
	QMutexLocker locker(&m_mutex);

	if (m_serviceWorkerThread != nullptr)
	{
		assert(false);
		return;
	}

	if (m_state != ServiceState::Stopped)
	{
		assert(false);
		return;
	}

	m_serviceWorkerStartTime = QDateTime::currentMSecsSinceEpoch();

	m_state = ServiceState::Starts;

	m_serviceWorker = m_serviceWorkerFactory.createInstance();

	m_serviceWorker->setService(this);

	connect(m_serviceWorker, &ServiceWorker::work, this, &Service::onServiceWork);
	connect(m_serviceWorker, &ServiceWorker::stopped, this, &Service::onServiceStopped);

	m_serviceWorkerThread = new SimpleThread(m_serviceWorker);
	m_serviceWorkerThread->start();
}

void Service::stopServiceWorkerThread()
{
	QMutexLocker locker(&m_mutex);

	if (m_serviceWorkerThread == nullptr)
	{
		return;
	}

	m_state = ServiceState::Stops;

	m_serviceWorkerThread->quit();
	m_serviceWorkerThread->wait();

	delete m_serviceWorkerThread;

	m_serviceWorkerThread = nullptr;
	m_serviceWorker = nullptr;

	m_state = ServiceState::Stopped;
}

void Service::startBaseRequestSocketThread()
{
	ServiceInfo sInfo = servicesInfo.value(m_serviceWorkerFactory.softwareType());

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::AnyIPv4, sInfo.port, m_logger);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &Service::onBaseRequest);
	connect(this, &Service::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	m_baseRequestSocketThread = new UdpSocketThread(serverSocket);

	m_baseRequestSocketThread->start();
}

void Service::stopBaseRequestSocketThread()
{
	m_baseRequestSocketThread->quitAndWait();

	delete m_baseRequestSocketThread;
}

void Service::getServiceInfo(Network::ServiceInfo& serviceInfo)
{
	ServiceWorker* serviceWorker = m_serviceWorker;
	if (serviceWorker == nullptr)
	{
		serviceWorker = &m_serviceWorkerFactory;
	}

	QMutexLocker locker(&m_mutex);

	Network::SoftwareInfo* si = new Network::SoftwareInfo();

	serviceWorker->softwareInfo().serializeTo(si);

	serviceInfo.set_allocated_softwareinfo(si);
	serviceInfo.set_uptime((QDateTime::currentMSecsSinceEpoch() - m_serviceStartTime) / 1000);

	serviceInfo.set_servicestate(TO_INT(m_state));

	if (m_serviceWorker != nullptr)
	{
		m_serviceWorker->getServiceSpecificInfo(serviceInfo);
	}

	if (m_state != ServiceState::Stopped)
	{
		serviceInfo.set_serviceuptime((QDateTime::currentMSecsSinceEpoch() - m_serviceWorkerStartTime) / 1000);
	}
	else
	{
		serviceInfo.set_serviceuptime(0);
	}
}

// -------------------------------------------------------------------------------------
//
// DaemonServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

DaemonServiceStarter::DaemonServiceStarter(QCoreApplication& app, ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger) :
	QtService(serviceWorker.argc(), serviceWorker.argv(), &app, serviceWorker.serviceInstanceName(), logger),
	m_app(app),
	m_serviceWorker(serviceWorker),
	m_logger(logger)
{
}

DaemonServiceStarter::~DaemonServiceStarter()
{
	stopAndDeleteService();
}

int DaemonServiceStarter::exec()
{
	setServiceFlags(QtServiceBase::ServiceFlag::NeedsStopOnShutdown);

	int result = QtService::exec();

	return result;
}

void DaemonServiceStarter::start()
{
	LOG_CALL(m_logger);

	m_service = new Service(m_serviceWorker, m_logger);

	m_service->start();
}

void DaemonServiceStarter::stop()
{
	stopAndDeleteService();

	LOG_CALL(m_logger);
}

void DaemonServiceStarter::stopAndDeleteService()
{
	if (m_service == nullptr)
	{
		return;
	}

	m_service->stop();

	delete m_service;
	m_service = nullptr;
}

// -------------------------------------------------------------------------------------
//
// ServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

ServiceStarter::ServiceStarter(QCoreApplication& app, ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger) :
	m_app(app),
	m_serviceWorker(serviceWorker),
	m_logger(logger)
{
	app.setOrganizationName(RADIY_ORG);
	app.setApplicationName(serviceWorker.serviceInstanceName());
}

int ServiceStarter::exec()
{
	LOG_MSG(m_logger, QString("Run: %1").arg(m_serviceWorker.cmdLine()));

	int result = privateRun();

	LOG_MSG(m_logger, QString("Exit: %1, result = %2").arg(m_serviceWorker.appPath()).arg(result));

	QThread::msleep(500);			// not delete! wait while logger flush buffers

	return result;
}

int ServiceStarter::privateRun()
{
	m_serviceWorker.initAndProcessCmdLineSettings();			// 1. init CommanLineParser
																// 2. process cmd line args
																// 3. update and store service settings
	bool pauseAndExit = false;
	bool startAsRegularApp = false;

	processCmdLineArguments(pauseAndExit, startAsRegularApp);

	if (pauseAndExit == true)
	{
		return 0;
	}

	int result = 0;

	if (startAsRegularApp == true)
	{
		if (m_serviceWorker.getStrSetting(ServiceWorker::SETTING_EQUIPMENT_ID).isEmpty() == true)
		{
			DEBUG_LOG_MSG(m_logger, "");
			DEBUG_LOG_ERR(m_logger, QString(tr("EquipmentID of service has NOT SET !!!")));
			return 7;
		}

		result = runAsRegularApplication();
	}
	else
	{
		DaemonServiceStarter daemonStarter(m_app, m_serviceWorker, m_logger);

		result = daemonStarter.exec();
	}

	return result;
}

// returns 'true' for application exit
//
void ServiceStarter::processCmdLineArguments(bool& pauseAndExit, bool& startAsRegularApp)
{
	pauseAndExit = false;
	startAsRegularApp = false;

	const CommandLineParser& cmdLineParser = m_serviceWorker.cmdLineParser();

	// print Help and exit if "-h" is set
	//
	if (cmdLineParser.optionIsSet("h") == true)
	{
		QString helpText = cmdLineParser.helpText();

		helpText += QString(tr("Run program without options to start service.\n"));

		std::cout << C_STR(helpText);

		LOG_MSG(m_logger, QString(tr("Help printed.")))

		pauseAndExit = true;
		return;
	}

	// print Version and exit if "-v" is set
	//
	if (cmdLineParser.optionIsSet("v") == true)
	{
		const SoftwareInfo& si = m_serviceWorker.softwareInfo();

		QString versionInfo =
			QString("\nApplication:\t%1\nVersion:\t%2.%3.%4 (%5)\nCommit SHA:\t%6\n").
				arg(m_serviceWorker.serviceInstanceName()).
				arg(si.majorVersion()).
				arg(si.minorVersion()).
				arg(si.commitNo()).
				arg(si.buildBranch()).
				arg(si.commitSHA());

		std::cout << C_STR(versionInfo);

		LOG_MSG(m_logger, QString(tr("Version printed.")))

		pauseAndExit = true;
		return;
	}

	// clear settings and exit if "-clr" is set
	//
	if (cmdLineParser.optionIsSet("clr") == true)
	{
		bool res = m_serviceWorker.clearSettings();

		if (res == true)
		{
			DEBUG_LOG_MSG(m_logger, QString(tr("\nService settings has been cleaned.\n\n")));
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("\nService settings cleaning error. Administrative rights required.\n\n")));
		}

		pauseAndExit = true;
		return;
	}

	// run service as a regular application if "-e" is set
	//
	if (cmdLineParser.optionIsSet("e") == true)
	{
		startAsRegularApp = true;
		return;
	}
}

int ServiceStarter::runAsRegularApplication()
{
	KeyReaderThread* keyReaderThread = new KeyReaderThread();

	keyReaderThread->start();

		// run service
	//
	Service* service = new Service(m_serviceWorker, m_logger);

	service->start();

	int result = m_app.exec();

	service->stop();
	delete service;

	keyReaderThread->quit();
	keyReaderThread->wait();
	delete keyReaderThread;

	return result;
}

void ServiceStarter::KeyReaderThread::run()
{
	char c = 0;
	std::cin >> c;
	QCoreApplication::exit(0);
}
