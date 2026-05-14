#include "ArchiveTrendTcpClient.h"
#include "MonitorAppSettings.h"


ArchiveTrendTcpClient::ArchiveTrendTcpClient(const SoftwareInfo& softwareInfo,
											 const SoftwareEndpoint::ArchiveService& server,
											 ILogFile* logFile) :
	Tcp::Client(softwareInfo, server.address, server.address, "ArchiveTrendTcpClient", server.equipmentId),
	TcpClientStatistics(this),
	m_server(server),
	m_logFile(logFile, "ArchiveTrendTcpClient")
{
	Q_ASSERT(logFile);

	m_logFile.writeMessage("ArchiveTrendTcpClient::ArchiveTrendTcpClient(), address " + m_server.address.toString());
	qDebug() << "ArchiveTrendTcpClient::ArchiveTrendTcpClient(...), address " << m_server.address.toString();

	setObjectName("ArchiveTrendTcpClient");

	qRegisterMetaType<TrendLib::TrendStateItem>("TrendLib::TrendStateItem");
	qRegisterMetaType<std::shared_ptr<TrendLib::OneHourData>>("shared_ptr<TrendLib::OneHourData>>");
	qRegisterMetaType<std::shared_ptr<TrendLib::RealtimeData>>("shared_ptr<TrendLib::RealtimeData>>");

	connect(this,
			&Tcp::Client::signal_wrongServerID,
			[this](const QString& errorMessage)
			{
				m_logFile.writeError(errorMessage);
			});

	return;
}

ArchiveTrendTcpClient::~ArchiveTrendTcpClient()
{
	qDebug() << "ArchiveTrendTcpClient::~ArchiveTrendTcpClient()";
}

void ArchiveTrendTcpClient::timerEvent(QTimerEvent* event)
{
	if (requestInProgress == true)
	{
		QString stat = QString("%1 - %2")
						   .arg(m_currentRequest.signalPlusServerId.appSignalId)
						   .arg(DateTimeToString::dateTimeSec(m_currentRequest.hourToRequest.toDateTime())); // Should add seconds???
		setStatText(stat);
	}
	else
	{
		setStatText(QString());
	}

	setStatRequestQueueSize(static_cast<int>(m_queue.size()));

	if (event->timerId() == m_periodicTimerId && requestInProgress == false && isClearToSendRequest() == true)
	{
		resetRequestCycle();
		return;
	}

	return;
}

void ArchiveTrendTcpClient::onClientThreadStarted()
{
	qDebug() << "ArchiveTrendTcpClient::onClientThreadStarted()";
	m_logFile.writeMessage("onClientThreadStarted()");

	m_periodicTimerId =
		startTimer(MonitorAppSettings::instance().requestTimeInterval()); // Start it here, as this function is running in the right thread

	return;
}

void ArchiveTrendTcpClient::onClientThreadFinished()
{
	qDebug() << "ArchiveTrendTcpClient::onClientThreadFinished()";
	m_logFile.writeMessage("onClientThreadFinished()");
}

void ArchiveTrendTcpClient::onConnection()
{
	qDebug() << "ArchiveTrendTcpClient::onConnection()";
	m_logFile.writeMessage("onConnection()");

	Q_ASSERT(isClearToSendRequest() == true);

	return;
}

void ArchiveTrendTcpClient::onDisconnection()
{
	qDebug() << "ArchiveTrendTcpClient::onDisconnection";
	m_logFile.writeMessage("onDisconnection()");

	requestInProgress = false;

	return;
}

void ArchiveTrendTcpClient::onReplyTimeout()
{
	qDebug() << "ArchiveTrendTcpClient::onReplyTimeout()";
	m_logFile.writeWarning("onReplyTimeout()");

	requestInProgress = false;

	return;
}

void ArchiveTrendTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	incStatReplyCount();

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

	default:
		Q_ASSERT(false);

		qDebug() << "Wrong requestID in TrendTcpClient::processReply() " << requestID;
		m_logFile.writeError(QString("processReply(), Wrong requestId %1").arg(requestID));

		resetRequestCycle();
	}

	return;
}

void ArchiveTrendTcpClient::resetRequestCycle()
{
	QThread::msleep(0);

	if (m_queue.empty() == false && requestInProgress == false && isClearToSendRequest() == true)
	{
		requestStart();
	}
	else {}

	return;
}

