#include "SimulatorWidget.h"
#include "Settings.h"
#include "SimulatorProjectWidget.h"
#include "SimulatorMemoryWidget.h"
#include "SimulatorOutputWidget.h"
#include "SimulatorSelectBuildDialog.h"
#include "SimulatorControlPage.h"
#include "../SimulatorTabPage.h"


SimulatorWidget::SimulatorWidget(std::shared_ptr<Sim::Simulator> simulator,
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
		m_simulator = std::make_shared<Sim::Simulator>();
	}

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
	connect(db, &DbController::projectOpened, this, &SimulatorWidget::projectOpened);
	connect(db, &DbController::projectClosed, this, &SimulatorWidget::closeBuild);

	connect(m_simulator.get(), &Sim::Simulator::projectUpdated, this, &SimulatorWidget::updateActions);

	connect(m_projectWidget, &SimulatorProjectWidget::signal_openControlTabPage, this, &SimulatorWidget::openControlTabPage);

	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &SimulatorWidget::tabCloseRequest);
	connect(m_tabWidget->tabBar(), &QTabWidget::customContextMenuRequested, this, &SimulatorWidget::tabBarContextMenuRequest);

	connect(this, &SimulatorWidget::needUpdateActions, this, &SimulatorWidget::updateActions);

	return;
}

SimulatorWidget::~SimulatorWidget()
{
	if (m_slaveWindow == false)
	{
		theSettings.m_simWigetState = saveState();
		theSettings.writeUserScope();
	}
}

void SimulatorWidget::createToolBar()
{
	m_toolBar = new SimulatorToolBar("ToolBar");
	addToolBar(m_toolBar);

	//m_toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	m_openProjectAction = new QAction(QIcon(":/Images/Images/SimOpen.svg"), tr("Open Build"), this);
	m_openProjectAction->setShortcut(QKeySequence::Open);
	connect(m_openProjectAction, &QAction::triggered, this, &SimulatorWidget::openBuild);
	m_toolBar->addAction(m_openProjectAction);


	m_closeProjectAction = new QAction(QIcon(":/Images/Images/SimClose.svg"), tr("Close"), this);
	m_closeProjectAction->setShortcut(QKeySequence::Close);
	connect(m_closeProjectAction, &QAction::triggered, this, &SimulatorWidget::closeBuild);
	m_toolBar->addAction(m_closeProjectAction);

	m_refreshProjectAction = new QAction(QIcon(":/Images/Images/SimRefresh.svg"), tr("Refresh"), this);
	m_refreshProjectAction->setShortcut(QKeySequence::Refresh);
	connect(m_refreshProjectAction, &QAction::triggered, this, &SimulatorWidget::refreshBuild);
	m_toolBar->addAction(m_refreshProjectAction);

	m_addWindowAction = new QAction(QIcon(":/Images/Images/SimAddWindow.svg"), tr("Add Window"), this);
	m_addWindowAction->setShortcut(QKeySequence::New);
	connect(m_addWindowAction, &QAction::triggered, this, &SimulatorWidget::addNewWindow);
	m_toolBar->addAction(m_addWindowAction);

	return;
}

void SimulatorWidget::createDocks()
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

	m_projectWidget = new SimulatorProjectWidget(m_simulator);
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
		outputDock->setWidget(new SimulatorOutputWidget());
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

QDockWidget* SimulatorWidget::createMemoryDock(QString caption)
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

	SimulatorMemoryWidget* memoryWidget = new SimulatorMemoryWidget(ram);
	dock->setWidget(memoryWidget);

	addDockWidget(Qt::BottomDockWidgetArea, dock);

	return dock;
}

void SimulatorWidget::showEvent(QShowEvent* e)
{
	QMainWindow::showEvent(e);

	// Restore docks states only after show event, otherwise the _floated_ docks will be behind main window
	//
	if (m_slaveWindow == false)
	{
		restoreState(theSettings.m_simWigetState);
	}

	return;
}

void SimulatorWidget::updateActions()
{
	bool projectIsLoaded = m_simulator->isLoaded();

	m_openProjectAction->setEnabled(true);
	m_closeProjectAction->setEnabled(projectIsLoaded);

	QString project = db()->currentProject().projectName().toLower();
	QString lastPath = QSettings().value("SimulatorWidget/ProjectLastPath/" + project).toString();
	bool lastPathExists = QDir(lastPath).exists() == true && lastPath.isEmpty() == false;

	m_refreshProjectAction->setEnabled(projectIsLoaded || lastPathExists);
	m_addWindowAction->setEnabled(projectIsLoaded);

	return;
}

void SimulatorWidget::projectOpened(DbProject)
{
	emit needUpdateActions();
}

void SimulatorWidget::openBuild()
{
	QSettings settings;
	SimulatorSelectBuildDialog::BuildType buildType = static_cast<SimulatorSelectBuildDialog::BuildType>(settings.value("SimulatorWidget/BuildType", 0).toInt());

	QString project = db()->currentProject().projectName().toLower();
	QString lastPath = settings.value("SimulatorWidget/ProjectLastPath/" + project).toString();

	SimulatorSelectBuildDialog d(project,
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

void SimulatorWidget::closeBuild()
{
	m_simulator->clear();
	emit needUpdateActions();

	SimulatorBasePage::deleteAllPages();

	return;
}

void SimulatorWidget::refreshBuild()
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

bool SimulatorWidget::loadBuild(QString buildPath)
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

void SimulatorWidget::addNewWindow()
{
	qDebug() << "SimulatorWidget::addNewWindow()";

	SimulatorWidget* widget = new SimulatorWidget(m_simulator, db(), this->parentWidget(), Qt::Window, true);
	widget->setWindowTitle(tr("u7 Simulator"));

	widget->show();

	return;
}

void SimulatorWidget::openControlTabPage(QString lmEquipmentId)
{
	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Check if such SimulatorControlPage already exists
	//
	SimulatorControlPage* cp = SimulatorBasePage::controlPage(lmEquipmentId, m_tabWidget);

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

	SimulatorControlPage* controlPage = new SimulatorControlPage(logicModule, m_tabWidget);

	int tabIndex = m_tabWidget->addTab(controlPage, lmEquipmentId);
	m_tabWidget->setCurrentIndex(tabIndex);

	return;
}

void SimulatorWidget::tabCloseRequest(int index)
{
	QWidget* w = m_tabWidget->widget(index);
	assert(w);

	delete w;
	return;
}

void SimulatorWidget::tabBarContextMenuRequest(const QPoint& pos)
{
	assert(m_tabWidget);
	QTabBar* tabBar = m_tabWidget->tabBar();

	int tabIndex = tabBar->tabAt(pos);
	if (tabIndex == -1)
	{
		return;
	}

	SimulatorBasePage* page = qobject_cast<SimulatorBasePage*>(m_tabWidget->widget(tabIndex));
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
SimulatorToolBar::SimulatorToolBar(const QString& title, QWidget* parent) :
	QToolBar(title, parent)
{
	setMovable(false);
	setObjectName("SimulatorToolBar");

	setIconSize(iconSize() / 1.5);

	toggleViewAction()->setDisabled(true);

	return;
}

SimulatorToolBar::~SimulatorToolBar()
{
}
