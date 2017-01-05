#include "../lib/Service.h"
#include "../lib/WUtils.h"

#include "version.h"

// -------------------------------------------------------------------------------------
//
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

ServiceWorker::ServiceWorker(ServiceType serviceType, const QString& serviceName, int& argc, char** argv,
							 int majorVersion, int minorVersion) :
	m_serviceType(serviceType),
	m_serviceName(serviceName),
	m_argc(argc),
	m_argv(argv),
	m_majorVersion(majorVersion),
	m_minorVersion(minorVersion),
	m_cmdLineParser(argc, argv),
	m_settings("Radiy", serviceName)

{
	TEST_PTR_RETURN(argv);
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


ServiceType ServiceWorker::serviceType() const
{
	return m_serviceType;
}


QString ServiceWorker::serviceName() const
{
	return m_serviceName;
}


int ServiceWorker::majorVersion() const
{
	return m_majorVersion;
}


int ServiceWorker::minorVersion() const
{
	return m_minorVersion;
}


int ServiceWorker::commitNo() const
{
	return USED_SERVER_COMMIT_NUMBER;
}


QString ServiceWorker::buildBranch() const
{
	return QString(BUILD_BRANCH);
}


QString ServiceWorker::commitSHA() const
{
	return QString(USED_SERVER_COMMIT_SHA);
}


void ServiceWorker::init()
{
	baseInitCmdLineParser();
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


void ServiceWorker::clearSettings()
{
	m_settings.clear();
}


void ServiceWorker::baseInitCmdLineParser()
{
	m_cmdLineParser.addSimpleOption("h", "Print this help.");
	m_cmdLineParser.addSimpleOption("v", "Display version of service.");
	m_cmdLineParser.addSimpleOption("e", "Run service as a regular application.");
	m_cmdLineParser.addSimpleOption("i", "Install the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("u", "Uninstall the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("t", "Terminate (stop) the service.");
	m_cmdLineParser.addSimpleOption("clr", "Clear all service settings.");

	initCmdLineParser();

	m_cmdLineParser.parse();
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

Service::Service(ServiceWorker* serviceWorker):
	m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
	m_serviceWorker(serviceWorker),
	m_timer500ms(this)
{
	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return;
	}
}


Service::~Service()
{
	delete m_serviceWorker;		// delete ServiceWorker object passed to constructor
}


void Service::start()
{
	startBaseRequestSocketThread();
	startServiceWorkerThread();
}


void Service::stop()
{
	stopServiceWorkerThread();
	stopBaseRequestSocketThread();
}


void Service::onServiceWork()
{
	m_state = ServiceState::Work;

	DEBUG_LOG_MSG(QString(tr("Service is working")));
}


void Service::onServiceStopped()
{
	m_state = ServiceState::Stopped;

	DEBUG_LOG_MSG(QString(tr("Service has been stoped")));
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
			LOG_MSG(QString("Service START request from SCM (%1).").arg(ha.addressStr()));
			startServiceWorkerThread();
			break;

		case RQID_SERVICE_STOP:
			LOG_MSG(QString("Service STOP request from SCM (%1).").arg(ha.addressStr()));
			stopServiceWorkerThread();
			break;

		case RQID_SERVICE_RESTART:
			LOG_MSG(QString("Service RESTART request from SCM (%1).").arg(ha.addressStr()));
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

	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return;
	}

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

	m_serviceStartTime = QDateTime::currentMSecsSinceEpoch();

	m_state = ServiceState::Starts;

	ServiceWorker* newServiceWorker = m_serviceWorker->createInstance();

	newServiceWorker->setService(this);

	connect(newServiceWorker, &ServiceWorker::work, this, &Service::onServiceWork);
	connect(newServiceWorker, &ServiceWorker::stopped, this, &Service::onServiceStopped);

	m_serviceWorkerThread = new SimpleThread(newServiceWorker);
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

	m_state = ServiceState::Stopped;
}


void Service::startBaseRequestSocketThread()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceInfo[TO_INT(m_serviceWorker->serviceType())].port);

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
	TEST_PTR_RETURN(m_serviceWorker);

	QMutexLocker locker(&m_mutex);

	serviceInfo.set_type(TO_INT(m_serviceWorker->serviceType()));
	serviceInfo.set_majorversion(m_serviceWorker->majorVersion());
	serviceInfo.set_minorversion(m_serviceWorker->minorVersion());
	serviceInfo.set_buildno(m_serviceWorker->commitNo());
	serviceInfo.set_crc(m_crc);
	serviceInfo.set_uptime((QDateTime::currentMSecsSinceEpoch() - m_serviceStartTime) / 1000);

	serviceInfo.set_servicestate(TO_INT(m_state));

	if (m_serviceWorker != nullptr)
	{
		m_serviceWorker->getServiceSpecificInfo(serviceInfo);
	}

	if (m_state != ServiceState::Stopped)
	{
		serviceInfo.set_serviceuptime((QDateTime::currentMSecsSinceEpoch() - m_serviceStartTime) / 1000);
	}
	else
	{
		serviceInfo.set_serviceuptime(0);
	}
}


