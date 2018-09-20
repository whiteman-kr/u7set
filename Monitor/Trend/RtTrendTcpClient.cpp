#include "Stable.h"
#include "RtTrendTcpClient.h"
#include "Settings.h"

RtTrendTcpClient::RtTrendTcpClient(MonitorConfigController* configController) :
	Tcp::Client(configController->softwareInfo(),
				configController->configuration().appDataServiceRealtimeTrend1.address(),
				configController->configuration().appDataServiceRealtimeTrend2.address()),
	m_cfgController(configController)
{
	qDebug() << "RtTrendTcpClient::RtTrendTcpClient(...)";

	setObjectName("RtTrendTcpClient");
	enableWatchdogTimer(true);

	return;
}

RtTrendTcpClient::~RtTrendTcpClient()
{
	qDebug() << "RtTrendTcpClient::~RtTrendTcpClient()";
}

bool RtTrendTcpClient::setData(E::RtTrendsSamplePeriod samplePeriod, const std::vector<TrendLib::TrendSignalParam> trendSignals)
{
	QMutexLocker ml(&m_dataMutex);

	m_samplePeriod = samplePeriod;

	m_signalSet.clear();
	for (const TrendLib::TrendSignalParam& sp : trendSignals)
	{
		m_signalSet.insert(sp.appSignalHash());
	}

	return true;
}

bool RtTrendTcpClient::clearData()
{
	QMutexLocker ml(&m_dataMutex);

	m_samplePeriod = E::RtTrendsSamplePeriod::sp_100ms;
	m_signalSet.clear();

	return true;
}

void RtTrendTcpClient::onClientThreadStarted()
{
	qDebug() << "RtTrendTcpClient::onClientThreadStarted()";

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &RtTrendTcpClient::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void RtTrendTcpClient::onClientThreadFinished()
{
	qDebug() << "RtTrendTcpClient::onClientThreadFinished()";
}

void RtTrendTcpClient::onConnection()
{
	qDebug() << "RtTrendTcpClient::onConnection()";
	assert(isClearToSendRequest() == true);

	startRequestCycle();

	return;
}

void RtTrendTcpClient::onDisconnection()
{
	qDebug() << "TrendTcpClient::onDisconnection";
	clearData();
	return;
}

void RtTrendTcpClient::onReplyTimeout()
{
	qDebug() << "RtTrendTcpClient::onReplyTimeout()";
	return;
}

void RtTrendTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	incStatReplyCount();

	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{
	case RT_TRENDS_MANAGEMENT:
		processTrendManagement(data);
		break;

	case RT_TRENDS_GET_STATE_CHANGES:
		processTrendStateChanges(data);
		break;

	default:
		assert(false);
		qDebug() << "Wrong requestID in RtTrendTcpClient::processReply() " << requestID;

		closeConnection();
	}

	return;
}

void RtTrendTcpClient::startRequestCycle()
{
	QThread::msleep(50);

	if (isClearToSendRequest() == false)
	{
		assert(isClearToSendRequest() == true);
		closeConnection();
		return;
	}

	requestTrendManagement();

	return;
}

void RtTrendTcpClient::requestTrendManagement()
{
	assert(isClearToSendRequest());
	incStatRequestCount();

	m_managementRequest.Clear();

	m_dataMutex.lock();
	E::RtTrendsSamplePeriod samplePeriod = m_samplePeriod;
	std::set<Hash> signalSet = m_signalSet;
	m_dataMutex.unlock();

	m_managementRequest.set_clientequipmentid(theSettings.instanceStrId().toStdString());
	m_managementRequest.set_sampleperiod(static_cast<int>(samplePeriod));

	// Add signals for tracking
	//
	for (Hash signalHash : signalSet)
	{
		if (m_trackedSignals.find(signalHash) == m_trackedSignals.end())
		{
			m_managementRequest.add_appendsignalhashes(signalHash);
		}
	}

	// Remove tracking signals
	//
	for (Hash trackedSignalHash : m_trackedSignals)
	{
		if (signalSet.find(trackedSignalHash) == signalSet.end())
		{
			m_managementRequest.add_deletesignalhashes(trackedSignalHash);
		}
	}

	// --
	//
	sendRequest(RT_TRENDS_MANAGEMENT, m_managementRequest);

	return;
}

