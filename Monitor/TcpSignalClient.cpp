#include "TcpSignalClient.h"
#include "MonitorAppSettings.h"

TcpSignalClient::TcpSignalClient(MonitorConfigController* configController, ILogFile* logFile) :
	Tcp::Client(configController->softwareInfo(),
				HostAddressPort{configController->configuration().appDataService1.address()},
				HostAddressPort{configController->configuration().appDataService2.address()},
				"TcpSignalClient"),
	TcpClientStatistics(this),
	m_cfgController(configController),
	m_logFile(logFile, "TcpSignalClient")
{
	Q_ASSERT(m_cfgController);
	Q_ASSERT(logFile);

	setObjectName("TcpSignalClient");

	qDebug() << "TcpSignalClient::TcpSignalClient(...)";

	qDebug() << m_cfgController->configuration().appDataService1.ip();
	qDebug() << m_cfgController->configuration().appDataService2.ip();

	m_startStateTimerId = startTimer(MonitorAppSettings::instance().requestTimeInterval());
}

TcpSignalClient::~TcpSignalClient()
{
	qDebug() << "TcpSignalClient::~TcpSignalClient()";
}

void TcpSignalClient::timerEvent(QTimerEvent* /*event*/)
{
	return;
}

void TcpSignalClient::onClientThreadStarted()
{
	qDebug() << "TcpSignalClient::onClientThreadStarted()";
	m_logFile.writeMessage("onClientThreadStarted()");

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &TcpSignalClient::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void TcpSignalClient::onClientThreadFinished()
{
	qDebug() << "TcpSignalClient::onClientThreadFinished()";
	m_logFile.writeMessage("onClientThreadFinished()");

	theSignals.reset();
}

void TcpSignalClient::onConnection()
{
	qDebug() << "TcpSignalClient::onConnection()";
	m_logFile.writeMessage("oConnection()");

	Q_ASSERT(isClearToSendRequest() == true);

	resetToGetSignalList();

	return;
}

void TcpSignalClient::onDisconnection()
{
	qDebug() << "TcpSignalClient::onDisconnection";
	m_logFile.writeMessage("onDisconnection()");

	theSignals.invalidateSignalStates();

	emit connectionReset();
}

void TcpSignalClient::onReplyTimeout()
{
	qDebug() << "TcpSignalClient::onReplyTimeout()";
	m_logFile.writeWarning("onReplyTimeout()");
}

void TcpSignalClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		Q_ASSERT(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{
	case ADS_GET_APP_SIGNAL_LIST_START:
		processSignalListStart(data);
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		processSignalListNext(data);
		break;

	case ADS_GET_APP_SIGNAL_PARAM:
		processSignalParam(data);
		break;

	case ADS_GET_APP_SIGNAL_STATE_CHANGES:
		processSignalStateChanges(data);
		break;

	case ADS_GET_APP_SIGNAL_STATE:
		processSignalState(data);
		break;

	default:
		Q_ASSERT(false);
		qDebug() << "Wrong requestID in TcpSignalClient::processReply()";
		m_logFile.writeError(QString("Wrong requestID in TcpSignalClient::processReply(), %1").arg(requestID));

		resetToGetState(true);
	}

	return;
}

void TcpSignalClient::resetToGetSignalList()
{
	QThread::msleep(MonitorAppSettings::instance().requestTimeInterval());

	theSignals.reset();
	m_signalList.clear();
	m_lastSignalParamStartIndex = 0;
	m_lastSignalStateStartIndex = 0;

	requestSignalListStart();
	return;
}

void TcpSignalClient::resetToGetState(bool resetStateIndex)
{
	QThread::msleep(MonitorAppSettings::instance().requestTimeInterval());

	if (resetStateIndex == true)
	{
		m_lastSignalStateStartIndex = 0;
	}

	if (m_signalList.empty() == false)
	{
		requestSignalStateChanges();
	}
	else
	{
		// There is no signals stae to request list again
		//
		resetToGetSignalList();
	}

	return;
}

void TcpSignalClient::requestSignalListStart()
{
	Q_ASSERT(isClearToSendRequest());
	sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
}

void TcpSignalClient::processSignalListStart(const QByteArray& data)
{
	bool ok = m_getSignalListStartReply.ParseFromArray(data.constData(), data.size());
	if (ok == false)
	{
		Q_ASSERT(ok);
		resetToGetSignalList();
		return;
	}

	if (m_getSignalListStartReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalListNext, error received: " << m_getSignalListStartReply.error();
		m_logFile.writeError(QString("processSignalListNext, error received: %1").arg(m_getSignalListStartReply.error()));

		Q_ASSERT(m_getSignalListStartReply.error() != 0);

		resetToGetSignalList();
		return;
	}

	qDebug() << "----------------- processSignalListStart -----------------";
	qDebug() << "error: " << m_getSignalListStartReply.error();
	qDebug() << "totalItemCount: " << m_getSignalListStartReply.totalitemcount();
	qDebug() << "partCount: " << m_getSignalListStartReply.partcount();
	qDebug() << "itemsPerPart: " << m_getSignalListStartReply.itemsperpart();

	m_logFile.writeMessage("----------------- processSignalListStart -----------------");
	if (m_getSignalListStartReply.error() == 0)
	{
		m_logFile.writeMessage(QString("-- error: %1").arg(m_getSignalListStartReply.error()));
	}
	else
	{
		m_logFile.writeError(QString("-- error: %1").arg(m_getSignalListStartReply.error()));
	}
	m_logFile.writeMessage(QString("-- totalItemCount: %1").arg(m_getSignalListStartReply.totalitemcount()));
	m_logFile.writeMessage(QString("-- partCount: %1").arg(m_getSignalListStartReply.partcount()));
	m_logFile.writeMessage(QString("-- itemsPerPart: %1").arg(m_getSignalListStartReply.itemsperpart()));

	if (m_getSignalListStartReply.totalitemcount() == 0 ||
		m_getSignalListStartReply.partcount() == 0)
	{
		// There is no signals, useless but can be
		//
		Q_ASSERT(m_getSignalListStartReply.totalitemcount() == 0);
		Q_ASSERT(m_getSignalListStartReply.partcount() == 0);

		m_signalList.clear();

		// request params
		//
		requestSignalParam(0);
		return;
	}

	m_signalList.clear();
	m_signalList.reserve(m_getSignalListStartReply.totalitemcount());

	requestSignalListNext(0);

	return;
}

void TcpSignalClient::requestSignalListNext(int part)
{
	Q_ASSERT(isClearToSendRequest());

	// if all parts were requested then sitch to next reply
	//
	if (part >= m_getSignalListStartReply.partcount())
	{
		if (m_signalList.size() != m_getSignalListStartReply.totalitemcount())
		{
			Q_ASSERT(m_signalList.size() != m_getSignalListStartReply.totalitemcount());
		}

		// Request params
		//
		requestSignalParam(0);
		return;
	}

	// Request part, partNo is set in processSignalListStart and is incremented in processSignalListNext
	//
	m_getSignalListNextRequest.set_part(part);

	sendRequest(ADS_GET_APP_SIGNAL_LIST_NEXT, m_getSignalListNextRequest);
	return;
}

void TcpSignalClient::processSignalListNext(const QByteArray& data)
{
	bool ok = m_getSignalListNextReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		Q_ASSERT(ok);
		resetToGetSignalList();
		return;
	}

	if (m_getSignalListNextReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalListNext, error received: " << m_getSignalListNextReply.error();
		m_logFile.writeError(QString("processSignalListNext, error received: %1").arg(m_getSignalListNextReply.error()));

		Q_ASSERT(m_getSignalListNextReply.error() != 0);

		resetToGetSignalList();
		return;
	}

	if (m_getSignalListNextReply.part() != m_getSignalListNextRequest.part())
	{
		// Asked for one part but got different
		//
		Q_ASSERT(m_getSignalListNextReply.part() == m_getSignalListNextRequest.part());
		resetToGetSignalList();
		return;
	}

	qDebug() << "----------------- processSignalListNext -----------------";
	qDebug() << "error: " << m_getSignalListNextReply.error();
	qDebug() << "part: " << m_getSignalListNextReply.part();

	m_logFile.writeMessage("----------------- processSignalListNext -----------------");
	if (m_getSignalListNextReply.error() == 0)
	{
		m_logFile.writeMessage(QString("-- error: %1").arg(m_getSignalListNextReply.error()));
	}
	else
	{
		m_logFile.writeError(QString("-- error: %1").arg(m_getSignalListNextReply.error()));
	}
	m_logFile.writeMessage(QString("-- part: %1").arg(m_getSignalListNextReply.part()));

	for (int i = 0; i < m_getSignalListNextReply.appsignalids_size(); i++)
	{
		m_signalList.push_back(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));
	}

	// Next request
	//
	requestSignalListNext(m_getSignalListNextReply.part() + 1);

	return;
}

