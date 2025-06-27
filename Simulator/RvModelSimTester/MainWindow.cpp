#include "MainWindow.h"
#include "DialogSettings.h"
#include "LogModule.h"
#include "RWMultyToolBox.h"
#include "RWToolBox.h"
#include "SimControlModule.h"
#include "SimTestUDPThread.h"
#include "CSVEditorDialog.h"

#include <QGroupBox>
#include <QMenuBar>
#include <QRadioButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

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

	QAction* editCsvAction = menu->addAction("Edit Signal CSV...");
	QAction* openMenuAction = menu->addAction("Settings..");

	// Create UDP thread
	SimTestUDPController* controller = new SimTestUDPController();

	// Create r/wToolBox
	RWToolBox* rwToolBox = new RWToolBox(this);
	RWMultyToolBox* rwMultiToolBox = new RWMultyToolBox(this);

	// Create a QTabWidget for read/write toolboxes
	QTabWidget* rwTabWidget = new QTabWidget(this);
	rwTabWidget->addTab(rwToolBox, "Single Read/Write");
	rwTabWidget->addTab(rwMultiToolBox, "Multi Read/Write");


	// Create buttons for different value types
	analogBtn = new QRadioButton("Analog Values", this);
	boolBtn = new QRadioButton("Bool Values", this);
	discreteBtn = new QRadioButton("Float Values", this);

	// Load saved selection
	loadValueTypeSelection();

	// Chack buttons
	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(analogBtn);
	buttonLayout->addWidget(boolBtn);
	buttonLayout->addWidget(discreteBtn);


	// GroupBox for RWToolBox and RWMultyToolBox
	QGroupBox* rwGroup = new QGroupBox("Read/Write Toolbox", this);
	QVBoxLayout* rwLayout = new QVBoxLayout;
	rwLayout->addWidget(rwTabWidget);
	rwLayout->addLayout(buttonLayout);
	rwGroup->setLayout(rwLayout);

	// Create simControl
	SimControlModule* simControl = new SimControlModule(this);

	// GroupBox for SimControl
	QGroupBox* simGroup = new QGroupBox("Simulation Control Action", this);
	QVBoxLayout* simLayout = new QVBoxLayout;
	simLayout->addWidget(simControl);
	simGroup->setLayout(simLayout);

	// Create log module
	LogModule* actionLog = new LogModule(this);

	// GroupBox for LogModule
	QGroupBox* logGroup = new QGroupBox("Simulation Log", this);
	QVBoxLayout* logLayout = new QVBoxLayout;
	logLayout->addWidget(actionLog);
	logGroup->setLayout(logLayout);

	// Main layout of app
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setContentsMargins(8, 8, 8, 8);
	mainLayout->setSpacing(10);
	mainLayout->setMenuBar(menuBar);
	mainLayout->addWidget(rwGroup);
	mainLayout->addWidget(simGroup);
	mainLayout->addWidget(logGroup, 1);
	setLayout(mainLayout);


	// All conections
	connect(showServerStateAction, &QAction::toggled, this, &MainWindow::serverState);
	connect(showServerStateAction, &QAction::toggled, controller, &SimTestUDPController::setShowServerState);

	connect(editCsvAction,
			&QAction::triggered,
			this,
			[this, rwMultiToolBox]()
			{
				CsvEditorDialog* dlg = new CsvEditorDialog("signals.csv", this);

				connect(dlg,
						&CsvEditorDialog::csvSaved,
						this,
						[rwMultiToolBox](const QString& path)
						{
							rwMultiToolBox->updateSignalFromCSV(path);
							// Optionally: update table view or notify user
						});

				dlg->exec();
			});
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

	connect(analogBtn,
			&QRadioButton::clicked,
			[rwMultiToolBox, controller]()
			{
				rwMultiToolBox->setValueType(SignalType::AnalogInt32);
				controller->setValueType(SignalType::AnalogInt32);
			});
	connect(boolBtn,
			&QRadioButton::clicked,
			[rwMultiToolBox, controller]()
			{
				rwMultiToolBox->setValueType(SignalType::Discrete);
				controller->setValueType(SignalType::Discrete);
			});
	connect(discreteBtn,
			&QRadioButton::clicked,
			[rwMultiToolBox, controller]()
			{
				rwMultiToolBox->setValueType(SignalType::AnalogFloat);
				controller->setValueType(SignalType::AnalogFloat);
			});

	connect(rwMultiToolBox, &RWMultyToolBox::requestRead, controller, &SimTestUDPController::operateRead);
	connect(rwMultiToolBox, &RWMultyToolBox::requestWrite, controller, &SimTestUDPController::operateWrite);


	auto saveSelection = [this]()
	{
		saveValueTypeSelection();
	};
	connect(analogBtn, &QRadioButton::toggled, this, saveSelection);
	connect(boolBtn, &QRadioButton::toggled, this, saveSelection);
	connect(discreteBtn, &QRadioButton::toggled, this, saveSelection);

	// Name and size
	setWindowTitle("Simulation Tester");
	resize(890, 660);
	setMinimumSize(660, 580);
}

MainWindow::~MainWindow()
{
	// Save window size
	QSettings settings("Geo", "SimulationTester");
	settings.setValue("MainWindow/geometry", saveGeometry());
}

 void MainWindow::saveValueTypeSelection()
{
	QSettings settings("Geo", "SimulationTester");
	int selected = 0;
	if (analogBtn->isChecked())
		selected = 0;
	else if (boolBtn->isChecked())
		selected = 1;
	else if (discreteBtn->isChecked())
		selected = 2;
	settings.setValue("MainWindow/valueTypeSelection", selected);
}

void MainWindow::loadValueTypeSelection()
{
	QSettings settings("Geo", "SimulationTester");
	int selected = settings.value("MainWindow/valueTypeSelection", 0).toInt();
	switch (selected)
	{
	case 0:
		analogBtn->setChecked(true);
		break;
	case 1:
		boolBtn->setChecked(true);
		break;
	case 2:
		discreteBtn->setChecked(true);
		break;
	default:
		analogBtn->setChecked(true);
		break;
	}
}