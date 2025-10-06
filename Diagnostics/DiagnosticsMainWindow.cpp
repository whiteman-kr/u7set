#include "DiagnosticsMainWindow.h"
#include "DiagnosticsAppSettings.h"
#include "DiagnosticsCentralWidget.h"
#include "DialogSettings.h"
// #include "MonitorSchemaWidget.h"
// #include "MonitorSchemaView.h"
// #include "MonitorSignalSnapshot.h"
// #include "./Archive/MonitorArchive.h"
// #include "DialogDataSources.h"
// #include "Globals.h"
// #include "./Trend/MonitorTrends.h"
// #include "../VFrame30/Schema.h"
// #include "../lib/Ui/DialogSignalSearch.h"

#include "../UtilsLib/Ui/UiTools.h"
#include <UiLib/DialogAbout.h>
// #include "../lib/Ui/SchemaListWidget.h"

DiagnosticsMainWindow::DiagnosticsMainWindow(InstanceResolver& instanceResolver, const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow{parent},
	m_LogFile{qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + softwareInfo.equipmentID()},
	m_instanceResolver{instanceResolver},
	m_configController{softwareInfo,
					   DiagnosticsAppSettings::instance().configuratorAddress1(),
					   DiagnosticsAppSettings::instance().configuratorAddress2(),
					   &m_LogFile},
	m_appSignalManager{&m_LogFile},
	m_schemaManager{m_configController, m_signalDataServerStub},
	m_dialogAlert(this)
{
	// Init translator
	//
	m_translator.addLanguage("en", "English");
	m_translator.addLanguage("uk", "Ukrainian");
	m_translator.addLanguage("bg", "Bulgarian");

	for (const QString& l : m_translator.languagesList())
	{
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/Diagnostics_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/ClientLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/SchemaClientLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/TrendView_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UiLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UtilsLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/qt_%1.qm").arg(l));
		// m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/AppSignalLists_%1.qm").arg(l));
	}

	if (DiagnosticsAppSettings::instance().language() != "en")
	{
		QStringList failedTranslations;
		if (m_translator.setLanguage(DiagnosticsAppSettings::instance().language(), failedTranslations) == false)
		{
			if (failedTranslations.isEmpty() == false)
			{
				m_LogFile.writeError("Failed to load translation files:\n" + failedTranslations.join('\n'));
			}
			else
			{
				m_LogFile.writeError("Failed to set language: " + DiagnosticsAppSettings::instance().language());
			}
		}
	}

	// -
	//
	setWindowTitle(DiagnosticsAppSettings::instance().windowCaption());

	// Set application name so all message boxes will have correct caption.
	//
	qApp->setApplicationName(DiagnosticsAppSettings::instance().windowCaption());

	connect(&m_configController, &DiagConfigController::configurationArrived, this, &DiagnosticsMainWindow::slot_configurationArrived);
	connect(&m_configController, &DiagConfigController::error, this, &DiagnosticsMainWindow::slot_configurationError);

	// DialogAlert
	//
	connect(&m_LogFile, &Log::LogFile::alertArrived, &m_dialogAlert, &UiLib::DialogAlert::onAlertArrived);
	connect(&m_LogFile, &Log::LogFile::writeFailure, &m_dialogAlert, &UiLib::DialogAlert::onAlertArrived);

	// Creating signals controllers for VFrame30
	//
	m_appSignalController = std::make_unique<VFrame30::AppSignalController>(m_appSignalManager);
	m_logController = std::make_unique<VFrame30::LogController>(&m_LogFile);

	// --
	//
	auto createSchemaWidgetFunc = [this](std::shared_ptr<VFrame30::Schema> schema, QWidget* parentWidget)
	{
		return new DiagSchemaWidget(schema,
									&m_schemaManager,
									m_appSignalController.get(),
									m_logController.get(),
									&m_schemaStats,
									parentWidget);
	};

	auto diagnosticsCentralWidget = new DiagnosticsCentralWidget(&m_schemaManager, std::move(createSchemaWidgetFunc), this);

	setCentralWidget(diagnosticsCentralWidget);

	// Create Menus, ToolBars, StatusBar
	//
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	// --
	//
	setMinimumSize(500, 300);
	restoreWindowState();

	// --
	//
	connect(diagnosticsCentralWidget,
			&DiagnosticsCentralWidget::signal_actionCloseTabUpdated,
			this,
			[this](bool allowed)
			{
				Q_ASSERT(m_closeTabAction);
				m_closeTabAction->setEnabled(DiagnosticsAppSettings::instance().showSchemasTabBar() && allowed);
			});

	connect(diagnosticsCentralWidget, &DiagnosticsCentralWidget::signal_historyChanged, this, &DiagnosticsMainWindow::slot_historyChanged);
	connect(diagnosticsCentralWidget, &DiagnosticsCentralWidget::signal_tabPageChanged, this, &DiagnosticsMainWindow::slot_updateActions);

	// connect(m_selectSchemaWidget, &SelectSchemaWidget::selectionChanged, monitorCentralWidget,
	// &MonitorCentralWidget::slot_selectSchemaForCurrentTab);

	// --
	//
	diagnosticsCentralWidget->setVisibleTabBar(DiagnosticsAppSettings::instance().showSchemasTabBar());
	diagnosticsCentralWidget->setZoomMode(DiagnosticsAppSettings::instance().zoomMode());

	centralWidget()->show();

	// --
	//
	m_configController.start();

	m_updateStatusBarTimerId = startTimer(200);

	//// Create SchemaList dock widget
	////
	// m_schemaListDock = new QDockWidget{tr("Schemas List"), this};
	// m_schemaListDock->setObjectName("SchemaList");
	// m_schemaListDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar);
	// m_schemaListDock->setTitleBarWidget(new QWidget{});		// Hides title bar

	// SchemaListWidget* schemaListWidget = new SchemaListWidget(
	//										 std::vector{SchemaListTreeColumns::SchemaID, SchemaListTreeColumns::Caption},
	//										 false,
	//										 m_schemaListDock);
	// m_schemaListDock->setWidget(schemaListWidget);

	// addDockWidget(Qt::LeftDockWidgetArea, m_schemaListDock);

	// m_schemaListDock->setVisible(QSettings().value("m_schemaListAction.checked").toBool());

	//// --
	////
	connect(&instanceResolver, &InstanceResolver::activate, this, &DiagnosticsMainWindow::activateRequested);

	//// SchemaListWidget
	////
	// connect(schemaListWidget, &SchemaListWidget::openSchemaRequest, monitorCentralWidget,
	// &MonitorCentralWidget::slot_selectSchemaForCurrentTab);

	// connect(&m_configController, &MonitorConfigController::configurationUpdated,
	//		[this, schemaListWidget]()
	//		{
	//			schemaListWidget->setDetails(m_configController.schemasDetailsSet());
	//		});

	return;
}

DiagnosticsMainWindow::~DiagnosticsMainWindow() = default;

void DiagnosticsMainWindow::closeEvent(QCloseEvent* e)
{
	saveWindowState();
	e->accept();

	return;
}

void DiagnosticsMainWindow::timerEvent(QTimerEvent* event)
{
	Q_ASSERT(event);

	if (event->timerId() == m_updateStatusBarTimerId)
	{
		updateStatusBar();
	}

	return;
}

void DiagnosticsMainWindow::showEvent(QShowEvent*)
{
	showLogo();
	showZoomControls();
	return;
}

bool DiagnosticsMainWindow::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_statusBarLogAlerts && event->type() == QEvent::MouseButtonPress && m_statusBarLogAlerts->text().isEmpty() == false)
	{
		showLog();
	}

	return QWidget::eventFilter(object, event);
}