void ArchiveTrendTcpClient::requestStart()
{
	Q_ASSERT(isClearToSendRequest());

	if (m_queue.empty() == true)
	{
		requestInProgress = false;
		return;
	}

	m_currentRequest = m_queue.back(); // THESE TWO LINES MUST BE IN CONSISTENCY!!!!
	m_queue.pop_back();                // Take the last one, let's assume it is shown now and more important

	incStatRequestCount();
	requestInProgress = true;

	m_currentSignalHash = ::calcHash(m_currentRequest.signalPlusServerId.appSignalId);

	m_startRequest.Clear();
	m_startRequest.set_clientequipmentid(MonitorAppSettings::instance().equipmentId().toStdString());

	m_startRequest.set_timetype(
		static_cast<int>(m_currentRequest.timeType)); // enum TymeType: 0 Plan, 1 SystemTime, 2 LocalTyme, 3 ArchiveId
	m_startRequest.set_starttime(m_currentRequest.hourToRequest.timeStamp);
	m_startRequest.set_endtime(m_currentRequest.hourToRequest.timeStamp + 1_hour);
	m_startRequest.add_signalhashes(m_currentSignalHash);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START, m_startRequest);

	m_startRequestTime.start();

	m_receivedData = std::make_shared<TrendLib::OneHourData>();

	m_logFile.writeMessage(
		QString("requestStart(), sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START...), %1").arg(m_currentRequest.toString()));
	return;
}

void ArchiveTrendTcpClient::processStart(const QByteArray& data)
{
	qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: " << m_startRequestTime.elapsed();
	m_logFile.writeMessage(QString("processStart(), ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: %1 ms, m_currentRequest %2")
							   .arg(m_startRequestTime.elapsed())
							   .arg(m_currentRequest.toString()));

	Q_ASSERT(m_connectedSoftwareInfo.equipmentID() == m_currentRequest.signalPlusServerId.archiveServerId);

	// --
	//
	bool ok = m_startReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		emit requestError(m_currentRequest.signalPlusServerId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

		requestInProgress = false;
		Q_ASSERT(ok);
		resetRequestCycle();
		return;
	}

	int error = m_startReply.error();
	int archError = m_startReply.archerror();
	m_currentRequestId = m_startReply.requestid();

	if (error != 0)
	{
		emit requestError(m_currentRequest.signalPlusServerId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

		requestInProgress = false;

		qDebug() << "RECEIVED ERROR:   TrendTcpClient::processStart, error = " << error << ", archError = " << archError
				 << ", RequestID = " << m_currentRequestId;

		m_logFile.writeError(QString("processStart(), error: %1, m_currentRequest %2").arg(archError).arg(m_currentRequest.toString()));

		resetRequestCycle();
		return;
	}

	requestNext();
	return;
}

void ArchiveTrendTcpClient::requestNext()
{
	Q_ASSERT(isClearToSendRequest());
	Q_ASSERT(m_currentRequestId != 0);

	m_nextRequest.Clear();
	m_nextRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT, m_nextRequest);

	incStatRequestCount();

	return;
}

