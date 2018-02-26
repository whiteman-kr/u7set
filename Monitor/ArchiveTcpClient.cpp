#include "ArchiveTcpClient.h"
#include "Settings.h"

ArchiveTcpClient::ArchiveTcpClient(MonitorConfigController* configController) :
	Tcp::Client(configController->softwareInfo(),
				configController->configuration().archiveService1.address(),
				configController->configuration().archiveService2.address()),
	m_cfgController(configController)
{
	qDebug()
			<< "ArchiveTcpClient::ArchiveTcpClient("
			<< configController->configuration().archiveService1.address().addressPortStr()
			<< ", "
			<< configController->configuration().archiveService2.address().addressPortStr()
			<< ");";

	enableWatchdogTimer(false);

	connect(this, &ArchiveTcpClient::signal_startRequest, this, &ArchiveTcpClient::slot_startRequest);
	connect(this, &ArchiveTcpClient::signal_cancelRequest, this, &ArchiveTcpClient::slot_cancelRequest);

	qRegisterMetaType<ArchiveChunk>("ArchiveChunk");
	qRegisterMetaType<std::shared_ptr<ArchiveChunk>>("std::shared_ptr<ArchiveChunk>");

	return;
}

ArchiveTcpClient::~ArchiveTcpClient()
{
	qDebug() << "ArchiveTcpClient::~ArchiveTcpClient()";
}

bool ArchiveTcpClient::requestData(TimeStamp startTime,
								   TimeStamp endTime,
								   E::TimeType timeType,
								   bool removePeriodicRecords,
								   const std::vector<AppSignalParam>& appSignals)
{
	if (appSignals.size() > ARCH_REQUEST_MAX_SIGNALS)
	{
		assert(appSignals.size() <= ARCH_REQUEST_MAX_SIGNALS);
		return false;
	}

	if (m_requestInProgress == true)
	{
		assert(m_requestInProgress == false);
		return false;
	}

	m_requestData.startTime = qMin(startTime, endTime);
	m_requestData.endTime = qMax(startTime, endTime);
	m_requestData.timeType = timeType;
	m_requestData.removePrioodicRecords = removePeriodicRecords;

	m_requestData.appSignals.clear();
	for (const AppSignalParam& sp : appSignals)
	{
		Hash appSignalHash = ::calcHash(sp.appSignalId());
		m_requestData.appSignals[appSignalHash] = sp.appSignalId();
	}

	emit signal_startRequest();		// emit signal as requestData func can be called from other thread

	return true;
}


bool ArchiveTcpClient::cancelRequest()
{
	if (m_requestInProgress == false)
	{
		return true;
	}

	emit signal_cancelRequest();		// emit signal as this func can be called from other thread

	QTime time;
	time.start();

	while (m_requestInProgress == true && time.elapsed() < 5000)
	{
		QThread::yieldCurrentThread();
	}

	assert(m_requestInProgress == false);	// Request was not cancelled
	return !m_requestInProgress;
}

bool ArchiveTcpClient::isRequestInProgress() const
{
	return m_requestInProgress;
}


void ArchiveTcpClient::timerEvent(QTimerEvent* )
{
	if (m_requestInProgress == true)
	{
		m_statRequestDescription = tr("Requesting data... received state %1").arg(m_statStateReceived);
	}
	else
	{
		m_statRequestDescription.clear();
	}

	emit statusUpdate(m_statRequestDescription, m_statStateReceived, m_statTcpRequestCount, m_statTcpReplyCount);

	return;
}

void ArchiveTcpClient::emitErrorResetState(QString errorMessage)
{
	resetState();
	emit requestError(errorMessage);
	return;
}

void ArchiveTcpClient::resetState()
{
	m_requestInProgress = false;
	m_currentRequestId = 0;
	m_needCancelRequest = false;

	emit requestIsFinished();
	return;
}


void ArchiveTcpClient::onClientThreadStarted()
{
	qDebug() << "ArchiveTcpClient::onClientThreadStarted()";

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &ArchiveTcpClient::slot_configurationArrived,
			Qt::QueuedConnection);

	startTimer(50);		// If timer starten in constructor it will not always work, as constructor runs in another thread

	return;
}

