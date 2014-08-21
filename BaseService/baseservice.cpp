#include "BaseService.h"


// MainWorker class implementation
//


MainWorker::MainWorker() :
    m_mainUdpServerSocket(nullptr)
{
}


MainWorker::~MainWorker()
{
    delete m_mainUdpServerSocket;
}


void MainWorker::onMainWorkerThreadStarted()
{
    m_mainUdpServerSocket = new UdpServerSocket(QHostAddress("192.168.122.122"), 3000);
}


// MainWorkerController class implementation
//


MainWorkerController::MainWorkerController()
{
    MainWorker *worker = new MainWorker;

    worker->moveToThread(&m_mainWorkerThread);

    connect(&m_mainWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&m_mainWorkerThread, &QThread::started, worker, &MainWorker::onMainWorkerThreadStarted);

    //connect(this, &Controller::operate, worker, &Worker::doWork);
    //connect(worker, &Worker::resultReady, this, &Controller::handleResults);

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





