#include "../lib/Service.h"
#include "../lib/Types.h"
#include "../lib/WUtils.h"


// -------------------------------------------------------------------------------------
//
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

ServiceWorker::ServiceWorker(ServiceType serviceType, const QString& serviceName, int& argc, char** argv) :
	m_serviceType(serviceType),
	m_serviceName(serviceName),
	m_argc(argc),
	m_argv(argv)
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


QString ServiceWorker::serviceEquipmentID() const
{
	return m_serviceEquipmentID;
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


void ServiceWorker::baseInitCmdLineParser()
{
	m_cmdLineParser.addSimpleOption("h", "Print this help");
	m_cmdLineParser.addSimpleOption("e", "Run service as a regular application.");
	m_cmdLineParser.addSimpleOption("i", "Install the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("u", "Uninstall the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleOption("s", "Start the service.");
	m_cmdLineParser.addSimpleOption("t", "Terminate (stop) the service.");
	m_cmdLineParser.addSingleValueOption("id", "Assign equipmentID of service. Use -id=EquipmentID.");

	initCmdLineParser();

	m_cmdLineParser.parse();
}


void ServiceWorker::onThreadStarted()
{
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
	delete m_serviceWorker;
}


void Service::start()
{
	startBaseRequestSocketThread();
	startServiceThread();
}


void Service::stop()
{
	stopServiceThread();
	stopBaseRequestSocketThread();
}


void Service::startServiceThread()
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

/*	ServiceWorker* newServiceWorker = m_serviceWorker->createInstance();

	newServiceWorker->setService(this);

	connect(newServiceWorker, &ServiceWorker::work, this, &Service::onServiceWork);
	connect(newServiceWorker, &ServiceWorker::stopped, this, &Service::onServiceStopped);

	m_serviceThread = new SimpleThread(newServiceWorker); */

	m_serviceWorker->setService(this);

	connect(m_serviceWorker, &ServiceWorker::work, this, &Service::onServiceWork);
	connect(m_serviceWorker, &ServiceWorker::stopped, this, &Service::onServiceStopped);

	m_serviceWorkerThread = new SimpleThread(m_serviceWorker);

	m_serviceWorkerThread->start();
}


void Service::stopServiceThread()
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


void Service::onBaseRequest(UdpRequest request)
{
	UdpRequest ack;

	ack.initAck(request);

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
			LOG_MSG(QString("Service START request from %1.").arg(request.address().toString()));
			startServiceThread();
			break;

		case RQID_SERVICE_STOP:
			LOG_MSG(QString("Service STOP request from %1.").arg(request.address().toString()));
			stopServiceThread();
			break;

		case RQID_SERVICE_RESTART:
			LOG_MSG(QString("Service RESTART request from %1.").arg(request.address().toString()));
			stopServiceThread();
			startServiceThread();
			break;

		default:
			assert(false);
			ack.setErrorCode(RQERROR_UNKNOWN_REQUEST);
			break;
	}

	emit ackBaseRequest(ack);
}


void Service::onServiceWork()
{
	qDebug() << "Called Service::onServiceWork";

	m_state = ServiceState::Work;
}


void Service::onServiceStopped()
{
	qDebug() << "Called Service::onServiceStopped";

	m_state = ServiceState::Stopped;
}


void Service::onTimer500ms()
{
	//checkMainFunctionState();
}


void Service::getServiceInfo(Network::ServiceInfo& serviceInfo)
{
	TEST_PTR_RETURN(m_serviceWorker);

	QMutexLocker locker(&m_mutex);

	serviceInfo.set_type(TO_INT(m_serviceWorker->serviceType()));
	serviceInfo.set_majorversion(m_majorVersion);
	serviceInfo.set_minorversion(m_minorVersion);
	serviceInfo.set_buildno(m_buildNo);
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

ServiceStarter::ServiceStarter(ServiceWorker* serviceWorker) :
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

	bool consoleMode = false;

	/*if (m_cmdLineParser.isSet("e") || m_cmdLineParser.isSet("h"))
	{
		consoleMode = true;
	}*/

	int result = 0;

	if (consoleMode == true)
	{
		ConsoleServiceStarter consoleStarter(m_serviceWorker);

		//result = consoleStarter.exec(m_cmdLineParser);
	}
	else
	{
		DaemonServiceStarter daemonStarter(m_serviceWorker);

		result = daemonStarter.exec();
	}

	return result;
}



// -------------------------------------------------------------------------------------
//
// DaemonServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

DaemonServiceStarter::DaemonServiceStarter(ServiceWorker* serviceWorker) :
	QtService(serviceWorker->argc(), serviceWorker->argv(), serviceWorker->serviceName()),
	m_serviceWorker(serviceWorker)
{
	assert(serviceWorker != nullptr);
}


DaemonServiceStarter::~DaemonServiceStarter()
{
	if (m_service != nullptr)
	{
		stop();
	}

	if (m_serviceWorkerDeleted == false)
	{
		// nessesery if DaemonServiceStarter::start() or DaemonServiceStarter::stop() is not called
		//
		delete m_serviceWorker;

		m_serviceWorkerDeleted = true;
	}
}


void DaemonServiceStarter::start()
{
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
	if (m_service == nullptr)
	{
		assert(false);
		return;
	}

	m_service->stop();

	delete m_service;

	m_service = nullptr;

	m_serviceWorkerDeleted = true;
}


// -------------------------------------------------------------------------------------
//
// ConsoleServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

ConsoleServiceStarter::ConsoleServiceStarter(ServiceWorker* serviceWorker) :
	QCoreApplication(serviceWorker->argc(), serviceWorker->argv()),
	m_serviceWorker(serviceWorker)
{
}


int ConsoleServiceStarter::exec(QCommandLineParser& cmdLineParser)
{
	if (cmdLineParser.isSet("h"))
	{
		qDebug() << "\n" << C_STR(cmdLineParser.helpText());
		return 0;
	}

	qDebug() << "\n======" << C_STR(m_name) << "started ======\n";
	qDebug() << "Press any key and RETURN to finish service\n";

	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return 0;
	}

	ConsoleServiceKeyReaderThread* keyReaderThread = new ConsoleServiceKeyReaderThread();

	keyReaderThread->start();

	// run service
	//
	m_service = new Service(m_serviceWorker);
	m_service->start();

	QCoreApplication::exec();

	// stop service
	//
	m_service->stop();
	delete m_service;
	m_service = nullptr;

	keyReaderThread->quit();
	keyReaderThread->wait();
	delete keyReaderThread;

	qDebug() << "\n======" << C_STR(m_name) << "finished ======\n";

	return 1;
}


