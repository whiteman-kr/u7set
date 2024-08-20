#include "RtTrendTcpClient.h"


namespace
{
	thread_local Network::RtTrendsManagementRequest tl_managementRequest;
	thread_local Network::RtTrendsManagementReply tl_managementReply;

	thread_local Network::RtTrendsGetStateChangesRequest tl_stateChangesRequest;
	thread_local Network::RtTrendsGetStateChangesReply tl_stateChangesReply;
}


namespace ClientLib
{
	RtTrendTcpClient::RtTrendTcpClient(const SoftwareInfo& softwareInfo,
									   const HostAddressPort& serverAddressPort,
									   QString serviceEquipmentId,
									   const ISignalDataServer& signalDataServer,
									   ILogFile* logFile) :
		Tcp::Client(softwareInfo, serverAddressPort, serverAddressPort, "RtTrendTcpClient", serviceEquipmentId),
		TcpClientStatistics(this),
		m_signalDataServer(signalDataServer),
		m_logFile(logFile, "RtTrendTcpClient")
	{
		Q_ASSERT(logFile);

		setObjectName("RtTrendTcpClient");

		m_logFile.writeMessage("RtTrendTcpClient::RtTrendTcpClient(), address " + serverAddressPort.toString());
		qDebug() << "RtTrendTcpClient::RtTrendTcpClient(...), address " << serverAddressPort.toString();

		connect(this, &Tcp::Client::signal_wrongServerID,
			[this](const QString& errorMessage)
			{
				m_logFile.writeError(errorMessage);
			});

		return;
	}

	RtTrendTcpClient::~RtTrendTcpClient()
	{
		m_logFile.writeMessage("RtTrendTcpClient::~RtTrendTcpClient(), address " + serverAddressPort1().toString());
		qDebug() << "RtTrendTcpClient::~RtTrendTcpClient(...), address " << serverAddressPort1().toString();
	}

	bool RtTrendTcpClient::setSignals(const QStringList& appSignalIds)
	{
		// Add all signals, now it does not matter if this signal is from a differents sorce,
		// It is imposiibple to know which source for signal till all signal params are loaded
		//
		QMutexLocker ml(&m_dataMutex);

		m_signalSet.clear();
		for (const QString& s : appSignalIds)
		{
			m_signalSet.insert(s);
		}

		return true;
	}

