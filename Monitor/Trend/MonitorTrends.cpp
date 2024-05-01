#include "MonitorTrends.h"

#include "../lib/ISignalHasTag.h"

#include <TrendView/DialogChooseTrendSignals.h>
#include <TrendView/TrendSignalSet.h>


std::list<MonitorTrendsWidget*> MonitorTrends::s_trendsList;

std::vector<MonitorTrendsWidget*> MonitorTrends::getTrendsList()
{
	std::vector<MonitorTrendsWidget*> result{s_trendsList.begin(), s_trendsList.end()};
	return result;
}

bool MonitorTrends::activateTrendWindow(MonitorTrendsWidget* trendWidget)
{
	if (trendWidget == nullptr)
	{
		Q_ASSERT(trendWidget);
		return false;
	}

#ifdef QT_DEBUG
	if (auto it = std::find(s_trendsList.begin(), s_trendsList.end(), trendWidget);
		it == s_trendsList.end())
	{
		Q_ASSERT(false);
		return false;
	}
#endif

	trendWidget->activateWindow();
	trendWidget->ensureVisible();

	return true;
}

bool MonitorTrends::startTrendApp(const ClientLib::AppSignalManager& signalManager,
								  const MonitorConfigController& configController,
								  const std::vector<AppSignalParam>& appSignals,
								  QWidget* parent)
{
	MonitorTrendsWidget* window = new MonitorTrendsWidget(signalManager, configController, parent);

	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(appSignals.size());

	auto configuration = configController.configuration();
	auto archiveServers = configuration.archiveServices;
	auto appDataServers = configuration.appDataRealTimeServices;

	for (const AppSignalParam& appSignal : appSignals)
	{
		// Take the first available archive server for the signal.
		//
		auto archServerIt = std::find_if(archiveServers.begin(), archiveServers.end(), [&appSignal, &signalManager](const auto& as)
										 {
											 return signalManager.dataServiceHasSignal(as.appDataServiceId, appSignal.appSignalId());
										 });

		auto appDataServerIt = std::find_if(appDataServers.begin(), appDataServers.end(), [&appSignal, &signalManager](const auto& ads)
											{
												return signalManager.dataServiceHasSignal(ads.equipmentId, appSignal.appSignalId());
											});

		if (archServerIt != archiveServers.end())
		{
			const auto& as = *archServerIt;

			TrendLib::ArchiveServer trendArchiveServer{as.equipmentId, as.shortenId, as.appDataServiceId};
			TrendLib::TrendSignalParam tsp{appSignal, trendArchiveServer};

			trendSignals.push_back(std::move(tsp));
			continue;
		};

		// This situation is possible if archive service is not configured, but still realtime trends is possible, add it as is.
		//
		if (appDataServerIt != appDataServers.end())
		{
			const auto& ads = *appDataServerIt;

			TrendLib::ArchiveServer trendArchiveServer{"", "", ads.equipmentId};
			TrendLib::TrendSignalParam tsp{appSignal, trendArchiveServer};

			trendSignals.push_back(std::move(tsp));
			continue;
		}
	}

	window->addSignals(trendSignals, true);

	window->show();

	return false;
}

void MonitorTrends::registerTrendWindow(MonitorTrendsWidget* window)
{
#ifdef QT_DEBUG
	auto it = std::find_if(s_trendsList.begin(), s_trendsList.end(), [&window](MonitorTrendsWidget* w)
						   {
							   return w == window;
						   });

	Q_ASSERT(it == s_trendsList.end());
#endif

	s_trendsList.push_back(window);
	return;
}

void MonitorTrends::unregisterTrendWindow(const MonitorTrendsWidget* window)
{
	[[maybe_unused]] auto removed = s_trendsList.remove_if([window](MonitorTrendsWidget* w)
														   {
															   return w == window;
														   });
	Q_ASSERT(removed == 1);

	return;
}