// void MonitorMainWindow::showTrends(const std::vector<AppSignalParam>& appSignals)
//{
//	MonitorTrends::startTrendApp(m_appSignalManager, m_configController, appSignals, this);
// }

void DiagnosticsMainWindow::saveWindowState()
{
	QSettings s{};

	s.setValue("MainWindow/pos", pos());
	s.setValue("MainWindow/geometry", saveGeometry());
	s.setValue("MainWindow/state", saveState());

	return;
}

void DiagnosticsMainWindow::restoreWindowState()
{
	QSettings s{};

	auto mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	auto mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	auto mainWindowState = s.value("MainWindow/state").toByteArray();

	move(mainWindowPos);
	restoreGeometry(mainWindowGeometry);

	restoreState(mainWindowState);

	// Full screen could be set by script, and then saved on exit
	// there is no way to unset full screen from UI, so application always start without full-screen
	//
	if ((windowState() & Qt::WindowFullScreen) != 0)
	{
		setWindowState(windowState() ^ Qt::WindowFullScreen);
	}

	// Ensure widget is visible
	//
	QScreen* screenAt = QGuiApplication::screenAt(pos());

	if (screenAt == nullptr)
	{
		setWindowState(Qt::WindowMaximized);
	}
	else
	{
		QRect screenGeometry = screenAt->geometry();

		QRect intersect = screenGeometry.intersected(geometry());
		if (intersect.width() * intersect.height() < (screenGeometry.width() * screenGeometry.height()) * 0.2)
		{
			move(screenGeometry.topLeft());
		}
	}

	return;
}

void DiagnosticsMainWindow::showZoomControls()
{
	auto zoomMode = DiagnosticsAppSettings::instance().zoomMode();

	bool visible = zoomMode == VFrame30::ZoomMode::Manual;

	if (m_zoomToolBarSeparator != nullptr)
	{
		m_zoomToolBarSeparator->setVisible(visible);
	}

	m_zoomInAction->setVisible(visible);
	m_zoomOutAction->setVisible(visible);
	m_zoom100Action->setVisible(visible);
	m_zoomToFitAction->setVisible(visible);

	return;
}

void DiagnosticsMainWindow::showLogo()
{
	if (m_toolBar == nullptr || m_toolBar->isVisible() == false || m_logoImage.isNull() == true)
	{
		m_logoLabel->clear();
		m_logoLabel->setFixedSize(0, 0);
		return;
	}

	static bool prevShowLogo = false;
	if (prevShowLogo == DiagnosticsAppSettings::instance().showLogo())
	{
		return;
	}

	prevShowLogo = DiagnosticsAppSettings::instance().showLogo();

	// Show logo if it was enabled in settings
	//
	if (DiagnosticsAppSettings::instance().showLogo() == true)
	{
		// This is the only way to get height of the toolbar.
		// Toolbar does not return correct margins, at least now (bug?)
		//
		auto fakeSeparator = m_toolBar->addSeparator();
		auto fakeSeparatorWidget = m_toolBar->widgetForAction(fakeSeparator);
		fakeSeparatorWidget->setVisible(true);
		double toHeight = fakeSeparatorWidget->geometry().height();
		delete fakeSeparator;

		m_logoLabel->setPixmap(m_logoImage);
		m_logoLabel->setScaledContents(true);

		// Always scale logo to the toolbar height.
		//
		QSizeF logoSize = m_logoLabel->sizeHint().toSizeF();

		double scale = toHeight / static_cast<double>(logoSize.height());
		logoSize *= scale;

		m_logoLabel->setFixedSize(logoSize.toSize());
	}
	else
	{
		// Hide logo.
		//
		m_logoLabel->clear();
		m_logoLabel->setFixedSize(0, 0);
	}

	return;
}

