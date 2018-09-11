#include "Stable.h"
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
		assert(m_trendsList.count(trendName) != 1);
		return false;
	}

	MonitorTrendsWidget* widget = m_trendsList[trendName];
	assert(widget);

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
	assert(m_trendsList.count(name) == 0);
	m_trendsList[name] = window;
}

void MonitorTrends::unregisterTrendWindow(QString name)
{
	assert(m_trendsList.count(name) == 1);
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
	assert(sb);

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
	assert(sb);

	if (trendMode() == E::TrendMode::Archive)
	{
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
	assert(m_configController);
	assert(m_archiveTcpClient == nullptr);
	assert(m_archiveTcpClientThread == nullptr);

	m_archiveTcpClient = new ArchiveTrendTcpClient(m_configController);

	m_archiveTcpClientThread = new SimpleThread(m_archiveTcpClient);	// Archive mode is default one
	m_archiveTcpClientThread->start();

	connect(&signalSet(), &TrendLib::TrendSignalSet::requestData, m_archiveTcpClient, &ArchiveTrendTcpClient::slot_requestData);
	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_dataReceived);
	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_requestError);

	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, this, &MonitorTrendsWidget::slot_dataReceived);

	return;
}

void MonitorTrendsWidget::createRealtimeConnection()
{
	assert(m_configController);
	assert(m_rtTcpClient == nullptr);
	assert(m_rtTcpClientThread == nullptr);

	m_rtTcpClient = new RtTrendTcpClient(m_configController);

	m_rtTcpClientThread = new SimpleThread(m_rtTcpClient);	// Archive mode is default one
	m_rtTcpClientThread->start();

	//connect(&signalSet(), &TrendLib::TrendSignalSet::requestData, m_archiveTcpClient, &ArchiveTrendTcpClient::slot_requestData);
	//connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_dataReceived);
	//connect(m_archiveTcpClient, &ArchiveTrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_requestError);

	//connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, this, &MonitorTrendsWidget::slot_dataReceived);

	return;
}

void MonitorTrendsWidget::slot_dataReceived(QString /*appSignalId*/, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> /*data*/)
{
	assert(m_trendWidget);
	assert(m_trendSlider);

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
