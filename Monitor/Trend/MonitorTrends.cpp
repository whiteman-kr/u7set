#include "MonitorTrends.h"
#include "../lib/ISignalHasTag.h"
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

bool MonitorTrends::startTrendApp(const MonitorSignalManager* signalManager,
								  const MonitorConfigController* configController,
                                  const std::vector<AppSignalParam>& appSignals,
                                  QWidget* parent)
{
	MonitorTrendsWidget* window = new MonitorTrendsWidget(signalManager, configController, parent);

	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(appSignals.size());

	auto archiveServers = configController->configuration().archiveServices;

	for (const AppSignalParam& appSignal : appSignals)
	{
		// Take the firss available srchive server for the signal
		//
		for (const auto& server : archiveServers)
		{
			if (signalManager->dataServiceHasSignal(server.appDataServiceId, appSignal.appSignalId()) == true)
			{
				TrendLib::ArchiveServer trendArchiveServer{server.equipmentId, server.shortenId, server.appDataServiceId};
				trendSignals.emplace_back(appSignal, std::move(trendArchiveServer));
				break;
			}
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


MonitorTrendsWidget::MonitorTrendsWidget(const MonitorSignalManager* m_signalManager,
										 const MonitorConfigController* configController,
										 QWidget* parent) :
	TrendLib::TrendMainWindow(parent),
	m_signalManager(m_signalManager),
	m_configController(configController)
{
static int no = 1;
	QString trendName = QString("Monitor Trends %1").arg(no++);
	MonitorTrends::registerTrendWindow(trendName, this);

	setWindowTitle(trendName);

	// Status bar
	//
	QStatusBar* sb = statusBar();
	Q_ASSERT(sb);

	m_statusBarTextLabel = new QLabel(sb);
	m_statusBarQueueSizeLabel = new QLabel(sb);
	m_statusBarNetworkRequestsLabel = new QLabel(sb);
	m_statusBarServerLabel = new QLabel(sb);
	m_statusBarConnectionStateLabel = new QLabel(sb);

	sb->addWidget(m_statusBarTextLabel, 1);
	sb->addWidget(m_statusBarQueueSizeLabel, 0);
	sb->addWidget(m_statusBarNetworkRequestsLabel, 0);
	sb->addWidget(m_statusBarServerLabel, 0);
	sb->addWidget(m_statusBarConnectionStateLabel, 0);

	// Communication thread
	//
	createArchiveConnection();

	// --
	//
	connect(m_trendWidget, &TrendLib::TrendWidget::trendModeChanged, this, &MonitorTrendsWidget::slot_trendModeChanged);
	//connect(m_tcpClient, &TcpSignalClient::connectionReset, this, &MonitorMainWindow::tcpSignalClient_connectionReset);
	// --
	//
	startTimer(100);

	return;
}

MonitorTrendsWidget::~MonitorTrendsWidget()
{
	MonitorTrends::unregisterTrendWindow(this->windowTitle());

	if (m_archiveTcpClientThread != nullptr)
	{
		m_archiveTcpClientThread->quitAndWait(10000);
		delete m_archiveTcpClientThread;
	}

	if (m_rtTcpClientThread != nullptr)
	{
		m_rtTcpClientThread->quitAndWait(10000);
		delete m_rtTcpClientThread;
	}

	return;
}

void MonitorTrendsWidget::timerEvent(QTimerEvent*)
{
	QStatusBar* sb = statusBar();
	Q_ASSERT(sb);

	if (trendMode() == E::TrendMode::Archive)
	{
		Q_ASSERT(m_archiveTcpClient);

		ArchiveTrendTcpClient::Stat stat = m_archiveTcpClient->stat();

		m_statusBarTextLabel->setText(stat.text);
		m_statusBarQueueSizeLabel->setText(QString(" Queue size: %1 ").arg(stat.requestQueueSize));
		m_statusBarNetworkRequestsLabel->setText(QString(" Network requests/replies: %1/%2 ")
												 .arg(stat.requestCount)
												 .arg(stat.replyCount));

		HostAddressPort server = m_archiveTcpClient->currentServerAddressPort();
		m_statusBarServerLabel->setText(QString(" ArchiveServer: %1 ").arg(server.addressPortStr()));

		if (m_archiveTcpClient->isConnected() == true)
		{
			m_statusBarConnectionStateLabel->setText(" Connected ");
		}
		else
		{
			m_statusBarConnectionStateLabel->setText(" NoConnection ");
		}
	}
	else
	{
		Q_ASSERT(m_rtTcpClient);

		// --
		//
		setRealtimeParams();

		// --
		//
		RtTrendTcpClient::Stat stat = m_rtTcpClient->stat();

		m_statusBarTextLabel->setText(stat.text);
		m_statusBarQueueSizeLabel->setText("");
		m_statusBarNetworkRequestsLabel->setText(QString(" Network requests/replies: %1/%2 ")
												 .arg(stat.requestCount)
												 .arg(stat.replyCount));

		HostAddressPort server = m_rtTcpClient->currentServerAddressPort();
		m_statusBarServerLabel->setText(QString(" RtSource: %1 ").arg(server.addressPortStr()));

		if (m_rtTcpClient->isConnected() == true)
		{
			m_statusBarConnectionStateLabel->setText(" Connected ");
		}
		else
		{
			m_statusBarConnectionStateLabel->setText(" NoConnection ");
		}
	}

	return;
}

void MonitorTrendsWidget::signalsButton()
{
	// Get archiev services
	//
	std::vector<MonitorSettings::ArchiveService> archiveServers = m_configController->configuration().archiveServices;
	std::vector<TrendLib::ArchiveServer> trendArchiveServers;
	trendArchiveServers.reserve(archiveServers.size());

	for (const auto& as : archiveServers)
	{
		trendArchiveServers.emplace_back(as.equipmentId, as.shortenId, as.appDataServiceId);
	}

	// Create signal list converted to TrendSignalParam	and expanded to diffrent archive services
	//
	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(m_signalManager->signalsCount());

	// Create additional signals for archive services
	//
	for (const AppSignalParam& sp : m_signalManager->signalList())
	{
		// Make signal copy for each ArchiveService which has this signal
		//
		for (const TrendLib::ArchiveServer& archiveService : trendArchiveServers)
		{
			if (m_signalManager->dataServiceHasSignal(archiveService.dataServiceId, sp.appSignalId()) == true)
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
		SignalHasTag(const MonitorSignalManager* ms) : monitorSignalManager(ms)
		{
		}

		virtual bool signalHasTag(const QString& signalId, const QString& tag) const  override
		{
			Q_ASSERT(monitorSignalManager);
			return monitorSignalManager->signalHasTag(signalId, tag);
		}

		const MonitorSignalManager* monitorSignalManager = nullptr;
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

	std::vector<TrendLib::TrendSignalParam> acceptedSignals = dialog.acceptedSignals();

	// Remove signals
	//
	std::vector<TrendLib::TrendSignalParam> discreteSignals = signalSet().discreteSignals();
	std::vector<TrendLib::TrendSignalParam> analogSignals = signalSet().analogSignals();

	for (const TrendLib::TrendSignalParam& ds : discreteSignals)
	{
		auto it = std::find_if(acceptedSignals.begin(), acceptedSignals.end(),
						[&ds](const auto& trendSignal)
						{
							return trendSignal.appSignalId() == ds.appSignalId();
						});

		if (it == acceptedSignals.end())
		{
			signalSet().removeSignal(ds.appSignalId());
		}
	}

	for (const TrendLib::TrendSignalParam& as : analogSignals)
	{
		auto it = std::find_if(acceptedSignals.begin(), acceptedSignals.end(),
						[&as](const auto& trendSignal)
						{
							return trendSignal.appSignalId() == as.appSignalId();
						});

		if (it == acceptedSignals.end())
		{
			signalSet().removeSignal(as.appSignalId());
		}
	}

	// Add new signals
	//
	for (const auto& signal : acceptedSignals)
	{
		addSignal(signal, false);
	}

	// Set default scale type if analog signals are empty and selected signals have special tags
	//

	if (analogSignals.empty() == true)
	{
		autoSelectScaleType(acceptedSignals);
	}

	updateWidget();

	return;
}

void MonitorTrendsWidget::createArchiveConnection()
{
	Q_ASSERT(m_configController);
	Q_ASSERT(m_archiveTcpClient == nullptr);
	Q_ASSERT(m_archiveTcpClientThread == nullptr);

	m_archiveTcpClient = new ArchiveTrendTcpClient(m_configController, m_configController->logFile());

	m_archiveTcpClientThread = new SimpleThread(m_archiveTcpClient);	// Archive mode is default one
	m_archiveTcpClientThread->start();

	connect(&signalSet(), &TrendLib::TrendSignalSet::requestData, m_archiveTcpClient, &ArchiveTrendTcpClient::slot_requestData);

	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_archiveDataReceived);
	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_archiveRequestError);

	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, this, &MonitorTrendsWidget::slot_archiveDataReceived);	// Fpr updating widget

	return;
}

void MonitorTrendsWidget::createRealtimeConnection()
{
	Q_ASSERT(m_configController);
	Q_ASSERT(m_rtTcpClient == nullptr);
	Q_ASSERT(m_rtTcpClientThread == nullptr);

	m_rtTcpClient = new RtTrendTcpClient(m_configController, m_configController->logFile());

	m_rtTcpClientThread = new SimpleThread(m_rtTcpClient);	// Archive mode is default one
	m_rtTcpClientThread->start();

	connect(m_rtTcpClient, &RtTrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeDataReceived);
	connect(m_rtTcpClient, &RtTrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeRequestError);
	connect(m_rtTcpClient, &RtTrendTcpClient::connectionLost, &signalSet(), qOverload<>(&TrendLib::TrendSignalSet::addNonValidPoint));

	connect(m_rtTcpClient, &RtTrendTcpClient::dataReady, this, &MonitorTrendsWidget::slot_realtimeDataReceived);

	setRealtimeParams();

	return;
}

void MonitorTrendsWidget::setRealtimeParams()
{
	if (m_rtTcpClient == nullptr ||
		m_rtTcpClientThread == nullptr)
	{
		Q_ASSERT(m_rtTcpClient);
		Q_ASSERT(m_rtTcpClientThread);
		return;
	}

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

	m_rtTcpClient->setData(samplePeriod, trendSignals);

	return;
}

void MonitorTrendsWidget::slot_archiveDataReceived(QString /*appSignalId*/, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> /*data*/)
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

void MonitorTrendsWidget::slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState)
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

	if (m_archiveTcpClientThread != nullptr)
	{
		m_archiveTcpClientThread->quitAndWait(10000);
		delete m_archiveTcpClientThread;

		m_archiveTcpClient = nullptr;
		m_archiveTcpClientThread = nullptr;
	}

	if (m_rtTcpClientThread != nullptr)
	{
		m_rtTcpClientThread->quitAndWait(10000);
		delete m_rtTcpClientThread;

		m_rtTcpClient = nullptr;
		m_rtTcpClientThread = nullptr;
	}

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
