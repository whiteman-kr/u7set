#include "MonitorArchive.h"
#include "Settings.h"

std::map<QString, MonitorArchiveWidget*> MonitorArchive::m_archiveList;

std::vector<QString> MonitorArchive::getArchiveList()
{
	std::vector<QString> result;
	result.reserve(m_archiveList.size());

	for (std::pair<QString, MonitorArchiveWidget*> p : m_archiveList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool MonitorArchive::activateWindow(QString archiveName)
{
	if (m_archiveList.count(archiveName) != 1)
	{
		assert(m_archiveList.count(archiveName) != 1);
		return false;
	}

	MonitorArchiveWidget* widget = m_archiveList[archiveName];
	assert(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorArchive::startNewWidget(MonitorConfigController* configController, const std::vector<AppSignalParam>& appSignals, QWidget* parent)
{
	MonitorArchiveWidget* window = new MonitorArchiveWidget(configController, parent);

	window->setSignals(appSignals);

	window->show();

	return false;
}

void MonitorArchive::registerWindow(QString name, MonitorArchiveWidget* window)
{
	assert(m_archiveList.count(name) == 0);
	m_archiveList[name] = window;

	return;
}

void MonitorArchive::unregisterWindow(QString name)
{
	assert(m_archiveList.count(name) == 1);
	m_archiveList.erase(name);

	return;
}

MonitorArchiveWidget::MonitorArchiveWidget(MonitorConfigController* configController, QWidget* parent) :
	QMainWindow(parent, Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_archiveService1(configController->configuration().archiveService1),
	m_archiveService2(configController->configuration().archiveService2)
{
	setAttribute(Qt::WA_DeleteOnClose);

static int no = 1;
	QString name = QString("Monitor Archive %1").arg(no++);
	MonitorArchive::registerWindow(name, this);

	setWindowTitle(name);

	setMinimumSize(QSize(750, 400));

	// ToolBar
	//
	m_toolBar = new QToolBar(tr("ToolBar"));
	m_toolBar->setMovable(false);

	m_exportButton = new QPushButton(tr("Export..."));
	m_printButton = new QPushButton(tr("Print..."));
	m_signalsButton = new QPushButton(tr("Signals..."));

	m_toolBar->addWidget(m_exportButton);
	m_toolBar->addWidget(m_printButton);
	m_toolBar->addSeparator();

	// Add stretecher
	//
	QWidget* empty = new QWidget();
	empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_toolBar->addWidget(empty);

	m_toolBar->addWidget(m_signalsButton);

	addToolBar(m_toolBar);

	// Status bar
	//
	m_statusBar = new QStatusBar;

	m_statusBarTextLabel = new QLabel(m_statusBar);
	m_statusBarQueueSizeLabel = new QLabel(m_statusBar);
	m_statusBarNetworkRequestsLabel = new QLabel(m_statusBar);
	m_statusBarServerLabel = new QLabel(m_statusBar);
	m_statusBarConnectionStateLabel = new QLabel(m_statusBar);

	m_statusBar->addWidget(m_statusBarTextLabel, 1);
	m_statusBar->addWidget(m_statusBarQueueSizeLabel, 0);
	m_statusBar->addWidget(m_statusBarNetworkRequestsLabel, 0);
	m_statusBar->addWidget(m_statusBarServerLabel, 0);
	m_statusBar->addWidget(m_statusBarConnectionStateLabel, 0);

	setStatusBar(m_statusBar);

	// Communication thread
	//
	//m_tcpClient = new TrendTcpClient(configController);

	//m_tcpClientThread = new SimpleThread(m_tcpClient);
	//m_tcpClientThread->start();

	//connect(&signalSet(), &TrendLib::TrendSignalSet::requestData, m_tcpClient, &TrendTcpClient::slot_requestData);
	//connect(m_tcpClient, &TrendTcpClient::dataReady, &signalSet(), &TrendLib::TrendSignalSet::slot_dataReceived);
	//connect(m_tcpClient, &TrendTcpClient::requestError, &signalSet(), &TrendLib::TrendSignalSet::slot_requestError);

	//connect(m_tcpClient, &TrendTcpClient::dataReady, this, &MonitorTrendsWidget::slot_dataReceived);

	// --
	//
	restoreWindowState();

	startTimer(100);

	return;
}

MonitorArchiveWidget::~MonitorArchiveWidget()
{
	MonitorArchive::unregisterWindow(this->windowTitle());

//	assert(m_tcpClientThread);
//	if (m_tcpClientThread != nullptr)
//	{
//		m_tcpClientThread->quitAndWait(10000);
//	}

	return;
}

void MonitorArchiveWidget::ensureVisible()
{
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		 intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		 frameGeometry().height() > screenRect.height()))
	{
		resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
	}
}

bool MonitorArchiveWidget::setSignals(const std::vector<AppSignalParam>& appSignals)
{
	m_appSignals = appSignals;
	return true;
}

bool MonitorArchiveWidget::addSignal(const AppSignalParam& appSignal)
{
	m_appSignals.push_back(appSignal);
	return true;
}

void MonitorArchiveWidget::closeEvent(QCloseEvent*e)
{
	saveWindowState();
	e->accept();
	return;
}

void MonitorArchiveWidget::saveWindowState()
{
	theSettings.m_archiveWindowPos = pos();
	theSettings.m_archiveWindowGeometry = saveGeometry();
	theSettings.m_archiveWindowState = saveState();

	//theSettings.m_viewType = m_viewCombo->currentIndex();
	//theSettings.m_laneCount = m_lanesCombo->currentIndex() + 1;
	//theSettings.m_timeTypeIndex = m_timeTypeCombo->currentIndex();

	theSettings.writeUserScope();
}

void MonitorArchiveWidget::restoreWindowState()
{
	move(theSettings.m_archiveWindowPos);
	restoreGeometry(theSettings.m_archiveWindowGeometry);
	restoreState(theSettings.m_archiveWindowState);

//	assert(m_viewCombo);
//	m_viewCombo->setCurrentIndex(theSettings.m_viewType);
//	m_timeTypeCombo->setCurrentIndex(theSettings.m_timeTypeIndex);

	// --
	//
	ensureVisible();

	return;
}