MonitorTrendsWidget::MonitorTrendsWidget(const ClientLib::AppSignalManager& signalManager,
										 const MonitorConfigController& configController,
										 QWidget* parent) :
	TrendLib::TrendMainWindow(parent),
	m_signalManager(signalManager),
	m_configController(configController),
	m_archiveDataProvider(m_configController, m_configController.logFile()),
	m_realtimeDataProvider(m_signalManager, m_configController.logFile())
{
	static int no = 1;
	QString trendName = tr("Monitor Trends %1").arg(no++);
	MonitorTrends::registerTrendWindow(this);

	setWindowTitle(trendName);

	// Status bar
	//
	QStatusBar* sb = statusBar();
	Q_ASSERT(sb);

	m_statusBarTextLabel = new QLabel(sb);
	m_statusBarQueueSizeLabel = new QLabel(sb);
	m_statusBarNetworkRequestsLabel = new QLabel(sb);
	m_statusBarConnectionStateLabel = new QLabel(sb);

	sb->addWidget(m_statusBarTextLabel, 1);
	sb->addWidget(m_statusBarQueueSizeLabel, 0);
	sb->addWidget(m_statusBarNetworkRequestsLabel, 0);
	sb->addWidget(m_statusBarConnectionStateLabel, 0);

	// Communication thread
	//
	createArchiveConnection();

	// --
	//
	connect(this, &TrendLib::TrendMainWindow::trendModeChanged, this, &MonitorTrendsWidget::slot_trendModeChanged);

	// Archive connection
	//
	connect(&signalSet(), &TrendLib::TrendSignalSet::requestData, this, &MonitorTrendsWidget::slot_requestData);
	connect(&m_archiveDataProvider, &MonitorTrendArchiveConnections::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_archiveDataReceived);
	connect(&m_archiveDataProvider, &MonitorTrendArchiveConnections::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_archiveRequestError);
	connect(&m_archiveDataProvider, &MonitorTrendArchiveConnections::dataReady, this, &MonitorTrendsWidget::slot_archiveDataReceived); // For updating widget

	// Realtime Trends connections
	// IMPORTANT: The next to slot connections must be in that order, as TrendLib::TrendSignalSet::slot_realtimeDataReceived
	// updates the "last realtime" point and MonitorTrendsWidget::slot_realtimeDataReceived makes autoshift based on the "last realtime" point.
	//
	// Qt doc says: If several slots are connected to one signal, the slots will be executed one after the other,
	// in the order they have been connected, when the signal is emitted.
	//
	connect(&m_realtimeDataProvider, &ClientLib::RtDataProvider::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeDataReceived);
	connect(&m_realtimeDataProvider, &ClientLib::RtDataProvider::dataReady, this, &MonitorTrendsWidget::slot_realtimeDataReceived);

	connect(&m_realtimeDataProvider, &ClientLib::RtDataProvider::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeRequestError);
	connect(&m_realtimeDataProvider, &ClientLib::RtDataProvider::connectionLost, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeConnectionLost);


	// --
	//
	connect(&m_configController, &MonitorConfigController::configurationArrived, this, &MonitorTrendsWidget::slot_configurationArrived);

	startTimer(100);

	return;
}

MonitorTrendsWidget::~MonitorTrendsWidget()
{
	MonitorTrends::unregisterTrendWindow(this);

	m_archiveDataProvider.clear();
	m_realtimeDataProvider.clear();

	return;
}

