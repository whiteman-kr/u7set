#pragma once
#include <Simulator/SimAppSignalManager.h>
#include <TrendView/TrendMainWindow.h>


class SimIdeSimulator;
class SimTrendsWidget;
class QLabel;


class SimTrends
{
public:
	static std::vector<SimTrendsWidget*> getTrendsList();
	static bool activateTrendWindow(SimTrendsWidget* trendWidget);
	static bool startTrendApp(std::shared_ptr<SimIdeSimulator> simulator, const std::vector<AppSignalParam>& appSignals, QWidget* parent);

	static void registerTrendWindow(SimTrendsWidget* window);
	static void unregisterTrendWindow(const SimTrendsWidget* window);

	template <typename Func>
	static void applyForAll(Func func)
	{
		for (auto& trendWidget : s_trendsList)
		{
			func(trendWidget);
		}
	}

private:
	static std::list<SimTrendsWidget*> s_trendsList;
};


class SimTrendsWidget : public TrendLib::TrendMainWindow
{
public:
	SimTrendsWidget(std::shared_ptr<SimIdeSimulator> simulator, QWidget* parent);
	virtual ~SimTrendsWidget();

	void trimTrendData(TimeStamp trimFrom);	// Trim data from time trimFrom to the end (right).
	void addNonValidPoints();
	void clear();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void signalsButton() override;

	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

	// Slots
	//
protected slots:
	void fetchTrendData();
	void slot_realtimeDataReceived(QString sourceEquipmentId,
								   std::shared_ptr<TrendLib::RealtimeData> data,
								   TrendLib::TrendStateItem minState,
								   TrendLib::TrendStateItem maxState);

	// Data
	//
private:
	std::shared_ptr<SimIdeSimulator> m_simulator;

	enum  StatusBarColumns
	{
		SB_Text,
		SB_QueueSize,
		SB_NetworkRequests,
		SB_NetworkRellies,
	};

	QLabel* m_statusBarTextLabel = nullptr;
	QLabel* m_statusBarQueueSizeLabel = nullptr;
	QLabel* m_statusBarNetworkRequestsLabel = nullptr;
	QLabel* m_statusBarServerLabel = nullptr;
	QLabel* m_statusBarConnectionStateLabel = nullptr;

	quint64 m_timerCounter = 0;
};

