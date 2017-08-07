#ifndef MONITORTRENDS_H
#define MONITORTRENDS_H
#include "TrendMainWindow.h"
#include "MonitorConfigController.h"
#include "TrendTcpClient.h"

class MonitorTrendsWidget;

class MonitorTrends
{
public:
	static std::vector<QString> getTrendsList();
	static bool activateTrendWindow(QString trendName);
	static bool startTrendApp(MonitorConfigController* configController, QWidget* parent);

	static void registerTrendWindow(QString name, MonitorTrendsWidget* window);
	static void unregisterTrendWindow(QString name);

private:
	static std::map<QString, MonitorTrendsWidget*> m_trendsList;
};


class MonitorTrendsWidget : public TrendLib::TrendMainWindow
{
public:
	explicit MonitorTrendsWidget(MonitorConfigController* configController, QWidget* parent);
	virtual ~MonitorTrendsWidget();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void signalsButton() override;

public:

	// Slots
	//
protected slots:
	void slot_dataReceived(QString appSignalId, TimeStamp requestedHour, TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);

	// Data
	//
private:
	ConfigConnection m_archiveService1;
	ConfigConnection m_archiveService2;

	TrendTcpClient* m_tcpClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;

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
