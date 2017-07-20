#include "MonitorTrends.h"
#include "DialogChooseTrendSignals.h"

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

void MonitorTrendsWidget::signalsButton()
{
	DialogChooseTrendSignals dialog(this);
	
	int result = dialog.exec();
	
	if (result == QDialog::Rejected)
	{
		return;
	}

	std::vector<AppSignalParam> acceptedSignals = dialog.acceptedSignals();

	// Remove signals
	//
	std::vector<TrendLib::TrendSignalParam> discreteSignals = signalSet().discreteSignals();
	std::vector<TrendLib::TrendSignalParam> analogSignals = signalSet().analogSignals();

	for (const TrendLib::TrendSignalParam& ds : discreteSignals)
	{
		auto it = std::find_if(acceptedSignals.begin(), acceptedSignals.end(),
						[&ds](const AppSignalParam& trendSignal)
						{
							return trendSignal.appSignalId() == ds.appSignalId();
						});

		if (it == acceptedSignals.end())
		{
			signalSet().removeSignal(ds.appSignalId());
		}
	}

	for (const TrendLib::TrendSignalParam& as : analogSignals)
	{
		auto it = std::find_if(acceptedSignals.begin(), acceptedSignals.end(),
						[&as](const AppSignalParam& trendSignal)
						{
							return trendSignal.appSignalId() == as.appSignalId();
						});

		if (it == acceptedSignals.end())
		{
			signalSet().removeSignal(as.appSignalId());
		}
	}

	// Add new signals
	//
	for (const AppSignalParam signal : acceptedSignals)
	{
		auto dit = std::find_if(discreteSignals.begin(), discreteSignals.end(),
						[&signal](const TrendLib::TrendSignalParam& trendSignal)
						{
							return trendSignal.appSignalId() == signal.appSignalId();
						});

		auto ait = std::find_if(analogSignals.begin(), analogSignals.end(),
						[&signal](const TrendLib::TrendSignalParam& trendSignal)
						{
							return trendSignal.appSignalId() == signal.appSignalId();
						});

		if (dit != discreteSignals.end() ||
			ait != analogSignals.end())
		{
			continue;
		}

static const QRgb StdColors[] = { qRgb(0x80, 0x00, 0x00), qRgb(0x00, 0x80, 0x00), qRgb(0x00, 0x00, 0x80), qRgb(0x00, 0x80, 0x80),
								  qRgb(0x80, 0x00, 0x80), qRgb(0xFF, 0x00, 0x00), qRgb(0x00, 0x00, 0xFF), qRgb(0x00, 0x00, 0x00) };

static int stdColorIndex = 0;


		TrendLib::TrendSignalParam trendSignal(signal);
		trendSignal.setColor(StdColors[stdColorIndex]);

		signalSet().addSignal(trendSignal);

		// --
		//
		stdColorIndex ++;
		if (stdColorIndex >= sizeof(StdColors) / sizeof(StdColors[0]))
		{
			stdColorIndex = 0;
		}
	}

	updateWidget();

	return;
}
