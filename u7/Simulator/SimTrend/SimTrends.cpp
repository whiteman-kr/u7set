#include "SimTrends.h"
#include "../SimIdeSimulator.h"

#include <AppSignalLists/SignalList.h>
#include <TrendView/DialogChooseTrendSignals.h>
#include <TrendView/TrendSignalSet.h>


std::list<SimTrendsWidget*> SimTrends::s_trendsList;

std::vector<SimTrendsWidget*> SimTrends::getTrendsList()
{
	std::vector<SimTrendsWidget*> result{s_trendsList.begin(), s_trendsList.end()};
	return result;
}

bool SimTrends::activateTrendWindow(SimTrendsWidget* trendWidget)
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

bool SimTrends::startTrendApp(std::shared_ptr<SimIdeSimulator> simulator, const std::vector<AppSignalParam>& appSignals, QWidget* parent)
{
	SimTrendsWidget* window = new SimTrendsWidget(simulator, parent);

	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(appSignals.size());

	for (const AppSignalParam& appSignal : appSignals)
	{
		TrendLib::TrendSignalParam tsp(appSignal, {});
		trendSignals.push_back(tsp);
	}

	window->addSignals(trendSignals, true);

	window->show();

	return false;
}

void SimTrends::registerTrendWindow(SimTrendsWidget* window)
{
#ifdef QT_DEBUG
	auto it = std::find_if(s_trendsList.begin(), s_trendsList.end(), [&window](auto w)
						   {
							   return w == window;
						   });

	Q_ASSERT(it == s_trendsList.end());
#endif

	s_trendsList.push_back(window);
	return;
}

void SimTrends::unregisterTrendWindow(const SimTrendsWidget* window)
{
	[[maybe_unused]] auto removed = s_trendsList.remove_if([window](auto w) { return w == window; });
	Q_ASSERT(removed == 1);

	return;
}


SimTrendsWidget::SimTrendsWidget(std::shared_ptr<SimIdeSimulator> simulator, QWidget* parent) :
	TrendLib::TrendMainWindow(parent),
	m_simulator(simulator)
{
	assert(m_simulator);

static int no = 1;
	QString trendName = tr("Simulator Trends %1").arg(no++);
	SimTrends::registerTrendWindow(this);

	setWindowTitle(trendName);

	// Set ruler step to 5ms, as in simulator cycle always multiple to 5
	//
	setRulerStep(5);

	// Set default lane duration (5m), it differs from default value (1h)
	//
	m_timeCombo->setCurrentIndex(5);	// 5 is index in combo box
	setLaneDuration(5_min);

	// Hide Refresh button as it is not required for simulator, no archive here just "realtime" data
	//
	m_refreshButton->setEnabled(false);				// This is button Refresh
	m_refreshActionForButton->setVisible(false);	// To hide button from toolbar the QAction for this button must be hidden

	m_refreshAction->setVisible(false);

	// TimeType, assume we have only simulated PlantTime
	//
	setTimeType(E::TimeType::Plant);
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
	SimTrends::unregisterTrendWindow(this);
	return;
}

void SimTrendsWidget::trimTrendData(TimeStamp trimFrom)
{
	signalSet().slot_trimData(E::TimeType::Plant, trimFrom);
}

void SimTrendsWidget::addNonValidPoints()
{
	signalSet().addNonValidPoint();
}

void SimTrendsWidget::clear()
{
	signalSet().clear(E::TimeType::Plant);
}

void SimTrendsWidget::timerEvent(QTimerEvent*)
{
	m_timerCounter ++;
	quint64 durationSec = duration() / 1000;

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
	std::vector<TrendLib::TrendSignalParam> acceptedTrendSignals = signalSet().trendSignals();
	std::vector<TrendLib::ArchiveServer> archiveServers;	// Simulation does not have archive servers;

	// --
	//
	std::vector<TrendLib::TrendSignalParam> trendSignals;
	trendSignals.reserve(m_simulator->appSignalManager().signalsCount());

	for (const auto& appSignal : m_simulator->appSignalManager().signalList())
	{
		trendSignals.emplace_back(appSignal, TrendLib::ArchiveServer{});
	}

	// Implement TrendLib::ISignalHasTag
	//
	class SignalHasTag : public TrendLib::ISignalHasTag
	{
	public:
		SignalHasTag(const Sim::AppSignalManager* sm) : signalManager(sm)
		{
		}

		virtual bool signalHasTag(const QString& signalId, const QString& tag) const  override
		{
			Q_ASSERT(signalManager);
			return signalManager->signalHasTag(signalId, tag);
		}

		const Sim::AppSignalManager* signalManager = nullptr;
	} signalHasTag{&m_simulator->appSignalManager()};

	const AppSignalLists::AppSignalListSet appSignalLists;

	TrendLib::DialogChooseTrendSignals dialog(&signalHasTag, trendSignals, acceptedTrendSignals, archiveServers, appSignalLists, this);
	
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

void SimTrendsWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void SimTrendsWidget::dropEvent(QDropEvent* event)
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

	// Parse data
	//
	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			// Simulator trends work only with realtime trends, so no need to set some archive server
			//
			TrendLib::ArchiveServer trendArchiveServer{};

			TrendLib::TrendSignalParam tsp{appSignalParam, trendArchiveServer};
			addSignal(tsp, false);
		}
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
		signalSet().slot_realtimeDataReceived(QLatin1String{"SIM"}, data, minState, maxState);
		this->slot_realtimeDataReceived(QLatin1String{"SIM"}, data, minState, maxState);
	}

	return;
}

void SimTrendsWidget::slot_realtimeDataReceived(QString /*sourceEquipmentId*/,
												std::shared_ptr<TrendLib::RealtimeData> data,
												TrendLib::TrendStateItem minState,
												TrendLib::TrendStateItem maxState)
{
	Q_ASSERT(data);

	if (data->signalData.empty() == true)
	{
		return;
	}

	TimeStamp minTime = minState.getTime(timeType());
	TimeStamp maxTime = maxState.getTime(timeType());

	if (maxTime == 0 || maxTime == 0)
	{
		return;
	}

	// Shift view area if auto-shift mode is turned on
	//
	if (isRealtimeAutoShift() == true)
	{
		setRealtimeAutoShift(maxTime);
	}

	// Update widget if received data somewhere in view
	//

	// Force to update trend every 250 ms, as signal values (indicator on the left)
	// should be updated even if the trend point not in the current view.
	//
	if (m_realtimeUpdateTimer.isValid() == false)
	{
		m_realtimeUpdateTimer.start();
	}

	bool updateByTimer = m_realtimeUpdateTimer.elapsed() > 250;

	if (updateByTimer == true ||
		(minTime >= TimeStamp{startTime().timeStamp - duration() / 10} && maxTime <= TimeStamp{finishTime().timeStamp + duration() / 10}))
	{
		updateWidget();
		m_realtimeUpdateTimer.restart();
	}

	return;
}