void MonitorTrendsWidget::timerEvent(QTimerEvent*)
{
	QStatusBar* sb = statusBar();
	Q_ASSERT(sb);

	if (trendMode() == E::TrendMode::Archive)
	{
		ArchiveTrendTcpClient::Stat stat = m_archiveDataProvider.statistics();

		m_statusBarTextLabel->setText(stat.text);
		m_statusBarQueueSizeLabel->setText(QString(tr(" Queue: %1 ")).arg(stat.requestQueueSize));
		m_statusBarNetworkRequestsLabel->setText(tr(" Requests/replies: %1/%2 ")
													 .arg(stat.requestCount)
													 .arg(stat.replyCount));

		m_statusBarConnectionStateLabel->setText(tr(" Connected %1/%2").arg(stat.isConnected).arg(m_archiveDataProvider.size()));
	}
	else
	{
		// --
		//
		setRealtimeParams();

		// --
		//
		auto stat = m_realtimeDataProvider.statistics();

		m_statusBarTextLabel->setText(stat.text);
		m_statusBarQueueSizeLabel->setText("             ");
		m_statusBarNetworkRequestsLabel->setText(QString(" Requests/replies: %1/%2 ")
													 .arg(stat.requestCount)
													 .arg(stat.replyCount));

		m_statusBarConnectionStateLabel->setText(QString(" Connected %1/%2").arg(stat.isConnected).arg(m_realtimeDataProvider.size()));
	}

	return;
}

void MonitorTrendsWidget::signalsButton()
{
	// Get archive services
	//
	auto archiveServers = m_configController.configuration().archiveServices;
	std::vector<TrendLib::ArchiveServer> trendArchiveServers;
	trendArchiveServers.reserve(archiveServers.size());

	for (const auto& as : archiveServers)
	{
		trendArchiveServers.emplace_back(as.equipmentId, as.shortenId, as.appDataServiceId);
	}

	// Create signal list converted to TrendSignalParam	and expanded to different archive services
	//
	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(m_signalManager.signalsCount());

	// Create additional signals for archive services
	//
	for (auto&& allSignals = m_signalManager.signalList();
		 const AppSignalParam& sp : allSignals)
	{
		// Make signal copy for each ArchiveService which has this signal
		//
		for (const TrendLib::ArchiveServer& archiveService : trendArchiveServers)
		{
			if (m_signalManager.dataServiceHasSignal(archiveService.dataServiceId, sp.appSignalId()) == true)
			{
				trendSignals.emplace_back(sp, archiveService);
			}
		}
	}

	// Get already added signals
	//
	std::vector<TrendLib::TrendSignalParam> addedTrendSignals = signalSet().trendSignals();

	// Implement ISignalHasTag
	//
	struct SignalHasTag : ISignalHasTag
	{
		SignalHasTag(const ClientLib::AppSignalManager& ms) :
			monitorAppSignalManager(ms)
		{
		}

		virtual bool signalHasTag(const QString& signalId, const QString& tag) const override
		{
			return monitorAppSignalManager.signalHasTag(signalId, tag);
		}

		const ClientLib::AppSignalManager& monitorAppSignalManager;
	} signalHasTag{m_signalManager};

	// --
	//
	TrendLib::DialogChooseTrendSignals dialog(&signalHasTag,
											  trendSignals,
											  addedTrendSignals,
											  trendArchiveServers,
											  this);

	int result = dialog.exec();
	if (result == QDialog::Rejected)
	{
		return;
	}

	std::vector<TrendLib::TrendSignalParam> oldAnalogSignals = signalSet().analogSignals();

	std::vector<TrendLib::TrendSignalParam> acceptedSignals = dialog.acceptedSignals();
	updateSignals(acceptedSignals);

	// Set default scale type if analog signals are empty and selected signals have special tags
	//
	if (oldAnalogSignals.empty() == true)
	{
		autoSelectScaleType(acceptedSignals);
	}

	updateWidget();
	return;
}

void MonitorTrendsWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void MonitorTrendsWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		Q_ASSERT(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
		event->setDropAction(Qt::DropAction::IgnoreAction);
		event->accept();
		return;
	}

	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	auto archiveServers = m_configController.configuration().archiveServices;
	auto appDataServers = m_configController.configuration().appDataRealTimeServices;

	// Parse data
	//
	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			// Find the first suitable archive server
			//
			auto archServerIt = std::find_if(archiveServers.begin(), archiveServers.end(),
						[&appSignalParam, this](const SoftwareEndpoint::ArchiveService& as)
											 {
												 return m_signalManager.dataServiceHasSignal(as.appDataServiceId, appSignalParam.appSignalId());
											 });

			auto appDataServerIt = std::find_if(appDataServers.begin(), appDataServers.end(),
						[&appSignalParam, this](const SoftwareEndpoint::AppDataService& ads)
												{
													return m_signalManager.dataServiceHasSignal(ads.equipmentId, appSignalParam.appSignalId());
												});

			if (archServerIt != archiveServers.end())
			{
				const SoftwareEndpoint::ArchiveService& as = *archServerIt;
				TrendLib::ArchiveServer trendArchiveServer{as.equipmentId, as.shortenId, as.appDataServiceId};

				TrendLib::TrendSignalParam tsp{appSignalParam, trendArchiveServer};
				addSignal(tsp, false);
			}
			else
			{
				qDebug() << "MonitorTrendsWidget::dropEvent: Archive server for signal " << appSignalParam.appSignalId() << " is not found.";

				// This situation is possible if archive service is not configured, but still realtime trends is possible, add it as is.
				//
				if (appDataServerIt != appDataServers.end())
				{
					const SoftwareEndpoint::AppDataService& ads = *appDataServerIt;

					TrendLib::ArchiveServer trendArchiveServer{"", "", ads.equipmentId};
					TrendLib::TrendSignalParam tsp{appSignalParam, trendArchiveServer};

					addSignal(tsp, false);
				}
			}
		}
	}

	updateWidget();

	return;
}

void MonitorTrendsWidget::createArchiveConnection()
{
	m_archiveDataProvider.createConnections();
	return;
}

void MonitorTrendsWidget::createRealtimeConnection()
{
	m_realtimeDataProvider.createConnections(m_configController.softwareInfo(),
											 m_configController.configuration().appDataRealTimeServices);
	setRealtimeParams();

	return;
}

