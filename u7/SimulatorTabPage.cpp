#include "SimulatorTabPage.h"
#include "Settings.h"
#include "GlobalMessanger.h"

//
//
// SimulatorTabPage
//
//
SimulatorTabPage::SimulatorTabPage(DbController* dbc, QWidget* parent) :
	MainTabPage(dbc, parent)
{
	assert(dbc != nullptr);

	// Controls
	//
	m_simultorWidget = new SimWidget(nullptr, nullptr, dbc, this, Qt::Widget);

	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_simultorWidget);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SimulatorTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SimulatorTabPage::projectClosed);

	return;
}

void SimulatorTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SimulatorTabPage::projectOpened()
{
	return;
}

void SimulatorTabPage::projectClosed()
{
	return;
}

