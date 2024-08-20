#ifndef MONITORTRENDS_H
#define MONITORTRENDS_H

#include "MonitorConfigController.h"
#include "MonitorTrendArchiveConnections.h"

#include <ClientLib/AppSignalManager.h>
#include <ClientLib/RtDataProvider.h>
#include <TrendView/TrendArchiveServer.h>
#include <TrendView/TrendMainWindow.h>

class MonitorTrendsWidget;
class QLabel;


class MonitorTrends
{
public:
	static std::vector<MonitorTrendsWidget*> getTrendsList();
	static bool activateTrendWindow(MonitorTrendsWidget* trendWidget);
	static bool startTrendApp(const ClientLib::AppSignalManager& signalManager,
							  const MonitorConfigController& configController,
							  const std::vector<AppSignalParam>& appSignals,
							  const AppSignalLists::AppSignalListSet& appSignalListSet,
							  QWidget* parent);

	static void registerTrendWindow(MonitorTrendsWidget* window);
	static void unregisterTrendWindow(const MonitorTrendsWidget* window);

private:
	static std::list<MonitorTrendsWidget*> s_trendsList;
};


class MonitorTrendsWidget : public TrendLib::TrendMainWindow
{
	Q_OBJECT

public:
	MonitorTrendsWidget(const ClientLib::AppSignalManager& signalManager,
						const MonitorConfigController& configController,
						const AppSignalLists::AppSignalListSet& appSignalListSet,
						QWidget* parent);
	virtual ~MonitorTrendsWidget();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void signalsButton() override;

	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

private:
	void createArchiveConnection();
	void createRealtimeConnection();
	void setRealtimeParams();

public:

	// Slots
	//
protected slots:
	void slot_requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId, TimeStamp hourToRequest, E::TimeType timeType);
	void slot_archiveDataReceived(TrendLib::TrendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);

	void slot_realtimeDataReceived(QString sourceEquipmentId,
								   std::shared_ptr<TrendLib::RealtimeData> data,
								   E::RtTrendsSamplePeriod samplePeriod,
								   TrendLib::TrendStateItem minState,
								   TrendLib::TrendStateItem maxState);
	void slot_trendModeChanged();

	void slot_configurationArrived(MonitorConfigSettings configuration);

	// Data
	//
private:
	const ClientLib::AppSignalManager& m_signalManager;
	const MonitorConfigController& m_configController;

	MonitorTrendArchiveConnections m_archiveDataProvider;
	ClientLib::RtDataProvider m_realtimeDataProvider;

	const AppSignalLists::AppSignalListSet& m_appSignalListSet;

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
	QLabel* m_statusBarConnectionStateLabel = nullptr;

	QElapsedTimer m_realtimeUpdateTimer;
};

#endif // MONITORTRENDS_H