void DiagnosticsMainWindow::createActions()
{
	m_pExportAction = new QAction(tr("Export Schema..."), this);
	m_pExportAction->setStatusTip(tr("Export current schema to a file"));
	m_pExportAction->setEnabled(true);
	m_pExportAction->setShortcuts(QList<QKeySequence>{} << QKeySequence{Qt::CTRL | Qt::Key_S});
	connect(m_pExportAction, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_export);

	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &DiagnosticsMainWindow::exit);

	m_pStatisticsAction = new QAction(tr("Connection Statistics..."), this);
	m_pStatisticsAction->setStatusTip(tr("View Connection Statistics"));
	m_pStatisticsAction->setIcon(QIcon(":/Images/Images/NetworkConnections.svg"));
	m_pStatisticsAction->setEnabled(true);
	connect(m_pStatisticsAction, &QAction::triggered, this, &DiagnosticsMainWindow::showStatistics);
	//
	//	m_pDataSourcesAction = new QAction(tr("Data Sources..."), this);
	//	m_pDataSourcesAction->setStatusTip(tr("View Data Sources"));
	//	m_pDataSourcesAction->setIcon(QIcon(":/Images/Images/AppDataSources.svg"));
	//	m_pDataSourcesAction->setEnabled(true);
	//	connect(m_pDataSourcesAction, &QAction::triggered, this, &MonitorMainWindow::showDataSources);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &DiagnosticsMainWindow::showSettings);
	//
	//	m_manualMatsAction = new QAction(tr("MATS User Manual"), this);
	//	m_manualMatsAction->setStatusTip(tr("Show MATS User Manual"));
	//	connect(m_manualMatsAction, &QAction::triggered, this, &MonitorMainWindow::showMatsUserManual);
	//
	//	m_pDevToolsAction = new QAction(tr("DevTools..."), this);
	//	m_pDevToolsAction->setStatusTip(tr("Show software statistics"));
	//	m_pDevToolsAction->setEnabled(true);
	//	connect(m_pDevToolsAction, &QAction::triggered, this, &MonitorMainWindow::devTools);
	//
	m_pDebugAction = new QAction(tr("Debug..."), this);
	m_pDebugAction->setStatusTip(tr("Perform some debug actions, developers tool."));
	m_pDebugAction->setEnabled(true);
	connect(m_pDebugAction, &QAction::triggered, this, &DiagnosticsMainWindow::debug);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	connect(m_pLogAction, &QAction::triggered, this, &DiagnosticsMainWindow::showLog);

	//    m_pTuningLogAction = new QAction(tr("Tuning Log..."), this);
	//    m_pTuningLogAction->setStatusTip(tr("Show tuning log"));
	//    connect(m_pTuningLogAction, &QAction::triggered, this, &MonitorMainWindow::showTuningLog);
	//    m_pTuningLogAction->setVisible(false);
	//
	m_pAboutQtAction = new QAction(tr("About Qt..."), this);
	m_pAboutQtAction->setStatusTip(tr("Show Qt information"));
	// m_pAboutAction->setEnabled(true);
	connect(m_pAboutQtAction, &QAction::triggered, this, &DiagnosticsMainWindow::showAboutQt);

	m_pAboutAction = new QAction(tr("About Diagnostics..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	// m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &DiagnosticsMainWindow::showAbout);
	//
	//	m_schemaListAction = new QAction(tr("Schemas"), this);
	//	m_schemaListAction->setStatusTip(tr("Open schema list page..."));
	//	m_schemaListAction->setIcon(QIcon(":/Images/Images/SchemaList.svg"));
	//	m_schemaListAction->setEnabled(true);
	//	m_schemaListAction->setCheckable(true);
	//	m_schemaListAction->setChecked(QSettings().value("m_schemaListAction.checked").toBool());
	//	m_schemaListAction->setShortcuts(QList<QKeySequence>{}
	//									 <<  QKeySequence{Qt::CTRL | Qt::Key_QuoteLeft}
	//									 <<  QKeySequence{Qt::CTRL | Qt::Key_AsciiTilde});
	//	connect(m_schemaListAction, &QAction::toggled, this, &MonitorMainWindow::schemaTreeListToggled);

	m_newTabAction = new QAction(tr("New Tab"), this);
	m_newTabAction->setStatusTip(tr("Open current schema in new tab page"));
	m_newTabAction->setIcon(QIcon(":/Images/Images/NewSchema.svg"));
	m_newTabAction->setEnabled(DiagnosticsAppSettings::instance().showSchemasTabBar());
	m_newTabAction->setVisible(DiagnosticsAppSettings::instance().showSchemasTabBar());
	QList<QKeySequence> newTabShortcuts;
	newTabShortcuts << QKeySequence::AddTab;
	newTabShortcuts << QKeySequence::New;
	m_newTabAction->setShortcuts(newTabShortcuts);
	connect(m_newTabAction, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_newTab);

	m_closeTabAction = new QAction(tr("Close Tab"), this);
	m_closeTabAction->setStatusTip(tr("Close current tab page"));
	m_closeTabAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_closeTabAction->setEnabled(DiagnosticsAppSettings::instance().showSchemasTabBar() && centralWidget()->count() > 1);
	m_closeTabAction->setVisible(DiagnosticsAppSettings::instance().showSchemasTabBar());
	m_closeTabAction->setShortcuts(QKeySequence::Close);
	connect(m_closeTabAction, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_closeCurrentTab);

	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setStatusTip(tr("Zoom in schema view"));
	m_zoomInAction->setIcon(QIcon(":/Images/Images/ZoomIn.svg"));
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(m_zoomInAction, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_zoomIn);

	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setStatusTip(tr("Zoom out schema view"));
	m_zoomOutAction->setIcon(QIcon(":/Images/Images/ZoomOut.svg"));
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(m_zoomOutAction, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_zoomOut);

	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setStatusTip(tr("Set zoom to 100%"));
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Asterisk));
	connect(m_zoom100Action, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_zoom100);

	m_zoomToFitAction = new QAction(tr("Fit to Screen"), this);
	m_zoomToFitAction->setStatusTip(tr("Set zoom to fit screen"));
	m_zoomToFitAction->setIcon(QIcon(":/Images/Images/ZoomFitToScreen.svg"));
	m_zoomToFitAction->setEnabled(true);
	m_zoomToFitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash));
	connect(m_zoomToFitAction, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_zoomToFit);

	m_historyBack = new QAction(tr("Go Back"), this);
	m_historyBack->setStatusTip(tr("Click to go back"));
	m_historyBack->setIcon(QIcon(":/Images/Images/Backward.svg"));
	m_historyBack->setEnabled(false);
	m_historyBack->setShortcut(QKeySequence::Back);
	connect(m_historyBack, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_historyBack);

	m_historyForward = new QAction(tr("Go Forward"), this);
	m_historyForward->setStatusTip(tr("Click to go forward"));
	m_historyForward->setIcon(QIcon(":/Images/Images/Forward.svg"));
	m_historyForward->setEnabled(false);
	m_historyForward->setShortcut(QKeySequence::Forward);
	connect(m_historyForward, &QAction::triggered, centralWidget(), &DiagnosticsCentralWidget::slot_historyForward);

	//	m_archiveAction = new QAction(tr("Archive"), this);
	//	m_archiveAction->setIcon(QIcon(":/Images/Images/Archive.svg"));
	//	m_archiveAction->setEnabled(true);
	//	m_archiveAction->setData(QVariant("IAmIndependentArchive"));	// This is required to find this action in MonitorToolBar for drag
	//and drop 	connect(m_archiveAction, &QAction::triggered, this, QOverload<>::of(&MonitorMainWindow::slot_archive));
	//
	//	m_trendsAction = new QAction(tr("Trends"), this);
	//	m_trendsAction->setIcon(QIcon(":/Images/Images/Trends.svg"));
	//	m_trendsAction->setEnabled(true);
	//	m_trendsAction->setData(QVariant("IAmIndependentTrend"));	// This is required to find this action in MonitorToolBar for drag and
	//drop 	connect(m_trendsAction, &QAction::triggered, this, &MonitorMainWindow::slot_trends);
	//
	//	m_signalSnapshotAction = new QAction(tr("Signals Snapshot"), this);
	//	m_signalSnapshotAction->setStatusTip(tr("View signals state in real time"));
	//	m_signalSnapshotAction->setIcon(QIcon(":/Images/Images/Snapshot.svg"));
	//	m_signalSnapshotAction->setEnabled(true);
	//	connect(m_signalSnapshotAction, &QAction::triggered, this, qOverload<>(&MonitorMainWindow::slot_signalSnapshot));
	//
	//	m_findSignalAction = new QAction(tr("Find Signal"), this);
	//	m_findSignalAction->setStatusTip(tr("Find signal by it's ID"));
	//	m_findSignalAction->setIcon(QIcon(":/Images/Images/FindSignal.svg"));
	//	m_findSignalAction->setEnabled(true);
	//	m_findSignalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
	//	connect(m_findSignalAction, &QAction::triggered, this, &MonitorMainWindow::slot_findSignal);
	//
	//	m_loginAction = new QAction(tr("Login"), this);
	//	m_loginAction->setToolTip(tr("Log in to change tunable values"));
	//	m_loginAction->setIcon(QIcon(":/Images/Images/KeyOff.svg"));
	//	m_loginAction->setEnabled(true);
	//	connect(m_loginAction, &QAction::triggered, this, &MonitorMainWindow::slot_login);
	//
	//	m_loginUserTimeoutAction = new QAction(tr("Logged Out"), this);
	//	m_loginUserTimeoutAction->setEnabled(false);
	//	connect(m_loginUserTimeoutAction, &QAction::triggered, this, &MonitorMainWindow::slot_reLogin);

	return;
}

