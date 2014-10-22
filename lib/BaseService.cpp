#include "../include/BaseService.h"


quint32 ReceivedFile::m_serialID = 0;


ReceivedFile::ReceivedFile(const QString& fileName, quint32 fileSize)
{
	if (fileSize > SEND_FILE_MAX_SIZE)
	{
		assert(fileSize <= SEND_FILE_MAX_SIZE);
		return;
	}

	m_serialID++;

	m_ID = m_serialID;

	m_fileName = fileName;
	m_fileSize = fileSize;

	m_data = new char [fileSize];

	assert(m_data != nullptr);
}


ReceivedFile::~ReceivedFile()
{
	delete m_data;
}


bool ReceivedFile::appendData(char* ptr, quint32 len)
{
	if (m_dataSize + len > m_fileSize)
	{
		assert(m_dataSize + len <= m_fileSize);
		return false;
	}

	if (m_data == nullptr)
	{
		assert(m_data != nullptr);
		return false;
	}

	memcpy(m_data + m_dataSize, ptr, len);

	m_dataSize += len;

	return true;
}


// BaseServiceWorker class implementation
//

BaseServiceWorker::BaseServiceWorker(BaseServiceController *baseServiceController, int serviceType) :
    m_baseServiceController(baseServiceController),
    m_serviceType(serviceType)
{
    assert(m_baseServiceController != nullptr);
}


BaseServiceWorker::~BaseServiceWorker()
{
	delete m_sendFileClientSocketThread;

	QHashIterator<quint32, ReceivedFile*> i(m_receivedFile);

	while (i.hasNext())
	{
		i.next();

		delete i.value();
	}

	m_receivedFile.clear();
}


void BaseServiceWorker::onThreadStarted()
{
	m_serverSocketThread = new UdpSocketThread;

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, serviceTypesInfo[m_serviceType].port);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &BaseServiceWorker::onBaseRequest);
    connect(this, &BaseServiceWorker::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	connect(this, &BaseServiceWorker::startMainFunction, m_baseServiceController, &BaseServiceController::startMainFunction);
	connect(this, &BaseServiceWorker::stopMainFunction, m_baseServiceController, &BaseServiceController::stopMainFunction);
	connect(this, &BaseServiceWorker::restartMainFunction, m_baseServiceController, &BaseServiceController::restartMainFunction);

	connect(m_baseServiceController, &BaseServiceController::sendFile, this, &BaseServiceWorker::onSendFile);
	connect(this, &BaseServiceWorker::endSendFile, m_baseServiceController, &BaseServiceController::onEndSendFile);

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

	switch(request.ID())
    {
		case RQID_GET_SERVICE_INFO:
			ServiceInformation si;
			m_baseServiceController->getServiceInfo(si);
			ack.writeData(reinterpret_cast<const char*>(&si), sizeof(si));
			break;

		case RQID_SERVICE_MF_START:
			emit startMainFunction();
			break;

		case RQID_SERVICE_MF_STOP:
			emit stopMainFunction();
			break;

		case RQID_SERVICE_MF_RESTART:
			emit restartMainFunction();
			break;

		case RQID_SEND_FILE_START:
			onSendFileStartRequest(request, ack);
			break;

		default:
			assert(false);
			ack.setErrorCode(RQERROR_UNKNOWN);
			break;
	}

	emit ackBaseRequest(ack);
}


void BaseServiceWorker::onSendFile(QHostAddress address, quint16 port, QString fileName)
{
	if (m_sendFileClientSocketThread != nullptr)
	{
		assert(m_sendFileClientSocketThread == nullptr);	// file send already running
		return;
	}

	m_fileToSend.setFileName(fileName);

	if (!m_fileToSend.open(QIODevice::ReadOnly))
	{
		emit endSendFile(false, fileName);
		return;
	}

	m_fileToSendInfo.setFile(m_fileToSend);

	if (m_fileToSendInfo.size() > SEND_FILE_MAX_SIZE)
	{
		m_fileToSend.close();
		emit endSendFile(false, fileName);
		return;
	}

	// run UDP client socket thread for send file
	//
	m_sendFileClientSocketThread = new UdpSocketThread;

	UdpClientSocket* clientSocket = new UdpClientSocket(address, port);

	connect(this, &BaseServiceWorker::sendFileRequest, clientSocket, &UdpClientSocket::sendRequest);
	connect(clientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceWorker::onSendFileAckReceived);
	connect(clientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceWorker::onSendFileAckTimeout);

	m_sendFileClientSocketThread->run(clientSocket);

	// compose SEND_FILE_START request
	//

	// set file  name
	//
	memset(m_sendFileStart.fileName, 0, sizeof(m_sendFileStart.fileName));

	QString fName = m_fileToSendInfo.fileName();

	QChar* strPtr = fName.data();

	int i = 0;

	int fileNameLen = sizeof(m_sendFileStart.fileName) / sizeof(ushort);

	do
	{
		if (strPtr->isNull())
		{
			break;
		}

		m_sendFileStart.fileName[i] = strPtr->unicode();

		strPtr++;

		i++;
	}
	while (i < fileNameLen - 1);

	m_sendFileStart.fileName[i] = 0;

	// set file size
	//
	m_sendFileStart.fileSize = m_fileToSendInfo.size();

	// send REND_FILE_START request
	//
	sendFileRequest(RQID_SEND_FILE_START, reinterpret_cast<char*>(&m_sendFileStart), sizeof(m_sendFileStart));

	m_sendFileFirstRead = true;

	m_sendFileReadNextPartOK = sendFileReadNextPart();
}


