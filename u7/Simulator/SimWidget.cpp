#include "SimWidget.h"
#include "Settings.h"
#include "SimProjectWidget.h"
#include "SimMemoryWidget.h"
#include "SimOutputWidget.h"
#include "SimSelectBuildDialog.h"
#include "SimControlPage.h"
#include "SimSchemaPage.h"
#include "SimCodePage.h"
#include "../SimulatorTabPage.h"


SimWidget::SimWidget(std::shared_ptr<SimIdeSimulator> simulator,
								 DbController* db,
								 QWidget* parent /*= nullptr*/,
								 Qt::WindowType windowType /*= Qt::Window*/,
								 bool slaveWindow /*= false*/)
	: QMainWindow(parent),
	  HasDbController(db),
	  m_slaveWindow(slaveWindow),
	  m_simulator(simulator)
{
	if (m_simulator == nullptr)
	{
		m_simulator = std::make_shared<SimIdeSimulator>();
	}

	// --
	//
	m_appSignalController = new VFrame30::AppSignalController(&m_simulator->appSignalManager(), this);
	m_tuningController = new VFrame30::TuningController(&m_simulator->tuningSignalManager(), &m_tuningTcpClient, this);

	// --
	//
	setWindowFlags(windowType);
	setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

	m_tabWidget = new QTabWidget;
	m_tabWidget->setTabsClosable(true);
	m_tabWidget->setMovable(true);
	m_tabWidget->setDocumentMode(true);
	m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

	setCentralWidget(m_tabWidget);
	centralWidget()->setBackgroundRole(QPalette::Dark);
	centralWidget()->setAutoFillBackground(true);

	QVBoxLayout* layout = new QVBoxLayout;
	centralWidget()->setLayout(layout);

	createToolBar();
	createDocks();

	updateActions();

	// --
	//
	connect(db, &DbController::projectOpened, this, &SimWidget::projectOpened);
	connect(db, &DbController::projectClosed, this, &SimWidget::closeBuild);

	connect(m_simulator.get(), &Sim::Simulator::projectUpdated, this, &SimWidget::updateActions);
	connect(&(m_simulator->control()), &Sim::Control::stateChanged, this, &SimWidget::updateActions);

	connect(m_projectWidget, &SimProjectWidget::signal_openControlTabPage, this, &SimWidget::openControlTabPage);
	connect(m_projectWidget, &SimProjectWidget::signal_openCodeTabPage, this, &SimWidget::openCodeTabPage);

	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &SimWidget::tabCloseRequest);
	connect(m_tabWidget->tabBar(), &QTabWidget::customContextMenuRequested, this, &SimWidget::tabBarContextMenuRequest);

	connect(this, &SimWidget::needUpdateActions, this, &SimWidget::updateActions);

	return;
}

SimWidget::~SimWidget()
{
	if (m_slaveWindow == false)
	{
		theSettings.m_simWigetState = saveState();
		theSettings.writeUserScope();
	}
}

void SimWidget::createToolBar()
{
	m_toolBar = new SimToolBar("ToolBar");
	addToolBar(m_toolBar);

	//m_toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	m_openProjectAction = new QAction(QIcon(":/Images/Images/SimOpen.svg"), tr("Open Build"), this);
	m_openProjectAction->setShortcut(QKeySequence::Open);
	connect(m_openProjectAction, &QAction::triggered, this, &SimWidget::openBuild);


	m_closeProjectAction = new QAction(QIcon(":/Images/Images/SimClose.svg"), tr("Close"), this);
	m_closeProjectAction->setShortcut(QKeySequence::Close);
	connect(m_closeProjectAction, &QAction::triggered, this, &SimWidget::closeBuild);

	m_refreshProjectAction = new QAction(QIcon(":/Images/Images/SimRefresh.svg"), tr("Refresh"), this);
	m_refreshProjectAction->setShortcut(QKeySequence::Refresh);
	connect(m_refreshProjectAction, &QAction::triggered, this, &SimWidget::refreshBuild);

	m_addWindowAction = new QAction(QIcon(":/Images/Images/SimAddWindow.svg"), tr("Add Window"), this);
	m_addWindowAction->setShortcut(QKeySequence::New);
	connect(m_addWindowAction, &QAction::triggered, this, &SimWidget::addNewWindow);
	m_toolBar->addAction(m_addWindowAction);

	m_runAction = new QAction(QIcon(":/Images/Images/SimRun.svg"), tr("Run simulation for complete project"), this);
	QList<QKeySequence> runsKeys;
	runsKeys << Qt::CTRL + Qt::Key_R;
	runsKeys << Qt::CTRL + Qt::Key_F5;
	m_runAction->setShortcuts(runsKeys);
	connect(m_runAction, &QAction::triggered, this, &SimWidget::runSimulation);

	m_pauseAction = new QAction(QIcon(":/Images/Images/SimPause.svg"), tr("Pause current simulation"), this);
	connect(m_pauseAction, &QAction::triggered, this, &SimWidget::pauseSimulation);

	m_stopAction = new QAction(QIcon(":/Images/Images/SimStop.svg"), tr("Stop current simulation"), this);
	m_stopAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
	connect(m_stopAction, &QAction::triggered, this, &SimWidget::stopSimulation);

	// --
	//
	m_toolBar->addAction(m_openProjectAction);
	m_toolBar->addAction(m_closeProjectAction);
	m_toolBar->addAction(m_refreshProjectAction);
	m_toolBar->addAction(m_addWindowAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_runAction);
	m_toolBar->addAction(m_pauseAction);
	m_toolBar->addAction(m_stopAction);

	return;
}

