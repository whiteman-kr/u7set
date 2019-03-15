#include "MonitorTrends.h"
#include "DialogChooseTrendSignals.h"
#include "../TrendView/TrendWidget.h"

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

bool MonitorTrends::startTrendApp(MonitorConfigController* configController, const std::vector<AppSignalParam>& appSignals, QWidget* parent)
{
	MonitorTrendsWidget* window = new MonitorTrendsWidget(configController, parent);

	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(appSignals.size());

	for (const AppSignalParam& appSignal : appSignals)
	{
		TrendLib::TrendSignalParam tsp(appSignal);
		trendSignals.push_back(tsp);
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


MonitorTrendsWidget::MonitorTrendsWidget(MonitorConfigController* configController, QWidget* parent) :
	TrendLib::TrendMainWindow(parent),
	m_configController(configController),
	m_archiveService1(configController->configuration().archiveService1),
	m_archiveService2(configController->configuration().archiveService2)
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
	std::vector<TrendLib::TrendSignalParam> trendSignals = signalSet().trendSignals();

	// --
	//
	DialogChooseTrendSignals dialog(trendSignals, this);
	
	int result = dialog.exec();
	
	if (result == QDialog::Rejected)
	{
		return;
	}

	std::vector<AppSignalParam> acceptedSignals = dialog.acceptedSignals();

	// Remove signals
	//
	std::vector<TrendLib::TrendSignalParam> discreteSignals = signalSet().discreteSignals();
	std::vector<TrendLib::TrendSignalParam> analogSignals = signalSet().analogSignals();

	for (const TrendLib::TrendSignalParam& ds : discreteSignals)
	{
		auto it = std::find_if(acceptedSignals.begin(), acceptedSignals.end(),
						[&ds](const AppSignalParam& trendSignal)
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
						[&as](const AppSignalParam& trendSignal)
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
	for (const AppSignalParam& signal : acceptedSignals)
	{
		TrendLib::TrendSignalParam tsp(signal);
		addSignal(tsp, false);
	}

	updateWidget();

	return;
}

void MonitorTrendsWidget::createArchiveConnection()
{
	Q_ASSERT(m_configController);
	Q_ASSERT(m_archiveTcpClient == nullptr);
	Q_ASSERT(m_archiveTcpClientThread == nullptr);

	m_archiveTcpClient = new ArchiveTrendTcpClient(m_configController);

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

	m_rtTcpClient = new RtTrendTcpClient(m_configController);

	m_rtTcpClientThread = new SimpleThread(m_rtTcpClient);	// Archive mode is default one
	m_rtTcpClientThread->start();

	connect(m_rtTcpClient, &RtTrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeDataReceived);
	connect(m_rtTcpClient, &RtTrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_realtimeRequestError);

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

	//	enum class RtTrendsSamplePeriod
	//	{
	//		sp_5ms,
	//		sp_10ms,
	//		sp_20ms,
	//		sp_50ms,
	//		sp_100ms,
	//		sp_250ms,
	//		sp_500ms,
	//		sp_1s,
	//		sp_5s,
	//		sp_15s,
	//		sp_30s,
	//		sp_60s,
	//	};

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

	std::vector<TrendLib::TrendSignalParam> signalSetVector = trend().signalSet().trendSignals();

	m_rtTcpClient->setData(samplePeriod, signalSetVector);

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
	if (minTime >= m_trendWidget->startTime().timeStamp - m_trendWidget->duration() / 10 &&
		maxTime <= m_trendWidget->finishTime().timeStamp + m_trendWidget->duration() / 10)
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
