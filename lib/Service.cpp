#include "../include/Service.h"
#include "../include/Types.h"
#include <iostream>


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

	// ------------------------------------------------------------------------------
	// same as DaemonServiceStarter::start() code
	//
	if (m_serviceWorker == nullptr)
	{
		assert(false);
		return 0;
	}

	m_service = new Service(m_serviceWorker);

	m_service->start();

	// ------------------------------------------------------------------------------

	// wait for user input
	//
	char a;

	std::cin >> a;

	qDebug() << "\n";

	// ------------------------------------------------------------------------------
	// same as DaemonServiceStarter::stop() code
	//
	m_service->stop();

	delete m_service;

	m_service = nullptr;

	// ------------------------------------------------------------------------------

	qDebug() << "\n======" << C_STR(m_name) << "finished ======\n";

	return 1;
}


// BaseServiceController class implementation
//
/*
BaseServiceController::BaseServiceController(unsigned int serviceType, MainFunctionWorker* mainFunctionWorker) :
	m_mainFunctionWorker(mainFunctionWorker),
	m_mainFunctionNeedRestart(false),
	m_mainFunctionStopped(false),
	m_serviceType(serviceType),
    m_mainFunctionStartTime(0),
	m_mainFunctionState(MainFunctionState::stopped),
    m_majorVersion(1),
    m_minorVersion(0),
    m_buildNo(123),
	m_crc(0xF0F1F2F3)
{
	assert(m_serviceType < SERVICE_TYPE_COUNT);

	qRegisterMetaType<QHostAddress>("QHostAddress");
	qRegisterMetaType<UdpRequest>("UdpRequest");

	initLog();

	APP_MSG(log, QString(serviceTypeStr[m_serviceType]) + " was started");

	// start timer
	//
	connect(&m_timer500ms, &QTimer::timeout, this, &BaseServiceController::onTimer500ms);

	m_timer500ms.start(500);

	// start BaseWorker
	//
    BaseServiceWorker *worker = new BaseServiceWorker(this, m_serviceType);

    worker->moveToThread(&m_baseWorkerThread);

	connect(&m_baseWorkerThread, &QThread::started, worker, &BaseServiceWorker::onThreadStarted);
	connect(&m_baseWorkerThread, &QThread::finished, worker, &BaseServiceWorker::onThreadFinished);

    m_baseWorkerThread.start();

	// start MainFunctionWorker
	//
	m_mainFunctionWorker->setController(this);


	startMainFunction();
}


BaseServiceController::~BaseServiceController()
{
	stopMainFunction();

	m_baseWorkerThread.quit();
    m_baseWorkerThread.wait();

	delete m_mainFunctionWorker;

	APP_MSG(log, QString(serviceTypeStr[m_serviceType]) + " was finished");
}


void BaseServiceController::getServiceInfo(ServiceInformation &serviceInfo)
{
    m_mutex.lock();

	serviceInfo.type = m_serviceType;
	serviceInfo.majorVersion = m_majorVersion;
	serviceInfo.minorVersion = m_minorVersion;
	serviceInfo.buildNo = m_buildNo;
	serviceInfo.crc = m_crc;
	serviceInfo.uptime = (QDateTime::currentMSecsSinceEpoch() - m_serviceStartTime) / 1000;

	serviceInfo.mainFunctionState = m_mainFunctionState;

	if (m_mainFunctionState != MainFunctionState::stopped)
	{
		serviceInfo.mainFunctionUptime = (QDateTime::currentMSecsSinceEpoch() - m_mainFunctionStartTime) / 1000;
	}
	else
	{
		serviceInfo.mainFunctionUptime = 0;
	}

	m_mutex.unlock();
}


void BaseServiceController::initLog()
{
	QFileInfo fi(qApp->applicationFilePath());

	log.initLog(fi.baseName(), 5, 10);
}


void BaseServiceController::stopMainFunction()
{
	qDebug() << "Called BaseServiceController::stopMainFunction";

	if (m_mainFunctionState != MainFunctionState::work)
	{
		return;
	}

	m_mainFunctionState = MainFunctionState::stops;

	m_mainFunctionThread->quit();
	m_mainFunctionThread->wait();

	delete m_mainFunctionThread;

	m_mainFunctionThread = nullptr;

	//m_mainFunctionWorker->moveToThread(thread());

	// m_mainFunctionState = MainFunctionState::Stopped setted in testMainFunctionState
	//

	APP_MSG(log, QString("Main function was stopped."));
}


void BaseServiceController::startMainFunction()
{
	qDebug() << "Called BaseServiceController::startMainFunction";

	if (m_mainFunctionState != MainFunctionState::stopped)
	{
		return;
	}

	m_mainFunctionStopped = false;
	m_mainFunctionNeedRestart = false;

	m_mainFunctionState = MainFunctionState::starts;

	//MainFunctionWorker* mainFunctionWorker = new MainFunctionWorker(this);

	assert(m_mainFunctionThread == nullptr);

	m_mainFunctionThread = new QThread();

	connect(m_mainFunctionThread, &QThread::started, m_mainFunctionWorker, &MainFunctionWorker::onThreadStartedSlot);
	connect(m_mainFunctionThread, &QThread::finished, m_mainFunctionWorker, &MainFunctionWorker::onThreadFinishedSlot);

	m_mainFunctionWorker->moveToThread(m_mainFunctionThread);

	m_mainFunctionStartTime = QDateTime::currentMSecsSinceEpoch();

	m_mainFunctionThread->start();

	APP_MSG(log, QString("Main function was started."));

	// m_mainFunctionState = MainFunctionState::Work setted in slot onMainFunctionWork
	//
}


void BaseServiceController::restartMainFunction()
{
	qDebug() << "Called BaseServiceController::restartMainFunction";

	switch(m_mainFunctionState)
	{
	case MainFunctionState::work:
		stopMainFunction();
		m_mainFunctionNeedRestart = true;
		break;

	case MainFunctionState::stopped:
		startMainFunction();
		break;

	case MainFunctionState::starts:
	case MainFunctionState::stops:
		break;
	}
}


void BaseServiceController::onTimer500ms()
{
	checkMainFunctionState();
}


void BaseServiceController::checkMainFunctionState()
{
	if (m_mainFunctionState == MainFunctionState::stops)
	{
		if (m_mainFunctionStopped && m_mainFunctionThread == nullptr)
		{
			m_mainFunctionStopped = false;

			m_mainFunctionState = MainFunctionState::stopped;

			m_mainFunctionStartTime = 0;

			if (m_mainFunctionNeedRestart)
			{
				m_mainFunctionNeedRestart = false;
				startMainFunction();
			}
		}
	}
}


void BaseServiceController::onMainFunctionWork()
{
	qDebug() << "Called BaseServiceController::onMainFunctionWork";

	m_mainFunctionState = MainFunctionState::work;
}


void BaseServiceController::onMainFunctionStopped()
{
	qDebug() << "Called BaseServiceController::onMainFunctionStopped";

	m_mainFunctionStopped = true;
}

*/
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
			ServiceInformation si;
			getServiceInfo(si);
			ack.writeData(reinterpret_cast<const char*>(&si), sizeof(si));
			break;

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


