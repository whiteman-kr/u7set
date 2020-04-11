#include "SimTrends.h"
#include "../SimIdeSimulator.h"
#include "../../TrendView/Forms/DialogChooseTrendSignals.h"
#include "../../TrendView/TrendWidget.h"


std::map<QString, SimTrendsWidget*> SimTrends::m_trendsList;

std::vector<QString> SimTrends::getTrendsList()
{
	std::vector<QString> result;
	result.reserve(m_trendsList.size());

	for (std::pair<QString, SimTrendsWidget*> p : m_trendsList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool SimTrends::activateTrendWindow(QString trendName)
{
	if (m_trendsList.count(trendName) != 1)
	{
		Q_ASSERT(m_trendsList.count(trendName) != 1);
		return false;
	}

	SimTrendsWidget* widget = m_trendsList[trendName];
	Q_ASSERT(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool SimTrends::startTrendApp(std::shared_ptr<SimIdeSimulator> simulator, const std::vector<AppSignalParam>& appSignals, QWidget* parent)
{
	SimTrendsWidget* window = new SimTrendsWidget(simulator, parent);

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

void SimTrends::registerTrendWindow(QString name, SimTrendsWidget* window)
{
	Q_ASSERT(m_trendsList.count(name) == 0);
	m_trendsList[name] = window;
}

void SimTrends::unregisterTrendWindow(QString name)
{
	Q_ASSERT(m_trendsList.count(name) == 1);
	m_trendsList.erase(name);
}


SimTrendsWidget::SimTrendsWidget(std::shared_ptr<SimIdeSimulator> simulator, QWidget* parent) :
	TrendLib::TrendMainWindow(parent),
	m_simulator(simulator)
{
	assert(m_simulator);

static int no = 1;
	QString trendName = QString("Simulator Trends %1").arg(no++);
	SimTrends::registerTrendWindow(trendName, this);

	setWindowTitle(trendName);

	// Set ruller step to 5ms, as in simulator cycle alway multiple to 5
	//
	trend().rulerSet().setRulerStep(5);

	// Set deafult lane duration (5m), it differs from default value (1h)
	//
	m_timeCombo->setCurrentIndex(5);	// 5 is index in combo box
	m_trendWidget->setLaneDuration(5_min);

	// Hide Refresh button as it is not required for simulator, no archive here just "realtime" data
	//
	m_refreshButton->setEnabled(false);				// This is button Refresh
	m_refreshActionForButton->setVisible(false);	// To hide button from toolbar the QAction for this button must be hidden

	m_refreshAction->setVisible(false);

	// TimeType, assume we have only simulated PlandTime
	//
	m_trendWidget->setTimeType(E::TimeType::Plant);
	m_timeTypeCombo->setCurrentIndex(m_timeTypeCombo->findData(QVariant::fromValue(E::TimeType::Plant)));
	m_timeTypeCombo->setEnabled(false);

	// Set realtime mode, and hide Realtime button
	//
	m_realtimeModeButton->setChecked(true);

	m_realtimeActionForButton->setVisible(false);	// To hide button from toolbar the QAction for this button must be hidden
	m_realtimeModeButton->setEnabled(false);

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

	// --
	//
	startTimer(50);

	return;
}

SimTrendsWidget::~SimTrendsWidget()
{
	SimTrends::unregisterTrendWindow(this->windowTitle());

	return;
}

void SimTrendsWidget::timerEvent(QTimerEvent*)
{
	m_timerCounter ++;
	quint64 durationSec = m_trendWidget->duration() / 1000;

	if (durationSec <= 30)
	{
		fetchTrendData();						// fetch every 50 ms
	}
	else
	{
		if (durationSec <= 1 * 60) 	// 1 min
		{
			if (m_timerCounter % 2 == 0)		// fetch every 100 ms
			{
				fetchTrendData();
			}
		}
		else
		{
			if (durationSec <= 5 * 60) 	// 1 min
			{
				if (m_timerCounter % 4 == 0)	// fetch every 200 ms
				{
					fetchTrendData();
				}
			}
			else
			{
				if (durationSec <= 30 * 60) 	// 1 min
				{
					if (m_timerCounter % 10 == 0)	// fetch every 500 ms
					{
						fetchTrendData();
					}
				}
				else
				{
					if (m_timerCounter % 20 == 0)	// fetch every 1 second
					{
						fetchTrendData();
					}
				}
			}
		}
	}


	QStatusBar* sb = statusBar();
	Q_ASSERT(sb);

//	{
//		m_statusBarTextLabel->setText(stat.text);
//		m_statusBarQueueSizeLabel->setText("");
//		m_statusBarNetworkRequestsLabel->setText(QString(" Network requests/replies: %1/%2 ")
//												 .arg(stat.requestCount)
//												 .arg(stat.replyCount));

//		HostAddressPort server = m_rtTcpClient->currentServerAddressPort();
//		m_statusBarServerLabel->setText(QString(" RtSource: %1 ").arg(server.addressPortStr()));

//		if (m_rtTcpClient->isConnected() == true)
//		{
//			m_statusBarConnectionStateLabel->setText(" Connected ");
//		}
//		else
//		{
//			m_statusBarConnectionStateLabel->setText(" NoConnection ");
//		}
//	}

	return;
}

void SimTrendsWidget::signalsButton()
{
	std::vector<TrendLib::TrendSignalParam> trendSignals = signalSet().trendSignals();

	// --
	//
	DialogChooseTrendSignals dialog(&m_simulator->appSignalManager(), trendSignals, this);
	
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

void SimTrendsWidget::fetchTrendData()
{
	// Fetch realtime trend data from Sim::AppSignalManager
	//
	Q_ASSERT(m_simulator);

	TrendLib::TrendStateItem minState;
	TrendLib::TrendStateItem maxState;

	minState.clear();
	maxState.clear();

	std::shared_ptr<TrendLib::RealtimeData> data = m_simulator->appSignalManager().trendData(windowTitle(),
																							 signalSet().trendSignalsHashes(),
																							 &minState,
																							 &maxState);

	if (data != nullptr)
	{
		signalSet().slot_realtimeDataReceived(data, minState, maxState);
		this->slot_realtimeDataReceived(data, minState, maxState);
	}

	return;
}

void SimTrendsWidget::slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState)
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
