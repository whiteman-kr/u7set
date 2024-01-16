#include "SimWidget.h"
#include "SimProjectWidget.h"
#include "SimOutputWidget.h"
#include "SimOverridePane.h"
#include "SimSelectBuildDialog.h"
#include "SimLogicModulePage.h"
#include "SimConnectionPage.h"
#include "SimSelectSchemaPage.h"
#include "SimSchemaPage.h"
#include "SimCodePage.h"
#include "SimTrend/SimTrends.h"
#include "../../lib/Ui/TabWidgetEx.h"
#include "../../lib/Ui/DialogSignalSearch.h"
#include "SimSignalSnapshot.h"
#include "SimSignalInfo.h"


SimWidget::SimWidget(std::shared_ptr<Sim::ConsoleLogFile> ideLogFile,
					 std::shared_ptr<SimIdeSimulator> simulator,
					 DbController* db,
					 QWidget* parent /*= nullptr*/,
					 Qt::WindowType windowType /*= Qt::Window*/,
					 bool slaveWindow /*= false*/) :
	QMainWindow(parent),
	HasDbController(db),
	m_slaveWindow(slaveWindow),
	m_ideLogFile(ideLogFile ? ideLogFile : std::make_shared<Sim::ConsoleLogFile>()),
	m_simulator(simulator ? simulator : std::make_shared<SimIdeSimulator>(m_ideLogFile.get(), true, nullptr)),
	m_schemaManager(m_simulator.get())
{
	// --
	//
	m_appSignalController = new VFrame30::AppSignalController{m_simulator->appSignalManager(), this};

	// --
	//
	setWindowFlags(windowType);
	setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

	m_tabWidget = new TabWidgetEx{this};
	m_tabWidget->setDocumentMode(false);
	m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

	setCentralWidget(m_tabWidget);
	centralWidget()->setAutoFillBackground(true);

	QVBoxLayout* layout = new QVBoxLayout;

	centralWidget()->setLayout(layout);

	auto margins = layout->contentsMargins();
	margins.setTop(0);
	layout->setContentsMargins(margins);

	createDocks();
	createToolBar();

	updateActions();

	// --
	//
	connect(db, &DbController::projectOpened, this, &SimWidget::closeBuild);		// SimProject could be opened manually by browsing
	connect(db, &DbController::projectOpened, this, &SimWidget::projectOpened);

	connect(db, &DbController::projectClosed, this, &SimWidget::closeBuild);
	connect(db, &DbController::projectClosed, this, &SimWidget::projectClosed);

	connect(m_simulator.get(), &Sim::Simulator::projectUpdated, this, &SimWidget::updateActions);
	connect(&(m_simulator->control()), &Sim::Control::stateChanged, this, &SimWidget::controlStateChanged);
	connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimWidget::updateTimeIndicator);

	connect(m_projectWidget, &SimProjectWidget::signal_openLogicModuleTabPage, this, &SimWidget::openLogicModuleTabPage);
	connect(m_projectWidget, &SimProjectWidget::signal_openCodeTabPage, this, &SimWidget::openCodeTabPage);
	connect(m_projectWidget, &SimProjectWidget::signal_openConnectionTabPage, this, &SimWidget::openConnectionTabPage);
	connect(m_projectWidget, &SimProjectWidget::signal_openAppSchemasTabPage, this, &SimWidget::openAppSchemasTabPage);

	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &SimWidget::tabCloseRequest);
	connect(m_tabWidget, &QTabWidget::currentChanged, this, &SimWidget::tabCurrentChanged);
	connect(m_tabWidget->tabBar(), &QTabWidget::customContextMenuRequested, this, &SimWidget::tabBarContextMenuRequest);

	connect(this, &SimWidget::needUpdateActions, this, &SimWidget::updateActions);

	if (m_slaveWindow == false)
	{
		connect(qApp, &QCoreApplication::aboutToQuit, this, &SimWidget::aboutToQuit);
	}

	// Add shortcut for switching to control tab page
	//
	m_showControlTabAccelerator = new QAction{tr("Schemas Control"), this};
	m_showControlTabAccelerator->setShortcuts(QList<QKeySequence>{}
											  <<  QKeySequence{Qt::CTRL | Qt::Key_QuoteLeft}
											  <<  QKeySequence{Qt::CTRL | Qt::Key_AsciiTilde}
											  );
	m_showControlTabAccelerator->setShortcutContext(Qt::ApplicationShortcut);

	addAction(m_showControlTabAccelerator);

	connect(m_showControlTabAccelerator, &QAction::triggered, this, &SimWidget::openAppSchemasTabPage);

	return;
}

SimWidget::~SimWidget()
{
}

void SimWidget::startTrends(const std::vector<AppSignalParam>& appSignals)
{
	SimTrends::startTrendApp(m_simulator, appSignals, this);
}

void SimWidget::signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu)
{
	// Compose menu
	//
	QMenu menu(this);

	for (const QString& s : signalList)
	{
		bool ok = false;
		AppSignalParam signal =	m_appSignalController->signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		auto f = [this, s]() -> void
				 {
					signalInfo(s);
				 };

		menu.addAction(signalId, f);
	}

	if (customMenu.empty() == false)
	{
		menu.addSeparator();

		for (auto cm : customMenu)
		{
			menu.addActions(cm->actions());
		}
	}

	menu.exec(QCursor::pos());
}

void SimWidget::signalInfo(QString appSignalId)
{
	SimSignalInfo::showDialog(appSignalId, m_simulator.get(), this);

	return;
}

