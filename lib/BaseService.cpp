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

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress("127.0.0.1"), serviceTypesInfo[m_serviceType].port);

    connect(serverSocket, &UdpServerSocket::request, this, &BaseServiceWorker::onBaseRequest);
    connect(this, &BaseServiceWorker::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

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

	ServiceInformation si;

	m_baseServiceController->getServiceInfo(si);

	ack.setData(reinterpret_cast<const char*>(&si), sizeof(si));

    switch(request.id())
    {
    case RQID_GET_SERVICE_INFO:
        emit ackBaseRequest(ack);
        return;
    }
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
    m_serviceType(serviceType)
{
    assert(m_serviceType >= 0 && m_serviceType < RQSTP_COUNT);

    BaseServiceWorker *worker = new BaseServiceWorker(this, m_serviceType);

    worker->moveToThread(&m_baseWorkerThread);

    connect(&m_baseWorkerThread, &QThread::started, worker, &BaseServiceWorker::onBaseServiceWorkerThreadStarted);
    connect(&m_baseWorkerThread, &QThread::finished, worker, &BaseServiceWorker::onBaseServiceWorkerThreadFinished);

    m_baseWorkerThread.start();
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

	serviceInfo.mainFunctionSate = m_mainFunctionState;

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
