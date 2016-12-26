#include "../lib/Service.h"
#include "../lib/Types.h"


// -------------------------------------------------------------------------------------
//
// ServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

ServiceStarter::ServiceStarter(int argc, char** argv, const QString& name, ServiceWorker* serviceWorker) :
	m_argc(argc),
	m_argv(argv),
	m_name(name),
	m_serviceWorker(serviceWorker)
{
	assert(logger.isInitialized() == true);

	if (argv == nullptr)
	{
		assert(false);
		return;
	}

	if (serviceWorker == nullptr)
	{
		assert(false);
		return;
	}

	for(int i = 0; i < argc; i++)
	{
		m_cmdLineArgs.push_back(QString(argv[i]));
	}
}


void ServiceStarter::initCmdLineParser()
{
	m_cmdLineParser.addOption(QCommandLineOption("h", "Print this help"));
	m_cmdLineParser.addOption(QCommandLineOption("e", "Run service as regular application."));

	m_serviceWorker->iniCmdLineParser(m_cmdLineParser);

	m_cmdLineParser.process(m_cmdLineArgs);
}


int ServiceStarter::exec()
{
	initCmdLineParser();

	bool consoleMode = false;

	if (m_cmdLineParser.isSet("e") || m_cmdLineParser.isSet("h"))
	{
		consoleMode = true;
	}

	int result = 0;

	if (consoleMode == true)
	{
		ConsoleServiceStarter consoleStarter(m_argc, m_argv, m_name, m_serviceWorker);

		result = consoleStarter.exec(m_cmdLineParser);
	}
	else
	{
		DaemonServiceStarter daemonStarter(m_argc, m_argv, m_name, m_serviceWorker);

		result = daemonStarter.exec();
	}

	return result;
}



// -------------------------------------------------------------------------------------
//
// DaemonServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

DaemonServiceStarter::DaemonServiceStarter(int argc, char ** argv, const QString& name, ServiceWorker* serviceWorker) :
	QtService(argc, argv, name),
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

ConsoleServiceStarter::ConsoleServiceStarter(int argc, char ** argv, const QString& name, ServiceWorker* serviceWorker) :
	QCoreApplication(argc, argv),
	m_name(name),
	m_serviceWorker(serviceWorker)
{
	assert(serviceWorker != nullptr);
}


int ConsoleServiceStarter::exec(QCommandLineParser& cmdLineParser)
{
	if (cmdLineParser.isSet("h"))
	{
		qDebug() << C_STR(cmdLineParser.helpText());
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


// -------------------------------------------------------------------------------------
//
// Service class implementation
//
// -------------------------------------------------------------------------------------

Service::Service(ServiceWorker* serviceWorker):
	m_startTime(QDateTime::currentMSecsSinceEpoch()),
	m_serviceWorker(serviceWorker)
{
	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return;
	}

	m_type = m_serviceWorker->serviceType();
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

	if (m_serviceThread != nullptr)
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

	m_serviceThread = new SimpleThread(m_serviceWorker);

	m_serviceThread->start();
}


void Service::stopServiceThread()
{
	QMutexLocker locker(&m_mutex);

	if (m_serviceThread == nullptr)
	{
		return;
	}

	m_state = ServiceState::Stops;

	m_serviceThread->quit();
	m_serviceThread->wait();

	delete m_serviceThread;

	m_serviceThread = nullptr;

	m_state = ServiceState::Stopped;
}


void Service::startBaseRequestSocketThread()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceInfo[TO_INT(m_type)].port);

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
	QMutexLocker locker(&m_mutex);

	serviceInfo.set_type(TO_INT(m_type));
	serviceInfo.set_majorversion(m_majorVersion);
	serviceInfo.set_minorversion(m_minorVersion);
	serviceInfo.set_buildno(m_buildNo);
	serviceInfo.set_crc(m_crc);
	serviceInfo.set_uptime((QDateTime::currentMSecsSinceEpoch() - m_startTime) / 1000);

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
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

ServiceWorker::ServiceWorker(ServiceType serviceType) :
	m_serviceType(serviceType)
{
}


ServiceWorker::~ServiceWorker()
{
}

/*
void ServiceWorker::parseCmdLineArgs()
{
	m_serviceEquipmentID = getCmdLineKeyValue("id");

	m_cfgServiceIP1 = getCmdLineKeyValue("cfgip1");
	m_cfgServiceIP2 = getCmdLineKeyValue("cfgip2");
	m_buildPath(buildPath)


	QString buildFolder = ServiceStarter::getCommandLineKeyValue(argc, argv, "b");
	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString ipStr = ServiceStarter::getCommandLineKeyValue(argc, argv, "ip");

	if (m_buildPath.isEmpty() == false)
	{
		m_cfgFileName = m_buildPath + "/" + m_serviceEquipmentID + "/configuration.xml";
	}
}
*/

QString ServiceWorker::getCmdLineKeyValue(const QString& key)
{
	QString value;

	QString keyStr = QString("-%1=").arg(key);

	int keyStrLen = keyStr.length();

	for(const QString& cmdLineArg : m_cmdLineArgs)
	{
		if (cmdLineArg.mid(0, keyStrLen) == keyStr)
		{
			value = cmdLineArg.mid(keyStrLen);
			break;
		}
	}

	return value;
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


