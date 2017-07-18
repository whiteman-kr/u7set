#include "MonitorTrends.h"

std::map<QString, MonitorTrendsWidget*> MonitorTrends::m_trendsList;

std::vector<QString> MonitorTrends::getTrendsList()
{
	std::vector<QString> result;
	result.reserve(m_trendsList.size());

	for (std::pair<QString, MonitorTrendsWidget*> p : m_trendsList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool MonitorTrends::activateTrendWindow(QString trendName)
{
	if (m_trendsList.count(trendName) != 1)
	{
		assert(m_trendsList.count(trendName) != 1);
		return false;
	}

	MonitorTrendsWidget* widget = m_trendsList[trendName];
	assert(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorTrends::startTrendApp(QString ads1ip, int ads1port, QString ads2ip, int ads2port, QWidget* parent)
{
	MonitorTrendsWidget* window = new MonitorTrendsWidget(parent);
	window->show();

	return false;
}

void MonitorTrends::registerTrendWindow(QString name, MonitorTrendsWidget* window)
{
	assert(m_trendsList.count(name) == 0);
	m_trendsList[name] = window;
}

void MonitorTrends::unregisterTrendWindow(QString name)
{
	assert(m_trendsList.count(name) == 1);
	m_trendsList.erase(name);
}


MonitorTrendsWidget::MonitorTrendsWidget(QWidget* parent) :
	TrendLib::TrendMainWindow(parent)
{
static int no = 1;
	QString trendName = QString("Monitor Trends %1").arg(no++);
	MonitorTrends::registerTrendWindow(trendName, this);

	setWindowTitle(trendName);

	return;
}

MonitorTrendsWidget::~MonitorTrendsWidget()
{
	MonitorTrends::unregisterTrendWindow(this->windowTitle());
}
