#include "BaseService.h"


// MainWorker class implementation
//


MainWorker::MainWorker() :
    m_baseUdpServerSocket(nullptr),
    m_serviceMainFunctionState(ServiceMainFunctionState::Stopped)
{
}


MainWorker::~MainWorker()
{
}


void MainWorker::onMainWorkerThreadStarted()
{
    m_baseUdpServerSocket = new UdpServerSocket(QHostAddress("192.168.122.122"), 3000);
}


void MainWorker::onMainWorkerThreadFinished()
{
    delete m_baseUdpServerSocket;

    deleteLater();
}



// MainWorkerController class implementation
//


MainWorkerController::MainWorkerController()
{
    MainWorker *worker = new MainWorker;

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


BaseService::BaseService(int argc, char ** argv, const QString & name):
    QtService(argc, argv, name)
{
}


BaseService::~BaseService()
{
}


void BaseService::start()
{
    m_mainWorkerController = new MainWorkerController;
}


void BaseService::stop()
{
    delete m_mainWorkerController;
}


void BaseService::getBindToAddress(QHostAddress& bindToAddress, quint16& port)
{
    //bindToAddress.setAddress(("127.0.0.1"), );
}



