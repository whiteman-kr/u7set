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
	virtual void signalsButton() override;

public:
//	const ConfigConnection& archiveService1() const;
//	void setArchiveService1(const ConfigConnection& config);

//	const ConfigConnection& archiveService2() const;
//	void setArchiveService2(const ConfigConnection& config);

	// Data
	//
private:
	ConfigConnection m_archiveService1;
	ConfigConnection m_archiveService2;

	TrendTcpClient* m_tcpClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
};

#endif // MONITORTRENDS_H