void SimWidget::openSchemaTabPage(QString schemaId, QStringList highlightIds)
{
	// Look for already opened schema, and activate it
	//
	{
		for (int i = 0; i < m_tabWidget->count(); i++)
		{
			SimSchemaPage* sp = dynamic_cast<SimSchemaPage*>(m_tabWidget->widget(i));

			if (sp != nullptr && sp->schemaId() == schemaId)
			{
				m_tabWidget->setCurrentIndex(i);
				sp->setHighlightIds(highlightIds);
				return;
			}
		}
	}

	// There is no such schema, load it and create a widget for it
	//

	// Create a fake context, later it will be changed in SimSchemaWidget::SimSchemaWidget(...)
	//
	auto fakeContext = VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr, nullptr);

	std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaId, std::move(fakeContext));
	if (schema == nullptr)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open file %1").arg(schemaId));
		return;
	}

	SimSchemaPage* page = new SimSchemaPage{schema,
			m_simulator.get(),
			&m_schemaManager,
			m_appSignalController,
			m_tabWidget};

	int tabIndex = m_tabWidget->addTab(page, schema->schemaId());
	m_tabWidget->setCurrentIndex(tabIndex);

	page->simSchemaWidget()->setZoom(0, false);
	page->setHighlightIds(highlightIds);

	return;
}