void RtTrendTcpClient::processTrendManagement(const QByteArray& data)
{
	bool ok = m_managementReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		emit requestError(tr("Cannot parse reply to RT_TRENDS_MANAGEMENT"));

		assert(ok);
		closeConnection();
		return;
	}

	int error = m_managementReply.error();
	if (error != 0)
	{
		emit requestError(QString::fromStdString(m_managementReply.errorstring()));
		closeConnection();
		return;
	}

	m_trackedSignals.clear();
	int trackedSignalCount = m_managementReply.trackedsignalhashes_size();
	for (int i = 0; i < trackedSignalCount; i++)
	{
		Hash h = m_managementReply.trackedsignalhashes(i);
		m_trackedSignals.insert(h);
	}

	// --
	//
	requestTrendStateChanges();

	return;
}

void RtTrendTcpClient::requestTrendStateChanges()
{
	assert(isClearToSendRequest());
	incStatRequestCount();

	m_stateChangesRequest.Clear();

	// --
	//
	sendRequest(RT_TRENDS_GET_STATE_CHANGES, m_managementRequest);

	return;
}

void RtTrendTcpClient::processTrendStateChanges(const QByteArray& data)
{
	bool ok = m_stateChangesReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);

		emit requestError(tr("Cannot parse reply to RT_TRENDS_GET_STATE_CHANGES"));
		closeConnection();
		return;
	}

	int error = m_stateChangesReply.error();
	if (error != 0)
	{
		emit requestError(QString::fromStdString(m_stateChangesReply.errorstring()));
		closeConnection();
		return;
	}

	// --
	//
	std::shared_ptr<TrendLib::RealtimeData> realtimeData = std::make_shared<TrendLib::RealtimeData>();
	std::map<Hash, TrendLib::RealtimeDataChunk> realtimeDataBySignals;

	TrendLib::TrendStateItem minState;
	TrendLib::TrendStateItem maxState;

	int stateCount = m_stateChangesReply.signalstates_size();

	//qDebug() << "RtTrendTcpClient::processTrendStateChanges: Received states  " << stateCount;

	for (int i = 0; i < stateCount; i++)
	{
		const ::Proto::AppSignalState& stateMessage = m_stateChangesReply.signalstates(i);

		TrendLib::RealtimeDataChunk& chunk = realtimeDataBySignals[stateMessage.hash()];

		if (chunk.appSignalHash == UNDEFINED_HASH)
		{
			// Chunk just created
			//
			chunk.appSignalHash = stateMessage.hash();
			chunk.states.reserve(32);
		}

		TrendLib::TrendStateItem& trendItemState = chunk.states.emplace_back(AppSignalState{stateMessage});
		trendItemState.setRealtimePointFlag();

		if (i == 0)
		{
			minState = chunk.states.back();
			maxState = chunk.states.back();
		}
		else
		{
			// Min/Max is defined by system time, it assume to be sequential,
			// Later UI will decide itself, which time to use
			//
			if (trendItemState.system < minState.system)
			{
				minState = trendItemState;
			}

			if (trendItemState.system > maxState.system)
			{
				maxState = trendItemState;
			}
		}
	}

	for (auto& [hash, chunk] : realtimeDataBySignals)
	{
		Q_UNUSED(hash);
		realtimeData->signalData.push_back(std::move(chunk));
	}

	// signal dataReady(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
	//

	if (realtimeData->signalData.empty() == false)
	{
		emit dataReady(realtimeData, minState, maxState);
	}

	// New network data exchange cycle
	//
	startRequestCycle();

	return;
}

void RtTrendTcpClient::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort s1 = configuration.appDataServiceRealtimeTrend1.address();
	HostAddressPort s2 = configuration.appDataServiceRealtimeTrend2.address();

	if (serverAddressPort(0) != s1 ||
		serverAddressPort(1) != s2)
	{
		setServers(s1, s2, true);
	}

	return;
}

RtTrendTcpClient::Stat RtTrendTcpClient::stat() const
{
	RtTrendTcpClient::Stat result;

	m_statMutex.lock();
	result = m_stat;
	m_statMutex.unlock();

	return result;
}

void RtTrendTcpClient::setStat(const Stat& stat)
{
	m_statMutex.lock();
	m_stat = stat;
	m_statMutex.unlock();

	return;
}

void RtTrendTcpClient::setStatText(const QString& text)
{
	m_statMutex.lock();
	m_stat.text = text;
	m_statMutex.unlock();

	return;
}

void RtTrendTcpClient::setStatRequestQueueSize(int value)
{
	m_statMutex.lock();
	m_stat.requestQueueSize = value;
	m_statMutex.unlock();

	return;
}

void RtTrendTcpClient::incStatRequestCount()
{
	m_statMutex.lock();
	m_stat.requestCount ++;
	m_statMutex.unlock();

	return;
}

void RtTrendTcpClient::incStatReplyCount()
{
	m_statMutex.lock();
	m_stat.replyCount ++;
	m_statMutex.unlock();

	return;
}