// AppSignalParam
//
void TcpSignalClient::requestSignalParam(int startIndex)
{
	Q_ASSERT(isClearToSendRequest());
	m_lastSignalParamStartIndex = startIndex;

	if (startIndex == 0)
	{
		theSignals.reset();
	}

	if (startIndex >= m_signalList.size())
	{
		emit signalParamAndUnitsArrived();

		resetToGetState(true);	// END OF RECEIVING SIGNALS PARAMS,
								// Here the new loop starts!!!
		return;
	}

	m_getSignalParamRequest.mutable_signalhashes()->Clear();
	m_getSignalParamRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_PARAM_MAX);

	for (int i = startIndex;
		 i < startIndex + ADS_GET_APP_SIGNAL_PARAM_MAX && i < m_signalList.size();
		 i++)
	{
		Hash signalHash = calcHash(m_signalList[i]);
		m_getSignalParamRequest.add_signalhashes(signalHash);
	}

	sendRequest(ADS_GET_APP_SIGNAL_PARAM, m_getSignalParamRequest);
	return;
}

void TcpSignalClient::processSignalParam(const QByteArray& data)
{
	bool ok = m_getSignalParamReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		Q_ASSERT(ok);
		resetToGetSignalList();
		return;
	}

	if (m_getSignalParamReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalParam, error received: " << m_getSignalParamReply.error();
		m_logFile.writeError(QString("processSignalParam, error received: %1").arg(m_getSignalParamReply.error()));

		Q_ASSERT(m_getSignalParamReply.error() != 0);

		resetToGetState(true);
		return;
	}

	std::vector<AppSignalParam> appSignals;
	appSignals.reserve(m_getSignalParamReply.appsignals_size());

	for (int i = 0; i < m_getSignalParamReply.appsignals_size(); i++)
	{
		const ::Proto::AppSignal& protoSignal = m_getSignalParamReply.appsignals(i);

		AppSignalParam& s = appSignals.emplace_back();
		s.load(protoSignal);

		if (s.hash() == 0 || s.appSignalId().isEmpty() == true)
		{
			qDebug() << s.appSignalId();
			qDebug() << s.caption();

			Q_ASSERT(s.hash() != 0);
			Q_ASSERT(s.appSignalId().isEmpty() == false);

			appSignals.pop_back();
		}
	}

	theSignals.addSignals(appSignals);

	requestSignalParam(m_lastSignalParamStartIndex + ADS_GET_APP_SIGNAL_PARAM_MAX);

	return;
}

