#include "ArchiveTcpClient2.h"

ArchiveTcpClient2::ArchiveTcpClient2(const ArchiveSource& request,
									 const SoftwareInfo& softwareInfo,
									 const SoftwareEndpoint::ArchiveService& archiveService,
									 ILogFile* logFile) :
	Tcp::Client(softwareInfo, archiveService.address, "ArchiveTcpClient", archiveService.equipmentId),
	TcpClientStatistics(this),
	m_logFile(logFile, QString("ArchTcp(%1)").arg(archiveService.address.addressPortStr())),
	m_serverSettings(archiveService)
{
	Q_ASSERT(logFile);

	setObjectName("ArchiveTcpClient");

	qRegisterMetaType<std::shared_ptr<ArchiveRequestResult>>();

	qDebug()
			<< "ArchiveTcpClient::ArchiveTcpClient("
			<< archiveService.equipmentId
			<< ", "
			<< archiveService.address.addressPortStr()
			<< ");";

	m_result.archiveServiceId = m_serverSettings.equipmentId;

	setRequestData(request);
	updateStatistics(tr("Construction"));

	connect(this, &Tcp::Client::signal_wrongServerID,
		[this](const QString& errorMessage)
		{
			m_logFile.writeError(errorMessage);
		});

	return;
}

ArchiveTcpClient2::~ArchiveTcpClient2()
{
	qDebug() << Q_FUNC_INFO;
}

void ArchiveTcpClient2::cancelRequest()
{
	m_logFile.writeMessage(tr("cancelRequest()"));
	m_needCancelRequest = true;

	updateStatistics(tr("Canceled"));

	return;
}


void ArchiveTcpClient2::setRequestData(const ArchiveSource& request)
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

	return;
}

void ArchiveTcpClient2::finish(QString error /*= QString{}*/)
{
	setServers({}, {}, false);	// This stops trying to connect to server
	closeConnection();

	if (error.isEmpty() == false)
	{
		updateStatistics(tr("Finished: ") + error);
		m_logFile.writeError(error);
	}
	else
	{
		updateStatistics(tr("Finished"));
	}

	auto result = std::make_shared<ArchiveRequestResult>(std::move(m_result));
	m_result = {};

	emit dataReady(result, error);

	error.clear();
	return;
}

void ArchiveTcpClient2::onTryConnectToServer(const HostAddressPort& serverAddr)
{
	updateStatistics(tr("TryConnect"));
	Tcp::Client::onTryConnectToServer(serverAddr);

	if (m_needCancelRequest == true)
	{
		finish();
		return;
	}

	if (m_tryToConnectCounter == 0)
	{
		// The connection was not established, report an error
		//
		QString error = tr("Cannot establish connection to ArchiveService %1, %2.")
				.arg(m_serverSettings.shortenId)
				.arg(serverAddr.addressPortStr());
		finish(error);
	}
	else
	{
		m_tryToConnectCounter--;
	}

	return;
}

void ArchiveTcpClient2::onConnection()
{
	updateStatistics(tr("Connected"));

	qDebug() << "ArchiveTcpClient::onConnection()";
	m_logFile.writeMessage(QString("onConnection()"));

	Q_ASSERT(isClearToSendRequest() == true);

	// Start data requesting
	//
	requestStart();

	return;
}

void ArchiveTcpClient2::onDisconnection()
{
	updateStatistics(tr("Disconnected"));

	qDebug() << "ArchiveTcpClient::onDisconnection";
	m_logFile.writeMessage(QString("onDisconnection()"));

	return;
}

void ArchiveTcpClient2::onReplyTimeout()
{
	updateStatistics(tr("Timeout"));

	QString error = tr("ArchiveService (%1, %2) reply timeout.")
			.arg(m_serverSettings.shortenId)
			.arg(currentServerAddressPort().toString());
	qDebug() << error;

	finish(error);

	return;
}


void ArchiveTcpClient2::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	updateStatistics(0, 1, 0);

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
		finish(tr("Wrong requestID in processReply(), requestId %1").arg(requestID));
	}

	return;
}

void ArchiveTcpClient2::requestStart()
{
	if (isConnected() == false)
	{
		Q_ASSERT(isConnected());

		QString error = tr("Try to request ArchiveService while connection was not established.");
		finish(error);

		return;
	}

	if (isClearToSendRequest() == false)
	{
		Q_ASSERT(isClearToSendRequest());
		finish(tr("requestStart() isClearToSendRequest() == false"));
		return;
	}

	if (m_requestData.appSignals.empty() == true)
	{
		QString error = tr("No signals were selected for requesting archive");
		finish(error);

		return;
	}

	updateStatistics(1, 0, 0);

	Network::GetAppSignalStatesFromArchiveStartRequest startRequest;

	startRequest.set_clientequipmentid(this->localSoftwareInfo().equipmentID().toStdString());

	startRequest.set_timetype(static_cast<int>(m_requestData.timeType));		// enum TymeType: 0 Plan, 1 SystemTime, 2 LocalTyme, 3 ArchiveId
	startRequest.set_starttime(m_requestData.startTime.timeStamp);
	startRequest.set_endtime(m_requestData.endTime.timeStamp);

	startRequest.set_removeperiodic(m_requestData.removePrioodicRecords);

	QStringList appSignlList;
	appSignlList.reserve(static_cast<int>(m_requestData.appSignals.size()));

	for (const std::pair<Hash, QString> sp : m_requestData.appSignals)
	{
		startRequest.add_signalhashes(sp.first);
		appSignlList.push_back(sp.second);
	}

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START, startRequest);

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