void DiagnosticsMainWindow::createMenus()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExportAction);
	pFileMenu->addSeparator();
	pFileMenu->addAction(m_pExitAction);

	// Schema
	//
	QMenu* schemaMenu = menuBar()->addMenu(tr("&Schema"));

	//	schemaMenu->addAction(m_schemaListAction);
	schemaMenu->addAction(m_newTabAction);
	schemaMenu->addAction(m_closeTabAction);

	// View
	//
	QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

	viewMenu->addAction(m_zoomInAction);
	viewMenu->addAction(m_zoomOutAction);
	viewMenu->addAction(m_zoom100Action);
	viewMenu->addAction(m_zoomToFitAction);
	viewMenu->addSeparator();

	viewMenu->addAction(m_historyForward);
	viewMenu->addAction(m_historyBack);

	// Tools
	//
	QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));

	//	toolsMenu->addAction(m_archiveAction);
	//	toolsMenu->addAction(m_trendsAction);
	//
	//	toolsMenu->addSeparator();
	//	toolsMenu->addAction(m_signalSnapshotAction);
	//	toolsMenu->addAction(m_findSignalAction);

	toolsMenu->addSeparator();
	toolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* helpMenu = menuBar()->addMenu(tr("&?"));

	//	helpMenu->addAction(m_pDataSourcesAction);
	helpMenu->addAction(m_pStatisticsAction);

	helpMenu->addSeparator();
	helpMenu->addAction(m_pLogAction);
//    helpMenu->addAction(m_pTuningLogAction);
//	helpMenu->addAction(m_pDevToolsAction);
#ifdef QT_DEBUG
	helpMenu->addAction(m_pDebugAction);
#endif // QT_DEBUG

	   //	helpMenu->addSeparator();
	//
	//	helpMenu->addAction(m_manualMatsAction);
	//
	helpMenu->addSeparator();

	helpMenu->addAction(m_pAboutQtAction);
	helpMenu->addAction(m_pAboutAction);

	return;
}

void DiagnosticsMainWindow::createToolBars()
{
	m_toolBar = new DiagnosticsToolBar(tr("ToolBar"), this);
	m_toolBar->setObjectName("DiagnosticsMainToolBar");

	//	m_toolBar->addAction(m_schemaListAction);
	m_toolBar->addAction(m_newTabAction);

	m_zoomToolBarSeparator = m_toolBar->addSeparator();
	m_toolBar->addAction(m_zoomInAction);
	m_toolBar->addAction(m_zoomOutAction);
	m_toolBar->addAction(m_zoomToFitAction);

	//	m_toolBar->addSeparator();
	//	m_selectSchemaWidget = new SelectSchemaWidget(&m_configController, monitorCentralWidget());
	//	m_selectSchemaWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//	m_selectSchemaWidget->setMaximumWidth(1280);
	//	m_toolBar->addWidget(m_selectSchemaWidget);
	//
	m_toolBar->addSeparator();
	m_toolBar->addAction(m_historyBack);
	m_toolBar->addAction(m_historyForward);


	//	m_toolBar->addSeparator();
	//	m_toolBar->addAction(m_signalSnapshotAction);
	//	m_toolBar->addAction(m_findSignalAction);
	//
	//	m_toolBar->addSeparator();
	//	m_toolBar->addAction(m_archiveAction);
	//	m_toolBar->addAction(m_trendsAction);
	//
	// Spacer between actions and logo
	//
	m_spacer = new QWidget(this);
	m_spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	m_toolBar->addWidget(m_spacer);

	//	// Login action and widget
	//	//
	//	m_toolBar->addAction(m_loginAction);
	//	m_loginAction->setVisible(false);
	//
	//	m_toolBar->addAction(m_loginUserTimeoutAction);
	//	m_loginUserTimeoutAction->setVisible(false);
	//
	//	m_logoSeparator = m_toolBar->addSeparator();
	//	m_logoSeparator->setVisible(false);
	//
	// Create logo for toolbar
	//
	m_logoLabel = new QLabel(m_toolBar);
	m_toolBar->addWidget(m_logoLabel);
	this->addToolBar(Qt::TopToolBarArea, m_toolBar);

	int space = m_toolBar->sizeHint().height() / 12;
	m_toolBar->setStyleSheet(QString("QToolBar{ padding: %1; }").arg(space));

	return;
}

void DiagnosticsMainWindow::createStatusBar()
{
	m_statusBarInfo = new QLabel();
	m_statusBarInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarInfo->setIndent(3);

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarConfigConnection->setMinimumWidth(100);

	m_statusBarAppDataConnection = new QLabel();
	m_statusBarAppDataConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarAppDataConnection->setMinimumWidth(100);

	//	m_statusBarTuningConnection = new QLabel();
	//	m_statusBarTuningConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	//	m_statusBarTuningConnection->setMinimumWidth(100);
	//
	m_statusBarProjectInfo = new QLabel;
	m_statusBarProjectInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarProjectInfo->setMinimumWidth(100);

	m_statusBarLogAlerts = new QLabel;
	m_statusBarLogAlerts->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));
	m_statusBarLogAlerts->installEventFilter(this);

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarAppDataConnection, 0);
	//	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarProjectInfo, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);

	return;
}

DiagnosticsCentralWidget* DiagnosticsMainWindow::centralWidget()
{
	auto cw = dynamic_cast<DiagnosticsCentralWidget*>(QMainWindow::centralWidget());
	Q_ASSERT(cw != nullptr);
	return cw;
}

const DiagnosticsCentralWidget* DiagnosticsMainWindow::centralWidget() const
{
	auto cw = dynamic_cast<const DiagnosticsCentralWidget*>(QMainWindow::centralWidget());
	Q_ASSERT(cw != nullptr);
	return cw;
}

