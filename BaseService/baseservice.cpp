#include "BaseService.h"


// MainWorker class implementation
//


MainWorker::MainWorker(quint16 port) :
    m_baseSocketThread(nullptr),
    m_serviceMainFunctionState(ServiceMainFunctionState::Stopped),
    m_servicePort(port)
{
}


MainWorker::~MainWorker()
{
}


void MainWorker::onMainWorkerThreadStarted()
{
    m_baseSocketThread = new UdpSocketThread;

    UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, m_servicePort);

    connect(serverSocket, &UdpServerSocket::request, this, &MainWorker::onBaseRequest);
    connect(this, &MainWorker::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

    m_baseSocketThread->run(serverSocket);

    mainWorkerThreadStarted();
}


void MainWorker::onMainWorkerThreadFinished()
{
    mainWorkerThreadFinished();

    delete m_baseSocketThread;

    deleteLater();
}


void MainWorker::onBaseRequest(UdpRequest request)
{
    UdpRequest ack;

    ack.initAck(request);

    switch(request.id())
    {
    case RQID_GET_SERVICE_INFO:
        emit ackBaseRequest(ack);
        return;
    }
}


// MainWorkerController class implementation
//


MainWorkerController::MainWorkerController(quint16 port)
{
    MainWorker *worker = new MainWorker(port);

    worker->moveToThread(&m_mainWorkerThread);

    connect(&m_mainWorkerThread, &QThread::started, worker, &MainWorker::onMainWorkerThreadStarted);
    connect(&m_mainWorkerThread, &QThread::finished, worker, &MainWorker::onMainWorkerThreadFinished);

    m_mainWorkerThread.start();
}


MainWorkerController::~MainWorkerController()
{
    m_mainWorkerThread.quit();
    m_mainWorkerThread.wait();
}

// BaseService class implementation
//


BaseService::BaseService(int argc, char ** argv, const QString & name, quint16 port):
    QtService(argc, argv, name),
    m_servicePort(port)
{
}


BaseService::~BaseService()
{
}


void BaseService::start()
{
    m_mainWorkerController = new MainWorkerController(m_servicePort);
}


void BaseService::stop()
{
    delete m_mainWorkerController;
}