bool BaseServiceWorker::sendFileReadNextPart()
{
	bool lastPart = false;

	if (m_sendFileFirstRead)
	{
		// init send file variables
		//
		m_sendFileNext.fileID = 0;
		m_sendFileNext.partNo = 0;
		m_sendFileNext.partCount = m_fileToSendInfo.size() / SEND_FILE_DATA_SIZE + (m_fileToSendInfo.size() % SEND_FILE_DATA_SIZE == 0 ? 0 : 1);
		m_sendFileNext.dataSize = 0;
		m_sendFileNext.CRC32 = CRC32_INITIAL_VALUE;

		m_sendFileFirstRead = false;
	}
	else
	{
		m_sendFileNext.partNo++;
	}

	if (m_sendFileNext.partNo == m_sendFileNext.partCount - 1)
	{
		lastPart = true;
	}

	quint64 readed = m_fileToSend.read(m_sendFileNext.data, SEND_FILE_DATA_SIZE);

	m_sendFileNext.dataSize = readed;

	m_sendFileNext.CRC32 = CRC32(m_sendFileNext.CRC32, m_sendFileNext.data, readed, lastPart);

	return true;
}


void BaseServiceWorker::onSendFileAckReceived(UdpRequest udpRequest)
{
	udpRequest.initRead();

	switch(udpRequest.ID())
	{
		case RQID_SEND_FILE_START:

			if (udpRequest.errorCode() == RQERROR_OK)
			{
				m_sendFileNext.fileID = udpRequest.readDword();

				sendFileRequest(RQID_SEND_FILE_NEXT, reinterpret_cast<char*>(&m_sendFileNext), sizeof(m_sendFileNext));
			}
			else
			{
				stopSendFile();
				emit endSendFile(false, m_fileToSendInfo.fileName());
			}

			break;

		case RQID_SEND_FILE_NEXT:

			if (udpRequest.errorCode() == RQERROR_OK)
			{
				if (m_sendFileNext.partNo <= m_sendFileNext.partCount - 1)
				{
					sendFileRequest(RQID_SEND_FILE_NEXT, reinterpret_cast<char*>(&m_sendFileNext), sizeof(m_sendFileNext));
				}

				if (m_sendFileNext.partNo != m_sendFileNext.partCount - 1)
				{
					sendFileReadNextPart();
				}
				else
				{
					stopSendFile();
					emit endSendFile(true, m_fileToSendInfo.fileName());
				}
			}
			else
			{
				stopSendFile();
				emit endSendFile(false, m_fileToSendInfo.fileName());
			}
			break;
	}
}


void BaseServiceWorker::onSendFileAckTimeout()
{
	stopSendFile();
	emit endSendFile(false, m_fileToSendInfo.fileName());
}


void BaseServiceWorker::stopSendFile()
{
	m_fileToSend.close();

	m_sendFileClientSocketThread->quit();

	delete m_sendFileClientSocketThread;

	m_sendFileClientSocketThread = nullptr;
}


void BaseServiceWorker::onSendFileStartRequest(const UdpRequest& request, UdpRequest& ack)
{
	const SendFileStart* sendFileStart = reinterpret_cast<const SendFileStart*>(request.data());

	QString fileName(reinterpret_cast<const QChar*>(sendFileStart->fileName));

	ReceivedFile* rf = new ReceivedFile(fileName, sendFileStart->fileSize);

	if (rf->ID() == 0)
	{
		// error
		//
		ack.writeDword(0);

		delete rf;
	}
	else
	{
		ack.writeDword(rf->ID());

		m_receivedFile.insert(rf->ID(), rf);
	}
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
	qRegisterMetaType<QHostAddress>("QHostAddress");

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


void BaseServiceController::onEndSendFile(bool result, QString fileName)
{
	if (result == false)
	{
		qDebug() << "Send file error: " << fileName;
	}
}