void SimWidget::createToolBar()
{
	m_toolBar = new SimToolBar{"ToolBar"};
	addToolBar(m_toolBar);

	m_openProjectAction = new QAction{QIcon(":/Images/Images/SimOpen.svg"), tr("Open Build"), this};
	m_openProjectAction->setShortcut(QKeySequence::Open);
	connect(m_openProjectAction, &QAction::triggered, this, &SimWidget::openBuild);

	m_closeProjectAction = new QAction{QIcon(":/Images/Images/SimClose.svg"), tr("Close"), this};
	m_closeProjectAction->setShortcut(QKeySequence::Close);
	connect(m_closeProjectAction, &QAction::triggered, this, &SimWidget::closeBuild);

	m_refreshProjectAction = new QAction{QIcon(":/Images/Images/SimRefresh.svg"), tr("Refresh"), this};
	m_refreshProjectAction->setShortcut(QKeySequence::Refresh);
	connect(m_refreshProjectAction, &QAction::triggered, this, &SimWidget::refreshBuild);

	m_addWindowAction = new QAction{QIcon(":/Images/Images/SimAddWindow.svg"), tr("Add Window"), this};
	m_addWindowAction->setShortcut(QKeySequence::New);
	connect(m_addWindowAction, &QAction::triggered, this, &SimWidget::addNewWindow);
	m_toolBar->addAction(m_addWindowAction);

	// --
	//
	m_simulationTimeEdit = new QLineEdit{this};
	m_simulationTimeEdit->setPlaceholderText("Infinite");
	m_simulationTimeEdit->setClearButtonEnabled(false);
	m_simulationTimeEdit->setToolTip("Simualtion time in seconds.\n\"0\" - at least one workcyle.\nClear the field for an infinite simulation (till Stop or Pause).\nExamples: \"0.500\" - 500ms, \"60\" - 1min, \"3600\" - 1hour.");
	m_simulationTimeEdit->setSizePolicy(QSizePolicy::Policy::Minimum, m_simulationTimeEdit->sizePolicy().verticalPolicy());
	m_simulationTimeEdit->setMaxLength(18);

	QFontMetrics fm(m_simulationTimeEdit->font());
	int pixelWidth = fm.horizontalAdvance("0000000000.000");
	m_simulationTimeEdit->setMaximumWidth(pixelWidth);

	m_simulationTimeLocale.setNumberOptions(m_simulationTimeLocale.numberOptions() & ~(QLocale::OmitGroupSeparator));
	m_simulationTimeEditValidator.setLocale(m_simulationTimeLocale);
	m_simulationTimeEditValidator.setNotation(QDoubleValidator::Notation::StandardNotation);
	m_simulationTimeEditValidator.setBottom(0.001);
	m_simulationTimeEditValidator.setDecimals(3);

	m_simulationTimeEdit->setValidator(&m_simulationTimeEditValidator);

	// --
	//
	m_speedComboBox = new QComboBox{this};
	m_speedComboBox->setToolTip("Simulation speed factor.\nFF - Fast Forward.\nNote: Simulation speed depends on hardware and project complexity.");
	m_speedComboBox->addItem("x0.1", QVariant{0.1});
	m_speedComboBox->addItem("x0.25", QVariant{0.25});
	m_speedComboBox->addItem("x0.5", QVariant{0.5});
	m_speedComboBox->addItem("x1", QVariant{1.0});
	m_speedComboBox->addItem("x2", QVariant{2.0});
	m_speedComboBox->addItem("x4", QVariant{4.0});
	m_speedComboBox->addItem("FF", QVariant{256.0});
	m_speedComboBox->setCurrentIndex(3);	// x1

	auto speedChangedFunc = [this](int)
		{
			bool ok;
			double d = m_speedComboBox->currentData().toDouble(&ok);
			m_simulator->control().setSpeedFactor(ok ? d : 1.0);
		};

	connect(m_speedComboBox, &QComboBox::currentIndexChanged, speedChangedFunc);

	speedChangedFunc(3);	// Call first time to init m_simualtor

	// --
	//
	m_runAction = new QAction{QIcon(":/Images/Images/SimRun.svg"), tr("Run simulation for complete project"), this};
	QList<QKeySequence> runsKeys;
	runsKeys << QKeySequence{Qt::CTRL | Qt::Key_R};
	runsKeys << QKeySequence{Qt::CTRL | Qt::Key_F5};
	m_runAction->setShortcuts(runsKeys);
	connect(m_runAction, &QAction::triggered, this, &SimWidget::runSimulation);

	m_pauseAction = new QAction{QIcon(":/Images/Images/SimPause.svg"), tr("Pause current simulation"), this};
	connect(m_pauseAction, &QAction::triggered, this, &SimWidget::pauseSimulation);

	m_stopAction = new QAction{QIcon(":/Images/Images/SimStop.svg"), tr("Stop current simulation"), this};
	m_stopAction->setShortcut(Qt::SHIFT | Qt::Key_F5);
	connect(m_stopAction, &QAction::triggered, this, &SimWidget::stopSimulation);

	m_allowLanComm = new QAction{QIcon(":/Images/Images/SimAllowRegData.svg"), tr("Allow LogicModules' Application Data transmittion to AppDataSrv"), this};
	m_allowLanComm->setCheckable(true);
	m_allowLanComm->setChecked(m_simulator->software().enabled());
	connect(m_allowLanComm, &QAction::toggled, this, &SimWidget::allowLanCommToggled);

	m_profilesComboBox = new QComboBox{};
	m_profilesComboBox->setMinimumContentsLength(15);
	connect(m_profilesComboBox, &QComboBox::currentTextChanged, this, &SimWidget::profileComboTextChanged);

	m_trendsAction = new QAction{QIcon(":/Images/Images/SimTrends.svg"), tr("Trends"), this};
	m_trendsAction->setEnabled(true);
	m_trendsAction->setData(QVariant("IAmIndependentTrend"));			// This is required to find this action in MonitorToolBar for drag and drop
	connect(m_trendsAction, &QAction::triggered, this, &SimWidget::showTrends);

	m_findSignalAction = new QAction{QIcon(":/Images/Images/SimFindSignal.svg"), tr("Find Signal"), this};
	m_findSignalAction->setEnabled(true);
	m_findSignalAction->setShortcut(QKeySequence::Find);
	connect(m_findSignalAction, &QAction::triggered, this, &SimWidget::showFindSignal);


	m_schemaListAction = new QAction{QIcon(":/Images/Images/SchemaList.svg"), tr("Show All Schemas"), this};
	m_schemaListAction->setEnabled(true);
	connect(m_schemaListAction, &QAction::triggered, this, &SimWidget::openAppSchemasTabPage);

	m_snapshotAction = new QAction{QIcon(":/Images/Images/SimSnapshot.svg"), tr("Signals Snapshot"), this};
	m_snapshotAction->setEnabled(true);
	connect(m_snapshotAction, &QAction::triggered, this, &SimWidget::showSnapshot);

	// --
	//
	m_timeIndicator = new QLabel;

#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#else
		QFont f = QFont("Courier");
#endif
	m_timeIndicator->setFont(f);
	updateTimeIndicator(Sim::ControlStatus{});

	// --
	//
	m_toolBar->addAction(m_openProjectAction);
	m_toolBar->addAction(m_closeProjectAction);
	m_toolBar->addAction(m_refreshProjectAction);
	m_toolBar->addAction(m_addWindowAction);

	m_toolBar->addSeparator();
	m_toolBar->addWidget(m_simulationTimeEdit);
	m_toolBar->addWidget(m_speedComboBox);
	m_toolBar->addAction(m_runAction);
	m_toolBar->addAction(m_pauseAction);
	m_toolBar->addAction(m_stopAction);

	m_toolBar->addSeparator();
	m_toolBar->addWidget(m_timeIndicator);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_allowLanComm);
	m_toolBar->addWidget(m_profilesComboBox);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_schemaListAction);
	m_toolBar->addAction(m_snapshotAction);
	m_toolBar->addAction(m_findSignalAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_trendsAction);

	// --
	//
	QWidget* trendsActionWidget = m_toolBar->widgetForAction(m_trendsAction);
	assert(trendsActionWidget);

	trendsActionWidget->setAcceptDrops(true);

	return;
}

void SimWidget::createDocks()
{
	setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
	setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::BottomDockWidgetArea);
	setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);

	// Project dock
	//
	QDockWidget* projectDock = new QDockWidget{"SimProjectBuild", this};
	projectDock->setObjectName(projectDock->windowTitle());
	projectDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	projectDock->setTitleBarWidget(new QWidget{});		// Hides title bar

	m_projectWidget = new SimProjectWidget{m_simulator.get()};
	projectDock->setWidget(m_projectWidget);

	addDockWidget(Qt::LeftDockWidgetArea, projectDock);

	// Quick Watch dock
	//
//	QDockWidget* watchDock = new QDockWidget("Watch", this);
//	watchDock->setObjectName(watchDock->windowTitle());
//	watchDock->setWidget(new QWidget());		// Dummy for now

//	addDockWidget(Qt::RightDockWidgetArea, watchDock);

	// Overriden Signals dock
	//
	m_overridePaneDock = new QDockWidget{"Overrides", this};
	m_overridePaneDock->setObjectName("SimOverridenSignals");
	m_overridePaneDock->setWidget(new SimOverridePane{m_simulator.get(), dbc(), m_overridePaneDock});
	m_overridePaneDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

	addDockWidget(Qt::BottomDockWidgetArea, m_overridePaneDock);

	// OutputLog dock
	//
	if (m_slaveWindow == false)
	{
		m_outputPaneDock = new QDockWidget{"Output", this};
		m_outputPaneDock->setObjectName("SimOutputWidget");

		m_outputWidget = new SimOutputWidget{m_outputPaneDock};

		m_outputPaneDock->setWidget(m_outputWidget);
		m_outputPaneDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_outputPaneDock);
	}

	// Memory Widget - at least one defaullt memory widget
	//
