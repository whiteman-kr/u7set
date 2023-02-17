#ifndef MONITORTRENDS_H
#define MONITORTRENDS_H

#include "TrendMainWindow.h"
#include "MonitorConfigController.h"
#include "MonitorSignalManager.h"
#include "MonitorTrendArchiveConnections.h"
#include "MonitorTrendRealtimeConnections.h"
#include "RtTrendTcpClient.h"

class MonitorTrendsWidget;
class QLabel;


class MonitorTrends
{
public:
	static std::vector<QString> getTrendsList();
	static bool activateTrendWindow(QString trendName);
	static bool startTrendApp(const MonitorSignalManager& signalManager,
							  const MonitorConfigController& configController,
							  const std::vector<AppSignalParam>& appSignals,
							  QWidget* parent);

	static void registerTrendWindow(QString name, MonitorTrendsWidget* window);
	static void unregisterTrendWindow(QString name);

private:
	static std::map<QString, MonitorTrendsWidget*> m_trendsList;
};


class MonitorTrendsWidget : public TrendLib::TrendMainWindow
{
public:
	MonitorTrendsWidget(const MonitorSignalManager& signalManager,
						const MonitorConfigController& configController,
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
								   TrendLib::TrendStateItem minState,
								   TrendLib::TrendStateItem maxState);
	void slot_trendModeChanged();

	void slot_configurationArrived(ConfigSettings configuration);

	// Data
	//
private:
	const MonitorSignalManager& m_signalManager;
	const MonitorConfigController& m_configController;

	MonitorTrendArchiveConnections m_archiveDataProvider;
	MonitorTrendRealtimeConnections m_realtimeDataProvider;

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
};

#endif // MONITORTRENDS_H
