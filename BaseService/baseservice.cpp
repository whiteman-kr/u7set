#include "BaseService.h"


// MainWorker class implementation
//


BaseServiceWorker::BaseServiceWorker(BaseServiceController *baseServiceController, quint16 port) :
    m_baseSocketThread(nullptr),
    m_baseServiceController(baseServiceController),
    m_servicePort(port)
{
    assert(m_baseServiceController != nullptr);
}


BaseServiceWorker::~BaseServiceWorker()
{
}


void BaseServiceWorker::onBaseServiceWorkerThreadStarted()
{
    m_baseSocketThread = new UdpSocketThread;

    UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, m_servicePort);

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

    ACK_GET_SERVICE_INFO agsi;

    agsi.buildNo = 111;

    ack.setData(reinterpret_cast<const char*>(&agsi), sizeof(agsi));

    switch(request.id())
    {
    case RQID_GET_SERVICE_INFO:
        emit ackBaseRequest(ack);
        return;
    }
}


BaseServiceController::BaseServiceController(quint16 port) :
    m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
    m_mainFunctionStartTime(0),
    m_mainFunctionState(MainFunctionState::Stopped),
    m_majorVersion(1),
    m_minorVersion(0),
    m_buildNo(123)
{
    BaseServiceWorker *worker = new BaseServiceWorker(this, port);

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


// BaseService class implementation
//


BaseService::BaseService(int argc, char ** argv, const QString & name, quint16 port):
    QtService(argc, argv, name),
    m_baseServiceController(nullptr),
    m_port(port)
{
}


BaseService::~BaseService()
{
}


void BaseService::start()
{
    m_baseServiceController = new BaseServiceController(m_port);
}


void BaseService::stop()
{
    delete m_baseServiceController;
}