//	QDockWidget* m1 = createMemoryDock("Memory 1");
//	QDockWidget* m2 = createMemoryDock("Memory 2");
//	QDockWidget* m3 = createMemoryDock("Memory 3");

//	if (outputDock != nullptr)
//	{
//		tabifyDockWidget(outputDock, m1);
//	}
//	tabifyDockWidget(m1, m2);
//	tabifyDockWidget(m2, m3);

	// --
	//

	return;
}

QDockWidget* SimWidget::createMemoryDock(QString /*caption*/)
{
	return nullptr;

	// -----------------
//static Sim::Ram ram;
//	ram.addMemoryArea(Sim::RamAccess::Read, 8192 * 0, 8192, "Input Module 1");
//	ram.addMemoryArea(Sim::RamAccess::Read, 8192 * 1, 8192, "Input Module 2");
//	ram.addMemoryArea(Sim::RamAccess::Read, 8192 * 2, 8192, "Input Module 3");

//	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 0, 8192, "Output Module 1");
//	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 1, 8192, "Output Module 2");
//	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 2, 8192, "Output Module 3");
//	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 3 + 4, 224, "Not event");
	//-----------------

//	QDockWidget* dock = new QDockWidget(caption, this, 0);
//	dock->setObjectName("SimDock-" + caption);
//	dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

//	SimMemoryWidget* memoryWidget = new SimMemoryWidget(ram);
//	dock->setWidget(memoryWidget);

//	addDockWidget(Qt::BottomDockWidgetArea, dock);

//	return dock;
}

void SimWidget::showEvent(QShowEvent* e)
{
	QMainWindow::showEvent(e);
	e->ignore();

	if (m_showEventFired == false)
	{
		// Restore docks states only after show event, otherwise the _floated_ docks will be behind main window
		//
		if (m_slaveWindow == false)
		{
			QVariant v = QSettings().value("SimWidget/state");
			if (v.isValid() == true)
			{
				restoreState(v.toByteArray());

				QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
				for (QDockWidget* dw : dockWidgets)
				{
					restoreDockWidget(dw);
					dw->setVisible(true);
				}
			}

			m_toolBar->setVisible(true);
		}
	}

	m_showEventFired = true;

	return;
}

void SimWidget::aboutToQuit()
{
	if (m_slaveWindow == false)
	{
		stopSimulation(true);
	}

	if (m_slaveWindow == false && m_showEventFired == true)
	{
		QSettings().setValue("SimWidget/state", saveState());
		qDebug() << "SimWidget::aboutToQuit(): saveState()";
	}

	return;
}

void SimWidget::controlStateChanged(Sim::SimControlState /*state*/)
{
	updateActions();
}

void SimWidget::updateTimeIndicator(Sim::ControlStatus state)
{
using namespace std::chrono;

	Q_ASSERT(m_timeIndicator);

	milliseconds durration = duration_cast<milliseconds>(state.m_duration);

	qint64 days = durration.count() / 1_day;
	qint64 hours = (durration.count() % 1_day)  / 1_hour;
	qint64 minutes = (durration.count() % 1_hour)  / 1_min;
	qint64 seconds = (durration.count() % 1_min)  / 1_sec;
	qint64 millisecond = durration.count() % 1_sec;

	auto ms = duration_cast<milliseconds>(state.m_currentTime);
	QDateTime utcOffset = QDateTime::currentDateTime();
	TimeStamp plantTime{ms.count() + utcOffset.offsetFromUtc() * 1000};

	QDateTime currentTime = plantTime.toDateTime();

	if (currentTime.date().year() == 1970)
	{
		currentTime = QDateTime::currentDateTime();
	}

	QLocale locale;

#if 1
	// IF UNCOMMENTING THIS CODE
	// and if you want to show milliseconds,
	// THEN do not forget to send message more frequently
	// in Sim::Control::processRun emit statusUpdate(ControlStatus{cd});
	//
	//        0d 00:20:03.580
	//05/17/2020 15:18:59.335

	QString dateText = QString("%6 %7")
					   .arg(locale.toString(currentTime.date(),  QLocale::FormatType::ShortFormat))
					   .arg(currentTime.toString(QStringLiteral("hh:mm:ss.zzz")));

	QString text = tr("%1d %2:%3:%4.%5\n%6")
					.arg(days, static_cast<int>(dateText.size() - 14), 10, QChar(' '))
					.arg(hours, 2, 10, QChar('0'))
					.arg(minutes, 2, 10, QChar('0'))
					.arg(seconds, 2, 10, QChar('0'))
					.arg(millisecond, 3, 10, QChar('0'))
					.arg(dateText);
#else
	//        0d 00:20:03
	//05/17/2020 15:18:59

	QString dateText = QString("%6 %7")
					   .arg(locale.toString(currentTime.date(),  QLocale::FormatType::ShortFormat))
					   .arg(currentTime.toString(QStringLiteral("hh:mm:ss")));

	QString text = tr("%1d %2:%3:%4\n%6")
					.arg(days, static_cast<int>(dateText.size()) - 10, 10, QChar(' '))
					.arg(hours, 2, 10, QChar('0'))
					.arg(minutes, 2, 10, QChar('0'))
					.arg(seconds, 2, 10, QChar('0'))
					.arg(dateText);
#endif

	m_timeIndicator->setText(text);

	return;
}

