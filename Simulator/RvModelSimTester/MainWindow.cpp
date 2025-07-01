#include "MainWindow.h"
#include "CsvEditorDialog.h"
#include "DialogSettings.h"
#include "LogModule.h"
#include "RWMultiToolBox.h"
#include "RWToolBox.h"
#include "SimControlModule.h"
#include "SimTestUDPThread.h"

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
	restoreGeometry(QSettings().value("MainWindow/geometry").toByteArray());
}

void MainWindow::setupUi()
{
	// Create main window
	QMenuBar* menuBar = new QMenuBar(this);
	QMenu* menu = menuBar->addMenu(tr("Menu"));

	QAction* showServerStateAction = menu->addAction(tr("Ping Server State Continuously"));
	showServerStateAction->setCheckable(true);
	showServerStateAction->setChecked(true);
	showServerStateAction->setToolTip(tr("Toggle continuous server state pinging"));

	QAction* editCsvAction = menu->addAction(tr("Edit Signal CSV..."));
	QAction* openMenuAction = menu->addAction(tr("Settings.."));

	// Create UDP thread
	SimTestUDPController* controller = new SimTestUDPController();

	// Create r/wToolBox
	RWToolBox* rwToolBox = new RWToolBox(this);
	RWMultiToolBox* rwMultiToolBox = new RWMultiToolBox(this);

	// Create a QTabWidget for read/write toolboxes
	QTabWidget* rwTabWidget = new QTabWidget(this);
	rwTabWidget->addTab(rwToolBox, tr("Single Read/Write"));
	rwTabWidget->addTab(rwMultiToolBox, tr("Multi Read/Write"));


	// Create buttons for different value types
	m_analogBtn = new QRadioButton(tr("Analog Values"), this);
	m_boolBtn = new QRadioButton(tr("Bool Values"), this);
	m_discreteBtn = new QRadioButton(tr("Float Values"), this);

	// Load saved selection
	loadValueTypeSelection();

	// Check buttons
	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(m_analogBtn);
	buttonLayout->addWidget(m_boolBtn);
	buttonLayout->addWidget(m_discreteBtn);


	// GroupBox for RWToolBox and RWMultyToolBox
	QGroupBox* rwGroup = new QGroupBox(tr("Read/Write Toolbox"), this);
	QVBoxLayout* rwLayout = new QVBoxLayout;
	rwLayout->addWidget(rwTabWidget);
	rwLayout->addLayout(buttonLayout);
	rwGroup->setLayout(rwLayout);

	// Create simControl
	SimControlModule* simControl = new SimControlModule(this);

	// GroupBox for SimControl
	QGroupBox* simGroup = new QGroupBox(tr("Simulation Control Action"), this);
	QVBoxLayout* simLayout = new QVBoxLayout;
	simLayout->addWidget(simControl);
	simGroup->setLayout(simLayout);

	// Create log module
	LogModule* actionLog = new LogModule(this);

	// GroupBox for LogModule
	QGroupBox* logGroup = new QGroupBox(tr("Simulation Log"), this);
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


	// All connections
	connect(showServerStateAction, &QAction::toggled, this, &MainWindow::serverState);
	connect(showServerStateAction, &QAction::toggled, controller, &SimTestUDPController::setShowServerState);

	connect(editCsvAction,
			&QAction::triggered,
			this,
			[this, rwMultiToolBox]()
			{
				onEditCsvActionTriggered(rwMultiToolBox);
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

	connect(m_analogBtn,
			&QRadioButton::clicked,
			[rwMultiToolBox, controller]()
			{
				rwMultiToolBox->setValueType(SignalType::AnalogInt32);
				controller->setValueType(SignalType::AnalogInt32);
			});

	connect(m_boolBtn,
			&QRadioButton::clicked,
			[rwMultiToolBox, controller]()
			{
				rwMultiToolBox->setValueType(SignalType::Discrete);
				controller->setValueType(SignalType::Discrete);
			});

	connect(m_discreteBtn,
			&QRadioButton::clicked,
			[rwMultiToolBox, controller]()
			{
				rwMultiToolBox->setValueType(SignalType::AnalogFloat);
				controller->setValueType(SignalType::AnalogFloat);
			});

	connect(rwMultiToolBox, &RWMultiToolBox::requestRead, controller, &SimTestUDPController::operateRead);
	connect(rwMultiToolBox, &RWMultiToolBox::requestWrite, controller, &SimTestUDPController::operateWrite);


	auto saveSelection = [this]()
	{
		saveValueTypeSelection();
	};
	connect(m_analogBtn, &QRadioButton::toggled, this, saveSelection);
	connect(m_boolBtn, &QRadioButton::toggled, this, saveSelection);
	connect(m_discreteBtn, &QRadioButton::toggled, this, saveSelection);

	// Name and size
	setWindowTitle(tr("Simulation Tester"));
	resize(890, 660);
	setMinimumSize(660, 580);
}

MainWindow::~MainWindow()
{
	// Save window size
	QSettings settings;
	settings.setValue("MainWindow/geometry", saveGeometry());
}

void MainWindow::saveValueTypeSelection()
{
	QSettings settings;
	int selected = 0;
	if (m_analogBtn->isChecked())
	{
		selected = 0;
	}
	else if (m_boolBtn->isChecked())
	{
		selected = 1;
	}
	else if (m_discreteBtn->isChecked())
	{
		selected = 2;
	}
	settings.setValue("MainWindow/valueTypeSelection", selected);
}

void MainWindow::loadValueTypeSelection()
{
	QSettings settings;
	int selected = settings.value("MainWindow/valueTypeSelection", 0).toInt();
	switch (selected)
	{
	case 0:
		m_analogBtn->setChecked(true);
		break;
	case 1:
		m_boolBtn->setChecked(true);
		break;
	case 2:
		m_discreteBtn->setChecked(true);
		break;
	default:
		m_analogBtn->setChecked(true);
		break;
	}
}

void MainWindow::onEditCsvActionTriggered(RWMultiToolBox* rwMultiToolBox)
{
	CsvEditorDialog* dlg = new CsvEditorDialog(RWMultiToolBox::m_signalsFileName, this);

	connect(dlg,
			&CsvEditorDialog::csvSaved,
			this,
			[rwMultiToolBox](const QString& path)
			{
				rwMultiToolBox->updateSignalFromCSV(path);
			});

	dlg->exec();
}