void ArchiveTcpClient::onClientThreadFinished()
{
	qDebug() << "ArchiveTcpClient::onClientThreadFinished()";

	resetState();

	return;
}

void ArchiveTcpClient::onConnection()
{
	qDebug() << "ArchiveTcpClient::onConnection()";

	assert(isClearToSendRequest() == true);

	resetState();

	return;
}

void ArchiveTcpClient::onDisconnection()
{
	qDebug() << "ArchiveTcpClient::onDisconnection";

	resetState();

	return;
}

void ArchiveTcpClient::onReplyTimeout()
{
	qDebug() << "ArchiveTcpClient::onReplyTimeout()";

	emitErrorResetState("Request timeout.");
	closeConnection();

	return;
}

void ArchiveTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	m_statTcpReplyCount ++;

	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{
	case ARCHS_GET_APP_SIGNALS_STATES_START:
		processStart(data);
		break;

	case ARCHS_GET_APP_SIGNALS_STATES_NEXT:
		processNext(data);
		break;

	case ARCHS_GET_APP_SIGNALS_STATES_CANCEL:
		processCancel(data);
		break;

	default:
		assert(false);
		qDebug() << "Wrong requestID in ArchiveTcpClient::processReply() " << requestID;
		emitErrorResetState("Wrong requestID in ArchiveTcpClient::processReply().");
	}

	return;
}

void ArchiveTcpClient::requestStart()
{
	if (isConnected() == false)
	{
		emitErrorResetState("No connection to Archive Service.");
		return;
	}

	if (isClearToSendRequest() == false)
	{
		assert(isClearToSendRequest());
		emitErrorResetState("No connection to Archive Service.");
		return;
	}

	if (m_requestData.appSignals.empty() == true)
	{
		resetState();
		return;
	}

	m_statTcpRequestCount ++;
	m_statStateReceived = 0;
	m_requestInProgress = true;

	m_startRequest.Clear();
	m_startRequest.set_clientequipmentid(theSettings.instanceStrId().toStdString());

	m_startRequest.set_timetype(static_cast<int>(m_requestData.timeType));		// enum TymeType: 0 Plan, 1 SystemTime, 2 LocalTyme, 3 ArchiveId
	m_startRequest.set_starttime(m_requestData.startTime.timeStamp);
	m_startRequest.set_endtime(m_requestData.endTime.timeStamp);

	m_startRequest.set_removeperiodic(m_requestData.removePrioodicRecords);

	for (const std::pair<Hash, QString> sp : m_requestData.appSignals)
	{
		m_startRequest.add_signalhashes(sp.first);
	}

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START, m_startRequest);

	m_startRequestTime.start();

	return;
}

void ArchiveTcpClient::processStart(const QByteArray& data)
{
	qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: " << m_startRequestTime.elapsed();

	// Parse protobuffer message
	//
	bool ok = m_startReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		emitErrorResetState("StartReply data parsing error.");
		return;
	}

	// Process received data
	//
	int error = m_startReply.error();
	//int archError = m_startReply.archerror();
	QString errorString = QString::fromStdString(m_startReply.errorstring());
	m_currentRequestId = m_startReply.requestid();

	if (errorString.isEmpty() == false ||
		error != 0)
	{
		emitErrorResetState(errorString);

		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processStart, error = "
				 << errorString
				 << ", error = " << error
				 << ", RequestID = " << m_currentRequestId;

		return;
	}

	// Go further
	//
	requestNext();
	return;
}

void ArchiveTcpClient::requestNext()
{
	if (isConnected() == false)
	{
		emitErrorResetState("No connection to Archive Service.");
		return;
	}

	if (isClearToSendRequest() == false)
	{
		assert(isClearToSendRequest());
		emitErrorResetState("No connection to Archive Service.");
		return;
	}

	if (m_needCancelRequest == true)
	{
		requestCancel();
		return;
	}

	assert(m_currentRequestId != 0);
	assert(m_requestInProgress == true);

	m_nextRequest.Clear();
	m_nextRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT, m_nextRequest);

	m_statTcpRequestCount ++;

	return;
}

