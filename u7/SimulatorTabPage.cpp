#include "Stable.h"
#include "SimulatorTabPage.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"
#include "DialogSettingsConfigurator.h"

//
//
// SimulatorTabPage
//
//
SimulatorTabPage::SimulatorTabPage(DbController* dbc, QWidget* parent) :
	MainTabPage(dbc, parent)//,
{
	assert(dbc != nullptr);

	// Controls
	//
	m_simultorWidget = new SimulatorWidget(dbc);

	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);

	layout->setContentsMargins(0, 6, 0, 0);
	layout->addWidget(m_simultorWidget);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SimulatorTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SimulatorTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);

	return;
}

SimulatorTabPage::~SimulatorTabPage()
{
}

void SimulatorTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}


void SimulatorTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void SimulatorTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

