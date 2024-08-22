#include "Main.h"
#include "MainWindow.h"

#include <AppSignalLists/DialogSignalListEditor.h>
#include <AppSignalLists/SignalListChecker.h>
#include <ClientLib/TuningLog.h>
#include <TuningLib/TuningFilter.h>
#include <TuningLib/TuningFilterToLists.h>
#include <UiLib/DialogAlert.h>
#include <UiLib/DialogAbout.h>
#include "../UtilsLib/LogFile.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "TuningCounters.h"
#include "TuningSchemaManager.h"

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_logFile("TuningClient", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + softwareInfo.equipmentID()),
	m_tuningLog(m_userManager, "TuningClientSignals", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + softwareInfo.equipmentID()),
	m_configController(softwareInfo, TuningClientAppSettings::instance().configuratorAddress1(), TuningClientAppSettings::instance().configuratorAddress2(), &m_logFile),
	m_tuningSignalManager(softwareInfo.equipmentID(), &m_logFile),
	m_tuningConnection{m_tuningSignalManager, m_tuningSignalManager, m_tuningSignalManager, m_userManager, &m_logFile, &m_tuningLog},
	m_tuningCounters(m_tuningUi, m_tuningSignalManager, m_tuningConnection, m_appSignalListSet)
{
	// Init translator
	//
	m_translator.addLanguage("en", "English");
	m_translator.addLanguage("uk", "Ukrainian");
	m_translator.addLanguage("bg", "Bulgarian");

	for (const QString& l : m_translator.languagesList())
	{
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/TuningClient_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/AppSignalLists_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/ClientLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/SchemaClientLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UiLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UtilsLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/qt_%1.qm").arg(l));
	}

	{
		QStringList failedTranslations;
		if (m_translator.setLanguage(TuningClientAppSettings::instance().language(), failedTranslations) == false)
		{
			if (failedTranslations.isEmpty() == false)
			{
				m_logFile.writeError("Failed to load translation files:\n" + failedTranslations.join('\n'));
			}
			else
			{
				m_logFile.writeError("Failed to set language: " + TuningClientAppSettings::instance().language());
			}
		}
	}

	// -
	m_sorTooltipText = QObject::tr("SOR counter (click for details)");

	if (TuningClientAppSettings::instance().user().m_mainWindowPos.x() != -1 && TuningClientAppSettings::instance().user().m_mainWindowPos.y() != -1)
	{
		move(TuningClientAppSettings::instance().user().m_mainWindowPos);
		restoreGeometry(TuningClientAppSettings::instance().user().m_mainWindowGeometry);
		restoreState(TuningClientAppSettings::instance().user().m_mainWindowState);
	}
	else
	{
		resize(1024, 768);
	}

	//
	//

	createActions();
	createMenu();
	createStatusBar();

	setWindowTitle(QString("TuningClient - ") + TuningClientAppSettings::instance().instanceStrId());

	setCentralWidget(new QLabel(tr("Waiting for configuration...")));

	// Global connections

	connect(&m_configController, &TuningConfigController::signalsArrived, this, &MainWindow::slot_signalsUpdated, Qt::DirectConnection);
	connect(&m_configController, &TuningConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);
	connect(&m_configController, &TuningConfigController::error, this, &MainWindow::slot_configurationError);

	connect(&m_appSignalListSet, &AppSignalLists::AppSignalListSet::updatePerformed, this, &MainWindow::slot_signalListsChanged);
	connect(&m_appSignalListSet, &AppSignalLists::AppSignalListSet::updatePerformed, &m_tuningCounters, &TuningCountersManager::slot_signalListsChanged);

	// DialogAlert

	m_dialogAlert = new UiLib::DialogAlert(this);
	connect(&m_logFile, &Log::LogFile::alertArrived, m_dialogAlert, &UiLib::DialogAlert::onAlertArrived);
	connect(&m_logFile, &Log::LogFile::writeFailure, m_dialogAlert, &UiLib::DialogAlert::onAlertArrived);

	// Load user filters

	loadLocalSignalLists();

	//

	m_mainWindowTimerId_500ms = startTimer(500);

	m_configController.start();
}

MainWindow::~MainWindow()
{
	deleteWorkspace();

	TuningClientAppSettings::instance().user().m_mainWindowPos = pos();
	TuningClientAppSettings::instance().user().m_mainWindowGeometry = saveGeometry();
	TuningClientAppSettings::instance().user().m_mainWindowState = saveState();
}

