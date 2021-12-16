#include "RtTrendTcpClient.h"
#include "MonitorAppSettings.h"

RtTrendTcpClient::RtTrendTcpClient(MonitorConfigController* configController, ILogFile* logFile) :
	Tcp::Client(configController->softwareInfo(),
				configController->configuration().appDataServiceRealtimeTrend1.address(),
				configController->configuration().appDataServiceRealtimeTrend2.address(),
				"RtTrendTcpClient"),
	TcpClientStatistics(this),
	m_cfgController(configController),
	m_logFile(logFile, "RtTrendTcpClient")
{
	Q_ASSERT(configController);
	Q_ASSERT(logFile);

	qDebug() << "RtTrendTcpClient::RtTrendTcpClient(...)";

	setObjectName("RtTrendTcpClient");

	return;
}

RtTrendTcpClient::~RtTrendTcpClient()
{
	qDebug() << "RtTrendTcpClient::~RtTrendTcpClient()";
}

bool RtTrendTcpClient::addSignals(const QStringList& appSignalIds)
{
	QMutexLocker ml(&m_dataMutex);

	m_signalSet.clear();
	for (const QString& s : appSignalIds)
	{
		m_signalSet.insert(::calcHash(s));
	}

	return true;
}

bool RtTrendTcpClient::setData(const QStringList& trendSignals)
{
	m_signalSet.clear();
	for (const QString& s : trendSignals)
	{
		m_signalSet.insert(::calcHash(s));
	}

	return true;
}

bool RtTrendTcpClient::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
{
	QMutexLocker ml(&m_dataMutex);

	m_samplePeriod = samplePeriod;

	m_signalSet.clear();
	for (const QString& s : trendSignals)
	{
		m_signalSet.insert(::calcHash(s));
	}

	return true;
}

void RtTrendTcpClient::setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod)
{
	QMutexLocker ml(&m_dataMutex);
	m_samplePeriod = samplePeriod;
	return;
}

E::RtTrendsSamplePeriod RtTrendTcpClient::samplePeriod() const
{
	QMutexLocker ml(&m_dataMutex);
	return m_samplePeriod;
}

void RtTrendTcpClient::onClientThreadStarted()
{
	qDebug() << "RtTrendTcpClient::onClientThreadStarted()";
	m_logFile.writeMessage("onClientThreadStarted()");

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &RtTrendTcpClient::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void RtTrendTcpClient::onClientThreadFinished()
{
	qDebug() << "RtTrendTcpClient::onClientThreadFinished()";
	m_logFile.writeMessage("onClientThreadFinished()");
}

void RtTrendTcpClient::onConnection()
{
	qDebug() << "RtTrendTcpClient::onConnection()";
	m_logFile.writeMessage("onConnection()");

	Q_ASSERT(isClearToSendRequest() == true);

	startRequestCycle();

	return;
}

void RtTrendTcpClient::onDisconnection()
{
	emit connectionLost();

	qDebug() << "TrendTcpClient::onDisconnection";
	m_logFile.writeMessage("onDisconnection()");
	return;
}

void RtTrendTcpClient::onReplyTimeout()
{
	qDebug() << "RtTrendTcpClient::onReplyTimeout()";
	m_logFile.writeWarning("onReplyTimeout()");
	return;
}

void RtTrendTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
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
	case RT_TRENDS_MANAGEMENT:
		processTrendManagement(data);
		break;

	case RT_TRENDS_GET_STATE_CHANGES:
		processTrendStateChanges(data);
		break;

	default:
		Q_ASSERT(false);
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
		Q_ASSERT(isClearToSendRequest() == true);
		closeConnection();
		return;
	}

	requestTrendManagement();

	return;
}

void RtTrendTcpClient::requestTrendManagement()
{
	Q_ASSERT(isClearToSendRequest());
	incStatRequestCount();

	m_managementRequest.Clear();

	m_dataMutex.lock();

	E::RtTrendsSamplePeriod samplePeriod = m_samplePeriod;
	std::set<Hash> signalSet = m_signalSet;
	m_dataMutex.unlock();

	m_managementRequest.set_clientequipmentid(MonitorAppSettings::instance().equipmentId().toStdString());
	m_managementRequest.set_sampleperiod(static_cast<int>(samplePeriod));

	// Add signals for tracking
	//
	for (Hash signalHash : signalSet)
	{
		if (m_trackedSignals.contains(signalHash) == false)
		{
			m_managementRequest.add_appendsignalhashes(signalHash);
		}
	}

	// Remove tracking signals
	//
	for (Hash trackedSignalHash : m_trackedSignals)
	{
		if (signalSet.contains(trackedSignalHash) == false)
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

		Q_ASSERT(ok);
		closeConnection();
		return;
	}

	int error = m_managementReply.error();
	if (error != 0)
	{
		emit requestError(QString::fromStdString(m_managementReply.errorstring()));
		m_logFile.writeError(QString("processTrendManagement, error received ") + QString::fromStdString(m_managementReply.errorstring()));

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
	Q_ASSERT(isClearToSendRequest());
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
		Q_ASSERT(ok);

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

	TrendLib::TrendStateItem minState{};	// Initialized by zeroes
	TrendLib::TrendStateItem maxState{};	// Initialized by zeroes

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
