#pragma once
#include "TrendMainWindow.h"
#include "SimAppSignalManager.h"


class SimIdeSimulator;
class SimTrendsWidget;
class QLabel;


class SimTrends
{
public:
	static std::vector<QString> getTrendsList();
	static bool activateTrendWindow(QString trendName);
	static bool startTrendApp(std::shared_ptr<SimIdeSimulator> simulator, const std::vector<AppSignalParam>& appSignals, QWidget* parent);

	static void registerTrendWindow(QString name, SimTrendsWidget* window);
	static void unregisterTrendWindow(QString name);

	template <typename Func>
	static void applyForAll(Func func)
	{
		for (auto& [key, trendWidget] : m_trendsList)
		{
			Q_UNUSED(key);
			func(trendWidget);
		}
	}

private:
	static std::map<QString, SimTrendsWidget*> m_trendsList;
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