ClientLib::TuningSignalManager& MainWindow::tuningSignalManager()
{
	return m_tuningSignalManager;
}

const ClientLib::TuningSignalManager& MainWindow::tuningSignalManager() const
{
	return m_tuningSignalManager;
}

ClientLib::TuningConnection& MainWindow::tuningConnection()
{
	return m_tuningConnection;
}

const ClientLib::TuningConnection& MainWindow::tuningConnection() const
{
	return m_tuningConnection;
}

ITuningAuthorization& MainWindow::tuningAuthorization()
{
	return m_userManager;
}

const ITuningAuthorization& MainWindow::tuningAuthorization() const
{
	return m_userManager;
}


void MainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	//m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MainWindow::exit);

	m_pAppSignalListsAction = new QAction(tr("Signal Lists..."), this);
	m_pAppSignalListsAction->setStatusTip(tr("Edit tuning signal Lists"));
	m_pAppSignalListsAction->setEnabled(true);
	connect(m_pAppSignalListsAction, &QAction::triggered, this, &MainWindow::showAppSignalListEditor);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

	m_pTuningSourcesAction = new QAction(tr("Tuning sources..."), this);
	m_pTuningSourcesAction->setStatusTip(tr("View tuning sources"));
	//m_pTuningSourcesAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pTuningSourcesAction->setEnabled(true);
	connect(m_pTuningSourcesAction, &QAction::triggered, this, &MainWindow::showTuningSources);

	m_pStatisticsAction = new QAction(tr("Connection Statistics..."), this);
	m_pStatisticsAction->setStatusTip(tr("View Connection Statistics"));
	m_pStatisticsAction->setEnabled(true);
	connect(m_pStatisticsAction, &QAction::triggered, this, &MainWindow::showStatistics);

	m_pAppLogAction = new QAction(tr("Application Log..."), this);
	m_pAppLogAction->setStatusTip(tr("Show application log"));
	connect(m_pAppLogAction, &QAction::triggered, this, &MainWindow::showAppLog);

	m_pSignalLogAction = new QAction(tr("Signals Log..."), this);
	m_pSignalLogAction->setStatusTip(tr("Show signals log"));
	connect(m_pSignalLogAction, &QAction::triggered, this, &MainWindow::showSignalsLog);

	m_aboutQtAction = new QAction(tr("About Qt..."), this);
	m_aboutQtAction->setStatusTip(tr("Show Qt information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::showAboutQt);

	m_pAboutAction = new QAction(tr("About TuningClient..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &MainWindow::showAbout);

	m_manualTuningAction = new QAction(tr("Tuning User Manual"), this);
	m_manualTuningAction->setStatusTip(tr("Show Tuning User Manual"));
	connect(m_manualTuningAction, &QAction::triggered, this, &MainWindow::showTuningUserManual);

}

void MainWindow::createMenu()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Tools
	//
	QMenu* pServiceMenu = menuBar()->addMenu(tr("&Service"));
	pServiceMenu->addAction(m_pAppSignalListsAction);
	pServiceMenu->addSeparator();
	pServiceMenu->addAction(m_pSettingsAction);


	// Help
	//
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_pTuningSourcesAction);
	pHelpMenu->addAction(m_pStatisticsAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_pAppLogAction);
	pHelpMenu->addAction(m_pSignalLogAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_manualTuningAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_aboutQtAction);
	pHelpMenu->addAction(m_pAboutAction);

	m_logonWidget = new LogonWidget(m_userManager, this);
	connect(this, &MainWindow::timerTick500, m_logonWidget, &LogonWidget::onTimer);
	menuBar()->setCornerWidget(m_logonWidget, Qt::TopRightCorner);
}

void MainWindow::createStatusBar()
{

	m_statusBarBuildInfo = new QLabel();
	m_statusBarBuildInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarBuildInfo->setIndent(3);

	m_statusBarLmControlMode = new QLabel();
	m_statusBarLmControlMode->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_statusBarLmErrors = new QLabel();
	m_statusBarLmErrors->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarLmErrors->setMinimumWidth(80);
	m_statusBarLmErrors->installEventFilter(this);
	m_statusBarLmErrors->setToolTip(tr("LM Errors (click for details)"));

	m_statusBarSor = new QLabel();
	m_statusBarSor->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarSor->setMinimumWidth(80);
	m_statusBarSor->installEventFilter(this);
	m_statusBarSor->setToolTip(m_sorTooltipText);

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarConfigConnection->setMinimumWidth(100);
	m_statusBarConfigConnection->installEventFilter(this);

	m_statusBarTuningConnection = new QLabel();
	m_statusBarTuningConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarTuningConnection->setMinimumWidth(100);
	m_statusBarTuningConnection->installEventFilter(this);

	m_statusBarLogAlerts = new QLabel();
	m_statusBarLogAlerts->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->installEventFilter(this);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));

	// --
	//
	statusBar()->addWidget(m_statusBarBuildInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarLmControlMode, 0);
	statusBar()->addPermanentWidget(m_statusBarLmErrors, 0);
	statusBar()->addPermanentWidget(m_statusBarSor, 0);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);
}