// -------------------------------------------------------------------------------------
//
// ServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

ServiceStarter::ServiceStarter(QCoreApplication& app, ServiceWorker* serviceWorker) :
	m_app(app),
	m_serviceWorker(serviceWorker)
{
	assert(serviceWorker != nullptr);
	assert(logger.isInitialized() == true);
}


int ServiceStarter::exec()
{
	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return -1;
	}

	m_serviceWorker->init();

	m_serviceWorker->processCmdLineSettings();			// update and store service settings

	bool pauseAndExit = false;
	bool startAsRegularApp = false;

	processCmdLineArguments(pauseAndExit, startAsRegularApp);

	if (pauseAndExit == true)
	{
		system("PAUSE");
		return 0;
	}

	int result = 0;

	if (startAsRegularApp == true)
	{
		result = startAsRegularApplication();
	}
	else
	{
		DaemonServiceStarter daemonStarter(&m_app, m_serviceWorker);

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

	const CommandLineParser& cmdLineParser = m_serviceWorker->cmdLineParser();

	// print Help and exit if "-h" is set
	//
	if (cmdLineParser.optionIsSet("h") == true)
	{
		QString helpText = cmdLineParser.helpText();

		helpText += QString(tr("Run program without options to start service.\n\n"));

		std::cout << C_STR(helpText);

		LOG_MSG(QString(tr("Help printed.")))

		pauseAndExit = true;
		return;
	}

	// print Version and exit if "-v" is set
	//
	if (cmdLineParser.optionIsSet("v") == true)
	{
		QString versionInfo =
			QString("\nApplication:\t%1\nVersion:\t%2.%3.%4 (%5)\nCommit SHA:\t%6\n\n").
				arg(m_serviceWorker->serviceName()).
				arg(m_serviceWorker->majorVersion()).
				arg(m_serviceWorker->minorVersion()).
				arg(m_serviceWorker->commitNo()).
				arg(m_serviceWorker->buildBranch()).
				arg(m_serviceWorker->commitSHA());

		std::cout << C_STR(versionInfo);

		LOG_MSG(QString(tr("Version printed.")))

		pauseAndExit = true;
		return;
	}

	// clear settings and exit if "-clr" is set
	//
	if (cmdLineParser.optionIsSet("clr") == true)
	{
		m_serviceWorker->clearSettings();

		std::cout << C_STR(QString(tr("\nService settings has been cleared.\n\n")));

		LOG_MSG(QString(tr("Service settings has been cleared.")))

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


int ServiceStarter::startAsRegularApplication()
{
	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return 0;
	}

	KeyReaderThread* keyReaderThread = new KeyReaderThread();

	keyReaderThread->start();

	// run service
	//
	Service* service = new Service(m_serviceWorker);
	service->start();

	int result = m_app.exec();

	// stop service
	//
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


// -------------------------------------------------------------------------------------
//
// DaemonServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

DaemonServiceStarter::DaemonServiceStarter(QCoreApplication* app, ServiceWorker* serviceWorker) :
	QtService(serviceWorker->argc(), serviceWorker->argv(), app, serviceWorker->serviceName()),
	m_serviceWorker(serviceWorker)
{
	assert(serviceWorker != nullptr);
}


DaemonServiceStarter::~DaemonServiceStarter()
{
	stopAndDeleteService();
}


int DaemonServiceStarter::exec()
{
	int result = QtService::exec();

	setServiceFlags(QtServiceBase::ServiceFlag::NeedsStopOnShutdown);

	if (result == QT_SERVICE_PAUSE_AND_EXIT)
	{
		system("PAUSE");
	}

	return result;
}



void DaemonServiceStarter::start()
{
	LOG_CALL();

	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return;
	}

	m_service = new Service(m_serviceWorker);

	m_service->start();
}


void DaemonServiceStarter::stop()
{
	LOG_CALL();

	stopAndDeleteService();
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