	bool RtTrendTcpClient::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
	{
		QMutexLocker ml(&m_dataMutex);

		m_samplePeriod = samplePeriod;

		// Add all signals, now it does not matter if this signal is from a differents sorce,
		// It is imposiibple to know which source for signal till all signal params are loaded
		//
		m_signalSet.clear();
		for (const QString& s : trendSignals)
		{
			m_signalSet.insert(s);
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

		return;
	}

	void RtTrendTcpClient::onClientThreadFinished()
	{
		qDebug() << "RtTrendTcpClient::onClientThreadFinished()";
		m_logFile.writeMessage("onClientThreadFinished()");
	}

	void RtTrendTcpClient::onConnection()
	{
		qDebug() << "RtTrendTcpClient::onConnection()" << serverAddressPort1().toString();
		m_logFile.writeMessage("onConnection() " + serverAddressPort1().toString());

		Q_ASSERT(isClearToSendRequest() == true);

		startRequestCycle();

		return;
	}

	void RtTrendTcpClient::onDisconnection()
	{
		emit connectionLost(connectToServerID());

		qDebug() << "TrendTcpClient::onDisconnection " << serverAddressPort1().toString();
		m_logFile.writeMessage("onDisconnection() " + serverAddressPort1().toString());
		return;
	}

	void RtTrendTcpClient::onReplyTimeout()
	{
		qDebug() << "RtTrendTcpClient::onReplyTimeout() " << serverAddressPort1().toString();
		m_logFile.writeWarning("onReplyTimeout() " + serverAddressPort1().toString());
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

		tl_managementRequest.Clear();

		// --
		//
		m_dataMutex.lock();

		E::RtTrendsSamplePeriod samplePeriod = m_samplePeriod;

		std::set<Hash> signalSet;
		for (const QString& signalId : m_signalSet)
		{
			if (m_signalDataServer.dataServiceHasSignal(connectedSoftwareInfo().equipmentID(), signalId) == true)
			{
				signalSet.insert(::calcHash(signalId));
			}
		}

		m_dataMutex.unlock();

		// --
		//
		tl_managementRequest.set_clientequipmentid(equipmentID().toStdString());
		tl_managementRequest.set_sampleperiod(static_cast<int>(samplePeriod));

		// Add signals for tracking
		//
		for (Hash signalHash : signalSet)
		{
			if (m_trackedSignals.contains(signalHash) == false)
			{
				tl_managementRequest.add_appendsignalhashes(signalHash);
			}
		}

		// Remove tracking signals
		//
		for (Hash trackedSignalHash : m_trackedSignals)
		{
			if (signalSet.contains(trackedSignalHash) == false)
			{
				tl_managementRequest.add_deletesignalhashes(trackedSignalHash);
			}
		}

		// --
		//
		sendRequest(RT_TRENDS_MANAGEMENT, tl_managementRequest);

		return;
	}

	void RtTrendTcpClient::processTrendManagement(const QByteArray& data)
	{
		bool ok = tl_managementReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			emit requestError(tr("Cannot parse reply to RT_TRENDS_MANAGEMENT"));

			Q_ASSERT(ok);
			closeConnection();
			return;
		}

		int error = tl_managementReply.error();
		if (error != 0)
		{
			emit requestError(QString::fromStdString(tl_managementReply.errorstring()));
			m_logFile.writeError(QString("processTrendManagement, error received ") + QString::fromStdString(tl_managementReply.errorstring()));

			closeConnection();
			return;
		}

		m_trackedSignals.clear();
		int trackedSignalCount = tl_managementReply.trackedsignalhashes_size();
		for (int i = 0; i < trackedSignalCount; i++)
		{
			Hash h = tl_managementReply.trackedsignalhashes(i);
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

		tl_stateChangesRequest.Clear();

		// --
		//
		sendRequest(RT_TRENDS_GET_STATE_CHANGES, tl_managementRequest);

		return;
	}

	void RtTrendTcpClient::processTrendStateChanges(const QByteArray& data)
	{
		bool ok = tl_stateChangesReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			Q_ASSERT(ok);

			emit requestError(tr("Cannot parse reply to RT_TRENDS_GET_STATE_CHANGES"));
			closeConnection();
			return;
		}

		int error = tl_stateChangesReply.error();
		if (error != 0)
		{
			emit requestError(QString::fromStdString(tl_stateChangesReply.errorstring()));
			closeConnection();
			return;
		}

		// --
		//
		std::shared_ptr<TrendLib::RealtimeData> realtimeData = std::make_shared<TrendLib::RealtimeData>();
		std::map<Hash, TrendLib::RealtimeDataChunk> realtimeDataBySignals;

		TrendLib::TrendStateItem minState{};	// Initialized by zeroes
		TrendLib::TrendStateItem maxState{};	// Initialized by zeroes

		int stateCount = tl_stateChangesReply.signalstates_size();

		//qDebug() << "RtTrendTcpClient::processTrendStateChanges: Received states  " << stateCount;

		for (int i = 0; i < stateCount; i++)
		{
			const ::Proto::AppSignalState& stateMessage = tl_stateChangesReply.signalstates(i);

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
			emit dataReady(connectedSoftwareInfo().equipmentID(), realtimeData, m_samplePeriod, minState, maxState);
		}

		// New network data exchange cycle
		//
		startRequestCycle();

		return;
	}

	RtTrendConnectionStatistics RtTrendTcpClient::stat() const
	{
		RtTrendConnectionStatistics result;

		m_statMutex.lock();
		result = m_stat;
		result.isConnected = static_cast<int>(this->isConnected());
		m_statMutex.unlock();

		return result;
	}

	void RtTrendTcpClient::setStat(const RtTrendConnectionStatistics& stat)
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
}