void MainWindow::loadLocalSignalLists() 
{
	// Load local lists from file
	//
	QString errorMessage;
	if (m_appSignalListSet.load(&errorMessage) == false)
	{
		m_logFile.writeError(errorMessage);
	}

	// Load legacy local filters
	//
	TuningFilters::TuningFilterStorage localFilterStorage;
	QString errorCode;
	if (localFilterStorage.load(TuningClientAppSettings::instance().userFiltersFile(), &errorCode) == false)
	{
		QString msg = tr("Failed to load legacy user filters: %1").arg(errorCode);
		m_logFile.writeError(msg);
		QMessageBox::critical(this, qAppName(), msg);
	}

	// Merge legacy local filters
	//
	bool ok = TuningFilters::TuningFilterToLists::convertGeneric(localFilterStorage, m_appSignalListSet);
	if (ok == false) 
	{
		QString msg = tr("Failed to merge legacy user filters: %1").arg(errorCode);
		m_logFile.writeError(msg);
		QMessageBox::critical(this, qAppName(), msg);
	}
	/*
	// Delete old filters file
	//
	if (localFilterStorage.root()->childFiltersCount() > 0)
	{
		QFile f(TuningClientAppSettings::instance().userFiltersFile());
		if (f.remove() == false)
		{
			QString msg = tr("Failed to remove legacy user filters file:\n%1").arg(TuningClientAppSettings::instance().userFiltersFile());
			m_logFile.writeError(msg);
			QMessageBox::critical(this, qAppName(), msg);
		}
		else
		{
			QString msg = tr("Legacy user filters were successfully merged with tuning signal lists,\nold file was deleted:\n%1")
							  .arg(TuningClientAppSettings::instance().userFiltersFile());
			m_logFile.writeError(msg);
			QMessageBox::information(this, qAppName(), msg);
		}
	}*/

	AppSignalLists::AppSignalListSetChecker::checkForDanglingItems(m_tuningSignalManager.signalHashes(), m_appSignalListSet, this, &m_logFile);
}

void MainWindow::loadIdeSignalLists()
{
	// Load Ide Lists
	//
	AppSignalLists::AppSignalListSet ideLists = m_configController.appSignalListSet();

	// Load Ui
	//
	m_tuningUi.reset();
	
	QString errorStr;
	if (m_tuningUi.load(m_configController.tuningUiData(), &errorStr) == false)
	{
		QString completeErrorMessage = QObject::tr("UI configuration file loading error: %1").arg(errorStr);
		m_logFile.writeError(completeErrorMessage);
		return;
	}

	// Place UI system tag for all lists
	//
	auto allUi = m_tuningUi.root()->childernToVector();
	for (auto& ui : allUi)
	{
		for (const QString& filterId : ui->filtersList())
		{
			const auto& uilist = ideLists.get(filterId);
			if (uilist == nullptr) 
			{
				Q_ASSERT(uilist);
				continue;
			}
			if (uilist->systemTagsList().contains(AppSignalLists::AppSignalList::tagUi) == false)
			{
				uilist->systemTagsList().insert(uilist->systemTagsList().begin(), AppSignalLists::AppSignalList::tagUi);
			}
		}
	}

	// Update AppSignalLists: remove all lists with Ide tag and add loaded IDE ones
	//
	m_appSignalListSet.remove(AppSignalLists::AppSignalList::tagIde);
	m_appSignalListSet.add(ideLists);
	m_appSignalListSet.fireUpdatePerformed();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	for (SchemasWorkspace* sw : m_schemasWorkspaces)
	{
		if (sw->isVisible() == true)
		{
			if (event->matches(QKeySequence::ZoomIn))
			{
				sw->zoomIn();
				return;
			}
			if (event->matches(QKeySequence::ZoomOut))
			{
				sw->zoomOut();
				return;
			}
			if (event->key() == Qt::Key_Asterisk && (event->modifiers() & Qt::ControlModifier))
			{
				sw->zoom100();
				return;
			}
			if (event->key() == Qt::Key_Slash && (event->modifiers() & Qt::ControlModifier))
			{
				sw->zoomToFit();
				return;
			}
		}
	}

	QWidget::keyPressEvent(event);

}


