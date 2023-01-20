#include "ArchiveTcpClient.h"
#include "MonitorAppSettings.h"
/*
namespace AAA
{

ArchiveTcpClient::ArchiveTcpClient(const ArchiveSource& request,
								   const SoftwareInfo& softwareInfo,
								   const MonitorSettings::ArchiveService& archiveService,
								   ILogFile* logFile) :
	Tcp::Client(softwareInfo, archiveService.address, "ArchiveTcpClient"),
	TcpClientStatistics(this),
	m_logFile(logFile, QString("ArchTcp(%1)").arg(archiveService.address.addressPortStr()))
{
	Q_ASSERT(logFile);

	setObjectName("ArchiveTcpClient");

	qDebug()
			<< "ArchiveTcpClient::ArchiveTcpClient("
			<< archiveService.equipmentId
			<< ", "
			<< archiveService.address.addressPortStr()
			<< ");";

	qRegisterMetaType<ArchiveChunk>("ArchiveChunk");
	qRegisterMetaType<std::shared_ptr<ArchiveChunk>>("std::shared_ptr<ArchiveChunk>");

	//connect(this, &ArchiveTcpClient::signal_startRequest, this, &ArchiveTcpClient::slot_startRequest);
	connect(this, &ArchiveTcpClient::signal_cancelRequest, this, &ArchiveTcpClient::slot_cancelRequest);

	// --
	//
	setRequestData(request);

	return;
}

ArchiveTcpClient::~ArchiveTcpClient()
{
	qDebug() << "ArchiveTcpClient::~ArchiveTcpClient()";
}

bool ArchiveTcpClient::setRequestData(const ArchiveSource& request)
{
	m_logFile.writeMessage(QString("requestData(), startTime %1, endTime %2, timeType %3, removePeriodicRecords %4, appSignals: %5")
							.arg(request.requestStartTime.toDateTime().toString())
							.arg(request.requestEndTime.toDateTime().toString())
							.arg(E::valueToString(request.timeType))
							.arg(request.removePeriodicRecords)
							.arg([](const auto& appSignals) -> QString
									{
										QStringList result;
										result.reserve(static_cast<int>(appSignals.size()));
										for (const ArchiveSignal& as : appSignals)
										{
											result.push_back(as.signalParam.appSignalId());
										}
										return result.join(", ");
									}(request.acceptedSignals))
						   );

	if (request.acceptedSignals.size() > ARCH_REQUEST_MAX_SIGNALS)
	{
		m_logFile.writeWarning(QString("requestData() appSignals.size()(%1) > ARCH_REQUEST_MAX_SIGNALS(%2), cancel request")
								.arg(request.acceptedSignals.size())
								.arg(ARCH_REQUEST_MAX_SIGNALS));
		return false;
	}

	if (m_requestInProgress == true)
	{
		Q_ASSERT(m_requestInProgress == false);
		m_logFile.writeError(QString("requestData() m_requestInProgress == true, cancel request"));
		return false;
	}

	m_requestData.startTime = qMin(request.requestStartTime, request.requestEndTime);
	m_requestData.endTime = qMax(request.requestStartTime, request.requestEndTime);
	m_requestData.timeType = request.timeType;
	m_requestData.removePrioodicRecords = request.removePeriodicRecords;

	m_requestData.appSignals.clear();
	for (const ArchiveSignal& as : request.acceptedSignals)
	{
		Hash appSignalHash = ::calcHash(as.signalParam.appSignalId());
		m_requestData.appSignals[appSignalHash] = as.signalParam.appSignalId();
	}

	//emit signal_startRequest();		// emit signal as requestData func can be called from other thread

	return true;
}

bool ArchiveTcpClient::cancelRequest()
{
	m_logFile.writeMessage(QString("cancelRequest()"));

	if (m_requestInProgress == false)
	{
		return true;
	}

	emit signal_cancelRequest();		// emit signal as this func can be called from other thread

	QElapsedTimer time;
	time.start();

	while (m_requestInProgress == true && time.elapsed() < 5000)
	{
		QThread::yieldCurrentThread();
	}

	Q_ASSERT(m_requestInProgress == false);	// Request was not cancelled
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
	emit requestError(std::move(errorMessage));
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
	m_logFile.writeMessage(QString("onClientThreadStarted()"));

	startTimer(50);		// If timer starten in constructor it will not always work, as constructor runs in another thread

	return;
}

void ArchiveTcpClient::onClientThreadFinished()
{
	qDebug() << "ArchiveTcpClient::onClientThreadFinished()";
	m_logFile.writeMessage(QString("onClientThreadFinished()"));

	resetState();

	return;
}

void ArchiveTcpClient::onTryConnectToServer(const HostAddressPort& serverAddr)
{
	Tcp::Client::onTryConnectToServer(serverAddr);

	if (m_tryToConnectCounter > 0)
	{
		m_tryToConnectCounter--;
	}
	else
	{
		// The connection was not established, report an error
		//
		QString error = tr("Connection to ArchiveService %1 cannot be established.")
							.arg(this->currentServerAddressPort().addressPortStr());

		emitErrorResetState(error);
	}

	return;
}

void ArchiveTcpClient::onConnection()
{
	qDebug() << "ArchiveTcpClient::onConnection()";
	m_logFile.writeMessage(QString("onConnection()"));

	Q_ASSERT(isClearToSendRequest() == true);

	// Start data requesting
	//
	requestStart();

	//resetState();
	//emit signal_connectionEstablished();

	return;
}

void ArchiveTcpClient::onDisconnection()
{
	qDebug() << "ArchiveTcpClient::onDisconnection";
	m_logFile.writeMessage(QString("onDisconnection()"));

	resetState();

	return;
}

void ArchiveTcpClient::onReplyTimeout()
{
	qDebug() << "ArchiveTcpClient::onReplyTimeout()";
	m_logFile.writeWarning(QString("onReplyTimeout()"));

	emitErrorResetState("Request timeout.");
	closeConnection();

	return;
}

void ArchiveTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	m_statTcpReplyCount ++;

	if (replyData == nullptr)
	{
		Q_ASSERT(replyData);
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
		Q_ASSERT(false);

		qDebug() << "Wrong requestID in ArchiveTcpClient::processReply() " << requestID;
		m_logFile.writeError(QString("Wrong requestID in processReply(), requestId %1").arg(requestID));

		emitErrorResetState("Wrong requestID in ArchiveTcpClient::processReply().");
	}

	return;
}

void ArchiveTcpClient::requestStart()
{
	if (isConnected() == false)
	{
		emitErrorResetState("No connection to Archive Service.");
		m_logFile.writeError(QString("requestStart() No connection to Archive Service"));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		Q_ASSERT(isClearToSendRequest());
		emitErrorResetState("No connection to Archive Service.");
		m_logFile.writeError(QString("requestStart() isClearToSendRequest() == false"));
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
	m_startRequest.set_clientequipmentid(MonitorAppSettings::instance().equipmentId().toStdString());

	m_startRequest.set_timetype(static_cast<int>(m_requestData.timeType));		// enum TymeType: 0 Plan, 1 SystemTime, 2 LocalTyme, 3 ArchiveId
	m_startRequest.set_starttime(m_requestData.startTime.timeStamp);
	m_startRequest.set_endtime(m_requestData.endTime.timeStamp);

	m_startRequest.set_removeperiodic(m_requestData.removePrioodicRecords);

	QStringList appSignlList;
	appSignlList.reserve(static_cast<int>(m_requestData.appSignals.size()));

	for (const std::pair<Hash, QString> sp : m_requestData.appSignals)
	{
		m_startRequest.add_signalhashes(sp.first);
		appSignlList.push_back(sp.second);
	}

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START, m_startRequest);

	m_startRequestTime.start();

	m_logFile.writeMessage(QString("requestStart() sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START), startTime %1, endTime %2, timeStamp %3, removePrioodicRecords %4, appSignals: %5")
							.arg(m_requestData.startTime.toDateTime().toString())
							.arg(m_requestData.endTime.toDateTime().toString())
							.arg(E::valueToString(m_requestData.timeType))
							.arg(m_requestData.removePrioodicRecords)
							.arg(appSignlList.join(", "))
						 );

	return;
}

void ArchiveTcpClient::processStart(const QByteArray& data)
{
	qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: " << m_startRequestTime.elapsed();
	m_logFile.writeMessage(QString("processStart(), Reqest->Reply time %1 ms, data.size() %2")
							.arg(m_startRequestTime.elapsed())
							.arg(data.size())
						 );

	// Parse protobuffer message
	//
	bool ok = m_startReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
	if (ok == false)
	{
		emitErrorResetState("StartReply data parsing error.");
		m_logFile.writeError("processStart() StartReply data parsing error.");
		return;
	}

	// Process received data
	//
	int error = m_startReply.error();
	QString errorString = QString::fromStdString(m_startReply.errorstring());
	m_currentRequestId = m_startReply.requestid();

	if (errorString.isEmpty() == false ||
		error != 0)
	{
		emitErrorResetState(errorString);

		m_logFile.writeError(QString("processStart() Received error from ArchiveService: requestId %1, error: %2.")
								.arg(m_currentRequestId)
								.arg(errorString));

		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processStart, error = "
				 << errorString
				 << ", error = " << error
				 << ", RequestID = " << m_currentRequestId;

		return;
	}

	m_logFile.writeMessage(QString("processStart() Received m_currentRequestId %1").arg(m_currentRequestId));

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
		m_logFile.writeError(QString("requestNext() No connection to Archive Service"));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		Q_ASSERT(isClearToSendRequest());
		emitErrorResetState("No connection to Archive Service.");
		m_logFile.writeError(QString("requestNext() isClearToSendRequest() == false"));
		return;
	}

	if (m_needCancelRequest == true)
	{
		requestCancel();
		return;
	}

	Q_ASSERT(m_currentRequestId != 0);
	Q_ASSERT(m_requestInProgress == true);

	m_nextRequest.Clear();
	m_nextRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT, m_nextRequest);

	m_logFile.writeMessage(QString("requestNext() sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT), m_currentRequestId = %1").arg(m_currentRequestId));

	m_statTcpRequestCount ++;

	return;
}

void ArchiveTcpClient::processNext(const QByteArray& data)
{
	m_logFile.writeMessage(QString("processNext(), data.size() = %1, requestId = %2").arg(data.size()).arg(m_currentRequestId));

	// Parse protobuffer message
	//
	bool ok = m_nextReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		emitErrorResetState("NextReply data parsing error.");
		m_logFile.writeError(QString("processNext() Data parsing error."));
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

		m_logFile.writeError(QString("processNext() Received error from ArchiveService: requestId %1, error: %2.")
								.arg(m_currentRequestId)
								.arg(errorString));

		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processNext, error = "
				 << errorString
				 << ", error = " << error
				 << ", RequestID = " << m_currentRequestId;

		return;
	}

	if (m_currentRequestId != m_nextReply.requestid())
	{
		Q_ASSERT(m_currentRequestId == m_nextReply.requestid());
		emitErrorResetState(tr("Received wrong RequestID, received %1, expected %2").arg(m_nextReply.requestid()).arg(m_currentRequestId));

		m_logFile.writeError(QString("processNext() Received wrong RequestID, received %1, expected %2")
								.arg(m_nextReply.requestid())
								.arg(m_currentRequestId));
		return;
	}

	if (m_nextReply.dataready() == false)
	{
		// Data not ready yet, request next part one more time
		//
		QThread::msleep(50);
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
	m_logFile.writeMessage(QString("processNext() stateCount = %1, requestId = %2").arg(stateCount).arg(m_currentRequestId));

	for (int i = 0; i < stateCount; i++)
	{
		const ::Proto::AppSignalState& stateMessage = m_nextReply.appsignalstates(i);
		states.emplace_back(stateMessage);		// Construct AppSignalState in the vector and load it
	}

	if (stateCount != 0)
	{
		std::shared_ptr<ArchiveChunk> chunk = std::make_shared<ArchiveChunk>();
		chunk->swap(states);
		emit dataReady(chunk);
	}

	// Request next or stop communication
	//
	if (m_nextReply.islastpart() == true)
	{
		qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_NEXT Reqest->Reply time: " << m_startRequestTime.elapsed();
		m_logFile.writeMessage(QString("processNext() End of request, time %1 ms, requestId %2").arg(m_startRequestTime.elapsed()).arg(m_currentRequestId));

		//
		// THE END OF REQUEST COMMUNICATION!
		//
		resetState();	// It emmits requestIsFinished
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
		m_logFile.writeError(QString("requestCancel() No connection to Archive Service"));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		Q_ASSERT(isClearToSendRequest());
		emitErrorResetState("No connection to Archive Service.");
		m_logFile.writeError(QString("requestCancel() isClearToSendRequest() == false"));
		return;
	}

	Q_ASSERT(m_requestInProgress == true);
	Q_ASSERT(m_currentRequestId != 0);

	m_statTcpRequestCount ++;

	m_cancelRequest.Clear();
	m_cancelRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_CANCEL, m_cancelRequest);

	m_logFile.writeMessage(QString("processCancel() sendRequest(ARCHS_GET_APP_SIGNALS_STATES_CANCEL), m_currentRequestId = %1").arg(m_currentRequestId));

	return;
}

void ArchiveTcpClient::processCancel(const QByteArray& data)
{
	m_logFile.writeMessage(QString("processCancel() data.size() = %1, requestId = %2").arg(data.size()).arg(m_currentRequestId));

	// Parse protobuffer message
	//
	bool ok = m_cancelReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		emitErrorResetState("CancelReply data parsing error.");
		m_logFile.writeError(QString("processNext() Data parsing error."));
		return;
	}

	// Process received data
	//
	QString errorString = QString::fromStdString(m_cancelReply.errorstring());

	if (errorString.isEmpty() == false)
	{
		emitErrorResetState(errorString);

		m_logFile.writeError(QString("processCancel() Received error from ArchiveService: requestId %1, error: %2.")
								.arg(m_currentRequestId)
								.arg(errorString));

		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processStart, error = "
				 << errorString
				 << ", RequestID = " << m_currentRequestId;
		return;
	}

	resetState();					// END OF REQUEST COMMUNICATION!

	return;
}

//void ArchiveTcpClient::slot_startRequest()
//{
//	Q_ASSERT(m_requestInProgress == false);
//	requestStart();
//	return;
//}

void ArchiveTcpClient::slot_cancelRequest()
{
	resetState();
	m_needCancelRequest = true;
	return;

//	Q_ASSERT(m_requestInProgress == true);
//
//	if (m_requestInProgress == false)
//	{
//		resetState();
//		return;
//	}
//
//	m_needCancelRequest = true;
//
//	return;
}

}
*/