void DiagnosticsMainWindow::updateStatusBar()
{
	// Update status bar
	//
	Q_ASSERT(m_statusBarConfigConnection);
	Q_ASSERT(m_statusBarAppDataConnection);
	//	Q_ASSERT(m_statusBarTuningConnection);

	// ConfigService connection
	//
	{
		std::vector<Tcp::ConnectionState> confiConnState = {m_configController.getConnectionState()};

		showSoftwareConnection(tr("CfgService"), confiConnState, m_statusBarConfigConnection);
	}

	//// AppDataService connection
	////
	//{
	//	showSoftwareConnection(tr("AppDataService"),
	//						   m_adsConnection.tcpSignalConnStates(),
	//						   m_statusBarAppDataConnection);
	//}

	// BuildNo
	//
	{
		auto configInfo = m_configController.configInfo();

		QString text = tr(" Project: %1   Build: %2  ").arg(configInfo.project).arg(configInfo.buildNo);

		m_statusBarProjectInfo->setText(text);
	}

	if (m_logErrorsCounter != std::clamp(m_LogFile.errorAckCounter(), 0, 999) ||
		m_logWarningsCounter != std::clamp(m_LogFile.warningAckCounter(), 0, 999))
	{
		m_logErrorsCounter = std::clamp(m_LogFile.errorAckCounter(), 0, 999);
		m_logWarningsCounter = std::clamp(m_LogFile.warningAckCounter(), 0, 999);

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(tr(" Log E: %1 W: %2 ").arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
			m_statusBarLogAlerts->setStyleSheet(m_statusBarInfo->styleSheet());
		}
		else
		{
			if (m_logErrorsCounter == 0)
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #F87217}");
			}
			else
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #C00000}");
			}
		}
	}

	return;
}

void DiagnosticsMainWindow::showSoftwareConnection(const QString& caption,
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
			statusOk++;
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
		statusText = tr("%1: %2 (Replies: %3)").arg(caption).arg(statusOk ? tr("ok") : tr("down")).arg(replyCount);
	}
	else
	{
		statusText = tr("%1: %2/%3 (Replies: %4)").arg(caption).arg(statusOk).arg(connectionStates.size()).arg(replyCount);
	}

	label->setText(statusText);

	return;
}

void DiagnosticsMainWindow::exit()
{
	close();
}

// void MonitorMainWindow::schemaTreeListToggled(bool checked)
//{
//	if (m_schemaListDock == nullptr)
//	{
//		Q_ASSERT(m_schemaListDock);
//		return;
//	}
//
//	m_schemaListDock->setVisible(checked);
//
//	QSettings().setValue("m_schemaListAction.checked", checked);
//
//	return;
// }
//
void DiagnosticsMainWindow::showLog()
{
	m_LogFile.view(this);
}

// void MonitorMainWindow::showTuningLog()
//{
//	m_tuningLogFile.viewTuningLog(this);
// }
//
// void MonitorMainWindow::showDataSources()
//{
//	DialogDataSources::create(m_configController,
//							  m_tuningConnection,
//							  &m_LogFile,
//							  this);
// }


void DiagnosticsMainWindow::showSettings()
{
	DialogSettings d(m_translator, this);
	d.setSettings(DiagnosticsAppSettings::instance().get());

	int result = d.exec();

	if (result == QDialog::DialogCode::Accepted)
	{
		// --
		//
		bool needReconnect = false;
		bool reinitIntanceResolver = false;

		auto currentSettings = DiagnosticsAppSettings::instance().get();

		if (currentSettings.equipmentId != d.settings().equipmentId || currentSettings.cfgSrvIpAddress1 != d.settings().cfgSrvIpAddress1 ||
			currentSettings.cfgSrvPort1 != d.settings().cfgSrvPort1 || currentSettings.cfgSrvIpAddress2 != d.settings().cfgSrvIpAddress2 ||
			currentSettings.cfgSrvPort2 != d.settings().cfgSrvPort2)
		{
			needReconnect = true;
		}

		if (currentSettings.equipmentId != d.settings().equipmentId)
		{
			reinitIntanceResolver = true;
		}

		// --
		//
		DiagnosticsAppSettings::instance().set(d.settings());
		DiagnosticsAppSettings::instance().save();

		// Apply settings here
		//
		showLogo();
		showZoomControls();
		setVisibleTabBar(DiagnosticsAppSettings::instance().showSchemasTabBar());
		centralWidget()->setZoomMode(DiagnosticsAppSettings::instance().zoomMode());

		// Reconnect
		//
		if (needReconnect == true)
		{
			m_configController.setConnectionParams(DiagnosticsAppSettings::instance().equipmentId(),
												   DiagnosticsAppSettings::instance().configuratorAddress1(),
												   DiagnosticsAppSettings::instance().configuratorAddress2());
		}

		// --
		//
		if (reinitIntanceResolver == true)
		{
			m_instanceResolver.reinit(DiagnosticsAppSettings::instance().equipmentId(),
									  DiagnosticsAppSettings::instance().singleInstance());
		}

		setWindowTitle(DiagnosticsAppSettings::instance().windowCaption());

		// Set application name so all message boxes will have correct caption.
		//
		qApp->setApplicationName(DiagnosticsAppSettings::instance().windowCaption());

		return;
	}

	return;
}

void DiagnosticsMainWindow::showStatistics()
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

void DiagnosticsMainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());

	return;
}

void DiagnosticsMainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to view schemas and trends.<br>");
	UiLib::DialogAbout::show(this, text, ":/Logo/RadiyLogo.png");
	return;
}