void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m_tuningWorkspace != nullptr && m_tuningWorkspace->hasPendingChanges() == true)
	{
		auto result = QMessageBox::warning(this,
		                                   qAppName(),
		                                   tr("Warning! Some values were modified but not written. Are you sure you want to exit?"),
		                                   QMessageBox::Yes | QMessageBox::No,
		                                   QMessageBox::No);

		if (result == QMessageBox::No)
		{
			event->ignore();
		}
	}
}

void MainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_mainWindowTimerId_500ms)
	{
		if (theSharedMemorySingleApp != nullptr)
		{
			bool ok = theSharedMemorySingleApp->lock();
			if (ok == true)
			{
				TuningClientSharedData* data = (TuningClientSharedData*)theSharedMemorySingleApp->data();

				if (data->showCommand == true)
				{
					data->showCommand = false;

					showMinimized(); // This is to bring up the window if not minimized but beneath some other window
					setWindowState(Qt::WindowActive);
					showNormal();
				}


				ok = theSharedMemorySingleApp->unlock();
				if (ok == false)
				{
					qDebug() << "Failed to unlock QSharedMemory object!";
					assert(false);
				}
			}
			else
			{
				qDebug() << "Failed to lock QSharedMemory object!";
				assert(false);
			}
		}

		// Update filter counters every 1 second
		//
		static int updateCountersCounter = 0;
		if (updateCountersCounter++ == 0)
		{
			m_tuningCounters.update(m_configController.configuration().lmStatusFlagMode());
		}
		updateCountersCounter %= 2;

		// Update user interface
		//
		updateStatusBar();

		emit timerTick500();
	}

	return;
}

void MainWindow::createWorkspace()
{
	// Check if previous workspace is deleted

	if (m_noWorkspaceLabel != nullptr ||
		m_tuningWorkspace != nullptr ||
		m_schemasWorkspaces.empty() == false ||
		m_tabWidget != nullptr)
	{
		Q_ASSERT(m_noWorkspaceLabel == nullptr);
		Q_ASSERT(m_tuningWorkspace == nullptr);
		Q_ASSERT(m_schemasWorkspaces.empty() == true);
		Q_ASSERT(m_tabWidget == nullptr);
		return;
	}

	// Create main layout

	if (m_mainLayout == nullptr)
	{
		QWidget* w = new QWidget(this);
		m_mainLayout = new QVBoxLayout(w);
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(w);
	}

	// Create new workspaces

	if (m_configController.showSchemas() == true && m_configController.schemaCount() != 0)
	{
		bool schemaFiltersFound = false;

		for (int i = 0; i < m_tuningUi.root()->childCount(); i++)
		{
			auto uiUtem = m_tuningUi.root()->child(i);
			if (uiUtem == nullptr)
			{
				Q_ASSERT(uiUtem);
				continue;
			}

			if (uiUtem->isSchemasTab() == true)
			{
				schemaFiltersFound = true;

				SchemasWorkspace* sw = new SchemasWorkspace(m_configController,
															uiUtem->caption(),
															uiUtem->tagsList(),
															uiUtem->startSchemaId(),
															&m_logFile,
															this);
				m_schemasWorkspaces.push_back(sw);
			}
		}

		if (schemaFiltersFound == false)
		{
			SchemasWorkspace* sw = new SchemasWorkspace(m_configController,
														tr("Schemas"),
														{},
														m_configController.startSchemaId(),
														&m_logFile,
														this);
			m_schemasWorkspaces.push_back(sw);
		}
	}

	if (m_configController.showSignals() == true)
	{
		m_tuningWorkspace = new TuningWorkspace(m_configController,
												m_tuningSignalManager,
												m_tuningUi,
												m_appSignalListSet,
												m_userManager,
												m_tuningConnection,
												*m_tuningUi.root(),
												m_tuningCounters,
												QUuid(),
												true /*hasFilterTree*/,
												this);
	}

	// Create login workspace

	if (m_userManager.enabled() == true && m_userManager.loginPerOperation() == false)
	{
		m_logonWidget->setVisible(true);
	}
	else
	{
		m_logonWidget->setVisible(false);
	}

	// Now choose, what workspace to display. If both exists, create a tab page.

	if (m_schemasWorkspaces.empty() == true && m_tuningWorkspace == nullptr)
	{
		m_noWorkspaceLabel = new QLabel(tr("No workspaces exist, configuration error."));
		m_mainLayout->addWidget(m_noWorkspaceLabel);
	}
	else
	{
		if (m_schemasWorkspaces.empty() == true && m_tuningWorkspace != nullptr)
		{
			// Show One Tuning Workspace
			//
			m_mainLayout->addWidget(m_tuningWorkspace, 2);
		}
		else
		{
			if (m_schemasWorkspaces.size() == 1 && m_tuningWorkspace == nullptr)
			{
				// Show One Schemas Workspace
				//
				m_mainLayout->addWidget(m_schemasWorkspaces[0], 2);
			}
			else
			{
				// Show both Workspaces
				//
				m_tabWidget = new QTabWidget();

				for (SchemasWorkspace* sw : m_schemasWorkspaces)
				{
					m_tabWidget->addTab(sw, sw->caption());
				}

				if (m_tuningWorkspace != nullptr)
				{
					m_tabWidget->addTab(m_tuningWorkspace, tr("Signals"));
				}

				m_mainLayout->addWidget(m_tabWidget, 2);
			}
		}
	}
}

