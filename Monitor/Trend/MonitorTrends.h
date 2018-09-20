#ifndef MONITORTRENDS_H
#define MONITORTRENDS_H
#include "TrendMainWindow.h"
#include "MonitorConfigController.h"
#include "ArchiveTrendTcpClient.h"
#include "RtTrendTcpClient.h"

class MonitorTrendsWidget;
class QLabel;

class MonitorTrends
{
public:
	static std::vector<QString> getTrendsList();
	static bool activateTrendWindow(QString trendName);
	static bool startTrendApp(MonitorConfigController* configController, const std::vector<AppSignalParam>& appSignals, QWidget* parent);

	static void registerTrendWindow(QString name, MonitorTrendsWidget* window);
	static void unregisterTrendWindow(QString name);

private:
	static std::map<QString, MonitorTrendsWidget*> m_trendsList;
};


class MonitorTrendsWidget : public TrendLib::TrendMainWindow
{
public:
	MonitorTrendsWidget(MonitorConfigController* configController, QWidget* parent);
	virtual ~MonitorTrendsWidget();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void signalsButton() override;

private:
	void createArchiveConnection();
	void createRealtimeConnection();
	void setRealtimeParams();

public:

	// Slots
	//
protected slots:
	void slot_archiveDataReceived(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
	void slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);

	void slot_trendModeChanged();


	// Data
	//
private:
	MonitorConfigController* m_configController = nullptr;

	ConfigConnection m_archiveService1;
	ConfigConnection m_archiveService2;

	ArchiveTrendTcpClient* m_archiveTcpClient = nullptr;
	SimpleThread* m_archiveTcpClientThread = nullptr;

	RtTrendTcpClient* m_rtTcpClient = nullptr;
	SimpleThread* m_rtTcpClientThread = nullptr;

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
};

#endif // MONITORTRENDS_H
