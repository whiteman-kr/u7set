#include "RtTrendSchema.h"

//
// RtConnection
//
RtConnection::~RtConnection()
{
	QMutexLocker locker(&m_mutex);

	if (m_tcpClientThread != nullptr)
	{
		m_tcpClientThread->quitAndWait(10000);
		delete m_tcpClientThread;
	}

	return;
}

bool RtConnection::trendData(const QString& appSignalId, std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const
{
	Q_ASSERT(outData);

	try
	{
		QMutexLocker locker(&m_mutex);

		const RtSignal& rtSignal = m_trendSignals.at(::calcHash(appSignalId));
		const TrendLib::TrendArchive& archive = rtSignal.archive();

		// Just return everything, as data trimmed by receiver in RtTrendSchema::trimArchive_unsafe()
		//
		for (const auto& [hour, hourData] : archive.m_hours)
		{
			Q_UNUSED(hour);

			auto copiedHourData = std::make_shared<TrendLib::OneHourData>(hourData.operator*());
			outData->push_back(copiedHourData);
		}
	}
	catch(std::out_of_range e)
	{
		Q_ASSERT(false);
		return false;
	}

	return true;
}

TimeStamp RtConnection::maxTimeStamp() const
{
	QMutexLocker locker(&m_mutex);

	TimeStamp result{};

	switch (m_timeType)
	{
	case E::TimeType::Local:		result = m_maxState.local;		break;
	case E::TimeType::System:		result = m_maxState.system;		break;
	case E::TimeType::Plant:		result = m_maxState.plant;		break;
	case E::TimeType::ArchiveId:	result = m_maxState.local;		break;
	default:
		Q_ASSERT(false);
		return {};
	}

	return result;
}

bool RtConnection::hasConnectionThread() const
{
	QMutexLocker locker(&m_mutex);
	return m_tcpClient != nullptr && m_tcpClientThread != nullptr;
}

void RtConnection::createConnectionThread(MonitorConfigController* configController)
{
	Q_ASSERT(configController);

	QMutexLocker locker(&m_mutex);

	m_tcpClient = new RtTrendTcpClient(configController, configController->logFile());
	m_tcpClientThread = new SimpleThread(m_tcpClient);
	m_tcpClientThread->start();

	connect(m_tcpClient, &RtTrendTcpClient::dataReady, this, &RtConnection::slot_realtimeDataReceived, Qt::ConnectionType::QueuedConnection);
	connect(m_tcpClient, &RtTrendTcpClient::connectionLost, this, &RtConnection::slot_connectionLost, Qt::ConnectionType::QueuedConnection);

	return;
}

void RtConnection::setParams(E::RtTrendsSamplePeriod samplePeriod, E::TimeType timeType, int durationSeconds)
{
	QMutexLocker locker(&m_mutex);

	if (m_tcpClient == nullptr || m_tcpClientThread == nullptr)
	{
		Q_ASSERT(m_tcpClient);
		Q_ASSERT(m_tcpClientThread);
		return;
	}

	m_tcpClient->setSamplePeriod(samplePeriod);
	m_durationSeconds = durationSeconds;
	m_timeType = timeType;
	m_samplePeriod = samplePeriod;

	return;
}

void RtConnection::updateSignals(const QStringList appSignalIds)
{
	QMutexLocker locker(&m_mutex);

	if (m_tcpClient == nullptr || m_tcpClientThread == nullptr)
	{
		Q_ASSERT(m_tcpClient);
		Q_ASSERT(m_tcpClientThread);
		return;
	}

	std::map<Hash, RtSignal> connectionSignals = std::move(m_trendSignals);
	m_trendSignals = {};

	for (const QString& appSignalId : appSignalIds)
	{
		Hash hash = ::calcHash(appSignalId);

		if (connectionSignals.contains(hash) == true)
		{
			m_trendSignals.insert(connectionSignals.extract(hash));
		}
		else
		{
			m_trendSignals.emplace(hash, appSignalId);
		}
	}

	Q_ASSERT(m_trendSignals.size() == static_cast<size_t>(appSignalIds.size()));

	m_tcpClient->setData(appSignalIds);

	return;
}

void RtConnection::slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data,
											 TrendLib::TrendStateItem minState,
											 TrendLib::TrendStateItem maxState)
{
	{
		QMutexLocker locker(&m_mutex);

		m_minState = minState;
		m_maxState = maxState;
	}

	for (const TrendLib::RealtimeDataChunk& chunk : data->signalData)
	{
		const Hash signalHash = chunk.appSignalHash;
		const std::vector<TrendLib::TrendStateItem>& states = chunk.states;

		appendRealtimeData(signalHash, states);
	}

	return;
}