// void MonitorMainWindow::showMatsUserManual()
//{
//	UiTools::openPdf(QApplication::applicationDirPath()+"/docs/D11.8_FSC_MATS_User_Manual.pdf", this);
// }
//
// void MonitorMainWindow::devTools()
//{
//	QDialog statsDialog{this};
//
//	QVBoxLayout* layout = new QVBoxLayout{};
//
//	QTextEdit* textEdit = new QTextEdit{};
//	layout->addWidget(textEdit);
//
//	// --
//	//
//	QString str;
//
//	for (auto modules = m_schemaStats.modules();
//		 const QString& module : modules)
//	{
//		for (auto items = m_schemaStats.items(module);
//			 const QString& item : items)
//		{
//			for (auto records = m_schemaStats.itemRecords(module, item);
//				 const auto& record: records)
//			{
//				str += QString("%1;%2;%3;%4\n")
//					   .arg(module)
//					   .arg(item)
//					   .arg(record.action)
//					   .arg(record.time.count());
//			}
//		}
//	}
//
//	textEdit->setText(str);
//
//	// --
//	//
//	statsDialog.setLayout(layout);
//	statsDialog.exec();
//
//	return;
// }
//
void DiagnosticsMainWindow::debug()
{
#ifdef QT_DEBUG
#endif // QT_DEBUG
}
//
// void MonitorMainWindow::slot_archive()
//{
//	qDebug() << "";
//	qDebug() << Q_FUNC_INFO;
//
//	// Get Archive list
//	//
//	std::vector<QString> archives = MonitorArchive::getArchiveList();
//
//	// Choose window
//	//
//	QString archiveWindowToActivate;
//
//	if (archives.empty() == true)
//	{
//		archiveWindowToActivate.clear();	// if archiveWindowToActivate is empty, then create new ArchiveWidget
//	}
//	else
//	{
//		QMenu menu;
//
//		QAction* newArchiveAction = menu.addAction("New Window...");
//		newArchiveAction->setData(QVariant::fromValue<int>(-1));		// Data -1 means, create new widget
//
//		menu.addSeparator();
//
//		for (size_t i = 0; i < archives.size(); i++)
//		{
//			QAction* a = menu.addAction(archives[i]);
//			Q_ASSERT(a);
//
//			a->setData(QVariant::fromValue<int>(static_cast<int>(i)));		// Data is index in archives vector
//		}
//
//		QAction* triggeredAction = menu.exec(QCursor::pos());
//		if (triggeredAction == nullptr)
//		{
//			return;
//		}
//
//		QVariant data = triggeredAction->data();
//
//		bool ok = false;
//		int archiveIndex = data.toInt(&ok);
//
//		if (archiveIndex < 0)
//		{
//			archiveWindowToActivate.clear();	// if trendToActivate is empty, then create new trend
//		}
//		else
//		{
//			if (ok == false || archiveIndex >= static_cast<int>(archives.size()))
//			{
//				Q_ASSERT(ok == true);
//				Q_ASSERT(archiveIndex < static_cast<int>(archives.size()));
//				return;
//			}
//
//			archiveWindowToActivate = archives.at(archiveIndex);
//		}
//	}
//
//	// Start new trend or activate chosen one
//	//
//	if (archiveWindowToActivate.isEmpty() == true)
//	{
//		std::vector<AppSignalParam> appSignals;
//		MonitorArchive::startNewWidget(&m_appSignalManager, &m_configController, appSignals, this);
//	}
//	else
//	{
//		MonitorArchive::activateWindow(archiveWindowToActivate);
//	}
//
//	return;
//}
//
// void MonitorMainWindow::slot_archive(QStringList signalsList, QDateTime startTime, QDateTime endTime, int timeType)
//{
//	std::vector<AppSignalParam> appSignals;
//	QStringList notFoundSignals;
//
//	if (m_appSignalManager.signalsCount() == 0)
//	{
//		QMessageBox::critical(this, qAppName(), tr("Signals database is not loaded!"));
//		return;
//	}
//
//	for (const QString& s : signalsList)
//	{
//		bool ok = false;
//		AppSignalParam asp = m_appSignalManager.signalParam(s, &ok);
//
//		if (ok == true)
//		{
//			appSignals.push_back(asp);
//		}
//		else
//		{
//			notFoundSignals.push_back(s);
//		}
//	}
//
//	if (notFoundSignals.empty() == false)
//	{
//		QString errorMsg;
//
//		int count = static_cast<int>(notFoundSignals.size());
//		if (count > 10)
//		{
//			notFoundSignals.erase(notFoundSignals.begin() + 10, notFoundSignals.end());
//
//			errorMsg = tr("Signals with specified identifiers were not found:\n\n%1\n\nand %2 more.")
//					   .arg(notFoundSignals.join('\n'))
//					   .arg(count - notFoundSignals.size());
//		}
//		else
//		{
//			errorMsg = tr("Signals with specified identifiers were not found:\n\n%1\n").arg(notFoundSignals.join('\n'));
//		}
//
//		QMessageBox::critical(this, qAppName(), errorMsg);
//		return;
//	}
//
//	if (appSignals.empty() == true)
//	{
//		QMessageBox::critical(this, qAppName(), tr("No signals supplied!"));
//		return;
//	}
//
//	if (timeType != static_cast<int>(E::TimeType::Plant) &&
//		timeType != static_cast<int>(E::TimeType::System) &&
//		timeType != static_cast<int>(E::TimeType::Local))
//	{
//		QMessageBox::critical(this, qAppName(), tr("Incorrect time type! Supported values: 0 - Plant, 1 - System, 2 - Local."));
//		return;
//	}
//
//	if (startTime > endTime)
//	{
//		QMessageBox::critical(this, qAppName(), tr("Archive request Start Time (%1) shoud be earlier than End Time (%2).")
//							  .arg(startTime.toString("dd/MM/yyyy hh:mm:ss"))
//							  .arg(endTime.toString("dd/MM/yyyy hh:mm:ss")));
//		return;
//	}
//
//	MonitorArchive::requestArchiveWithNewWidget(&m_appSignalManager, &configController(), appSignals, startTime, endTime,
//static_cast<E::TimeType>(timeType), this); 	return;
//}