void MainWindow::deleteWorkspace()
{
	// Delete old workspaces

	if (m_noWorkspaceLabel != nullptr)
	{
		delete m_noWorkspaceLabel;
		m_noWorkspaceLabel = nullptr;
	}

	if (m_tuningWorkspace != nullptr)
	{
		delete m_tuningWorkspace;
		m_tuningWorkspace = nullptr;
	}

	if (m_schemasWorkspaces.empty() == false)
	{
		for (SchemasWorkspace* sw : m_schemasWorkspaces)
		{
			delete sw;
		}
		m_schemasWorkspaces.clear();
	}

	if (m_tabWidget != nullptr)
	{
		delete m_tabWidget;
		m_tabWidget = nullptr;
	}
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_statusBarLmErrors &&
		m_statusBarLmErrors->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showTuningSources();
	}

	if (object == m_statusBarSor &&
		m_statusBarSor->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showTuningSources();
	}

	if ((object == m_statusBarConfigConnection || object == m_statusBarTuningConnection) &&
		event->type() == QEvent::MouseButtonPress)
	{
		showStatistics();
	}

	if (object == m_statusBarLogAlerts &&
		m_statusBarLogAlerts->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showAppLog();
	}

	return QWidget::eventFilter(object, event);
}

void MainWindow::updateStatusBar()
{
	// Status bar
	//
	assert(m_statusBarLmControlMode);
	assert(m_statusBarConfigConnection);

	// BuildInfo
	//
	auto configInfo = m_configController.configInfo();
	QString text = tr("Project %1, build %2").arg(configInfo.project).arg(configInfo.buildNo);

	if (m_statusBarBuildInfo->text() != text)
	{
		m_statusBarBuildInfo->setText(text);
	}

	// LM Control Tooltip

	{
		QString str = m_tuningConnection.clientControlInfo();

		if (m_statusBarLmControlMode->toolTip() != str)
		{
			m_statusBarLmControlMode->setToolTip(str);
		}
	}


	// ConfigService
	//
	Tcp::ConnectionState configConnState =  m_configController.getConnectionState();

	text = tr(" ConfigService: ");

	if (configConnState.isConnected == false)
	{
		text += tr(" no connection");
	}
	else
	{
		text += tr("%1").arg(QString::number(configConnState.replyCount));
	}

	if (text != m_statusBarConfigConnection->text())
	{
		m_statusBarConfigConnection->setText(text);
	}

	// --
	//
	QString tooltip = tr("Address: %1").arg(configConnState.peerAddr.toString());

	if (tooltip != m_statusBarConfigConnection->toolTip())
	{
		m_statusBarConfigConnection->setToolTip(tooltip);
	}

	// TuningService
	//
	// TuningService connection
	//
	{
		showSoftwareConnection("TuningService",
							   m_tuningConnection.tcpTuningConnStates(),
							   m_statusBarTuningConnection);
	}

	// Counters

	{
		int labelCount = 0;

		// Compute labels count
		//
		for (int i = 0; i < m_tuningUi.root()->childCount(); i++)
		{
			auto uiItem = m_tuningUi.root()->child(i).get();
			if (uiItem == nullptr)
			{
				Q_ASSERT(uiItem);
				return;
			}
			if (uiItem->isCounter() == true && uiItem->counterType() == TuningLib::TuningUiItem::CounterType::StatusBar)
			{
				labelCount++;
			}
		}

		// If counters count changed, recreate labels
		//
		if (static_cast<int>(m_statusDiscreteCount.size()) != labelCount)
		{
			for (QLabel* l : m_statusDiscreteCount)
			{
				delete l;
			}
			m_statusDiscreteCount.clear();

			for (int i = 0; i < labelCount; i++)
			{
				QLabel* l = new QLabel();
				l->setAlignment(Qt::AlignLeft);
				l->setMinimumWidth(80);
				l->setToolTip(tr("Counter %1").arg(i));

				statusBar()->insertPermanentWidget(2 + i, l, 0);

				m_statusDiscreteCount.push_back(l);
			}
		}

		// Update statusBar counter label state 
		//
		size_t labelIndex = 0;

		for (int i = 0; i < m_tuningUi.root()->childCount(); i++)
		{
			auto uiItem = m_tuningUi.root()->child(i).get();
			if (uiItem == nullptr)
			{
				Q_ASSERT(uiItem);
				return;
			}

			if (uiItem->isCounter() == false || uiItem->counterType() != TuningLib::TuningUiItem::CounterType::StatusBar)
			{
				continue;
			}

			if (labelIndex >= m_statusDiscreteCount.size())
			{
				Q_ASSERT(labelIndex < m_statusDiscreteCount.size());
				return;
			}

			QLabel* l = m_statusDiscreteCount[labelIndex++];
			if (l == nullptr)
			{
				Q_ASSERT(l);
				return;
			}

			TuningCounters counters = m_tuningCounters.counters(uiItem->filters());
			text = tr(" %1 %2 ").arg(uiItem->caption()).arg(counters.discreteCounter);

			if (l->text() != text)
			{
				l->setText(text);
			}

			if (counters.discreteCounter == 0)
			{
				if (l->styleSheet() != "")
				{
					l->setStyleSheet("");
				}
			}
			else
			{
				QString styleSheet = QString("QLabel {background-color : %1; color: %2}").arg(uiItem->backAlertedColor().name()).arg(uiItem->textAlertedColor().name());

				if (l->styleSheet() != styleSheet)
				{
					l->setStyleSheet(styleSheet);
				}
			}
		}
	}

	// LM Errors

	TuningCounters rootCounters = m_tuningCounters.totalCounters();

	{
		assert(m_statusBarLmErrors);

		// Lm Errors tool

		text = tr(" LM Errors: %1 ").arg(rootCounters.errorCounter);

		if (m_statusBarLmErrors->text() != text)
		{
			m_statusBarLmErrors->setText(text);
		}

		if (rootCounters.errorCounter == 0)
		{
			if (m_statusBarLmErrors->styleSheet() != "")
			{
				m_statusBarLmErrors->setStyleSheet(QString());
			}
		}
		else
		{
			QString styleSheet = QString("QLabel {color : white; background-color: %1}").arg(redColor.name());

			if (m_statusBarLmErrors->styleSheet() != styleSheet)
			{
				m_statusBarLmErrors->setStyleSheet(styleSheet);
			}
		}

	}

	// SOR counter

	if (m_configController.lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::SOR)
	{
		if (rootCounters.sorActive == false)
		{
			text = tr(" SOR: -");
		}
		else
		{
			if (rootCounters.sorValid == false)
			{
				text = tr(" SOR: ? ");
			}
			else
			{
				if (rootCounters.sorCounter == 0)
				{
					text = tr(" SOR: No ");
				}
				else
				{
					if (rootCounters.sorCounter == 1)
					{
						text = tr(" SOR: Yes ");
					}
					else
					{
						text = tr(" SOR: Yes [%1] ").arg(rootCounters.sorCounter);
					}
				}
			}
		}

		if (m_statusBarSor->text() != text)
		{
			assert(m_statusBarSor);

			m_statusBarSor->setText(text);

			QString stylesheet;

			if ((rootCounters.sorActive == true && rootCounters.sorValid == false) || rootCounters.sorCounter > 0)
			{
				stylesheet = QString("QLabel {color : white; background-color: %1}").arg(redColor.name());
			}

			if (m_statusBarSor->styleSheet() != stylesheet)
			{
				m_statusBarSor->setStyleSheet(stylesheet);
			}
		}
	}
	else
	{
		m_statusBarSor->setText(QString());
		m_statusBarSor->setStyleSheet(QString());
	}

	// Log alerts tool

	if (m_logErrorsCounter != m_logFile.errorAckCounter() || m_logWarningsCounter != m_logFile.warningAckCounter())
	{
		m_logErrorsCounter = m_logFile.errorAckCounter();
		m_logWarningsCounter = m_logFile.warningAckCounter();

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(tr(" Log E: %1 W: %2 ").arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
			m_statusBarLogAlerts->setStyleSheet(m_statusBarLmControlMode->styleSheet());
		}
		else
		{
			if (m_logErrorsCounter == 0)
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #F87217}");
			}
			else
			{
				m_statusBarLogAlerts->setStyleSheet(QString("QLabel {color : white; background-color: %1}").arg(redColor.name()));
			}
		}
	}
}