void SimWidget::createDocks()
{
	setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
	setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::BottomDockWidgetArea);
	setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);

	// Project dock
	//
	QDockWidget* projectDock = new QDockWidget("Project/Build", this, 0);
	projectDock->setObjectName(projectDock->windowTitle());
	projectDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	projectDock->setTitleBarWidget(new QWidget());		// Hides title bar

	m_projectWidget = new SimProjectWidget(m_simulator.get());
	projectDock->setWidget(m_projectWidget);

	addDockWidget(Qt::LeftDockWidgetArea, projectDock);

	// Quick Watch dock
	//
	QDockWidget* watchDock = new QDockWidget("Watch", this, 0);
	watchDock->setObjectName(watchDock->windowTitle());
	watchDock->setWidget(new QWidget());		// Dummy for now

	addDockWidget(Qt::RightDockWidgetArea, watchDock);

	// OutputLog dock
	//
	QDockWidget* outputDock = nullptr;
	if (m_slaveWindow == false)
	{
		outputDock = new QDockWidget("Output", this, 0);
		outputDock->setObjectName(watchDock->windowTitle());
		outputDock->setWidget(new SimOutputWidget());
		outputDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, outputDock);
	}

	// Memory Widget - at least one defaullt memory widget
	//
	QDockWidget* m1 = createMemoryDock("Memory 1");
	QDockWidget* m2 = createMemoryDock("Memory 2");
	QDockWidget* m3 = createMemoryDock("Memory 3");

	if (outputDock != nullptr)
	{
		tabifyDockWidget(outputDock, m1);
	}
	tabifyDockWidget(m1, m2);
	tabifyDockWidget(m2, m3);

	// --
	//

	return;
}

QDockWidget* SimWidget::createMemoryDock(QString caption)
{
	// -----------------
static Sim::Ram ram;
	ram.addMemoryArea(Sim::RamAccess::Read, 8192 * 0, 8192, "Input Module 1");
	ram.addMemoryArea(Sim::RamAccess::Read, 8192 * 1, 8192, "Input Module 2");
	ram.addMemoryArea(Sim::RamAccess::Read, 8192 * 2, 8192, "Input Module 3");

	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 0, 8192, "Output Module 1");
	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 1, 8192, "Output Module 2");
	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 2, 8192, "Output Module 3");
	ram.addMemoryArea(Sim::RamAccess::Write, 8192 * 3 + 4, 224, "Not event");
	//-----------------

	QDockWidget* dock = new QDockWidget(caption, this, 0);
	dock->setObjectName("SimDock-" + caption);
	dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

	SimMemoryWidget* memoryWidget = new SimMemoryWidget(ram);
	dock->setWidget(memoryWidget);

	addDockWidget(Qt::BottomDockWidgetArea, dock);

	return dock;
}

void SimWidget::showEvent(QShowEvent* e)
{
	QMainWindow::showEvent(e);

static bool firstEvent = true;

	if (firstEvent == true)
	{
		// Restore docks states only after show event, otherwise the _floated_ docks will be behind main window
		//
		if (m_slaveWindow == false)
		{
			restoreState(theSettings.m_simWigetState);
		}

		firstEvent = false;
	}
	return;
}

void SimWidget::updateActions()
{
	bool projectIsLoaded = m_simulator->isLoaded();

	m_openProjectAction->setEnabled(true);
	m_closeProjectAction->setEnabled(projectIsLoaded);

	QString project = db()->currentProject().projectName().toLower();
	QString lastPath = QSettings().value("SimulatorWidget/ProjectLastPath/" + project).toString();
	bool lastPathExists = QDir(lastPath).exists() == true && lastPath.isEmpty() == false;

	m_refreshProjectAction->setEnabled(projectIsLoaded || lastPathExists);
	m_addWindowAction->setEnabled(projectIsLoaded);

	// Run, Pause, Stop
	//
	m_runAction->setEnabled((m_simulator->isStopped() == true || m_simulator->isPaused()) && projectIsLoaded == true);
	m_pauseAction->setEnabled(m_simulator->isRunning() == true && projectIsLoaded == true);
	m_stopAction->setEnabled(m_simulator->isStopped() == false  && projectIsLoaded == true);

	return;
}

void SimWidget::projectOpened(DbProject)
{
	emit needUpdateActions();
}