void RtConnection::slot_connectionLost()
{
	QMutexLocker locker(&m_mutex);

	//m_minState = {};
	//m_maxState = {};

	try
	{
		Q_ASSERT(m_tcpClient);
		Q_ASSERT(m_tcpClientThread);
		Q_ASSERT(m_tcpClient->samplePeriod() == m_samplePeriod);

		for (auto& [signalHash, rtSignal] : m_trendSignals)
		{
			Q_UNUSED(signalHash);
			TrendLib::TrendSignalSet::addNonValidPoint(&rtSignal.archive());
		}
	}
	catch (std::out_of_range /*e*/)
	{
		// Connection not found or signal not found
		//
		Q_ASSERT(false);
	}


	return;
}

void RtConnection::appendRealtimeData(Hash signalHash, const std::vector<TrendLib::TrendStateItem>& states)
{
	try
	{
		QMutexLocker wl(&m_mutex);

		Q_ASSERT(m_tcpClient);
		Q_ASSERT(m_tcpClientThread);
		Q_ASSERT(m_tcpClient->samplePeriod() == m_samplePeriod);

		RtSignal& rtSignal = m_trendSignals.at(signalHash);
		Q_ASSERT(::calcHash(rtSignal.appSignalId()) == signalHash);

		TrendLib::TrendArchive& archive = rtSignal.archive();

		appendRealtimeData_unsafe(m_timeType, states, &archive);
		trimArchive_unsafe(m_durationSeconds, &archive);
	}
	catch (std::out_of_range e)
	{
		// Connection not found or signal not found
		//
		Q_ASSERT(false);
	}

	return;
}

void RtConnection::appendRealtimeData_unsafe(E::TimeType timeType, const std::vector<TrendLib::TrendStateItem>& states, TrendLib::TrendArchive* archive)
{
	Q_ASSERT(archive);

	TimeStamp lastHourTime{0};
	std::shared_ptr<TrendLib::OneHourData> hourData;

	for (const TrendLib::TrendStateItem& state : states)
	{
		TimeStamp ts = state.getTime(timeType).roundedToHour();
		if (ts == TimeStamp{0})
		{
			qDebug() << "TrendSignalSet::appendRealtimeDataToArchive: Received wrong timestamp: " << ts.timeStamp << ", " << timeType;
			continue;
		}

		if (lastHourTime == ts)
		{
			Q_ASSERT(hourData);
		}
		else
		{
			hourData = archive->m_hours[ts];

			if (hourData.get() == nullptr)	// Just created
			{
				hourData = std::make_shared<TrendLib::OneHourData>();
				archive->m_hours[ts] = hourData;
			}

			lastHourTime = ts;
		}

		hourData->state = TrendLib::OneHourData::State::Received;

		if (hourData->data.empty() == true)
		{
			TrendLib::TrendStateRecord& record = hourData->data.emplace_back();
			record.states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
		}
		else
		{
			TrendLib::TrendStateRecord& lastRecord = hourData->data.back();

			if (lastRecord.states.size() >= TrendLib::TrendStateRecord::RecomendedSize)
			{
				TrendLib::TrendStateRecord& record = hourData->data.emplace_back();
				record.states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
			}
		}

		// Add state
		//
		TrendLib::TrendStateRecord& recordToAddState = hourData->data.back();
		recordToAddState.states.push_back(state);
	}

	return;
}

