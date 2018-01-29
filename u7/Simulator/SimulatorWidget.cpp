#include "SimulatorWidget.h"
#include "Settings.h"
#include "SimulatorProjectWidget.h"
#include "SimulatorMemoryWidget.h"
#include "SimulatorOutputWidget.h"
#include "SimulatorSelectBuildDialog.h"


SimulatorWidget::SimulatorWidget(DbController* db, QWidget* parent)
	: QMainWindow(parent),
	  HasDbController(db)
{
	setWindowFlags(Qt::Widget);
	setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

	setCentralWidget(new QWidget);
	centralWidget()->setBackgroundRole(QPalette::Dark);
	centralWidget()->setAutoFillBackground(true);

	QVBoxLayout* layout = new QVBoxLayout;
	centralWidget()->setLayout(layout);

	createToolBar();
	createDocks();

	updateActions();
	// --
	//

	return;
}

SimulatorWidget::~SimulatorWidget()
{
	theSettings.m_simWigetState = saveState();
	theSettings.writeUserScope();
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
	QDockWidget* outputDock = new QDockWidget("Output", this, 0);
	outputDock->setObjectName(watchDock->windowTitle());
	outputDock->setWidget(new SimulatorOutputWidget());
	outputDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

	addDockWidget(Qt::BottomDockWidgetArea, outputDock);

	// Memory Widget - at least one defaullt memory widget
	//
	QDockWidget* m1 = createMemoryDock("Memory 1");
	QDockWidget* m2 = createMemoryDock("Memory 2");
	QDockWidget* m3 = createMemoryDock("Memory 3");

	tabifyDockWidget(outputDock, m1);
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
	restoreState(theSettings.m_simWigetState);

	return;
}

void SimulatorWidget::updateActions()
{
	m_openProjectAction->setEnabled(true);
	m_closeProjectAction->setEnabled(m_simulator.isLoaded());
	m_refreshProjectAction->setEnabled(m_simulator.isLoaded());

	return;
}

void SimulatorWidget::openBuild()
{
	QSettings settings;
	SimulatorSelectBuildDialog::BuildType buildType = static_cast<SimulatorSelectBuildDialog::BuildType>(settings.value("SimulatorWidget/BuildType", 0).toInt());
	static QString lastPath;

	SimulatorSelectBuildDialog d(db()->currentProject().projectName().toLower(),
								 buildType,
								 lastPath,
								 this);
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		settings.setValue("SimulatorWidget/BuildType", static_cast<int>(d.resultBuildType()));
		lastPath = d.resultBuildPath();

		loadBuild(lastPath);
	}

	updateActions();
	return;
}

void SimulatorWidget::closeBuild()
{
	m_simulator.clear();
	updateActions();
	return;
}

void SimulatorWidget::refreshBuild()
{
	loadBuild(m_simulator.buildPath());
	updateActions();
	return;
}

void SimulatorWidget::loadBuild(QString buildPath)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool ok = m_simulator.load(buildPath);
	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot open project for simultaion. For details see Output window."));
	}

	return;
}

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