// AppSignalStateChanges
//
void TcpSignalClient::requestSignalStateChanges()
{
	Q_ASSERT(isClearToSendRequest());

	sendRequest(ADS_GET_APP_SIGNAL_STATE_CHANGES, m_getSignalStateChangesRequest);

	return;
}

void TcpSignalClient::processSignalStateChanges(const QByteArray& data)
{
	if (bool ok = m_getSignalStateChangesReply.ParseFromArray(data.constData(), data.size());
		ok == false)
	{
		Q_ASSERT(ok);
		resetToGetState(true);
		return;
	}

//	optional int32 error = 1 [default = 0];
//	optional int64 serverTimeUtc = 2;
//	optional int64 serverTimeLocal = 3;
//	optional int32 pendingStatesCount = 4 [default = 0];
//	repeated Proto.AppSignalState appSignalStates = 5;		// Limited to ADS_GET_APP_SIGNAL_STATE_MAX (2000)

	if (m_getSignalStateChangesReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalStateChanges, error received: " << m_getSignalStateChangesReply.error();
		m_logFile.writeError(QString("processSignalStateChanges, error received: %1").arg(m_getSignalStateChangesReply.error()));
		Q_ASSERT(m_getSignalStateChangesReply.error() != 0);

		resetToGetState(true);
		return;
	}

	int signalStateCount = m_getSignalStateChangesReply.appsignalstates_size();

	std::vector<AppSignalState> states;
	states.reserve(signalStateCount);

	for (int i = 0; i < signalStateCount; i++)
	{
		const AppSignalState& state = states.emplace_back(m_getSignalStateChangesReply.appsignalstates(i));
		Q_ASSERT(state.hash() != 0);
	}

	theSignals.setState(states);

	if (m_getSignalStateChangesReply.pendingstatescount() >= ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		// A lot of signals are in teh event queue, request one more time
		//
		requestSignalStateChanges();
	}
	else
	{
		// Update all signals
		//
		requestSignalState(m_lastSignalStateStartIndex + ADS_GET_APP_SIGNAL_STATE_MAX);
	}

	return;
}