void RtConnection::trimArchive_unsafe(int durationSeconds, TrendLib::TrendArchive* archive)
{
	Q_ASSERT(archive);

	int64_t durationHours = durationSeconds	/ 3600 + (durationSeconds % 3600 ? 1 : 0);
	int64_t durationMs = durationHours * 1_hour;

	TimeStamp lastHour = archive->m_hours.crbegin()->first;
	TimeStamp limitHour = lastHour;
	limitHour -= durationMs;

	std::erase_if(archive->m_hours,
				 [limitHour, archive](const auto& item)
				 {
					auto const& [key, value] = item;
					return (key < limitHour);
				 });

	return;
}

//
// RtTrendSchema
//
RtTrendSchema::RtTrendSchema() :
	m_configController(nullptr)
{
	Q_ASSERT(false);
}

RtTrendSchema::RtTrendSchema(MonitorConfigController* configController) :
	m_configController(configController)
{
	Q_ASSERT(m_configController);
}

RtTrendSchema::~RtTrendSchema()
{
	{
		QMutexLocker ml(&m_mutex);
		m_rtConnections.clear();
	}

	return;
}

// Updates m_rtConnections from SchemaDetaisSet
//
void RtTrendSchema::updateRealtimeConnections()
{
	if (m_configController == nullptr)
	{
		Q_ASSERT(m_configController);
		return;
	}

	auto schemaItems = m_configController->trendSchemaItems();

	{
		QMutexLocker ml(&m_mutex);

		// Create new RtConnections
		//
		for (const VFrame30::SchemaDetails::TrendIndicatorSchemaItems& schemaItem : schemaItems)
		{
			RtConnection& rtConnection = m_rtConnections[schemaItem.itemUuid];

			if (rtConnection.hasConnectionThread() == false)
			{
				// Just created object
				//
				rtConnection.createConnectionThread(m_configController);
			}

			rtConnection.setParams(schemaItem.samplePeriod, schemaItem.timeType, schemaItem.durationSeconds);
			rtConnection.updateSignals(schemaItem.appSignalIds);
		}

		// Remove unwanted RtConnections
		//
		std::vector<QUuid> connectionsToRemove;
		connectionsToRemove.reserve(m_rtConnections.size());

		for (const auto&[schemaItemUuid, rtConnection] : m_rtConnections)
		{
			auto it = std::find_if(schemaItems.begin(), schemaItems.end(),
								   [connUuid = schemaItemUuid](const VFrame30::SchemaDetails::TrendIndicatorSchemaItems& si)
								   {
										return si.itemUuid == connUuid;
								   });

			if (it == schemaItems.end())
			{
				connectionsToRemove.push_back(schemaItemUuid);
			}
		}

		for (QUuid uuid : connectionsToRemove)
		{
			m_rtConnections.erase(uuid);
		}
	}

	return;
}

bool RtTrendSchema::trendData(QUuid trendUuid,
							  const QString& appSignalId,
							  std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const
{
	QMutexLocker locker(&m_mutex);

	try
	{
		const RtConnection& rtc = m_rtConnections.at(trendUuid);
		return rtc.trendData(appSignalId, outData);
	}
	catch (std::out_of_range e)
	{
		Q_ASSERT(false);
		return false;
	}
}

TimeStamp RtTrendSchema::maxTimeStamp(QUuid trendUuid) const
{
	QMutexLocker locker(&m_mutex);

	try
	{
		const RtConnection& rtc = m_rtConnections.at(trendUuid);
		return rtc.maxTimeStamp();
	}
	catch (std::out_of_range e)
	{
		Q_ASSERT(false);
		return {};
	}
}