void SimWidget::updateActions()
{
	bool projectIsLoaded = m_simulator->isLoaded();

	{
		m_openProjectAction->setEnabled(true);
		m_closeProjectAction->setEnabled(projectIsLoaded);

		QString project = db()->currentProject().projectName().toLower();
		QString lastPath = QSettings().value("SimulatorWidget/ProjectLastPath/" + project).toString();
		bool lastPathExists = QDir(lastPath).exists() == true && lastPath.isEmpty() == false;

		m_refreshProjectAction->setEnabled(projectIsLoaded || lastPathExists);
		m_addWindowAction->setEnabled(projectIsLoaded);
	}

	// Run, Pause, Stop
	//
	{
		m_simulationTimeEdit->setEnabled((m_simulator->isStopped() == true || m_simulator->isPaused()) && projectIsLoaded == true);
		m_speedComboBox->setEnabled(projectIsLoaded);

		m_runAction->setEnabled((m_simulator->isStopped() == true || m_simulator->isPaused()) && projectIsLoaded == true);
		m_pauseAction->setEnabled(m_simulator->isRunning() == true && projectIsLoaded == true);
		m_stopAction->setEnabled(m_simulator->isStopped() == false  && projectIsLoaded == true);

		m_timeIndicator->setEnabled(m_simulator->isStopped() == false  && projectIsLoaded == true);
	}

	// Update profile combo box
	//
	{
		m_profilesComboBox->setEnabled(m_simulator->isStopped() == true && projectIsLoaded == true);

		bool hasLastSelected = false;
		QString lastSelectedProfile;

		if (projectIsLoaded == true)
		{
			lastSelectedProfile = QSettings().value(QString("SimWidget/lastSelectedProfile_%1").
													arg(m_simulator->projectName())).toString();
		}

		m_profilesComboBox->blockSignals(true);
		m_profilesComboBox->clear();

		if (projectIsLoaded == true)
		{
			QStringList profiles;

			profiles += m_simulator->profiles().profiles();

			for (QString p : profiles)
			{
				m_profilesComboBox->addItem(p, p);

				if (p.compare(lastSelectedProfile, Qt::CaseInsensitive) == 0)
				{
					hasLastSelected = true;
				}
			}
		}

		m_profilesComboBox->blockSignals(false);

		if (projectIsLoaded == true && hasLastSelected == true)
		{
			m_profilesComboBox->setCurrentText(lastSelectedProfile);
		}
	}

	return;
}

void SimWidget::projectOpened(DbProject)
{
	emit needCloseChildWindows();
	emit needUpdateActions();
}

void SimWidget::projectClosed()
{
	emit needCloseChildWindows();
}

void SimWidget::openBuild()
{
	m_simulator->control().stop();

	if (dbc()->isProjectOpened() == true)
	{
		QSettings settings;

		QString project = db()->currentProject().projectName().toLower();
		QString lastPath = settings.value("SimulatorWidget/ProjectLastPath/" + project).toString();

		SimSelectBuildDialog d(project, lastPath, this);
		int result = d.exec();

		if (result == QDialog::Accepted)
		{
			lastPath = d.resultBuildPath();

			if (bool ok = loadBuild(lastPath);
				ok == true)
			{
				settings.setValue("SimulatorWidget/ProjectLastPath/" + project, lastPath);
			}
		}
	}
	else
	{
		QString lastPath = QSettings{}.value("SimulatorWidget/ProjectLastPath/NoOpenProject").toString();

		lastPath = QFileDialog::getExistingDirectory(this, "Open Build", lastPath);

		if (lastPath.isEmpty() == false)
		{
			loadBuild(lastPath);

			QSettings{}.setValue("SimulatorWidget/ProjectLastPath/NoOpenProject", lastPath);
		}
	}

	emit needUpdateActions();

	return;
}

void SimWidget::closeBuild()
{
	m_simulator->control().stop();

	m_simulator->clear();
	emit needUpdateActions();

	SimBasePage::deleteAllPages();

	if (m_outputWidget != nullptr)
	{
		// m_outputWidget exist only for the main simualtor tab page.
		//
		m_outputWidget->clear();
	}

	return;
}

void SimWidget::refreshBuild()
{
	m_simulator->control().stop();

	if (m_outputWidget != nullptr)	// Detached window does not have OutputWidget
	{
		m_outputWidget->clear();
	}

	QString buildPath = m_simulator->buildPath();
	if (buildPath.isEmpty() == true)
	{
		QString project = db()->currentProject().projectName().toLower();
		buildPath = QSettings().value("SimulatorWidget/ProjectLastPath/" + project).toString();
	}

	if (buildPath.isEmpty() == false)
	{
		loadBuild(buildPath);
	}

	emit needUpdateActions();
	return;
}