void MonitorTrendsWidget::setRealtimeParams()
{
	E::RtTrendsSamplePeriod samplePeriod = E::RtTrendsSamplePeriod::sp_100ms;
	qint64 duration = this->duration();

	if (duration <= 2_sec)
	{
		samplePeriod = E::RtTrendsSamplePeriod::sp_5ms;
	}
	else
	{
		if (duration <= 5_sec)
		{
			samplePeriod = E::RtTrendsSamplePeriod::sp_10ms;
		}
		else
		{
			if (duration <= 10_sec)
			{
				samplePeriod = E::RtTrendsSamplePeriod::sp_20ms;
			}
			else
			{
				if (duration <= 20_sec)
				{
					samplePeriod = E::RtTrendsSamplePeriod::sp_50ms;
				}
				else
				{
					if (duration <= 1_min)
					{
						samplePeriod = E::RtTrendsSamplePeriod::sp_100ms;
					}
					else
					{
						if (duration <= 1_min + 30_sec)
						{
							samplePeriod = E::RtTrendsSamplePeriod::sp_250ms;
						}
						else
						{
							if (duration <= 3_min)
							{
								samplePeriod = E::RtTrendsSamplePeriod::sp_500ms;
							}
							else
							{
								if (duration <= 15_min)
								{
									samplePeriod = E::RtTrendsSamplePeriod::sp_1s;
								}
								else
								{
									if (duration <= 60_min)
									{
										samplePeriod = E::RtTrendsSamplePeriod::sp_5s;
									}
									else
									{
										samplePeriod = E::RtTrendsSamplePeriod::sp_10s;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	QStringList trendSignals = signalSet().trendSignalIds();

	m_realtimeDataProvider.setData(samplePeriod, trendSignals);

	return;
}

void MonitorTrendsWidget::slot_requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId, TimeStamp hourToRequest, E::TimeType timeType)
{
	m_archiveDataProvider.requestData(signalPlusServerId, hourToRequest, timeType);
	return;
}

void MonitorTrendsWidget::slot_archiveDataReceived(TrendLib::TrendSignalPlusServerId /*appSignalId*/, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> /*data*/)
{
	TimeStamp plus1hour(requestedHour.timeStamp + 1_hour);
	TimeStamp minus1hour(requestedHour.timeStamp - 1_hour);

	if (timeType != this->timeType() ||
		(isTimeInRange(requestedHour) == false &&
		 isTimeInRange(plus1hour) == false &&
		 isTimeInRange(minus1hour) == false))
	{
		return;
	}

	updateWidget();
	return;
}

void MonitorTrendsWidget::slot_realtimeDataReceived(QString /*sourceEquipmentId*/,
													std::shared_ptr<TrendLib::RealtimeData> data,
													TrendLib::TrendStateItem minRecState,
													TrendLib::TrendStateItem maxRecState)
{
	Q_ASSERT(data);

	if (data->signalData.empty() == true)
	{
		return;
	}

	const TrendLib::RealtimeDataChunk& chunk = data->signalData.front();
	if (chunk.states.empty() == true)
	{
		return;
	}

#if 0
	// This is the old way of shifting time axis, it is good when LMs have the same time,
	// but it starts to flick when LMs have time disparency.
	//  
	TimeStamp minTime = minState.getTime(m_trendWidget->timeType());
	TimeStamp maxTime = maxState.getTime(m_trendWidget->timeType());

	// Shift view area if autoshift mode is turned on
	//
	if (isRealtimeAutoShift() == true)
	{
		setRealtimeAutoShift(maxTime);
	}

	// Update widget if received data somewhere in view
	//
	if (minTime >= TimeStamp{m_trendWidget->startTime().timeStamp - m_trendWidget->duration() / 10} &&
	    maxTime <= TimeStamp{m_trendWidget->finishTime().timeStamp + m_trendWidget->duration() / 10})
	{
		m_trendWidget->updateWidget();
	}
#else
	auto timeType = this->timeType();
	TrendLib::TrendStateItem maxState{};

	for (const auto trendSignals = signalSet().trendSignalsHashes();
		 const auto& trendSignalHash : trendSignals)
	{
		std::optional<TrendLib::TrendStateItem> state = signalSet().lastRealtimeState(trendSignalHash, timeType);

		if (state.has_value() == true && state->getTime(timeType) > maxState.getTime(timeType))
		{
			maxState = *state;
		}
	}

	// Shift view area if autoshift mode is turned on and maxState is present.
	//
	if (isRealtimeAutoShift() == true && maxState.getTime(timeType) != 0)
	{
		setRealtimeAutoShift(maxState.getTime(timeType));
	}

	// Update widget if received data somewhere in the current view.
	//
	TimeStamp minTime = minRecState.getTime(this->timeType());
	TimeStamp maxTime = maxRecState.getTime(this->timeType());

	if (m_realtimeUpdateTimer.isValid() == false)
	{
		m_realtimeUpdateTimer.start();
	}

	// Force to update trend every 500 ms, as signal values (indicator on the left)
	// should be updated even if the trend point not in the current view.
	//
	bool updateByTimer = m_realtimeUpdateTimer.elapsed() > 500;

	if (updateByTimer == true ||
		(minTime >= TimeStamp{this->startTime().timeStamp - this->duration() / 10} &&
		 maxTime <= TimeStamp{this->finishTime().timeStamp + this->duration() / 10}))
	{
		updateWidget();
		m_realtimeUpdateTimer.restart();
	}
#endif

	return;
}

void MonitorTrendsWidget::slot_trendModeChanged()
{
	qDebug() << __FUNCTION__ << ", TrendMode = " << trendMode();

	m_archiveDataProvider.clear();
	m_realtimeDataProvider.clear();

	if (trendMode() == E::TrendMode::Archive)
	{
		createArchiveConnection();
	}
	else
	{
		createRealtimeConnection();
	}

	return;
}

void MonitorTrendsWidget::slot_configurationArrived(MonitorConfigSettings /*configuration*/)
{
	if (trendMode() == E::TrendMode::Archive)
	{
		m_archiveDataProvider.updateConnections();
	}
	else
	{
		m_realtimeDataProvider.updateConnections(m_configController.softwareInfo(),
												 m_configController.configuration().appDataRealTimeServices);
	}
}
