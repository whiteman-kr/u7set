#include "MonitorTrends.h"
#include "../lib/ISignalHasTag.h"
#include "../ClientLib/RtTrendTcpClient.h"
#include "../TrendView/TrendWidget.h"
#include "../TrendView/DialogChooseTrendSignals.h"

std::map<QString, MonitorTrendsWidget*> MonitorTrends::m_trendsList;

std::vector<QString> MonitorTrends::getTrendsList()
{
	std::vector<QString> result;
	result.reserve(m_trendsList.size());

	for (std::pair<QString, MonitorTrendsWidget*> p : m_trendsList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool MonitorTrends::activateTrendWindow(QString trendName)
{
	if (m_trendsList.count(trendName) != 1)
	{
		Q_ASSERT(m_trendsList.count(trendName) != 1);
		return false;
	}

	MonitorTrendsWidget* widget = m_trendsList[trendName];
	Q_ASSERT(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorTrends::startTrendApp(const MonitorSignalManager& signalManager,
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
		auto archServerIt = std::find_if(archiveServers.begin(), archiveServers.end(),
					[&appSignal, &signalManager](const auto& as)
					{
						return signalManager.dataServiceHasSignal(as.appDataServiceId, appSignal.appSignalId());
					});

		auto appDataServerIt = std::find_if(appDataServers.begin(), appDataServers.end(),
					[&appSignal, &signalManager](const auto& ads)
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

void MonitorTrends::registerTrendWindow(QString name, MonitorTrendsWidget* window)
{
	Q_ASSERT(m_trendsList.count(name) == 0);
	m_trendsList[name] = window;
}

void MonitorTrends::unregisterTrendWindow(QString name)
{
	Q_ASSERT(m_trendsList.count(name) == 1);
	m_trendsList.erase(name);
}


MonitorTrendsWidget::MonitorTrendsWidget(const MonitorSignalManager& signalManager,
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
	MonitorTrends::registerTrendWindow(trendName, this);

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
	connect(m_trendWidget, &TrendLib::TrendWidget::trendModeChanged, this, &MonitorTrendsWidget::slot_trendModeChanged);

	// Acrhive connection
	//
	connect(&m_trendWidget->signalSet(), &TrendLib::TrendSignalSet::requestData, this, &MonitorTrendsWidget::slot_requestData);
	connect(&m_archiveDataProvider, &MonitorTrendArchiveConnections::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_archiveDataReceived);
	connect(&m_archiveDataProvider, &MonitorTrendArchiveConnections::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_archiveRequestError);
	connect(&m_archiveDataProvider, &MonitorTrendArchiveConnections::dataReady, this, &MonitorTrendsWidget::slot_archiveDataReceived);	// For updating widget

	// Realtime Trends connections
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
	MonitorTrends::unregisterTrendWindow(this->windowTitle());

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
		ClientLib::RtTrendTcpClient::Stat stat = m_realtimeDataProvider.statistics();

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
	// Get archiev services
	//
	auto archiveServers = m_configController.configuration().archiveServices;
	std::vector<TrendLib::ArchiveServer> trendArchiveServers;
	trendArchiveServers.reserve(archiveServers.size());

	for (const auto& as : archiveServers)
	{
		trendArchiveServers.emplace_back(as.equipmentId, as.shortenId, as.appDataServiceId);
	}

	// Create signal list converted to TrendSignalParam	and expanded to diffrent archive services
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

	// Get alread added signals
	//
	std::vector<TrendLib::TrendSignalParam> addedTrendSignals = signalSet().trendSignals();

	// Implement ISignalHasTag
	//
	struct SignalHasTag : ISignalHasTag
	{
		SignalHasTag(const MonitorSignalManager& ms) :
			monitorSignalManager(ms)
		{
		}

		virtual bool signalHasTag(const QString& signalId, const QString& tag) const  override
		{
			return monitorSignalManager.signalHasTag(signalId, tag);
		}

		const MonitorSignalManager& monitorSignalManager;
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
	qint64 duration = m_trendWidget->duration();

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

	QStringList trendSignals = trend().signalSet().trendSignalIds();

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
	Q_ASSERT(m_trendWidget);
	Q_ASSERT(m_trendSlider);

	TimeStamp plus1hour(requestedHour.timeStamp + 1_hour);
	TimeStamp minus1hour(requestedHour.timeStamp - 1_hour);

	if (timeType != m_trendWidget->timeType() ||
		(m_trendSlider->isTimeInRange(requestedHour) == false &&
		 m_trendSlider->isTimeInRange(plus1hour) == false &&
		 m_trendSlider->isTimeInRange(minus1hour) == false))
	{
		return;
	}

	m_trendWidget->updateWidget();
	return;
}

void MonitorTrendsWidget::slot_realtimeDataReceived(QString /*sourceEquipmentId*/,
													std::shared_ptr<TrendLib::RealtimeData> data,
													TrendLib::TrendStateItem minState,
													TrendLib::TrendStateItem maxState)
{
	Q_ASSERT(m_trendWidget);
	Q_ASSERT(m_trendSlider);
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

void MonitorTrendsWidget::slot_configurationArrived(ConfigSettings /*configuration*/)
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
