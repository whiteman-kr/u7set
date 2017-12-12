#include "SimulatorWidget.h"
#include "Settings.h"
#include "SimulatorMemoryWidget.h"

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

	m_projectWidget = new SimulatorProjectWidget(db());

	projectDock->setWidget(m_projectWidget);

	addDockWidget(Qt::LeftDockWidgetArea, projectDock);

	// Quick Watch dock
	//
	QDockWidget* watchDock = new QDockWidget("Watch", this, 0);
	watchDock->setObjectName(watchDock->windowTitle());
	watchDock->setWidget(new QWidget());		// Dummy for now

	addDockWidget(Qt::RightDockWidgetArea, watchDock);

	// Memory Widget - at least one defaullt memory widget
	//
	QDockWidget* m1 = createMemoryDock("Memory 1");
	QDockWidget* m2 = createMemoryDock("Memory 2");
	QDockWidget* m3 = createMemoryDock("Memory 3");

	tabifyDockWidget(m1, m2);
	tabifyDockWidget(m2, m3);

	// --
	//
	connect(m_projectWidget, &SimulatorProjectWidget::loadBuild, this, &SimulatorWidget::loadBuild);

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

void SimulatorWidget::loadBuild(QString buildPath)
{
	qDebug() << "SimulatorWidget: Load build " << buildPath;
	return;
}


SimulatorProjectWidget::SimulatorProjectWidget(DbController* db, QWidget* parent) :
	QWidget(parent),
	HasDbController(db)
{
	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);

	QVBoxLayout* layout = new QVBoxLayout;

	m_debugReleaseCombo = new QComboBox;
	m_debugReleaseCombo->addItem("Debug");
	m_debugReleaseCombo->addItem("Release");
	m_debugReleaseCombo->setCurrentText("Debug");

	m_refreshButton = new QPushButton("Refresh");
	m_refreshButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	m_loadButton = new QPushButton("Load Build");
	m_loadButton->setEnabled(false);
	m_loadButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	m_buildLabel = new QLabel("Build");
	m_buildList = new QListWidget;
	m_buildList->setSortingEnabled(false);
	QWidget* buildWidget = new QWidget;
	QGridLayout* buildWidgetLayout = new QGridLayout;
	buildWidgetLayout->setMargin(0);
	buildWidgetLayout->addWidget(m_buildLabel, 0, 0, 1, 1);
	buildWidgetLayout->addWidget(m_refreshButton, 0, 1, 1, 1);
	buildWidgetLayout->addWidget(m_buildList, 1, 0, 1, 2);
	buildWidget->setLayout(buildWidgetLayout);

	m_equipmentLabel = new QLabel("Equipment");
	m_equipmentTree = new QTreeWidget;
	QWidget* equipWidget = new QWidget;
	QGridLayout* equipWidgetLayout = new QGridLayout;
	equipWidgetLayout->setMargin(0);
	equipWidgetLayout->addWidget(m_equipmentLabel, 0, 0, 1, 1);
	equipWidgetLayout->addWidget(m_loadButton, 0, 1, 1, 1);
	equipWidgetLayout->addWidget(m_equipmentTree, 1, 0, 1, 2);
	equipWidget->setLayout(equipWidgetLayout);

	layout->addWidget(m_debugReleaseCombo);

	m_splitter = new QSplitter(Qt::Vertical);
	m_splitter->setChildrenCollapsible(false);

	m_splitter->addWidget(buildWidget);
	m_splitter->addWidget(equipWidget);

	layout->addWidget(m_splitter);

	setLayout(layout);

	m_splitter->restoreState(theSettings.m_simBuildSplitter);

	// --
	//
	connect(m_debugReleaseCombo, &QComboBox::currentTextChanged, this, &SimulatorProjectWidget::debugReleaseChanged);

	connect(m_buildList, &QListWidget::currentRowChanged, this, &SimulatorProjectWidget::buildListSelectionChanged);
	connect(m_buildList, &QListWidget::itemDoubleClicked, this, &SimulatorProjectWidget::buildListItemDoubleClicked);

	connect(m_refreshButton, &QPushButton::clicked, this, &SimulatorProjectWidget::fillBuildList);
	connect(m_loadButton, &QPushButton::clicked, this, &SimulatorProjectWidget::loadButtonClicked);

	return;
}

SimulatorProjectWidget::~SimulatorProjectWidget()
{
	theSettings.m_simBuildSplitter = m_splitter->saveState();
	theSettings.writeUserScope();
}

QString SimulatorProjectWidget::buildsPath()
{
	QString configurationType = m_debugReleaseCombo->currentText().toLower();

	QString projectName = db()->currentProject().projectName().toLower();
	QString buildSearchPath = QString("%1%2%3-%4")
							  .arg(theSettings.buildOutputPath())
							  .arg(QDir::separator())
							  .arg(projectName)
							  .arg(configurationType);

	return buildSearchPath;
}

void SimulatorProjectWidget::showEvent(QShowEvent*)
{
	fillBuildList();
}

void SimulatorProjectWidget::fillBuildList()
{
	int lastSelectedBuild = -1;
	if (m_buildList->selectionModel()->hasSelection() == true)
	{
		lastSelectedBuild = m_buildList->currentRow();
	}

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	QDir dir(buildsPath());
	QFileInfoList buildDirsList = dir.entryInfoList(QStringList("build*"), QDir::Dirs | QDir::NoSymLinks, QDir::Name);

	m_buildList->clear();

	for (const QFileInfo& fi : buildDirsList)
	{
		QListWidgetItem* item = new QListWidgetItem(fi.fileName(), m_buildList);
		item->setData(Qt::UserRole, fi.absoluteFilePath());
	}

	if (lastSelectedBuild != -1)
	{
		m_buildList->setCurrentRow(lastSelectedBuild);
	}
	else
	{
		m_buildList->setCurrentRow(0);
	}

	return;
}

void SimulatorProjectWidget::debugReleaseChanged(const QString&)
{
	fillBuildList();
}

void SimulatorProjectWidget::loadButtonClicked()
{
	assert(m_buildList);
	QList<QListWidgetItem*> selected = m_buildList->selectedItems();

	if (selected.isEmpty() == true)
	{
		return;
	}

	QListWidgetItem* item = selected.front();
	QString buildPath = item->data(Qt::UserRole).toString();

	emit loadBuild(buildPath);

	return;
}

void SimulatorProjectWidget::buildListSelectionChanged(int currentRow)
{
	m_loadButton->setEnabled(currentRow != -1);
}

void SimulatorProjectWidget::buildListItemDoubleClicked(QListWidgetItem*)
{
	loadButtonClicked();
}

SimulatorToolBar::SimulatorToolBar(const QString& title, QWidget* parent) :
	QToolBar(title, parent)
{
	setMovable(false);
	setObjectName("SimulatorToolBar");
}

SimulatorToolBar::~SimulatorToolBar()
{
}
