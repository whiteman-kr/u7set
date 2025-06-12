#include "MainWindow.h"
#include "DialogSettings.h"
#include "LogModule.h"
#include "RWToolBox.h"
#include "SimControlModule.h"
#include "SimTestUDPThread.h"

#include <QMenuBar>
#include <QVBoxLayout>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent) :
	QWidget(parent)
{
	setupUi();

	// Load settings size
	QSettings settings("Geo", "SimulationTester");
	restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
}

void MainWindow::setupUi()
{
	// Create main window
	QMenuBar* menuBar = new QMenuBar(this);
	QMenu* menu = menuBar->addMenu("Menu");

	QAction* showServerStateAction = menu->addAction("Ping Server State Continiously");
	showServerStateAction->setCheckable(true);
	showServerStateAction->setChecked(true);
	showServerStateAction->setToolTip("Toggle continuous server state pinging");

	QAction* openMenuAction = menu->addAction("Settings..");

	// Create UDP thread
	SimTestUDPController* controller = new SimTestUDPController();

	// Create r/wToolBox
	RWToolBox* rwToolBox = new RWToolBox(this);

	// Create simControl
	SimControlModule* simControl = new SimControlModule(this);

	// Create log module
	LogModule* actionLog = new LogModule(this);


	// Main layout of app
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setMenuBar(menuBar);
	mainLayout->addWidget(rwToolBox);
	mainLayout->addWidget(simControl);
	mainLayout->addStretch();
	mainLayout->addWidget(actionLog, 1); // logBox max space
	setLayout(mainLayout);


	// All conections
	connect(showServerStateAction, &QAction::toggled, this, &MainWindow::serverState);
	connect(showServerStateAction, &QAction::toggled, controller, &SimTestUDPController::setShowServerState);


	connect(openMenuAction,
			&QAction::triggered,
			this,
			[this, controller]()
			{
				DialogSettings* menuWidget = new DialogSettings;
				connect(menuWidget, &DialogSettings::accepted, controller, &SimTestUDPController::reloadSettings);
				menuWidget->exec();
			});

	
	connect(controller, &SimTestUDPController::resultReady, actionLog, &LogModule::logAction);
	connect(simControl, &SimControlModule::simAction, actionLog, &LogModule::logAction);
	
	
	connect(controller, &SimTestUDPController::simStateReady, simControl, &SimControlModule::onSimStateReady);

	
	connect(simControl, &SimControlModule::simControlMode, controller, &SimTestUDPController::simControlMode);

	
	
	connect(rwToolBox, &RWToolBox::requestRead, controller, &SimTestUDPController::operateRead);
	connect(rwToolBox, &RWToolBox::requestWrite, controller, &SimTestUDPController::operateWrite);

	

	// Name and size
	setWindowTitle("Simulation Tester");
    resize(430, 370);
	setMinimumSize(430, 370);
}

MainWindow::~MainWindow()
{
	// Save window size
	QSettings settings("Geo", "SimulationTester");
	settings.setValue("MainWindow/geometry", saveGeometry());
}