void ArchiveTrendTcpClient::processNext(const QByteArray& data)
{
	bool ok = m_nextReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
	Q_ASSERT(ok);

	if (ok == false)
	{
		emit requestError(m_currentRequest.signalPlusServerId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

		requestInProgress = false;

		resetRequestCycle();
		return;
	}

	int error = m_nextReply.error();
	int archError = m_nextReply.archerror();

	if (m_currentRequestId != m_nextReply.requestid())
	{
		Q_ASSERT(m_currentRequestId == m_nextReply.requestid());

		emit requestError(m_currentRequest.signalPlusServerId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

		requestInProgress = false;

		qDebug() << "TrendTcpClient::processNext, wrong RequestID, expected " << m_currentRequestId << ", received "
				 << m_nextReply.requestid();

		m_logFile.writeError(
			QString("processNext(), wrong RequestId, expected %1, received %2").arg(m_currentRequestId).arg(m_nextReply.requestid()));

		resetRequestCycle();
		return;
	}

	if (error != 0)
	{
		emit requestError(m_currentRequest.signalPlusServerId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

		requestInProgress = false;

		qDebug() << "ERROR: TrendTcpClient::processNext, AppSignalID = " << m_currentRequest.signalPlusServerId.appSignalId
				 << ", error = " << error << ", archError = " << archError << ", RequestID = " << m_currentRequestId
				 << ", requestedTime = " << m_currentRequest.hourToRequest.toDateTime();

		m_logFile.writeError(
			QString("processNext(), error: %1, archError: %2, request: %3").arg(error).arg(archError).arg(m_currentRequest.toString()));

		resetRequestCycle();
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
	Q_ASSERT(m_receivedData);

	int stateCount = m_nextReply.appsignalstates_size();

	// qDebug() << "TrendTcpClient::processNext, stateCount " << stateCount;

	// --
	//
	TrendLib::TrendStateRecord* record = nullptr;

	if (stateCount != 0)
	{
		if (m_receivedData->data_.empty() == true)
		{
			m_receivedData->data_.emplace_back();
			m_receivedData->data_.back().states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
		}

		record = &m_receivedData->data_.back();

		// --
		//
		for (int i = 0; i < stateCount; i++)
		{
			const ::Proto::AppSignalState& stateMessage = m_nextReply.appsignalstates(i);

			AppSignalState s;
			Hash hash = s.load(stateMessage);

			if (hash != m_currentSignalHash)
			{
				Q_ASSERT(hash == m_currentSignalHash);
			}
			else
			{
				Q_ASSERT(record);

				if (record->states.size() >= record->states.max_size())
				{
					m_receivedData->data_.emplace_back();
					m_receivedData->data_.back().states.reserve(TrendLib::TrendStateRecord::RecomendedSize);

					record = &m_receivedData->data_.back();
					Q_ASSERT(record);
				}

				record->states.emplace_back(s);
			}
		}
	}

	// Request next or stop communication
	//
	if (m_nextReply.islastpart() == true)
	{
		qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_NEXT Request->Reply time: " << m_startRequestTime.elapsed();
		m_logFile.writeMessage(QString("processNext(), Requested completed, time: %1 ms, request: %2")
								   .arg(m_startRequestTime.elapsed())
								   .arg(m_currentRequest.toString()));

		Q_ASSERT(m_receivedData);

		m_receivedData->state_ = TrendLib::OneHourData::State::Received;

		emit dataReady(m_currentRequest.signalPlusServerId, m_currentRequest.hourToRequest, m_currentRequest.timeType, m_receivedData);

		requestInProgress = false; // END OF REQUEST COMMUNICATION!
		m_receivedData.reset();

		resetRequestCycle();       // start new cycle
	}
	else
	{
		// Request next part
		//
		requestNext();
	}

	return;
}

void ArchiveTrendTcpClient::slot_requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId,
											 TimeStamp hourToRequest,
											 E::TimeType timeType)
{
	RequestQueue request;

	request.signalPlusServerId = signalPlusServerId;
	request.hourToRequest = hourToRequest;
	request.timeType = timeType;

	// Check if such request already in the queue
	//
	for (const RequestQueue& rq : m_queue)
	{
		if (rq == request)
		{
			return;
		}
	}

	// Add request to the queue
	//
	m_queue.push_back(request);

	m_logFile.writeMessage(QString("slot_requestData(), %1").arg(request.toString()));
	return;
}

ArchiveTrendTcpClient::Stat ArchiveTrendTcpClient::stat() const
{
	ArchiveTrendTcpClient::Stat result;

	m_statMutex.lock();
	result = m_stat;
	result.isConnected = static_cast<int>(this->isConnected());
	m_statMutex.unlock();

	return result;
}

void ArchiveTrendTcpClient::setStat(const Stat& stat)
{
	m_statMutex.lock();
	m_stat = stat;
	m_statMutex.unlock();

	return;
}

void ArchiveTrendTcpClient::setStatText(const QString& text)
{
	m_statMutex.lock();
	m_stat.text = text;
	m_statMutex.unlock();

	return;
}

void ArchiveTrendTcpClient::setStatRequestQueueSize(int value)
{
	m_statMutex.lock();
	m_stat.requestQueueSize = value;
	m_statMutex.unlock();

	return;
}

void ArchiveTrendTcpClient::incStatRequestCount()
{
	m_statMutex.lock();
	m_stat.requestCount++;
	m_statMutex.unlock();

	return;
}

void ArchiveTrendTcpClient::incStatReplyCount()
{
	m_statMutex.lock();
	m_stat.replyCount++;
	m_statMutex.unlock();

	return;
}
