#include "MainWindow.h"
#include "LogModule.h"
#include "DialogSettings.h"
#include "RWToolBox.h"
#include "SimControlModule.h"

#include <QMenuBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) :
	QWidget(parent)
{
	setupUi();
}

void MainWindow::setupUi()
{
	// Create main window
	QMenuBar* menuBar = new QMenuBar(this);
	QMenu* menu = menuBar->addMenu("Menu");
	QAction* openMenuAction = menu->addAction("Settings..");

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
	mainLayout->addWidget(actionLog);
	setLayout(mainLayout);

	// All conections
	connect(openMenuAction,
			&QAction::triggered,
			this,
			[this]()
			{
				DialogSettings* menuWidget = new DialogSettings;
				menuWidget->exec();
			});
	connect(simControl, &SimControlModule::simAction, actionLog, &LogModule::logAction);
	connect(rwToolBox, &RWToolBox::readAction, actionLog, &LogModule::logAction);
	connect(rwToolBox, &RWToolBox::writeAction, actionLog, &LogModule::logAction);

	// Name and size
	setWindowTitle("Simulation Tester");
	setFixedSize(430, 370);
}