// void MonitorMainWindow::slot_trends()
//{
//	// Get Trends list
//	//
//	std::vector<MonitorTrendsWidget*> trends = MonitorTrends::getTrendsList();
//
//	// Choose trend
//	//
//	MonitorTrendsWidget* trendToActivate = nullptr;
//
//	if (trends.empty() == true)
//	{
//		trendToActivate = nullptr;	// if trendToActivate is nullptr, then create a new trend.
//	}
//	else
//	{
//		QMenu menu;
//
//		QAction* newTrendAction = menu.addAction("New Trend...");
//		newTrendAction->setData(QVariant::fromValue<int>(-1));		// Data -1 means, create new trend widget
//
//		menu.addSeparator();
//
//		for (size_t i = 0; i < trends.size(); i++)
//		{
//			QAction* a = menu.addAction(trends[i]->windowTitle());
//			Q_ASSERT(a);
//
//			a->setData(QVariant::fromValue<int>(static_cast<int>(i)));		// Data is index in trend vector
//		}
//
//		QAction* triggeredAction = menu.exec(QCursor::pos());
//		if (triggeredAction == nullptr)
//		{
//			return;
//		}
//
//		QVariant data = triggeredAction->data();
//
//		bool ok = false;
//		int trendIndex = data.toInt(&ok);
//
//		if (trendIndex == -1)
//		{
//			trendToActivate = nullptr;	// if trendToActivate is nullptr, then create a new trend.
//		}
//		else
//		{
//			if (ok == false || trendIndex < 0 || trendIndex >= static_cast<int>(trends.size()))
//			{
//				Q_ASSERT(ok == true);
//				Q_ASSERT(trendIndex >= 0 && trendIndex < static_cast<int>(trends.size()));
//				return;
//			}
//
//			trendToActivate = trends.at(trendIndex);
//		}
//	}
//
//	// Start new trend or activate chosen one
//	//
//	if (trendToActivate == nullptr)
//	{
//		std::vector<AppSignalParam> appSignals;
//		MonitorTrends::startTrendApp(m_appSignalManager, m_configController, appSignals, this);
//	}
//	else
//	{
//		MonitorTrends::activateTrendWindow(trendToActivate);
//	}
//
//	return;
// }
//
// void MonitorMainWindow::slot_signalSnapshot()
//{
//	MonitorDialogSignalSnapshot* d = MonitorDialogSignalSnapshot::createDialog(&m_configController,
//																			   &m_appSignalManager,
//																			   monitorCentralWidget());
//	d->show();
//
//	return;
// }
//
// void MonitorMainWindow::slot_signalSnapshot(QStringList signalsList)
//{
//	MonitorDialogSignalSnapshot* d = MonitorDialogSignalSnapshot::createDialog(
//										 &configController(),
//										 &m_appSignalManager,
//										 monitorCentralWidget());
//
//	std::vector<AppSignalParam> specialSignals;
//
//	QStringList notFoundSignals;
//
//	for (const QString& appSignalId : signalsList)
//	{
//		bool found = false;
//
//		AppSignalParam asp = m_appSignalManager.signalParam(appSignalId, &found);
//		if (found == true)
//		{
//			specialSignals.push_back(asp);
//		}
//		else
//		{
//			notFoundSignals.push_back(appSignalId);
//		}
//	}
//
//
//	if (notFoundSignals.empty() == false)
//	{
//		QString errorMsg;
//
//		int count = static_cast<int>(notFoundSignals.size());
//		if (count > 10)
//		{
//			notFoundSignals.erase(notFoundSignals.begin() + 10, notFoundSignals.end());
//
//			errorMsg = tr("Signals with specified identifiers were not found:\n\n%1\n\nand %2 more.")
//								  .arg(notFoundSignals.join('\n'))
//								  .arg(count - notFoundSignals.size());
//
//		}
//		else
//		{
//			errorMsg = tr("Signals with specified identifiers were not found!\n\n%1").arg(notFoundSignals.join('\n'));
//		}
//
//		QMessageBox::critical(this, qAppName(), errorMsg);
//		return;
//
//	}
//
//	if (specialSignals.empty() == true)
//	{
//		return;
//	}
//
//	d->resetSignalsType();
//	d->setSignalsMask({});
//	d->setSignalsTags({});
//
//	d->setSpecificSignals(specialSignals);
//
//	d->show();
// }
//
// void MonitorMainWindow::slot_signalSnapshotByMask(QStringList masks)
//{
//	auto d = MonitorDialogSignalSnapshot::createDialog(&configController(),
//													   &m_appSignalManager,
//													   monitorCentralWidget());
//
//	d->resetSignalsType();
//	d->setSignalsMask(masks);
//	d->setSignalsTags({});
//
//	d->show();
// }
//
// void MonitorMainWindow::slot_signalSnapshotByTag(QStringList tags)
//{
//	auto d = MonitorDialogSignalSnapshot::createDialog(&configController(),
//													   &m_appSignalManager,
//													   monitorCentralWidget());
//
//	d->resetSignalsType();
//	d->setSignalsMask({});
//	d->setSignalsTags(tags);
//
//	d->show();
// }
//
// void MonitorMainWindow::slot_findSignal()
//{
//	MonitorCentralWidget* cw = monitorCentralWidget();
//	if (cw == nullptr)
//	{
//		Q_ASSERT(cw);
//		return;
//	}
//
//	DialogSignalSearch* dsi = new DialogSignalSearch(this, &m_appSignalManager);
//
//	connect(&m_appSignalManager, &MonitorAppSignalManager::signalParamsUpdated, dsi, &DialogSignalSearch::signalsUpdated);
//
//	connect(dsi, &DialogSignalSearch::signalContextMenu, cw, &MonitorCentralWidget::slot_signalContextMenu);
//	connect(dsi, &DialogSignalSearch::signalInfo, cw, &MonitorCentralWidget::slot_signalInfo);
//
//	dsi->show();
//
//	return;
// }

void DiagnosticsMainWindow::slot_historyChanged(bool enableBack, bool enableForward)
{
	if (m_historyBack == nullptr || m_historyForward == nullptr)
	{
		Q_ASSERT(m_historyBack);
		Q_ASSERT(m_historyForward);
		return;
	}

	m_historyBack->setEnabled(enableBack);
	m_historyForward->setEnabled(enableForward);

	return;
}

void DiagnosticsMainWindow::slot_updateActions(bool schemaWidgetSelected)
{
	m_zoomInAction->setEnabled(schemaWidgetSelected);
	m_zoomOutAction->setEnabled(schemaWidgetSelected);
	m_zoom100Action->setEnabled(schemaWidgetSelected);
	m_zoomToFitAction->setEnabled(schemaWidgetSelected);

	m_historyBack->setEnabled(schemaWidgetSelected);
	m_historyForward->setEnabled(schemaWidgetSelected);

	//	m_selectSchemaWidget->setEnabled(schemaWidgetSelected);

	return;
}

void DiagnosticsMainWindow::slot_configurationArrived(DiagConfigSettings configuration)
{
	qDebug() << "MonitorMainWindow::slot_configurationArrived()";

	centralWidget()->updateConfiguration(configuration);

	// Update AppSignalManager with specific data
	//

	// m_adsConnection.updateConnections(m_configController.softwareInfo(), configuration.appDataServices);
	// m_appSignalManager.setSetpoints(m_configController.setpoints());

	m_logoImage = configuration.logoImage;

	showLogo();

	return;
}

void DiagnosticsMainWindow::slot_configurationError(QString error)
{
	QMessageBox::critical(this, qAppName(), tr("Configuration error: %1").arg(error));
	return;
}

void DiagnosticsMainWindow::activateRequested()
{
	// To move window to top, add WindowStaysOnTopHint flag. In linux X11Bypass tag required
	// to do this. When flags added - activateWindow and show it to apply changes. After that, window
	// will be every time on top, so we need remove WindowStaysOnTop flag, apply changes, and only then remove
	// X11Bypass flag.
	//
	//	this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
	//	this->activateWindow();
	//	this->show();
	//	this->setWindowFlags(this->windowFlags() & (~Qt::WindowStaysOnTopHint));
	//	this->activateWindow();
	//	this->show();
	//	this->setWindowFlags(this->windowFlags() & (~Qt::X11BypassWindowManagerHint));
	//	this->activateWindow();
	//	this->show();

	// WARNING: Windows prevents from stealing focus, to avoid it set registry key
	// Computer\HKEY_CURRENT_USER\Control Panel\Desktop\ForegroundLockTimeout to 0
	// Restart computer
	//
	this->activateWindow();
	this->show();

	return;
}

// void MonitorMainWindow::slot_login()
//{
//	if (m_tuningUserManager.isLoggedIn() == true)
//	{
//		m_tuningUserManager.logout();
//	}
//	else
//	{
//		m_tuningUserManager.login(this);
//	}
// }
//
// void MonitorMainWindow::toggleSchemaTree()
//{
//	if (m_schemaListAction != nullptr)
//	{
//		m_schemaListAction->toggle();
//	}
//
//	return;
// }
//
// void MonitorMainWindow::setVisibleSchemaTree(bool visible)
//{
//	if (m_schemaListAction != nullptr)
//	{
//		m_schemaListAction-> setChecked(visible);
//	}
//
//	return;
// }
//

void DiagnosticsMainWindow::setVisibleTabBar(bool visible)
{
	auto m = centralWidget();
	Q_ASSERT(m);

	if (m != nullptr)
	{
		m->tabBar()->setVisible(visible);
	}

	m_newTabAction->setVisible(visible);
	m_newTabAction->setEnabled(visible);

	m_closeTabAction->setEnabled(visible);
	m_closeTabAction->setVisible(visible);

	return;
}