void MainWindow::showSoftwareConnection(const QString& caption,
											   const std::vector<Tcp::ConnectionState>& connectionStates,
											   QLabel* label)
{
	if (label == nullptr)
	{
		Q_ASSERT(label);
		return;
	}

	QString toolTipText = tr("%1:\n").arg(caption);

	if (connectionStates.empty() == true)
	{
		toolTipText += tr("Not configured");
	}

	int statusOk = 0;
	qint64 replyCount = 0;
	for (const Tcp::ConnectionState& state : connectionStates)
	{
		if (state.isConnected == true)
		{
			statusOk ++;
		}

		replyCount += state.replyCount;

		toolTipText += QString("%1 %2 (%3)\n")
							.arg(state.connectedSoftwareInfo.equipmentID())
							.arg(state.peerAddr.addressPortStr())
							.arg(state.isConnected ? tr("ok") : tr("down"));
	}
	toolTipText = toolTipText.trimmed();

	label->setText(caption);
	label->setToolTip(toolTipText);

	QString statusText;

	if (connectionStates.size() <= 1)
	{
		statusText = tr("%1: %2 (Replies: %3)")
					 .arg(caption)
					 .arg(statusOk ? tr("ok") : tr("down"))
					 .arg(replyCount);
	}
	else
	{
		statusText = tr("%1: %2/%3 (Replies: %4)")
					 .arg(caption)
					 .arg(statusOk)
					 .arg(connectionStates.size())
					 .arg(replyCount);
	}

	label->setText(statusText);

	return;
}


