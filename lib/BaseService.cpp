#include "../include/BaseService.h"


// BaseServiceWorker class implementation
//

BaseServiceWorker::BaseServiceWorker(BaseServiceController *baseServiceController, int serviceType) :
    m_baseSocketThread(nullptr),
    m_baseServiceController(baseServiceController),
    m_serviceType(serviceType)
{
    assert(m_baseServiceController != nullptr);
}


BaseServiceWorker::~BaseServiceWorker()
{
}


void BaseServiceWorker::onBaseServiceWorkerThreadStarted()
{
	m_baseSocketThread = new UdpSocketThread;

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceTypesInfo[m_serviceType].port);

    connect(serverSocket, &UdpServerSocket::request, this, &BaseServiceWorker::onBaseRequest);
    connect(this, &BaseServiceWorker::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	connect(this, &BaseServiceWorker::startMainFunction, m_baseServiceController, &BaseServiceController::startMainFunction);
	connect(this, &BaseServiceWorker::stopMainFunction, m_baseServiceController, &BaseServiceController::stopMainFunction);
	connect(this, &BaseServiceWorker::restartMainFunction, m_baseServiceController, &BaseServiceController::restartMainFunction);

	m_baseSocketThread->run(serverSocket);

    baseServiceWorkerThreadStarted();
}


void BaseServiceWorker::onBaseServiceWorkerThreadFinished()
{
	baseServiceWorkerThreadFinished();

	delete m_baseSocketThread;

    deleteLater();
}


void BaseServiceWorker::onBaseRequest(UdpRequest request)
{
    UdpRequest ack;

    ack.initAck(request);

    switch(request.id())
    {
    case RQID_GET_SERVICE_INFO:

		ServiceInformation si;

		m_baseServiceController->getServiceInfo(si);

		ack.setData(reinterpret_cast<const char*>(&si), sizeof(si));

		emit ackBaseRequest(ack);
        return;

	case RQID_SERVICE_MF_START:
		emit startMainFunction();
		emit ackBaseRequest(ack);
		return;

	case RQID_SERVICE_MF_STOP:
		emit stopMainFunction();
		emit ackBaseRequest(ack);
		return;

	case RQID_SERVICE_MF_RESTART:
		emit restartMainFunction();
		emit ackBaseRequest(ack);
		return;
    }


}


// BaseServiceWorker class implementation
//

MainFunctionWorker::MainFunctionWorker(BaseServiceController *baseServiceController) :
	m_baseServiceController(baseServiceController)
{
	assert(m_baseServiceController != nullptr);
}


MainFunctionWorker::~MainFunctionWorker()
{
	deleteLater();
}


void MainFunctionWorker::onThreadStartedSlot()
{
	onThreadStarted();

	emit mainFunctionWork();
}


void MainFunctionWorker::onThreadFinishedSlot()
{
	onThreadFinished();

	emit mainFunctionStopped();
}


// BaseServiceController class implementation
//


BaseServiceController::BaseServiceController(int serviceType) :
    m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
    m_mainFunctionStartTime(0),
    m_mainFunctionState(MainFunctionState::Stopped),
    m_majorVersion(1),
    m_minorVersion(0),
    m_buildNo(123),
	m_serviceType(serviceType),
	m_mainFunctionNeedRestart(false),
	m_mainFunctionStopped(false)
{
    assert(m_serviceType >= 0 && m_serviceType < RQSTP_COUNT);

	// start timer
	//
	connect(&m_timer500ms, &QTimer::timeout, this, &BaseServiceController::onTimer500ms);

	m_timer500ms.start(500);

	// start BaseWorker
	//
    BaseServiceWorker *worker = new BaseServiceWorker(this, m_serviceType);

    worker->moveToThread(&m_baseWorkerThread);

    connect(&m_baseWorkerThread, &QThread::started, worker, &BaseServiceWorker::onBaseServiceWorkerThreadStarted);
    connect(&m_baseWorkerThread, &QThread::finished, worker, &BaseServiceWorker::onBaseServiceWorkerThreadFinished);

    m_baseWorkerThread.start();

	// start MainFunctionWorker
	//
	startMainFunction();
}


BaseServiceController::~BaseServiceController()
{
    m_baseWorkerThread.quit();
    m_baseWorkerThread.wait();
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

	if (m_mainFunctionState != MainFunctionState::Stopped)
	{
		serviceInfo.mainFunctionUptime = (QDateTime::currentMSecsSinceEpoch() - m_mainFunctionStartTime) / 1000;
	}
	else
	{
		serviceInfo.mainFunctionUptime = 0;
	}

	m_mutex.unlock();
}


void BaseServiceController::stopMainFunction()
{
	if (m_mainFunctionState != MainFunctionState::Work)
	{
		return;
	}

	m_mainFunctionState = MainFunctionState::Stops;

	m_mainFunctionThread.quit();

	// m_mainFunctionState = MainFunctionState::Stopped setted in testMainFunctionState
	//
}


void BaseServiceController::startMainFunction()
{
	if (m_mainFunctionState != MainFunctionState::Stopped)
	{
		return;
	}

	m_mainFunctionStopped = false;
	m_mainFunctionNeedRestart = false;

	m_mainFunctionState = MainFunctionState::Starts;

	MainFunctionWorker* mainFunctionWorker = new MainFunctionWorker(this);

	connect(mainFunctionWorker, &MainFunctionWorker::mainFunctionWork, this, &BaseServiceController::onMainFunctionWork);
	connect(mainFunctionWorker, &MainFunctionWorker::mainFunctionStopped, this, &BaseServiceController::onMainFunctionStopped);

	connect(&m_mainFunctionThread, &QThread::started, mainFunctionWorker, &MainFunctionWorker::onThreadStarted);
	connect(&m_mainFunctionThread, &QThread::finished, mainFunctionWorker, &MainFunctionWorker::onThreadFinished);

	mainFunctionWorker->moveToThread(&m_mainFunctionThread);

	m_mainFunctionThread.start();

	// m_mainFunctionState = MainFunctionState::Work setted in slot onMainFunctionWork
	//
}


void BaseServiceController::restartMainFunction()
{
	stopMainFunction();

	m_mainFunctionNeedRestart = true;
}


void BaseServiceController::onTimer500ms()
{
	checkMainFunctionState();
}


void BaseServiceController::checkMainFunctionState()
{
	if (m_mainFunctionState == MainFunctionState::Stops)
	{
		if (m_mainFunctionStopped && m_mainFunctionThread.isFinished())
		{
			m_mainFunctionStopped = false;

			m_mainFunctionState = MainFunctionState::Stopped;

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
	m_mainFunctionState = MainFunctionState::Work;
}


void BaseServiceController::onMainFunctionStopped()
{
	m_mainFunctionStopped = true;
}