void ArchiveTcpClient2::processStart(const QByteArray& data)
{
	qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: " << m_startRequestTime.elapsed();
	m_logFile.writeMessage(QString("processStart(), Reqest->Reply time %1 ms, data.size() %2")
							.arg(m_startRequestTime.elapsed())
							.arg(data.size())
						 );

	// Parse protobuffer message
	//
	Network::GetAppSignalStatesFromArchiveStartReply startReply;

	bool ok = startReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
	if (ok == false)
	{
		QString error = tr("processStart() StartReply data parsing error, ArchiveService %1.").arg(m_serverSettings.shortenId);
		finish(error);

		return;
	}

	// Process received data
	//
	int error = startReply.error();
	QString errorString = QString::fromStdString(startReply.errorstring());
	m_currentRequestId = startReply.requestid();

	if (errorString.isEmpty() == false ||
		error != 0)
	{
		QString errorMsg = tr("processStart() Received error from ArchiveService: requestId %1, error: %2.")
								.arg(m_currentRequestId)
								.arg(errorString);
		finish(errorMsg);

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

void ArchiveTcpClient2::requestNext()
{
	if (isConnected() == false)
	{
		finish(tr("requestNext() No connection to Archive Service"));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		Q_ASSERT(isClearToSendRequest());
		finish(tr("requestNext() isClearToSendRequest() == false"));
		return;
	}

	if (m_needCancelRequest == true)
	{
		requestCancel();
		return;
	}

	Q_ASSERT(m_currentRequestId != 0);

	m_nextRequest.Clear();
	m_nextRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT, m_nextRequest);

	m_logFile.writeMessage(QString("requestNext() sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT), m_currentRequestId = %1").arg(m_currentRequestId));

	updateStatistics(1, 0, 0);

	return;
}

void ArchiveTcpClient2::processNext(const QByteArray& data)
{
	m_logFile.writeMessage(QString("processNext(), data.size() = %1, requestId = %2").arg(data.size()).arg(m_currentRequestId));

	// Parse protobuffer message
	//
	bool ok = m_nextReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		finish(tr("processNext() Data parsing error."));
		return;
	}

	// Process received data
	//
	int error = m_nextReply.error();
	QString errorString = QString::fromStdString(m_nextReply.errorstring());

	if (errorString.isEmpty() == false ||
		error != 0)
	{
		finish(tr("processNext() Received error from ArchiveService: requestId %1, error: %2.")
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

		finish(tr("processNext() Received wrong RequestID, received %1, expected %2")
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
	updateStatistics(0, 0, stateCount);

	// --
	//
	qDebug() << "ArchiveTcpClient::processNext, stateCount " << stateCount;
	m_logFile.writeMessage(QString("processNext() stateCount = %1, requestId = %2").arg(stateCount).arg(m_currentRequestId));

	std::vector<ArchiveSignalState> states;
	states.reserve(stateCount);

	for (const auto& stateMessage : m_nextReply.appsignalstates())
	{
		// Filter Periodic points
		//
		if (m_requestData.removePrioodicRecords == true &&
			(stateMessage.flags() & AppSignalStateFlags::MASK_AUTO_POINT_REASONE) != 0)
		{
			continue;
		}

		states.emplace_back(AppSignalState{stateMessage}, m_serverSettings.equipmentId, m_serverSettings.shortenId);
	}

	m_result.states.push_back(std::move(states));

	if (stateCount != 0)
	{
		// Some data left, if you want to send incomplete data, send it from here
		//
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
		finish();
	}
	else
	{
		// Request next part
		//
		requestNext();
	}

	return;
}

void ArchiveTcpClient2::requestCancel()
{
	if (isConnected() == false || isClearToSendRequest() == false)
	{
		finish();
		return;
	}

	Q_ASSERT(m_currentRequestId != 0);

	updateStatistics(1, 0, 0);

	Network::GetAppSignalStatesFromArchiveCancelRequest cancelRequest;
	cancelRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_CANCEL, cancelRequest);

	m_logFile.writeMessage(QString("processCancel() sendRequest(ARCHS_GET_APP_SIGNALS_STATES_CANCEL), m_currentRequestId = %1").arg(m_currentRequestId));

	return;
}

void ArchiveTcpClient2::processCancel(const QByteArray& data)
{
	m_logFile.writeMessage(QString("processCancel() data.size() = %1, requestId = %2").arg(data.size()).arg(m_currentRequestId));

	// Parse protobuffer message
	//
	Network::GetAppSignalStatesFromArchiveCancelReply cancelReply;

	if (bool ok = cancelReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
		ok == false)
	{
		finish(tr("CancelReply data parsing error."));
		return;
	}

	// Process received data
	//
	QString errorString = QString::fromStdString(cancelReply.errorstring());

	if (errorString.isEmpty() == false)
	{
		finish(tr("processCancel() Received error from ArchiveService: requestId %1, error: %2.")
								.arg(m_currentRequestId)
								.arg(errorString));

		qDebug() << "RECEIVED ERROR:   ArchiveTcpClient::processStart, error = "
				 << errorString
				 << ", RequestID = " << m_currentRequestId;
		return;
	}

	finish();					// END OF REQUEST COMMUNICATION!

	return;
}

void ArchiveTcpClient2::updateStatistics(QString state)
{
	m_statMutex.lock();
	m_statState = state;
	m_statMutex.unlock();

	updateStatistics(0, 0, 0);
}

void ArchiveTcpClient2::updateStatistics(int incRequests, int incReplies, int incStates)
{
	m_statMutex.lock();

	m_statTcpRequestCount += incRequests;
	m_statTcpReplyCount += incReplies;
	m_statStateReceived += incStates;

	emit statistics(m_serverSettings.shortenId, m_statState, m_statTcpRequestCount, m_statTcpReplyCount, m_statStateReceived);

	m_statMutex.unlock();
	return;
}