void SimWidget::runSimulation()
{
	if (m_simulator->isLoaded() == false)
	{
		qDebug() << "SimWidget::runSimulation(): Project is not loaded";
		m_simulator->log().writeError("Cannot start simulation, project is not loaded.");
		return;
	}

	if (m_simulator->isRunning() == true)
	{
		qDebug() << "SimWidget::runSimulation(): Simulation is already running";
		return;
	}

	// Trim all trends only if it is the start of the new simualtion.
	//
	if (m_simulator->isStopped() == true)
	{
		TimeStamp currentTime{QDateTime::currentDateTime()};
		auto trimFunc = [currentTime](SimTrendsWidget* simTrendsWidget)
		{
// Choose what to do, trim or clear.
//
#if 1
			simTrendsWidget->trimTrendData(currentTime);
			simTrendsWidget->addNonValidPoints();
#else
			simTrendsWidget->clear();
#endif
		};

		SimTrends::applyForAll(trimFunc);
	}

	// Get simulation time
	//
	std::chrono::microseconds duration{-1};

	if (QString simTimeText = m_simulationTimeEdit->text();
		simTimeText.isEmpty() == false)
	{
		QStringList splitted = simTimeText.split(m_simulationTimeLocale.decimalPoint());
		QString secondsText;
		QString millisecondsText;

		if (splitted.size() >= 1)
		{
			secondsText = splitted[0].remove(m_simulationTimeLocale.groupSeparator());
		}

		if (splitted.size() == 2)
		{
			millisecondsText = splitted[1];
		}

		if (secondsText.isEmpty() == false)
		{
			bool ok;
			auto s = secondsText.toULongLong(&ok, 10);

			if (ok == true)
			{
				duration = std::chrono::seconds{s};
			}
		}

		if (millisecondsText.isEmpty() == false)
		{
			uint64_t order = 100;
			uint64_t ms = 0;

			for (qsizetype i = 0; i < millisecondsText.size(); i++)
			{
				QChar ch = millisecondsText[i];
				Q_ASSERT(ch.isDigit());

				ms += ch.digitValue() * order;
				order /= 10;
			}

			if (duration.count() < 0)
			{
				duration = std::chrono::microseconds{0};
			}

			duration += std::chrono::milliseconds{ms};
		}
	}

	if (duration.count() == 0)
	{
		duration = std::chrono::microseconds{1};	// It will run one work cycle
	}

	// Set profile to simulator
	//
	Q_ASSERT(m_profilesComboBox);

	QString selectedProfile = m_profilesComboBox->currentText();

	if (bool ok = m_simulator->setCurrentProfile(selectedProfile);
		ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Profile %1 not found").arg(selectedProfile));
		return;
	}

	// --
	//
	Sim::Control& mutableControl = m_simulator->control();

	if (m_simulator->isPaused() == true)
	{
		mutableControl.startSimulation(duration);
	}
	else
	{
		m_simulator->appSignalManager().resetRam();		// It prevents from short show of previouse run results

		// Star simulation for all project
		//
		mutableControl.reset();

		// Get all modules to simulation
		//
		QStringList equipmentIds;
		auto lms = m_simulator->logicModules();

		for (const auto& lm : lms)
		{
			equipmentIds << lm->equipmentId();
		}

		if (equipmentIds.isEmpty() == true)
		{
			m_simulator->log().writeWarning(tr("Nothing to simulate, no LogicModules are found."));
			// Nothing to simulate
			//
			return;
		}

		// Start simulation
		//
		mutableControl.setRunList(equipmentIds);
		mutableControl.startSimulation(duration);
	}

	return;
}

void SimWidget::pauseSimulation()
{
	qDebug() << "SimWidget::pauseSimulation()";

	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	if (m_simulator->isRunning() == false)
	{
		return;
	}

	Sim::Control& control = m_simulator->control();
	control.pause();

	return;
}

void SimWidget::stopSimulation(bool stopSimulationThread)
{
	qDebug() << "SimWidget::stopSimulation()";

	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	if (m_simulator->isRunning() == false &&
		m_simulator->isPaused() == false)
	{
		return;
	}

	Sim::Control& control = m_simulator->control();
	control.stop();

	if (stopSimulationThread == true)
	{
		control.stopThread();
	}

	return;
}

void SimWidget::allowLanCommToggled(bool state)
{
	m_simulator->software().setEnabled(state);
	return;
}

void SimWidget::profileComboTextChanged(QString text)
{
	if (m_simulator->isLoaded() == true && text.isEmpty() == false)
	{
		QSettings{}.setValue(QString("SimWidget/lastSelectedProfile_%1").arg(m_simulator->projectName()), text);
	}

	return;
}

void SimWidget::showSnapshot()
{
	// 1. Use this->m_appSignalController for getting signal list and state
	// 2. this->m_simulator (Sim::Simulator) has signal 'projectUpdated' and function 'isLoaded()' use these to
	//	  update signal list
	// 3. You can pass and store 'this->m_appSignalController' and 'this->m_simulator.get()'  to your function
	//    it is guarantee will not be deleted
	//

	SimDialogSignalSnapshot::showDialog(m_simulator.get(), m_appSignalController, QString(), this);

	return;
}

void SimWidget::showFindSignal()
{
	// 1. Use this->m_appSignalController for getting signal list and state
	// 2. this->m_simulator (Sim::Simulator) has signal 'projectUpdated' and function 'isLoaded()' use these to
	//	  update signal list
	// 3. You can pass and store 'this->m_appSignalController' and 'this->m_simulator.get()'  to your function
	//    it is guarantee will not be deleted
	//
	DialogSignalSearch* dsi = new DialogSignalSearch(this, &m_appSignalController->appSignalManager());

	connect(m_simulator.get(), &SimIdeSimulator::projectUpdated, dsi, &DialogSignalSearch::signalsUpdated);

	connect(dsi, &DialogSignalSearch::signalContextMenu, this, &SimWidget::signalContextMenu);
	connect(dsi, &DialogSignalSearch::signalInfo, this, &SimWidget::signalInfo);

	connect(this, &SimWidget::needCloseChildWindows, dsi, &QDialog::accept);

	dsi->show();

	return;
}

