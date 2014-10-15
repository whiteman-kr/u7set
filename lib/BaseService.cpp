#include "../include/BaseService.h"
#include <QFileInfo>


// BaseServiceWorker class implementation
//

BaseServiceWorker::BaseServiceWorker(BaseServiceController *baseServiceController, int serviceType) :
	m_serverSocketThread(nullptr),
	m_sendFileClientSocketThread(nullptr),
	m_sendFileStartBuffer(nullptr),
	m_fileToSend(nullptr),
    m_baseServiceController(baseServiceController),
    m_serviceType(serviceType)
{
    assert(m_baseServiceController != nullptr);
}


BaseServiceWorker::~BaseServiceWorker()
{
	delete m_sendFileClientSocketThread;
	delete m_fileToSend;
	delete m_sendFileStartBuffer;
}


void BaseServiceWorker::onThreadStarted()
{
	m_serverSocketThread = new UdpSocketThread;

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceTypesInfo[m_serviceType].port);

    connect(serverSocket, &UdpServerSocket::request, this, &BaseServiceWorker::onBaseRequest);
    connect(this, &BaseServiceWorker::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	connect(this, &BaseServiceWorker::startMainFunction, m_baseServiceController, &BaseServiceController::startMainFunction);
	connect(this, &BaseServiceWorker::stopMainFunction, m_baseServiceController, &BaseServiceController::stopMainFunction);
	connect(this, &BaseServiceWorker::restartMainFunction, m_baseServiceController, &BaseServiceController::restartMainFunction);

	m_serverSocketThread->run(serverSocket);

	threadStarted();
}


void BaseServiceWorker::onThreadFinished()
{
	threadFinished();

	delete m_serverSocketThread;

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


void BaseServiceWorker::onSendFile(QHostAddress address, quint16 port, QString fileName)
{
	if (m_sendFileClientSocketThread != nullptr)
	{
		assert(m_sendFileClientSocketThread == nullptr);	// file send already running
		return;
	}

	assert(m_fileToSend == nullptr);	// must be null
	assert(m_sendFileStartBuffer == nullptr);

	m_fileToSend = new QFile(fileName);

	if (!m_fileToSend->open(QIODevice::ReadOnly))
	{
		delete m_fileToSend;

		m_fileToSend = nullptr;

		emit endSendFile(false);

		return;
	}

	m_sendFileStartBuffer = new char[MAX_DATAGRAM_SIZE];

	m_sendFileClientSocketThread = new UdpSocketThread;

	UdpClientSocket* clientSocket = new UdpClientSocket(address, port);

	connect(this, &BaseServiceWorker::sendFileRequest, clientSocket, &UdpClientSocket::sendRequest);
	connect(clientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceWorker::onSendFileRequestAck);

	m_sendFileClientSocketThread->run(clientSocket);

	QFileInfo fi(*m_fileToSend);

	QString fName = fi.fileName();
	quint64 fileSize = fi.size();

	QChar* strPtr = fName.data();

	ushort* bufferPtr = reinterpret_cast<ushort*>(m_sendFileStartBuffer);

	int i = 0;
	do
	{
		if (strPtr->isNull())
		{
			break;
		}

		bufferPtr[i] = strPtr->unicode();

		strPtr++;

		i++;
	}
	while (i < 63);

	bufferPtr[i] = 0;

	i++;

	*((quint32*)(bufferPtr + i)) = fileSize;

	sendFileRequest(RQID_SEND_FILE_START, m_sendFileStartBuffer, i * sizeof(ushort) + sizeof(quint32));
}


void BaseServiceWorker::onSendFileRequestAck(RequestHeader header, QByteArray data)
{

}

// MainFunctionWorker class implementation
//

MainFunctionWorker::MainFunctionWorker(BaseServiceController *baseServiceController) :
	m_baseServiceController(baseServiceController)
{
	assert(m_baseServiceController != nullptr);
}


MainFunctionWorker::~MainFunctionWorker()
{
}


void MainFunctionWorker::onThreadStartedSlot()
{
	threadStarted();

	emit mainFunctionWork();
}


void MainFunctionWorker::onThreadFinishedSlot()
{
	threadFinished();

	emit mainFunctionStopped();

	deleteLater();
}


// BaseServiceController class implementation
//


BaseServiceController::BaseServiceController(int serviceType) :
    m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
    m_mainFunctionStartTime(0),
	m_mainFunctionState(MainFunctionState::stopped),
    m_majorVersion(1),
    m_minorVersion(0),
    m_buildNo(123),
	m_crc(0xF0F1F2F3),
	m_serviceType(serviceType),
	m_mainFunctionNeedRestart(false),
	m_mainFunctionStopped(false)
{
    assert(m_serviceType >= 0 && m_serviceType < RQSTP_COUNT);

	qRegisterMetaType<UdpRequest>("UdpRequest");

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

	connect(this, &BaseServiceController::sendFile, worker, &BaseServiceWorker::onSendFile);
	connect(worker, &BaseServiceWorker::endSendFile, this, &BaseServiceController::onEndSendFile);

    m_baseWorkerThread.start();

	// start MainFunctionWorker
	//
	startMainFunction();
}


BaseServiceController::~BaseServiceController()
{
	stopMainFunction();

	m_mainFunctionThread.wait();		// !!!!

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


void BaseServiceController::stopMainFunction()
{
	qDebug() << "Called BaseServiceController::stopMainFunction";

	if (m_mainFunctionState != MainFunctionState::work)
	{
		return;
	}

	m_mainFunctionState = MainFunctionState::stops;

	m_mainFunctionThread.quit();

	// m_mainFunctionState = MainFunctionState::Stopped setted in testMainFunctionState
	//
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

	MainFunctionWorker* mainFunctionWorker = new MainFunctionWorker(this);

	connect(mainFunctionWorker, &MainFunctionWorker::mainFunctionWork, this, &BaseServiceController::onMainFunctionWork);
	connect(mainFunctionWorker, &MainFunctionWorker::mainFunctionStopped, this, &BaseServiceController::onMainFunctionStopped);

	connect(&m_mainFunctionThread, &QThread::started, mainFunctionWorker, &MainFunctionWorker::onThreadStartedSlot);
	connect(&m_mainFunctionThread, &QThread::finished, mainFunctionWorker, &MainFunctionWorker::onThreadFinishedSlot);

	mainFunctionWorker->moveToThread(&m_mainFunctionThread);

	m_mainFunctionStartTime = QDateTime::currentMSecsSinceEpoch();

	m_mainFunctionThread.start();

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
		if (m_mainFunctionStopped && m_mainFunctionThread.isFinished())
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


void BaseServiceController::onEndSendFile(bool result)
{
}