void DiagnosticsMainWindow::setVisibleToolBar(bool visible)
{
	if (m_toolBar != nullptr)
	{
		m_toolBar->setVisible(visible);
	}

	return;
}

void DiagnosticsMainWindow::setVisibleStatusBar(bool visible)
{
	if (auto sb = statusBar(); sb != nullptr)
	{
		sb->setVisible(visible);
	}

	return;
}

void DiagnosticsMainWindow::setVisibleMenu(bool visible)
{
	if (auto m = menuBar(); m != nullptr)
	{
		m->setVisible(visible);
	}

	return;
}

void DiagnosticsMainWindow::setFullScreen(bool value)
{
	if (value == true)
	{
		setWindowState(windowState() | Qt::WindowFullScreen);
	}
	else
	{
		if ((windowState() & Qt::WindowFullScreen) != 0)
		{
			setWindowState(windowState() ^ Qt::WindowFullScreen);
		}
	}

	return;
}

DiagConfigController& DiagnosticsMainWindow::configController()
{
	return m_configController;
}

const DiagConfigController& DiagnosticsMainWindow::configController() const
{
	return m_configController;
}

ClientLib::AppSignalManager& DiagnosticsMainWindow::appSignalManager()
{
	return m_appSignalManager;
}

const ClientLib::AppSignalManager& DiagnosticsMainWindow::appSignalManager() const
{
	return m_appSignalManager;
}

DiagnosticsToolBar::DiagnosticsToolBar(const QString& tittle, QWidget* parent) :
	QToolBar(tittle, parent)
{
	setAcceptDrops(true);
	setMovable(false);

	return;
}

void DiagnosticsToolBar::addAction(QAction* action)
{
	Q_ASSERT(action);

	QWidget::addAction(action);
	widgetForAction(action)->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	return;
}

void DiagnosticsToolBar::dragEnterEvent(QDragEnterEvent* /*event*/)
{
	//	// Find Trend action
	//	//
	//	QWidget* trendActionWidget = nullptr;
	//	QWidget* archiveActionWidget = nullptr;
	//
	//	QList<QAction*> allActions = actions();
	//	for (QAction* a : allActions)
	//	{
	//		QVariant d = a->data();
	//
	//		if (d.isValid() &&
	//			d.typeId() == QMetaType::QString)
	//		{
	//			if (d.toString() == QLatin1String("IAmIndependentTrend"))
	//			{
	//				trendActionWidget = widgetForAction(a);
	//				trendActionWidget->setAcceptDrops(true);
	//			}
	//
	//			if (d.toString() == QLatin1String("IAmIndependentArchive"))
	//			{
	//				archiveActionWidget = widgetForAction(a);
	//				archiveActionWidget->setAcceptDrops(true);
	//			}
	//		}
	//	}
	//
	//	if (trendActionWidget != nullptr &&
	//		trendActionWidget->geometry().contains(event->position().toPoint()) &&
	//		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	//	{
	//		event->acceptProposedAction();
	//	}
	//
	//	if (archiveActionWidget != nullptr &&
	//		archiveActionWidget->geometry().contains(event->position().toPoint()) &&
	//		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	//	{
	//		event->acceptProposedAction();
	//	}
	//
	return;
}

void DiagnosticsToolBar::dropEvent(QDropEvent* /*event*/)
{
	//	// Find Trend action
	//	//
	//	QWidget* trendActionWidget = nullptr;
	//	QAction* trendAction = nullptr;
	//
	//	QWidget* archiveActionWidget = nullptr;
	//	QAction* archiveAction = nullptr;
	//
	//	QList<QAction*> allActions = actions();
	//
	//	for (QAction* a : allActions)
	//	{
	//		QVariant d = a->data();
	//		if (d.isValid() &&
	//			d.typeId() == QMetaType::QString)
	//		{
	//			if (d.toString() == QLatin1String("IAmIndependentTrend"))
	//			{
	//				trendAction = a;
	//				trendActionWidget = widgetForAction(trendAction);
	//			}
	//
	//			if (d.toString() == QLatin1String("IAmIndependentArchive"))
	//			{
	//				archiveAction = a;
	//				archiveActionWidget = widgetForAction(archiveAction);
	//			}
	//		}
	//	}
	//
	//	if (trendAction != nullptr &&
	//		trendActionWidget != nullptr &&
	//		trendActionWidget->geometry().contains(event->position().toPoint()) &&
	//		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	//	{
	//		// Lets assume parent isMaonitorMainWindow
	//		//
	//		MonitorMainWindow* m = dynamic_cast<MonitorMainWindow*>(this->parent());
	//		if (m == nullptr)
	//		{
	//			Q_ASSERT(m);
	//			return;
	//		}
	//
	//		// Load data from drag and drop
	//		//
	//		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);
	//
	//		::Proto::AppSignalSet protoSetMessage;
	//		bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));
	//
	//		if (ok == false)
	//		{
	//			event->acceptProposedAction();
	//			return;
	//		}
	//
	//		std::vector<AppSignalParam> appSignals;
	//		appSignals.reserve(protoSetMessage.appsignal_size());
	//
	//		// Parse data
	//		//
	//		for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	//		{
	//			const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);
	//
	//			AppSignalParam appSignalParam;
	//			ok = appSignalParam.load(appSignalMessage);
	//
	//			if (ok == true)
	//			{
	//				appSignals.push_back(appSignalParam);
	//			}
	//		}
	//
	//		if (appSignals.empty() == false)
	//		{
	//			m->showTrends(appSignals);
	//		}
	//	}
	//
	//	if (archiveAction != nullptr &&
	//		archiveActionWidget != nullptr &&
	//		archiveActionWidget->geometry().contains(event->position().toPoint()) &&
	//		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	//	{
	//		// Lets assume parent isMonitorMainWindow
	//		//
	//		MonitorMainWindow* mainWindow = dynamic_cast<MonitorMainWindow*>(this->parent());
	//		if (mainWindow == nullptr)
	//		{
	//			Q_ASSERT(mainWindow);
	//			return;
	//		}
	//
	//		// Load data from drag and drop
	//		//
	//		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);
	//
	//		::Proto::AppSignalSet protoSetMessage;
	//		bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));
	//
	//		if (ok == false)
	//		{
	//			event->acceptProposedAction();
	//			return;
	//		}
	//
	//		std::vector<AppSignalParam> appSignals;
	//		appSignals.reserve(protoSetMessage.appsignal_size());
	//
	//		// Parse data
	//		//
	//		for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	//		{
	//			const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);
	//
	//			AppSignalParam appSignalParam;
	//			ok = appSignalParam.load(appSignalMessage);
	//
	//			if (ok == true)
	//			{
	//				appSignals.push_back(appSignalParam);
	//			}
	//		}
	//
	//		if (appSignals.empty() == false)
	//		{
	//			MonitorArchive::startNewWidget(&mainWindow->signalManager(), &mainWindow->configController(), appSignals, mainWindow);
	//		}
	//	}

	return;
}