void MainWindow::slot_configurationArrived(TuningClientConfigSettings configuration)
{
	// Modify logon mode
	//
	if (m_userManager.isLoggedIn() == true)
	{
		m_userManager.logout();
	}

	// --
	//
	QWidget* wm = QApplication::activeModalWidget();
	QWidget* wp = QApplication::activePopupWidget();

	if (wm != nullptr || wp != nullptr)
	{
		// Some modal or popup window is active, so deleting workspace is not available.
		// In this case we need to restart the application.
		//
		QMessageBox::warning(this, tr("Warning"), tr("Program configuraton has been changed. Press OK to restart the program."));

		// Remove the shared memory single application object
		//
		delete theSharedMemorySingleApp;
		theSharedMemorySingleApp = nullptr;

		// Restart the program
		//
		QString program = qApp->arguments()[0];
		QStringList arguments = qApp->arguments().mid(1); // remove the 1st argument - the program name
		qApp->quit();
		QProcess::startDetached(program, arguments);

		return;
	}

	if (m_tuningWorkspace != nullptr || m_schemasWorkspaces.empty() == false)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Program configuration has been changed and will be updated."));
	}

	deleteWorkspace();

	m_userManager.setConfiguration(configuration.clientSettings.tuningLogin,
								   configuration.clientSettings.getUsersAccounts(),
								   configuration.clientSettings.loginPerOperation,
								   configuration.clientSettings.tuningSessionTimeout,
								   configuration.matsUsers.users());

	loadIdeSignalLists();

	m_tuningConnection.updateConnections(m_configController.softwareInfo(),
										 configuration.clientSettings.tuningServices,
										 configuration.clientSettings.applyMode == TuningClientSettings::ApplyMode::Auto,
										 configuration.clientSettings.statusFlagFunction);

	createWorkspace();

	m_statusBarSor->setToolTip(configuration.lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::SOR ? m_sorTooltipText : QString());

	// LM Control Mode Label

	{
		static QString m_singleLmControlModeText = QObject::tr("Single LM Control Mode");
		static QString m_multipleLmControlModeText = QObject::tr("Multiple LM Control Mode");
		static QString m_mixedLmControlModeText = QObject::tr("Mixed LM Control Mode");

		const std::vector<SoftwareEndpoint::TuningService>& tuningServices = configuration.clientSettings.tuningServices;

		int singleLmControlModeCount = 0;

		for (const SoftwareEndpoint::TuningService& tsc : tuningServices)
		{
			if (tsc.singleLmControl == true)
			{
				singleLmControlModeCount++;
			}
		}

		if (tuningServices.size() == singleLmControlModeCount)
		{
			m_statusBarLmControlMode->setText(m_singleLmControlModeText);
		}
		else
		{
			if (singleLmControlModeCount > 0)
			{
				m_statusBarLmControlMode->setText(m_mixedLmControlModeText);
			}
			else
			{
				m_statusBarLmControlMode->setText(m_multipleLmControlModeText);
			}
		}
	}

	return;
}

