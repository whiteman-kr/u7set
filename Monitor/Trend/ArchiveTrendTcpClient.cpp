#include "ArchiveTrendTcpClient.h"
#include "Settings.h"

ArchiveTrendTcpClient::ArchiveTrendTcpClient(MonitorConfigController* configController) :
	Tcp::Client(configController->softwareInfo(),
				configController->configuration().archiveService1.address(),
				configController->configuration().archiveService2.address()),
	m_cfgController(configController)
{
	qDebug() << "ArchiveTrendTcpClient::ArchiveTrendTcpClient(...)";

	setObjectName("ArchiveTrendTcpClient");
	enableWatchdogTimer(false);

	qRegisterMetaType<TrendLib::TrendStateItem>("TrendLib::TrendStateItem");
	qRegisterMetaType<std::shared_ptr<TrendLib::OneHourData>>("shared_ptr<TrendLib::OneHourData>>");
	qRegisterMetaType<std::shared_ptr<TrendLib::RealtimeData>>("shared_ptr<TrendLib::RealtimeData>>");

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
		QString stat = QString("%1 - %2").arg(m_currentRequest.appSignalId).arg(m_currentRequest.hourToRequest.toDateTime().toString("dd.MM.yyyy hh:mm"));
		setStatText(stat);
	}
	else
	{
		setStatText(QString());
	}

	setStatRequestQueueSize(static_cast<int>(m_queue.size()));

	if (event->timerId() == m_periodicTimerId &&
		requestInProgress == false &&
		isClearToSendRequest() == true)
	{
		resetRequestCycle();
		return;
	}

	return;
}

void ArchiveTrendTcpClient::onClientThreadStarted()
{
	qDebug() << "ArchiveTrendTcpClient::onClientThreadStarted()";

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &ArchiveTrendTcpClient::slot_configurationArrived,
			Qt::QueuedConnection);

	m_periodicTimerId = startTimer(theSettings.requestTimeInterval());	// Start it here, as this function is running in the right thread

	return;
}

void ArchiveTrendTcpClient::onClientThreadFinished()
{
	qDebug() << "ArchiveTrendTcpClient::onClientThreadFinished()";
}

void ArchiveTrendTcpClient::onConnection()
{
	qDebug() << "ArchiveTrendTcpClient::onConnection()";
	Q_ASSERT(isClearToSendRequest() == true);

	return;
}

void ArchiveTrendTcpClient::onDisconnection()
{
	qDebug() << "ArchiveTrendTcpClient::onDisconnection";
	requestInProgress = false;
}

void ArchiveTrendTcpClient::onReplyTimeout()
{
	qDebug() << "ArchiveTrendTcpClient::onReplyTimeout()";
	requestInProgress = false;
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

		resetRequestCycle();
	}

	return;
}

void ArchiveTrendTcpClient::resetRequestCycle()
{
	QThread::msleep(0);

	if (m_queue.empty() == false &&
		requestInProgress == false &&
		isClearToSendRequest() == true)
	{
		requestStart();
	}
	else
	{
	}

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

	m_currentRequest = m_queue.back();	// THESE TWO LINES MUST BE IN CONSISTENCY!!!!
	m_queue.pop_back();					// Take the last one, let's assume it is shown now and more important

	incStatRequestCount();
	requestInProgress = true;

	m_currentSignalHash = ::calcHash(m_currentRequest.appSignalId);

	m_startRequest.Clear();
	m_startRequest.set_clientequipmentid(theSettings.instanceStrId().toStdString());

	m_startRequest.set_timetype(static_cast<int>(m_currentRequest.timeType));		// enum TymeType: 0 Plan, 1 SystemTime, 2 LocalTyme, 3 ArchiveId
	m_startRequest.set_starttime(m_currentRequest.hourToRequest.timeStamp);
	m_startRequest.set_endtime(m_currentRequest.hourToRequest.timeStamp + 1_hour);
	m_startRequest.add_signalhashes(m_currentSignalHash);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START, m_startRequest);

	m_startRequestTime.start();

	m_receivedData = std::make_shared<TrendLib::OneHourData>();

	return;
}

void ArchiveTrendTcpClient::processStart(const QByteArray& data)
{
	qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: " << m_startRequestTime.elapsed();

	bool ok = m_startReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

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
		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);
		requestInProgress = false;

		qDebug() << "RECEIVED ERROR:   TrendTcpClient::processStart, error = " << error
				 << ", archError = " << archError
				 << ", RequestID = " << m_currentRequestId;

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
	bool ok = m_nextReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		Q_ASSERT(ok);

		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);
		requestInProgress = false;

		resetRequestCycle();
		return;
	}

	int error = m_nextReply.error();
	int archError = m_nextReply.archerror();

	if (m_currentRequestId != m_nextReply.requestid())
	{
		Q_ASSERT(m_currentRequestId == m_nextReply.requestid());

		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);
		requestInProgress = false;

		qDebug() << "TrendTcpClient::processNext, wrong RequestID, expected " << m_currentRequestId
				 << ", received " << m_nextReply.requestid();

		resetRequestCycle();
		return;
	}

	if (error != 0)
	{
		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);
		requestInProgress = false;

		qDebug() << "ERROR: TrendTcpClient::processNext, AppSignalID = " << m_currentRequest.appSignalId
				 << ", error = " << error
				 << ", archError = " << archError
				 << ", RequestID = " << m_currentRequestId
				 << ", requestedTime = " << m_currentRequest.hourToRequest.toDateTime();

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

	//qDebug() << "TrendTcpClient::processNext, stateCount " << stateCount;

	// --
	//
	TrendLib::TrendStateRecord* record = nullptr;

	if (stateCount != 0)
	{
		if (m_receivedData->data.empty() == true)
		{
			m_receivedData->data.emplace_back();
			m_receivedData->data.back().states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
		}

		record = &m_receivedData->data.back();

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
					m_receivedData->data.emplace_back();
					m_receivedData->data.back().states.reserve(TrendLib::TrendStateRecord::RecomendedSize);

					record = &m_receivedData->data.back();
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
		qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_NEXT Reqest->Reply time: " << m_startRequestTime.elapsed();
		Q_ASSERT(m_receivedData);

		m_receivedData->state = TrendLib::OneHourData::State::Received;

		emit dataReady(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType, m_receivedData);

		requestInProgress = false;			// END OF REQUEST COMMUNICATION!
		m_receivedData.reset();

		resetRequestCycle();				// start new cycle
	}
	else
	{
		// Request next part
		//
		requestNext();
	}

	return;
}

void ArchiveTrendTcpClient::slot_requestData(QString appSignalId, TimeStamp hourToRequest, E::TimeType timeType)
{
	//qDebug() << "ArchiveTrendTcpClient::slot_requestData, AppSignalID = " << appSignalId << ", Time = " << hourToRequest.toDateTime();

	RequestQueue request;

	request.appSignalId = appSignalId;
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

	return;
}

void ArchiveTrendTcpClient::slot_configurationArrived(ConfigSettings configuration)
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

ArchiveTrendTcpClient::Stat ArchiveTrendTcpClient::stat() const
{
	ArchiveTrendTcpClient::Stat result;

	m_statMutex.lock();
	result = m_stat;
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
	m_stat.requestCount ++;
	m_statMutex.unlock();

	return;
}

void ArchiveTrendTcpClient::incStatReplyCount()
{
	m_statMutex.lock();
	m_stat.replyCount ++;
	m_statMutex.unlock();

	return;
}
