#include "../include/Service.h"
#include "../include/Types.h"


// -------------------------------------------------------------------------------------
//
// ServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

ServiceStarter::ServiceStarter(int argc, char ** argv, const QString& name, ServiceWorker* serviceWorker) :
	m_argc(argc),
	m_argv(argv),
	m_name(name),
	m_serviceWorker(serviceWorker)
{
	assert(serviceWorker != nullptr);
}


int ServiceStarter::exec()
{
	bool consoleMode = false;

	for(int i = 0; i < m_argc; i++)
	{
		if (QString("-e") == m_argv[i])
		{
			consoleMode = true;
			break;
		}
	}

	int result = 0;

	if (consoleMode == true)
	{
		ConsoleServiceStarter consoleStarter(m_argc, m_argv, m_name, m_serviceWorker);

		result = consoleStarter.exec();
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


int ConsoleServiceStarter::exec()
{
	qDebug() << "\n======" << C_STR(m_name) << "sarted ======\n";
	qDebug() << "Press any key and <Return> to finish service\n";

	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return 0;
	}

	// run keyboard reading thread
	//
	ConsoleServiceKeyReaderThread* keyReader = new ConsoleServiceKeyReaderThread();
	connect(keyReader, &ConsoleServiceKeyReaderThread::keyReaded, this, &QCoreApplication::quit);
	keyReader->start();

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

	// delete keyboard reading thread
	//
	keyReader->wait();
	delete keyReader;

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

	initLog();
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

	ServiceWorker* newServiceWorker = m_serviceWorker->createInstance();

	newServiceWorker->setService(this);

	connect(newServiceWorker, &ServiceWorker::work, this, &Service::onServiceWork);
	connect(newServiceWorker, &ServiceWorker::stopped, this, &Service::onServiceStopped);

	m_serviceThread = new SimpleThread(newServiceWorker);

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
	m_baseRequestSocketThread = new UdpSocketThread;

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceInfo[m_type].port);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &Service::onBaseRequest);
	connect(this, &Service::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	m_baseRequestSocketThread->run(serverSocket);
}


void Service::stopBaseRequestSocketThread()
{
	m_baseRequestSocketThread->quit();

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
			ServiceInformation si;
			getServiceInfo(si);
			ack.writeData(reinterpret_cast<const char*>(&si), sizeof(si));
			break;
		}

		case RQID_SERVICE_START:
			APP_MSG(m_log, QString("Service START request from %1.").arg(request.address().toString()));
			startServiceThread();
			break;

		case RQID_SERVICE_STOP:
			APP_MSG(m_log, QString("Service STOP request from %1.").arg(request.address().toString()));
			stopServiceThread();
			break;

		case RQID_SERVICE_RESTART:
			APP_MSG(m_log, QString("Service RESTART request from %1.").arg(request.address().toString()));
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
	qDebug() << "Called Service::onServicenWork";

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


void Service::getServiceInfo(ServiceInformation &serviceInfo)
{
	QMutexLocker locker(&m_mutex);

	serviceInfo.type = m_type;
	serviceInfo.majorVersion = m_majorVersion;
	serviceInfo.minorVersion = m_minorVersion;
	serviceInfo.buildNo = m_buildNo;
	serviceInfo.crc = m_crc;
	serviceInfo.uptime = (QDateTime::currentMSecsSinceEpoch() - m_startTime) / 1000;

	serviceInfo.serviceState = m_state;

	if (m_state != ServiceState::Stopped)
	{
		serviceInfo.serviceUptime = (QDateTime::currentMSecsSinceEpoch() - m_serviceStartTime) / 1000;
	}
	else
	{
		serviceInfo.serviceUptime = 0;
	}
}


void Service::initLog()
{
	QFileInfo fi(qApp->applicationFilePath());

	m_log.initLog(fi.baseName(), 5, 10);
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