void SimWidget::openBuild()
{
	QSettings settings;
	SimSelectBuildDialog::BuildType buildType = static_cast<SimSelectBuildDialog::BuildType>(settings.value("SimulatorWidget/BuildType", 0).toInt());

	QString project = db()->currentProject().projectName().toLower();
	QString lastPath = settings.value("SimulatorWidget/ProjectLastPath/" + project).toString();

	SimSelectBuildDialog d(project,
								 buildType,
								 lastPath,
								 this);
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		settings.setValue("SimulatorWidget/BuildType", static_cast<int>(d.resultBuildType()));
		lastPath = d.resultBuildPath();

		bool ok = loadBuild(lastPath);

		if (ok == true)
		{
			settings.setValue("SimulatorWidget/ProjectLastPath/" + project, lastPath);
		}
	}

	emit needUpdateActions();
	return;
}

void SimWidget::closeBuild()
{
	m_simulator->clear();
	emit needUpdateActions();

	SimBasePage::deleteAllPages();

	return;
}

void SimWidget::refreshBuild()
{
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
	qDebug() << "SimWidget::runSimulation()";

	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	if (m_simulator->isRunning() == true)
	{
		return;
	}

	Sim::Control& control = m_simulator->control();

	if (m_simulator->isPaused() == true)
	{
		// Continue running what was simualted before
		//
		int cycles = control.leftTime();
		control.start(cycles);
	}
	else
	{
		// Star simulation for all project
		//
		control.reset();

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
			// Nothing to simulate
			//
			return;
		}

		// Start simulation
		//
		control.addToRunList(equipmentIds);
		control.start();
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

void SimWidget::stopSimulation()
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

	return;
}

bool SimWidget::loadBuild(QString buildPath)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	bool ok = m_simulator->load(buildPath);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open project for simultaion. For details see Output window."));
	}

	return ok;
}

void SimWidget::addNewWindow()
{
	qDebug() << "SimulatorWidget::addNewWindow()";

	SimWidget* widget = new SimWidget(m_simulator, db(), this->parentWidget(), Qt::Window, true);
	widget->setWindowTitle(tr("u7 Simulator"));

	widget->show();

	return;
}

void SimWidget::openControlTabPage(QString lmEquipmentId)
{
	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Check if such SimulatorControlPage already exists
	//
	SimControlPage* cp = SimBasePage::controlPage(lmEquipmentId, m_tabWidget);

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

	// Create new SimulatorControlPage
	//
	auto logicModule = m_simulator->logicModule(lmEquipmentId);
	if (logicModule == nullptr)
	{
		assert(logicModule);
		return;
	}
	assert(lmEquipmentId == logicModule->equipmentId());

	SimControlPage* controlPage = new SimControlPage(m_simulator.get(), lmEquipmentId, m_tabWidget);

	int tabIndex = m_tabWidget->addTab(controlPage, lmEquipmentId);
	m_tabWidget->setCurrentIndex(tabIndex);

	connect(controlPage, &SimControlPage::openSchemaRequest, this, &SimWidget::openLogicSchemaTabPage);
	connect(controlPage, &SimControlPage::openCodePageRequest, this, &SimWidget::openCodeTabPage);

	return;
}

void SimWidget::openLogicSchemaTabPage(QString schemaId)
{
	QString buildPath = QDir::fromNativeSeparators(m_simulator->buildPath());
	if (buildPath.isEmpty() == true)
	{
		return;
	}

	if (buildPath.endsWith('/') == false)
	{
		buildPath += "/";
	}

	// Load schema
	//
	QString fileName = buildPath + QLatin1String("LogicSchemas/") + schemaId + "." + ::AlFileExtension;

	openSchemaTabPage(fileName);
	return;
}

void SimWidget::openSchemaTabPage(QString fileName)
{
	std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(fileName);
	if (schema == nullptr)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open file %1").arg(fileName));
		return;
	}

	SimSchemaPage* page = new SimSchemaPage(schema,
											m_simulator.get(),
											&m_schemaManager,
											m_appSignalController,
											m_tuningController,
											m_tabWidget);

	int tabIndex = m_tabWidget->addTab(page, schema->schemaId());
	m_tabWidget->setCurrentIndex(tabIndex);

	return;
}

void SimWidget::openCodeTabPage(QString lmEquipmentId)
{
	auto lm = m_simulator->logicModule(lmEquipmentId);
	if (lm == nullptr)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot find LogicModuel %1").arg(lmEquipmentId));
		return;
	}

	SimCodePage* page = new SimCodePage(m_simulator.get(), lmEquipmentId, m_tabWidget);

	int tabIndex = m_tabWidget->addTab(page, lmEquipmentId);
	m_tabWidget->setCurrentIndex(tabIndex);

	return;
}

void SimWidget::tabCloseRequest(int index)
{
	QWidget* w = m_tabWidget->widget(index);
	assert(w);

	delete w;
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

	QMenu menu;
	menu.addAction(tr("Detach"));
	menu.addAction(tr("Close"));

	menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));

	return;
}


//
//	SimulatorToolBar
//
SimToolBar::SimToolBar(const QString& title, QWidget* parent) :
	QToolBar(title, parent)
{
	setMovable(false);
	setObjectName("SimulatorToolBar");

	setIconSize(iconSize() / 1.5);

	toggleViewAction()->setDisabled(true);

	return;
}

SimToolBar::~SimToolBar()
{
}