void SimWidget::showTrends()
{
	// Get Trends list
	//
	std::vector<SimTrendsWidget*> trends = SimTrends::getTrendsList();

	// Choose trend
	//
	SimTrendsWidget* trendToActivate = nullptr;

	if (trends.empty() == true)
	{
		trendToActivate = nullptr;	// if trendToActivate is nullptr, then create a new trend.
	}
	else
	{
		QMenu menu;

		QAction* newTrendAction = menu.addAction("New Trend...");
		newTrendAction->setData(QVariant::fromValue<int>(-1));		// Data -1 means, create new trend widget

		menu.addSeparator();

		for (size_t i = 0; i < trends.size(); i++)
		{
			QAction* a = menu.addAction(trends[i]->windowTitle());
			Q_ASSERT(a);

			a->setData(QVariant::fromValue<int>(static_cast<int>(i)));		// Data is index in trend vector
		}

		QAction* triggeredAction = menu.exec(QCursor::pos());
		if (triggeredAction == nullptr)
		{
			return;
		}

		QVariant data = triggeredAction->data();

		bool ok = false;
		int trendIndex = data.toInt(&ok);

		if (trendIndex == -1)
		{
			trendToActivate = nullptr;	// if trendToActivate is nullptr, then create a new trend.
		}
		else
		{
			if (ok == false || trendIndex < 0 || trendIndex >= static_cast<int>(trends.size()))
			{
				Q_ASSERT(ok == true);
				Q_ASSERT(trendIndex >= 0 && trendIndex < static_cast<int>(trends.size()));
				return;
			}

			trendToActivate = trends.at(trendIndex);
		}	
	}

	// Start new trend or activate chosen one
	//
	if (trendToActivate == nullptr)
	{
		std::vector<AppSignalParam> appSignals;
		SimTrends::startTrendApp(m_simulator, appSignals, this);
	}
	else
	{
		SimTrends::activateTrendWindow(trendToActivate);
	}

	return;
}

bool SimWidget::loadBuild(QString buildPath)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	bool ok = m_simulator->load(buildPath);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open project for simulation. For details see Output window."));
	}

	return ok;
}

void SimWidget::addNewWindow()
{
	qDebug() << "SimulatorWidget::addNewWindow()";

	SimWidget* widget = new SimWidget{m_ideLogFile, m_simulator, db(), this->parentWidget(), Qt::Window, true};
	widget->setWindowTitle(tr("u7 Simulator"));

	widget->show();

	return;
}

void SimWidget::openLogicModuleTabPage(QString lmEquipmentId)
{
	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Check if such SimulatorControlPage already exists
	//
	SimLogicModulePage* cp = SimBasePage::logicModulePage(lmEquipmentId, m_tabWidget);

	if (cp != nullptr)
	{
		int tabIndex = m_tabWidget->indexOf(cp);
		if (tabIndex != -1)
		{
			m_tabWidget->setCurrentIndex(tabIndex);
		}
		else
		{
			cp->show();
			cp->activateWindow();
		}

		return;
	}

	// Create new SimLogicModulePage
	//
	auto logicModule = m_simulator->logicModule(lmEquipmentId);
	if (logicModule == nullptr)
	{
		assert(logicModule);
		return;
	}
	assert(lmEquipmentId == logicModule->equipmentId());

	SimLogicModulePage* controlPage = new SimLogicModulePage{m_simulator.get(), m_appSignalController, lmEquipmentId, m_tabWidget};

	int tabIndex = m_tabWidget->addTab(controlPage, lmEquipmentId);
	m_tabWidget->setTabIcon(tabIndex, QIcon{QPixmap{":/Images/Images/SimLogicModuleIcon.svg"}});

	m_tabWidget->setCurrentIndex(tabIndex);

	connect(controlPage, &SimLogicModulePage::openSchemaRequest, this, &SimWidget::openSchemaTabPage);
	connect(controlPage, &SimLogicModulePage::openCodePageRequest, this, &SimWidget::openCodeTabPage);

	return;
}

void SimWidget::openCodeTabPage(QString lmEquipmentId)
{
	auto lm = m_simulator->logicModule(lmEquipmentId);
	if (lm == nullptr)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot find LogicModule %1").arg(lmEquipmentId));
		return;
	}

	SimCodePage* page = new SimCodePage{m_simulator.get(), lmEquipmentId, m_tabWidget};

	int tabIndex = m_tabWidget->addTab(page, lmEquipmentId);
	m_tabWidget->setCurrentIndex(tabIndex);

	return;
}

void SimWidget::openConnectionTabPage(QString connectionId)
{
	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Check if such SimulatorControlPage already exists
	//
	SimConnectionPage* cp = SimBasePage::connectionPage(connectionId, m_tabWidget);

	if (cp != nullptr)
	{
		int tabIndex = m_tabWidget->indexOf(cp);
		if (tabIndex != -1)
		{
			m_tabWidget->setCurrentIndex(tabIndex);
		}
		else
		{
			cp->show();
			cp->activateWindow();
		}

		return;
	}

	// Create new SimConnectionPage
	//
	SimConnectionPage* page = new SimConnectionPage{m_simulator.get(), connectionId, m_tabWidget};

	int tabIndex = m_tabWidget->addTab(page, connectionId);
	m_tabWidget->setTabIcon(tabIndex, QIcon{QPixmap{":/Images/Images/SimConnectionIcon.svg"}});

	m_tabWidget->setCurrentIndex(tabIndex);

	return;
}

