#include "RtSchemaTrend.h"

//
// RtConnection
//
RtSchemaTrendDataProvider::RtSchemaTrendDataProvider(const ClientLib::ISignalDataServer& signalDataServer, ILogFile* logFile) :
	m_dataProvider{signalDataServer, logFile}
{

	connect(&m_dataProvider, &ClientLib::RtDataProvider::dataReady, this, &RtSchemaTrendDataProvider::slot_realtimeDataReceived, Qt::ConnectionType::QueuedConnection);
	connect(&m_dataProvider, &ClientLib::RtDataProvider::connectionLost, this, &RtSchemaTrendDataProvider::slot_connectionLost, Qt::ConnectionType::QueuedConnection);
}

RtSchemaTrendDataProvider::~RtSchemaTrendDataProvider()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	//QMutexLocker locker(&m_signalMutex);

	m_dataProvider.clear();
	return;
}

void RtSchemaTrendDataProvider::updateConnections(const SoftwareInfo& softwareInfo,
												  const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	//QMutexLocker locker(&m_signalMutex);

	m_dataProvider.updateConnections(softwareInfo, appDataServices);
	return;
}


bool RtSchemaTrendDataProvider::trendData(const QString& appSignalId, std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const
{
	Q_ASSERT(outData);

	try
	{
		QMutexLocker locker(&m_signalMutex);

		const RtSchemaTrendSignal& rtSignal = m_trendSignals.at(::calcHash(appSignalId));
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
	catch(std::out_of_range& /*e*/)
	{
		Q_ASSERT(false);
		return false;
	}

	return true;
}

TimeStamp RtSchemaTrendDataProvider::maxTimeStamp() const
{
	qint64 maxTimeStamp = 0;

	QMutexLocker locker(&m_signalMutex);

	// Go through all signals and get maximum time state.
	//
	for (const auto& [signalHash, rtSignal] : m_trendSignals)
	{
		Q_UNUSED(signalHash);

		const TrendLib::TrendArchive& archive = rtSignal.archive();

		if (archive.m_hours.empty() == true)
		{
			continue;
		}

		const std::shared_ptr<TrendLib::OneHourData>& lastHourData = archive.m_hours.crbegin()->second;
		const std::vector<TrendLib::TrendStateRecord>& lastHourRecords = lastHourData->data;

		if (lastHourRecords.empty() == true || lastHourRecords.back().states.empty() == true)
		{
			continue;
		}

		const TrendLib::TrendStateItem& lastState = lastHourRecords.back().states.back();

		switch (m_timeType)
		{
		case E::TimeType::Local:		maxTimeStamp = std::max(maxTimeStamp, lastState.local);		break;
		case E::TimeType::System:		maxTimeStamp = std::max(maxTimeStamp, lastState.system);		break;
		case E::TimeType::Plant:		maxTimeStamp = std::max(maxTimeStamp, lastState.plant);		break;
		case E::TimeType::ArchiveId:	maxTimeStamp = std::max(maxTimeStamp, lastState.local);		break;
		default:
			Q_ASSERT(false);
		}
	}

	return maxTimeStamp;
}

void RtSchemaTrendDataProvider::setParams(E::RtTrendsSamplePeriod samplePeriod, E::TimeType timeType, int durationSeconds)
{
	QMutexLocker locker(&m_signalMutex);

	m_dataProvider.setSamplePeriod(samplePeriod);

	m_durationSeconds = durationSeconds;
	m_timeType = timeType;
	m_samplePeriod = samplePeriod;

	return;
}

void RtSchemaTrendDataProvider::updateSignals(const QStringList& appSignalIds)
{
	QMutexLocker locker(&m_signalMutex);

	std::map<Hash, RtSchemaTrendSignal> connectionSignals = std::move(m_trendSignals);
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

	m_dataProvider.setData(m_samplePeriod, appSignalIds);

	return;
}

void RtSchemaTrendDataProvider::slot_realtimeDataReceived(QString sourceEquipmentId,
														  std::shared_ptr<TrendLib::RealtimeData> data,
														  TrendLib::TrendStateItem minState,
														  TrendLib::TrendStateItem maxState)
{
	Q_UNUSED(minState);
	Q_UNUSED(maxState);

	for (const TrendLib::RealtimeDataChunk& chunk : data->signalData)
	{
		const Hash signalHash = chunk.appSignalHash;
		const std::vector<TrendLib::TrendStateItem>& states = chunk.states;

		appendRealtimeData(sourceEquipmentId, signalHash, states);
	}

	return;
}

void RtSchemaTrendDataProvider::slot_connectionLost(QString sourceEquipmentId)
{
	QMutexLocker locker(&m_signalMutex);

	try
	{
		for (auto& [signalHash, rtSignal] : m_trendSignals)
		{
			Q_UNUSED(signalHash);

			if (sourceEquipmentId == rtSignal.archive().realTimeActiveServiceId)
			{
				TrendLib::TrendSignalSet::addNonValidPoint(&rtSignal.archive());
			}
		}
	}
	catch (std::out_of_range& /*e*/)
	{
		// Connection not found or signal not found
		//
		Q_ASSERT(false);
	}


	return;
}

void RtSchemaTrendDataProvider::appendRealtimeData(QString sourceEquipmentId,
												   Hash signalHash,
												   const std::vector<TrendLib::TrendStateItem>& states)
{
	if (states.empty() == true)
	{
		return;
	}

	try
	{
		QMutexLocker wl(&m_signalMutex);

		RtSchemaTrendSignal& rtSignal = m_trendSignals.at(signalHash);
		Q_ASSERT(::calcHash(rtSignal.appSignalId()) == signalHash);

		TrendLib::TrendArchive& archive = rtSignal.archive();

		appendRealtimeData_unsafe(sourceEquipmentId, m_timeType, states, &archive);
		trimArchive_unsafe(m_durationSeconds, &archive);
	}
	catch (std::out_of_range& /*e*/)
	{
		// Connection not found or signal not found
		//
		Q_ASSERT(false);
	}

	return;
}

void RtSchemaTrendDataProvider::appendRealtimeData_unsafe(QString sourceEquipmentId, E::TimeType timeType, const std::vector<TrendLib::TrendStateItem>& states, TrendLib::TrendArchive* archive)
{
	Q_ASSERT(archive);

	if (states.empty() == true)
	{
		return;
	}

	if (archive->serviceUpdateTimer.isValid() == false)
	{
		// The first start. Timer is created invalid, using anything before start() is UB.
		//
		archive->serviceUpdateTimer.start();
	}

	if (archive->realTimeActiveServiceId == sourceEquipmentId)
	{
		// Ok, this is correcect source, check if it is still valid
		//
		if (states.back().isValid() == false)
		{
			// This source has lost connections, reset active source
			//
			archive->realTimeActiveServiceId.clear();

			// Do not return, add these non valid points to the trend
			//
		}
	}
	else
	{
		if (archive->realTimeActiveServiceId.isEmpty() == true)
		{
			if (states.back().isValid() == true)
			{
				// This source has valid points, set it as active
				//
				archive->realTimeActiveServiceId = sourceEquipmentId;

				// Source is changed, we can add these points to the trend
				//
			}
			else
			{
				// This source is not valid either, ingore it
				//
				return;
			}
		}
		else
		{
			// This is the wrong source, skip it, but check timeouit firts
			//
			if (archive->serviceUpdateTimer.hasExpired(2000) == true)
			{
				// We have not received from the active server data some time,
				// switch to other server
				//
				archive->realTimeActiveServiceId = sourceEquipmentId;
			}
			else
			{
				// This is the wrong source and data is comming for active connection (to timeout)
				//
				return;
			}
		}
	}

	archive->serviceUpdateTimer.restart();

	// --
	//
	TimeStamp lastHourTime{0};
	std::shared_ptr<TrendLib::OneHourData> hourData;

	for (const TrendLib::TrendStateItem& state : states)
	{
		TimeStamp ts = state.getTime(timeType).roundedToHour();
		if (ts == TimeStamp{0})
		{
			qDebug() << "RtSchemaTrendDataProvider::appendRealtimeData_unsafe: Received wrong timestamp: " << ts.timeStamp << ", " << timeType;
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

void RtSchemaTrendDataProvider::trimArchive_unsafe(int durationSeconds, TrendLib::TrendArchive* archive)
{
	Q_ASSERT(archive);

	int64_t durationHours = durationSeconds	/ 3600 + ((durationSeconds % 3600) ? 1 : 0);
	int64_t durationMs = durationHours * 1_hour;

	TimeStamp lastHour = archive->m_hours.crbegin()->first;
	TimeStamp limitHour = lastHour;
	limitHour -= durationMs;

	std::erase_if(archive->m_hours,
				 [limitHour, archive](const auto& item)
				 {
					Q_UNUSED(archive);
					auto const& [key, value] = item;
					return (key < limitHour);
				 });

	return;
}

//
// RtTrendSchema
//
RtSchemaTrend::RtSchemaTrend(const MonitorConfigController& configController, const ClientLib::ISignalDataServer& signalDataServer) :
	m_configController(configController),
	m_signalDataServer(signalDataServer)
{
}

RtSchemaTrend::~RtSchemaTrend()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	{
		QMutexLocker ml(&m_mutex);
		m_dataProviders.clear();
	}

	return;
}

// Updates m_rtConnections from SchemaDetailsSet
//
void RtSchemaTrend::updateRealtimeConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	// We could make this function slot and connect it to MonitorConfigController::configurationArrived
	// But we need to be sure that
	//
	auto softwareInfo = m_configController.softwareInfo();
	auto appDataRealTimeServices = m_configController.configuration().appDataRealTimeServices;

	auto schemaItems = m_configController.trendSchemaItems();

	{
		QMutexLocker ml(&m_mutex);

		// Update or create new RtConnections
		//
		for (const VFrame30::SchemaDetails::TrendIndicatorSchemaItems& schemaItem : schemaItems)
		{
			if (m_dataProviders.contains(schemaItem.itemUuid) == false)
			{
				m_dataProviders.try_emplace(schemaItem.itemUuid,
											m_signalDataServer,
											m_configController.logFile());
			}

			auto& rtConnection = m_dataProviders.at(schemaItem.itemUuid);

			rtConnection.updateConnections(softwareInfo, appDataRealTimeServices);

			rtConnection.setParams(schemaItem.samplePeriod, schemaItem.timeType, schemaItem.durationSeconds);
			rtConnection.updateSignals(schemaItem.appSignalIds);
		}

		// Remove unwanted RtConnections
		//
		std::list<QUuid> connectionsToRemove;

		for (const auto&[schemaItemUuid, rtConnection] : m_dataProviders)
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
			m_dataProviders.erase(uuid);
		}
	}

	return;
}

bool RtSchemaTrend::trendData(QUuid trendUuid,
							  const QString& appSignalId,
							  std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const
{
	QMutexLocker locker(&m_mutex);
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	try
	{
		const RtSchemaTrendDataProvider& rtc = m_dataProviders.at(trendUuid);
		return rtc.trendData(appSignalId, outData);
	}
	catch (std::out_of_range& /*e*/)
	{
		Q_ASSERT(false);
		return false;
	}
}

TimeStamp RtSchemaTrend::maxTimeStamp(QUuid trendUuid) const
{
	QMutexLocker locker(&m_mutex);
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	try
	{
		const RtSchemaTrendDataProvider& rtc = m_dataProviders.at(trendUuid);
		return rtc.maxTimeStamp();
	}
	catch (std::out_of_range& /*e*/)
	{
		Q_ASSERT(false);
		return {};
	}
}