void ArchiveTcpClient::processNext(const QByteArray& data)
{
	// Parse protobuffer message
	//
	bool ok = m_nextReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		emitErrorResetState("NextReply data parsing error.");
		return;
	}

	// Process received data
	//
	int error = m_nextReply.error();
	QString errorString = QString::fromStdString(m_nextReply.errorstring());

	if (errorString.isEmpty() == false ||
		error != 0)
	{
		emitErrorResetState(errorString);

		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processNext, error = "
				 << errorString
				 << ", error = " << error
				 << ", RequestID = " << m_currentRequestId;

		return;
	}

	if (m_currentRequestId != m_nextReply.requestid())
	{
		assert(m_currentRequestId == m_nextReply.requestid());
		emitErrorResetState(tr("Received wrong RequestID, received %1, expected %2").arg(m_nextReply.requestid()).arg(m_currentRequestId));
		return;
	}

	if (m_nextReply.dataready() == false)
	{
		// Data not ready yet, request next part one more time
		//
		QThread::msleep(5);
		requestNext();
		return;
	}

	// Parse data
	//
	int stateCount = m_nextReply.appsignalstates_size();
	m_statStateReceived += stateCount;

	std::vector<AppSignalState> states;
	states.reserve(stateCount);

	// --
	//
	qDebug() << "ArchiveTcpClient::processNext, stateCount " << stateCount;

	for (int i = 0; i < stateCount; i++)
	{
		const ::Proto::AppSignalState& stateMessage = m_nextReply.appsignalstates(i);
		states.emplace_back(stateMessage);		// Construct AppSignalState in the vector and load it
	}

	if (stateCount != 0)
	{
		std::shared_ptr<ArchiveChunk> chunk = std::make_shared<ArchiveChunk>();
		chunk->states.swap(states);
		emit dataReady(chunk);
	}

	// Request next or stop communication
	//
	if (m_nextReply.islastpart() == true)
	{
		qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_NEXT Reqest->Reply time: " << m_startRequestTime.elapsed();
		resetState();							// END OF REQUEST COMMUNICATION!
	}
	else
	{
		// Request next part
		//
		requestNext();
	}

	return;
}

void ArchiveTcpClient::requestCancel()
{
	if (isConnected() == false)
	{
		emitErrorResetState("No connection to Archive Service.");
		return;
	}

	if (isClearToSendRequest() == false)
	{
		assert(isClearToSendRequest());
		emitErrorResetState("No connection to Archive Service.");
		return;
	}

	assert(m_requestInProgress == true);
	assert(m_currentRequestId != 0);

	m_statTcpRequestCount ++;

	m_cancelRequest.Clear();
	m_cancelRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_CANCEL, m_cancelRequest);

	return;
}

void ArchiveTcpClient::processCancel(const QByteArray& data)
{
	// Parse protobuffer message
	//
	bool ok = m_cancelReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		emitErrorResetState("CancelReply data parsing error.");
		return;
	}

	// Process received data
	//
	QString errorString = QString::fromStdString(m_cancelReply.errorstring());

	if (errorString.isEmpty() == false)
	{
		emitErrorResetState(errorString);
		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processStart, error = "
				 << errorString
				 << ", RequestID = " << m_currentRequestId;
		return;
	}

	resetState();					// END OF REQUEST COMMUNICATION!

	return;
}

void ArchiveTcpClient::slot_startRequest()
{
	assert(m_requestInProgress == false);
	requestStart();
	return;
}

void ArchiveTcpClient::slot_cancelRequest()
{
	assert(m_requestInProgress == true);

	if (m_requestInProgress == false)
	{
		resetState();
		return;
	}

	m_needCancelRequest = true;

	return;
}

void ArchiveTcpClient::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort s1 = configuration.archiveService1.address();
	HostAddressPort s2 = configuration.archiveService2.address();

	if (serverAddressPort(0) != s1 ||
		serverAddressPort(1) != s2)
	{
		setServers(s1, s2, true);
	}

	return;
}
