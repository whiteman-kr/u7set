#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "Settings.h"
#include "ObjectFilter.h"

MainWindow::MainWindow(QWidget *parent) :
	m_configController(theSettings.configuratorAddress1(), theSettings.configuratorAddress2()),
	QMainWindow(parent)
{
	if (theSettings.m_mainWindowPos.x() != -1 && theSettings.m_mainWindowPos.y() != -1)
	{
		move(theSettings.m_mainWindowPos);
		restoreGeometry(theSettings.m_mainWindowGeometry);
		restoreState(theSettings.m_mainWindowState);
	}

	setCentralWidget(new QLabel("Waiting for configuration..."));

	createStatusBar();

	m_updateStatusBarTimerId = startTimer(100);

	connect(&m_configController, &ConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);
	m_configController.start();
}

MainWindow::~MainWindow()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();


	theFilters.save("ObjectFilters1.xml");
	theUserFilters.save("ObjectFiltersUser1.xml");
}

void MainWindow::createStatusBar()
{
	m_statusBarInfo = new QLabel();
	m_statusBarInfo->setAlignment(Qt::AlignLeft);
	m_statusBarInfo->setIndent(3);

	m_statusBarConnectionStatistics = new QLabel();
	m_statusBarConnectionStatistics->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionStatistics->setMinimumWidth(100);

	m_statusBarConnectionState = new QLabel();
	m_statusBarConnectionState->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionState->setMinimumWidth(100);

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConnectionStatistics, 0);
	statusBar()->addPermanentWidget(m_statusBarConnectionState, 0);
}

void MainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);

	// Update status bar
	//
	if  (event->timerId() == m_updateStatusBarTimerId)
	{
		assert(m_statusBarConnectionState);
		assert(m_statusBarConnectionStatistics);

		Tcp::ConnectionState confiConnState =  m_configController.getConnectionState();
		//Tcp::ConnectionState signalClientState =  m_tcpSignalClient->getConnectionState();

		// State
		//
		QString text = QString(" ConfigSrv: %1   TuningSrv: %2 ")
					   .arg(confiConnState.isConnected ? confiConnState.host.addressStr() : "NoConnection")
						.arg(confiConnState.isConnected ? confiConnState.host.addressStr() : "NoConnection");

		m_statusBarConnectionState->setText(text);

		// Statistics
		//
		text = QString(" ConfigSrv: %1   TuningSrv: %2 ")
			   .arg(QString::number(confiConnState.replyCount))
			   .arg(QString::number(confiConnState.replyCount));

		m_statusBarConnectionStatistics->setText(text);

		return;
	}

	return;
}

void MainWindow::slot_configurationArrived(bool updateFilters, bool updateSchemas, bool updateSignals)
{
	if (updateFilters == false && updateSignals == false && updateSchemas == false)
	{
		return;
	}

	if (m_tuningWorkspace != nullptr)
	{
		QMessageBox::warning(this, "Warning", "Program configuration was changed and will be reloaded.");

		delete m_tuningWorkspace;
		m_tuningWorkspace = nullptr;
	}

	if (updateFilters == true)
	{
		if (m_configController.getObjectFilters() == false)
		{

		}
	}

	if (updateSchemas == true)
	{
		if (m_configController.getSchemasDetails() == false)
		{

		}
	}

	if (updateSignals == true)
	{
		if (m_configController.getTuningSignals() == false)
		{

		}
	}

	theFilters.createAutomaticFilters();

	m_tuningWorkspace = new TuningWorkspace(this);
	setCentralWidget(m_tuningWorkspace);

	return;
}