void MainWindow::slot_signalsUpdated(QByteArray data)
{
	if (m_tuningSignalManager.load(data) == false)
	{
		QString completeErrorMessage = QObject::tr("Tuning signals file loading error.");
		m_logFile.writeError(completeErrorMessage);
	}

	AppSignalLists::AppSignalListSetChecker::checkForDanglingItems(m_tuningSignalManager.signalHashes(), m_appSignalListSet, this, &m_logFile);
}

void MainWindow::slot_configurationError(QString error)
{
	QMessageBox::critical(this,
						  qAppName(),
						  tr("Configuration error: %1")
						  .arg(error));
}

void MainWindow::exit()
{
	close();
}

void MainWindow::showSettings()
{
	DialogSettings d(m_translator, this);
	d.setSettings(TuningClientAppSettings::instance().system());

	if (d.exec() == QDialog::Accepted)
	{

		TuningClientAppSettings::instance().setSystem(d.settings());
		TuningClientAppSettings::instance().save();
	}
}


void MainWindow::showTuningSources()
{
	if (m_dialogTuningSources == nullptr)
	{
		m_dialogTuningSources = new DialogTuningSources(m_tuningConnection, m_userManager, true, this);
		m_dialogTuningSources->show();

		auto f = [this]() -> void
		{
			m_dialogTuningSources = nullptr;
		};

		connect(m_dialogTuningSources, &DialogTuningSources::dialogClosed, this, f);

	}
	else
	{
		m_dialogTuningSources->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogTuningSources);
}

void MainWindow::showStatistics()
{
	if (m_dialogStatistics == nullptr)
	{
		m_dialogStatistics = new SchemaClientLib::DialogTcpStatistics(this);
		m_dialogStatistics->show();

		auto f = [this]() -> void
		{
			m_dialogStatistics = nullptr;
		};

		connect(m_dialogStatistics, &SchemaClientLib::DialogTcpStatistics::dialogClosed, this, f);
	}
	else
	{
		m_dialogStatistics->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogStatistics);
}

void MainWindow::showAppLog()
{
	m_logFile.view(this);
}

void MainWindow::showSignalsLog()
{
	m_tuningLog.viewTuningLog(this);
}

void MainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void MainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to modify tuning values.");
	UiLib::DialogAbout::show(this, text, ":/Logo/RadiyLogo.png");
}

void MainWindow::showTuningUserManual()
{
	UiTools::openPdf(QApplication::applicationDirPath()+"/docs/D11.9_FSC_Tuning_User_Manual.pdf", this);
}

void MainWindow::showAppSignalListEditor()
{
	if (m_userManager.login(this) == false)
	{
		return;
	}

	AppSignalLists::DialogSignalListEditor::showDialog(m_appSignalListSet, m_tuningSignalManager, &m_tuningSignalManager, this);
}

void MainWindow::slot_signalListsChanged() 
{
	if (m_tuningWorkspace != nullptr)
	{
		m_tuningWorkspace->updateFilters();
	}
}
