#ifndef MONITORTRENDS_H
#define MONITORTRENDS_H
#include "TrendMainWindow.h"

class MonitorTrendsWidget;

class MonitorTrends
{
public:
	static std::vector<QString> getTrendsList();
	static bool activateTrendWindow(QString trendName);
	static bool startTrendApp(QString ads1ip, int ads1port, QString ads2ip, int ads2port, QWidget* parent);

	static void registerTrendWindow(QString name, MonitorTrendsWidget* window);
	static void unregisterTrendWindow(QString name);

private:
	static std::map<QString, MonitorTrendsWidget*> m_trendsList;
};


class MonitorTrendsWidget : public TrendLib::TrendMainWindow
{
public:
	explicit MonitorTrendsWidget(QWidget* parent);
	virtual ~MonitorTrendsWidget();

protected:
	virtual void signalsButton() override;

	// Data
	//
private:
};

#endif // MONITORTRENDS_H
