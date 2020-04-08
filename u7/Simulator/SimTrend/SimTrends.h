#pragma once
#include "TrendMainWindow.h"
#include "SimAppSignalManager.h"
//#include "SimConfigController.h"
//#include "ArchiveTrendTcpClient.h"
//#include "RtTrendTcpClient.h"

class SimTrendsWidget;
class QLabel;

class SimTrends
{
public:
	static std::vector<QString> getTrendsList();
	static bool activateTrendWindow(QString trendName);
	static bool startTrendApp(Sim::AppSignalManager* signalManager, const std::vector<AppSignalParam>& appSignals, QWidget* parent);

	static void registerTrendWindow(QString name, SimTrendsWidget* window);
	static void unregisterTrendWindow(QString name);

private:
	static std::map<QString, SimTrendsWidget*> m_trendsList;
};


class SimTrendsWidget : public TrendLib::TrendMainWindow
{
public:
	SimTrendsWidget(Sim::AppSignalManager* signalManager, QWidget* parent);
	virtual ~SimTrendsWidget();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void signalsButton() override;

private:
	//void createArchiveConnection();
	//void createRealtimeConnection();
	//void setRealtimeParams();

public:

	// Slots
	//
protected slots:
	//void slot_archiveDataReceived(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
	//void slot_newRaltimeData(QString equipmentId, TimeStamp plantTime, TimeStamp systemTime, TimeStamp localTime, quint64 ms);
	void fetchTrendData();
	void slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);

	//void slot_trendModeChanged();


	// Data
	//
private:
	Sim::AppSignalManager* m_signalManager = nullptr;
	//SimConfigController* m_configController = nullptr;

	//ConfigConnection m_archiveService1;
	//ConfigConnection m_archiveService2;

	//ArchiveTrendTcpClient* m_archiveTcpClient = nullptr;
	//SimpleThread* m_archiveTcpClientThread = nullptr¸¸¸;

	//RtTrendTcpClient* m_rtTcpClient = nullptr;
	//SimpleThread* m_rtTcpClientThread = nullptr;

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