// AppSignalState
//
void TcpSignalClient::requestSignalState(int startIndex)
{
	Q_ASSERT(isClearToSendRequest());

	if (startIndex >= static_cast<int>(m_signalList.size()))
	{
		startIndex = 0;
	}

	m_lastSignalStateStartIndex = startIndex;

	m_getSignalStateRequest.mutable_signalhashes()->Clear();
	m_getSignalStateRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_STATE_MAX);

	for (int i = startIndex;
		 i < startIndex + ADS_GET_APP_SIGNAL_STATE_MAX && i < static_cast<int>(m_signalList.size());
		 i++)
	{
		Hash signalHash = calcHash(m_signalList[i]);
		m_getSignalStateRequest.add_signalhashes(signalHash);
	}

	sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
	return;
}

void TcpSignalClient::processSignalState(const QByteArray& data)
{
	if (bool ok = m_getSignalStateReply.ParseFromArray(data.constData(), data.size());
		ok == false)
	{
		Q_ASSERT(ok);
		resetToGetState(true);
		return;
	}

	if (m_getSignalStateReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalState, error received: " << m_getSignalStateReply.error();
		m_logFile.writeError(QString("processSignalState, error received: %1").arg(m_getSignalStateReply.error()));

		Q_ASSERT(m_getSignalStateReply.error() != 0);

		resetToGetState(true);
		return;
	}

	int signalStateCount = m_getSignalStateReply.appsignalstates_size();

	std::vector<AppSignalState> states;
	states.reserve(signalStateCount);

	for (int i = 0; i < signalStateCount; i++)
	{
		const AppSignalState& state = states.emplace_back(m_getSignalStateReply.appsignalstates(i));
		Q_ASSERT(state.m_hash != 0);
	}

	theSignals.setState(states);

	resetToGetState(false);
	return;
}

void TcpSignalClient::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort s1 = configuration.appDataService1.address();
	HostAddressPort s2 = configuration.appDataService2.address();

	if (serverAddressPort(0) != s1 ||
		serverAddressPort(1) != s2)
	{
		setServers(s1, s2, true);
	}

	return;
}

