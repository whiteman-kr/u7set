#include "TrendTcpClient.h"
#include "Settings.h"

TrendTcpClient::TrendTcpClient(MonitorConfigController* configController) :
	Tcp::Client(configController->configuration().archiveService1.address(),
				configController->configuration().archiveService2.address(),
				E::SoftwareType::Monitor, theSettings.instanceStrId(), -1, -1, -1),
	m_cfgController(configController)
{
	qDebug() << "TrendTcpClient::TrendTcpClient(...)";

	enableWatchdogTimer(false);

	m_periodicTimerId = startTimer(theSettings.requestTimeInterval());

	qRegisterMetaType<std::shared_ptr<TrendLib::OneHourData>>("share_ptr<TrendLib::OneHourData>>");

	return;
}

TrendTcpClient::~TrendTcpClient()
{
	qDebug() << "TrendTcpClient::~TrendTcpClient()";
}

void TrendTcpClient::timerEvent(QTimerEvent* event)
{
	if (requestInProgress == true)
	{
		m_statRequestDescription = QString("%1 - %2").arg(m_currentRequest.appSignalId).arg(m_currentRequest.hourToRequest.toDateTime().toString("dd.MM.yyyy hh:mm"));
	}
	else
	{
		m_statRequestDescription.clear();
	}
	m_statRequestQueueSize = static_cast<int>(m_queue.size());

	if (event->timerId() == m_periodicTimerId &&
		requestInProgress == false &&
		isClearToSendRequest() == true)
	{
		resetRequestCycle();
		return;
	}

	return;
}

void TrendTcpClient::onClientThreadStarted()
{
	qDebug() << "TcpSignalClient::TrendTcpClient()";

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &TrendTcpClient::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void TrendTcpClient::onClientThreadFinished()
{
	qDebug() << "TrendTcpClient::onClientThreadFinished()";
}

void TrendTcpClient::onConnection()
{
	qDebug() << "TrendTcpClient::onConnection()";
	assert(isClearToSendRequest() == true);

	return;
}

void TrendTcpClient::onDisconnection()
{
	qDebug() << "TrendTcpClient::onDisconnection";
}

void TrendTcpClient::onReplyTimeout()
{
	qDebug() << "TrendTcpClient::onReplyTimeout()";
}

void TrendTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
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

	default:
		assert(false);
		qDebug() << "Wrong requestID in TrendTcpClient::processReply() " << requestID;

		resetRequestCycle();
	}

	return;
}

void TrendTcpClient::resetRequestCycle()
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

void TrendTcpClient::requestStart()
{
	assert(isClearToSendRequest());

	if (m_queue.empty() == true)
	{
		requestInProgress = false;
		return;
	}

	m_currentRequest = m_queue.front();
	m_queue.pop();

	m_statTcpRequestCount ++;
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

void TrendTcpClient::processStart(const QByteArray& data)
{
	qDebug() << "ARCHS_GET_APP_SIGNALS_STATES_START Reqest->Reply time: " << m_startRequestTime.elapsed();

	bool ok = m_startReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);

		requestInProgress = false;
		assert(ok);
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

void TrendTcpClient::requestNext()
{
	assert(isClearToSendRequest());
	assert(m_currentRequestId != 0);

	m_nextRequest.Clear();
	m_nextRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT, m_nextRequest);

	m_statTcpRequestCount ++;

	return;
}

void TrendTcpClient::processNext(const QByteArray& data)
{
	bool ok = m_nextReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);

		emit requestError(m_currentRequest.appSignalId, m_currentRequest.hourToRequest, m_currentRequest.timeType);
		requestInProgress = false;

		resetRequestCycle();
		return;
	}

	int error = m_nextReply.error();
	int archError = m_nextReply.archerror();

	if (m_currentRequestId != m_nextReply.requestid())
	{
		assert(m_currentRequestId == m_nextReply.requestid());

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
		//QThread::msleep(theSettings.requestTimeInterval());
		requestNext();
		return;
	}

	// Parse data
	//
	assert(m_receivedData);

	int stateCount = m_nextReply.appsignalstates_size();

	// --
	//
	TrendLib::TrendStateRecord* record = nullptr;

	if (m_receivedData->data.empty() == true)
	{
		m_receivedData->data.emplace_back();
		m_receivedData->data.back().states.reserve(TrendLib::TrendStateRecord::recomendedSize);
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
			assert(hash == m_currentSignalHash);
		}
		else
		{
			assert(record);

			if (record->states.size() >= record->states.max_size())
			{
				m_receivedData->data.emplace_back();
				m_receivedData->data.back().states.reserve(TrendLib::TrendStateRecord::recomendedSize);

				record = &m_receivedData->data.back();
				assert(record);
			}

			record->states.emplace_back(s);
		}
	}

	// Request next or stop communication
	//
	if (m_nextReply.islastpart() == true)
	{
		assert(m_receivedData);

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

void TrendTcpClient::slot_requestData(QString appSignalId, TimeStamp hourToRequest, TimeType timeType)
{
	//qDebug() << "TrendTcpClient::slot_requestData, AppSignalID = " << appSignalId << ", Time = " << hourToRequest.toDateTime();

	RequestQueue request;

	request.appSignalId = appSignalId;
	request.hourToRequest = hourToRequest;
	request.timeType = timeType;

	m_queue.push(request);

	return;
}

void TrendTcpClient::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort s1 = configuration.archiveService1.address();
	HostAddressPort s2 = configuration.archiveService2.address();

	if (serverAddressPort(0) == s1 ||
		serverAddressPort(1) != s2)
	{
		setServers(s1, s2, true);
	}

	return;
}
