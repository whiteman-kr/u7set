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

bool MonitorTrends::startTrendApp(MonitorConfigController* configController, QWidget* parent)
{
	MonitorTrendsWidget* window = new MonitorTrendsWidget(configController, parent);

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
	m_tcpClient =  new TrendTcpClient(configController);

	m_tcpClientThread = new SimpleThread(m_tcpClient);
	m_tcpClientThread->start();

	connect(&signalSet(), &TrendLib::TrendSignalSet::requestData, m_tcpClient, &TrendTcpClient::slot_requestData);
	connect(m_tcpClient, &TrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_dataReceived);
	connect(m_tcpClient, &TrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_requestError);

	connect(m_tcpClient, &TrendTcpClient::dataReady, this, &MonitorTrendsWidget::slot_dataReceived);

	//connect(m_tcpClient, &TcpSignalClient::signalParamAndUnitsArrived, this, &MonitorMainWindow::tcpSignalClient_signalParamAndUnitsArrived);
	//connect(m_tcpClient, &TcpSignalClient::connectionReset, this, &MonitorMainWindow::tcpSignalClient_connectionReset);
	// --
	//
	startTimer(100);

	return;
}

MonitorTrendsWidget::~MonitorTrendsWidget()
{
	MonitorTrends::unregisterTrendWindow(this->windowTitle());

	m_tcpClientThread->quitAndWait(10000);
	delete m_tcpClientThread;

	return;
}

void MonitorTrendsWidget::timerEvent(QTimerEvent*)
{
	QStatusBar* sb = statusBar();
	assert(sb);

	m_statusBarTextLabel->setText(m_tcpClient->m_statRequestDescription);
	m_statusBarQueueSizeLabel->setText(QString(" Queue size: %1 ").arg(m_tcpClient->m_statRequestQueueSize));
	m_statusBarNetworkRequestsLabel->setText(QString(" Network requests/replies: %1/%2 ").arg(m_tcpClient->m_statTcpRequestCount).arg(m_tcpClient->m_statTcpReplyCount));
	HostAddressPort server = m_tcpClient->currentServerAddressPort();
	m_statusBarServerLabel->setText(QString(" ArchiveServer: %1 ").arg(server.addressPortStr()));

	if (m_tcpClient->isConnected() == true)
	{
		m_statusBarConnectionStateLabel->setText(" Connected ");
	}
	else
	{
		m_statusBarConnectionStateLabel->setText(" NoConnection ");
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

void MonitorTrendsWidget::slot_dataReceived(QString appSignalId, TimeStamp requestedHour, TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data)
{
	assert(m_trendWidget);
	assert(m_trendSlider);

	if (timeType != m_trendWidget->timeType() ||
		m_trendSlider->isTimeInRange(requestedHour) == false)
	{
		return;
	}

	m_trendWidget->updateWidget();
	return;
}
