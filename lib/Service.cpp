#include "../lib/Service.h"
#include "../lib/WUtils.h"

// -------------------------------------------------------------------------------------
//
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

int ServiceWorker::m_instanceNo = 0;

ServiceWorker::ServiceWorker(ServiceType serviceType,
							 const QString& serviceName,
							 int& argc,
							 char** argv,
							 const VersionInfo& versionInfo,
							 std::shared_ptr<CircularLogger> logger) :
	m_serviceType(serviceType),
	m_serviceName(serviceName),
	m_argc(argc),
	m_argv(argv),
	m_versionInfo(versionInfo),
	m_logger(logger),
	m_cmdLineParser(argc, argv),
	m_settings(QSettings::SystemScope, RADIY_ORG, serviceName, this)
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


ServiceType ServiceWorker::serviceType() const
{
	return m_serviceType;
}


QString ServiceWorker::serviceName() const
{
	return m_serviceName;
}


const VersionInfo& ServiceWorker::versionInfo() const
{
	return m_versionInfo;
}


void ServiceWorker::init()
{
	if (m_instanceNo > 1)
	{
		assert(false);			// call init() for first ServiceWorker instance only!
		return;
	}

	m_cmdLineParser.addSimpleOption("h", "Print this help.");
	m_cmdLineParser.addSimpleOption("v", "Display version of service.");
	m_cmdLineParser.addSimpleOption("e", "Run service as a regular application.");
	m_cmdLineParser.addSimpleOption("i", "Install the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("u", "Uninstall the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("t", "Terminate (stop) the service.");
	m_cmdLineParser.addSimpleOption("clr", "Clear all service settings.");

	initCmdLineParser();

	m_cmdLineParser.parse();

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


CommandLineParser &ServiceWorker::cmdLineParser()
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

	return checkSettingWriteStatus("");
}


bool ServiceWorker::setStrSetting(const QString& settingName, const QString& value)
{
	m_settings.setValue(settingName, QVariant(value));

	m_settings.sync();

	return checkSettingWriteStatus(settingName);
}


QString ServiceWorker::getStrSetting(const QString& settingName)
{
	return m_settings.value(settingName).toString();
}


bool ServiceWorker::checkSettingWriteStatus(const QString& settingName)
{
	QSettings::Status s = m_settings.status();

	bool result = false;

	switch(s)
	{
	case QSettings::Status::AccessError:
		if (settingName.isEmpty() == true)
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("Settings write error: QSettings::Status::AccessError.")))
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("Setting '%1' write error: QSettings::Status::AccessError.")).arg(settingName))
		}
		break;

	case QSettings::Status::FormatError:
		if (settingName.isEmpty() == true)
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("Settings write error: QSettings::Status::FormatError.")))
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("Setting '%1' write error: QSettings::Status::FormatError.")).arg(settingName))
		}
		break;

	case QSettings::Status::NoError:
		result = true;
		break;

	default:
		assert(false);
	}

	return result;
}


void ServiceWorker::onThreadStarted()
{
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
	m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
	m_serviceWorkerFactory(serviceWorker),
	m_logger(logger),
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
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::AnyIPv4, serviceInfo[TO_INT(m_serviceWorkerFactory.serviceType())].port, m_logger);

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

	const VersionInfo& vi = serviceWorker->versionInfo();

	serviceInfo.set_type(TO_INT(serviceWorker->serviceType()));

	serviceInfo.set_majorversion(vi.majorVersion);
	serviceInfo.set_minorversion(vi.minorVersion);
	serviceInfo.set_commitno(vi.commitNo);
	serviceInfo.set_buildbranch(C_STR(vi.buildBranch));
	serviceInfo.set_commitsha(C_STR(vi.commitSHA));

	serviceInfo.set_crc(m_crc);
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
	QtService(serviceWorker.argc(), serviceWorker.argv(), &app, serviceWorker.serviceName(), logger),
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
	app.setApplicationName(serviceWorker.serviceName());
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
	m_serviceWorker.init();			// 1. init CommanLineParser
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
		const VersionInfo& vi = m_serviceWorker.versionInfo();

		QString versionInfo =
			QString("\nApplication:\t%1\nVersion:\t%2.%3.%4 (%5)\nCommit SHA:\t%6\n").
				arg(m_serviceWorker.serviceName()).
				arg(vi.majorVersion).
				arg(vi.minorVersion).
				arg(vi.commitNo).
				arg(vi.buildBranch).
				arg(vi.commitSHA);

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
			DEBUG_LOG_MSG(m_logger, QString(tr("\nService settings has been cleared.\n\n")));
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("\nError cleaning of service settings. Adminirative rights rquired.\n\n")));
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