void SimWidget::openAppSchemasTabPage()
{
	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Check if such SimulatorControlPage already exists
	//
	SimSelectSchemaPage* cp = SimBasePage::selectSchemaPage(m_tabWidget);

	if (cp != nullptr)
	{
		int tabIndex = m_tabWidget->indexOf(cp);
		if (tabIndex != -1)
		{
			m_tabWidget->setCurrentIndex(tabIndex);
		}
		else
		{
			cp->show();
			cp->activateWindow();
		}

		return;
	}

	// Create new SimConnectionPage
	//
	SimSelectSchemaPage* page = new SimSelectSchemaPage{m_simulator.get(), m_tabWidget};

	int tabIndex = m_tabWidget->addTab(page, tr("AppLogic Schemas"));
	m_tabWidget->setTabIcon(tabIndex, QIcon{QPixmap{":/Images/Images/SimAppLogicSchemas.svg"}});
	m_tabWidget->setTabToolTip(0, tr("Application Logic Schemas\n"
									 "[CTRL + `]"));

	m_tabWidget->setCurrentIndex(tabIndex);

	connect(page, &SimSelectSchemaPage::openSchemaTabPage, this, &SimWidget::openSchemaTabPage);

	return;
}

void SimWidget::tabCloseRequest(int index)
{
	QByteArray state = saveState();

	QWidget* w = m_tabWidget->widget(index);
	assert(w);

	delete w;

	restoreState(state);
	return;
}

void SimWidget::tabCurrentChanged(int index)
{
	// Show/hide close button for inactive tab bar
	//
	QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, m_tabWidget->tabBar());

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		QWidget* w = m_tabWidget->tabBar()->tabButton(i, closeSide);

		if (w != nullptr)
		{
			w->setVisible(i == index);
		}
	}

	return;
}

void SimWidget::tabBarContextMenuRequest(const QPoint& pos)
{
	assert(m_tabWidget);
	QTabBar* tabBar = m_tabWidget->tabBar();

	int tabIndex = tabBar->tabAt(pos);
	if (tabIndex == -1)
	{
		return;
	}

	SimBasePage* page = qobject_cast<SimBasePage*>(m_tabWidget->widget(tabIndex));
	if (page == nullptr)
	{
		assert(page);
		return;
	}

	return;
}

SimSchemaManager& SimWidget::schemaManager()
{
	return m_schemaManager;
}

const SimSchemaManager& SimWidget::schemaManager() const
{
	return m_schemaManager;
}

//
//	SimulatorToolBar
//
SimToolBar::SimToolBar(const QString& title, QWidget* parent) :
	QToolBar(title, parent)
{
	setMovable(false);
	setAcceptDrops(true);

	setObjectName("SimToolBar");

	setStyleSheet("QToolButton { padding-top: 3px; padding-bottom: 3px; padding-left: 3px; padding-right: 3px;}");
	setIconSize(iconSize() * 0.9);

	toggleViewAction()->setDisabled(true);

	return;
}

SimToolBar::~SimToolBar()
{
}

void SimToolBar::dragEnterEvent(QDragEnterEvent* event)
{
	// Find Trend action
	//
	QWidget* trendActionWidget = nullptr;

	QList<QAction*> allActions = actions();
	for (QAction* a : allActions)
	{
		QVariant d = a->data();

		if (d.isValid() &&
			d.typeId() == QMetaType::QString)
		{
			if (d.toString() == QLatin1String("IAmIndependentTrend"))
			{
				trendActionWidget = widgetForAction(a);
				trendActionWidget->setAcceptDrops(true);
			}
		}
	}

	if (trendActionWidget != nullptr &&
		trendActionWidget->geometry().contains(event->position().toPoint()) &&
		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void SimToolBar::dropEvent(QDropEvent* event)
{
	// Find Trend action
	//
	QWidget* trendActionWidget = nullptr;
	QAction* trendAction = nullptr;


	QList<QAction*> allActions = actions();

	for (QAction* a : allActions)
	{
		QVariant d = a->data();
		if (d.isValid() &&
			d.typeId() == QMetaType::QString)
		{
			if (d.toString() == QLatin1String("IAmIndependentTrend"))
			{
				trendAction = a;
				trendActionWidget = widgetForAction(trendAction);
			}
		}
	}

	if (trendAction != nullptr &&
		trendActionWidget != nullptr &&
		trendActionWidget->geometry().contains(event->position().toPoint()) &&
		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		// Lets assume parent isMonitorMainWindow
		//
		SimWidget* sw = dynamic_cast<SimWidget*>(this->parent());
		if (sw == nullptr)
		{
			Q_ASSERT(sw);
			return;
		}

		// Load data from drag and drop
		//
		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

		::Proto::AppSignalSet protoSetMessage;
		bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			event->acceptProposedAction();
			return;
		}

		std::vector<AppSignalParam> appSignals;
		appSignals.reserve(protoSetMessage.appsignal_size());

		// Parse data
		//
		for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
		{
			const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

			AppSignalParam appSignalParam;
			ok = appSignalParam.load(appSignalMessage);

			if (ok == true)
			{
				appSignals.push_back(appSignalParam);
			}
		}

		if (appSignals.empty() == false)
		{
			sw->startTrends(appSignals);
		}
	}

	return;
}
