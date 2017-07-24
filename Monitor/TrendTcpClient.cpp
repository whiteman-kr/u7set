#include "TrendTcpClient.h"
#include "Settings.h"

TrendTcpClient::TrendTcpClient(MonitorConfigController* configController) :
	Tcp::Client(configController->configuration().archiveService1.address(),
				configController->configuration().archiveService2.address(),
				E::SoftwareType::Monitor, theSettings.instanceStrId(), -1, -1, -1),
	m_cfgController(configController)
{
	qDebug() << "TrendTcpClient::TrendTcpClient(...)";

	m_timerId = startTimer(theSettings.requestTimeInterval());

}

TrendTcpClient::~TrendTcpClient()
{
	qDebug() << "TrendTcpClient::~TrendTcpClient()";
}

void TrendTcpClient::timerEvent(QTimerEvent* )
{
	if (isClearToSendRequest() == false)
	{
		// Some communication in process
		//
		return;
	}

	resetRequestCycle();

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

//	theSignals.reset();
}

void TrendTcpClient::onConnection()
{
	qDebug() << "TrendTcpClient::onConnection()";

	assert(isClearToSendRequest() == true);

//	resetToGetSignalList();

	return;
}

void TrendTcpClient::onDisconnection()
{
	qDebug() << "TrendTcpClient::onDisconnection";

//	theSignals.invalidateAllSignalStates();

//	emit connectionReset();
}

void TrendTcpClient::onReplyTimeout()
{
	qDebug() << "TrendTcpClient::onReplyTimeout()";
}

void TrendTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
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
	QThread::msleep(theSettings.requestTimeInterval());

	if (m_queue.empty() == false &&
		isClearToSendRequest() == true)
	{
		requestStart();
	}

	return;
}

void TrendTcpClient::requestStart()
{
	assert(isClearToSendRequest());

	if (m_queue.empty() == true)
	{
		return;
	}

	m_currentRequest = m_queue.front();
	m_queue.pop();

	m_currentSignalHash = ::calcHash(m_currentRequest.appSignalId);

	m_startRequest.set_clientequipmentid(theSettings.instanceStrId().toStdString());
	m_startRequest.set_timetype(1);						// 0 Plan, 1 SystemTime, 2 LocalTyme, 3 ArchiveId
	m_startRequest.set_starttime(m_currentRequest.hourToRequest.timeStamp);
	m_startRequest.set_endtime(m_currentRequest.hourToRequest.timeStamp + 1_hour);
	m_startRequest.add_signalhashes(m_currentSignalHash);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_START, m_startRequest);

	return;
}

void TrendTcpClient::processStart(const QByteArray& data)
{
	bool ok = m_startReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetRequestCycle();
		return;
	}

	int error = m_startReply.error();
	int archError = m_startReply.archerror();
	m_currentRequestId = m_startReply.requestid();

	if (error != 0)
	{
		qDebug() << "RECEIVED ERROR:   TrendTcpClient::processStart, error = " << error << ", archError = " << archError << ", RequestID = " << m_currentRequestId;
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

	m_nextRequest.set_requestid(m_currentRequestId);

	sendRequest(ARCHS_GET_APP_SIGNALS_STATES_NEXT, m_nextRequest);

	return;
}

void TrendTcpClient::processNext(const QByteArray& data)
{
	bool ok = m_nextReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetRequestCycle();
		return;
	}

	int error = m_nextReply.error();
	int archError = m_nextReply.archerror();

	if (m_currentRequestId != m_nextReply.requestid())
	{
		assert(m_currentRequestId == m_nextReply.requestid());
		qDebug() << "TrendTcpClient::processNext, wrong RequestID, expected " << m_currentRequestId << ", received " << m_nextReply.requestid();
		resetRequestCycle();
		return;
	}

	if (error != 0)
	{
		qDebug() << "RECEIVED ERROR:   TrendTcpClient::processNext, error = " << error << ", archError = " << archError << ", RequestID = " << m_currentRequestId;
		resetRequestCycle();
		return;
	}

	if (m_nextReply.dataready() == false)
	{
		// Data not ready yet, request next one more time
		//
		QThread::msleep(theSettings.requestTimeInterval());
		requestNext();

		return;
	}

	// Parse data
	//
	//m_currentSignalHash

	int stateCount = m_nextReply.appsignalstates_size();
	assert(m_nextReply.sentstatescount() == stateCount);

	for (int i = 0; i < stateCount; i++)
	{
		const ::Proto::AppSignalState& stateMessage = m_nextReply.appsignalstates(i);

		AppSignalState s;
		Hash hash = s.load(stateMessage);

		if (hash != m_currentSignalHash)
		{
			assert(hash == m_currentSignalHash);
		}
	}

	// Request next or stop communication
	//
	if (m_nextReply.islastpart() == true)
	{
		return;
	}
	else
	{
		requestNext();
	}

	return;
}

void TrendTcpClient::slot_requestData(QString appSignalId, TimeStamp hourToRequest)
{
	qDebug() << "TrendTcpClient::slot_requestData, AppSignalID = " << appSignalId << ", Time = " << hourToRequest.toDateTime();

	RequestQueue request;

	request.appSignalId = appSignalId;
	request.hourToRequest = hourToRequest;

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
