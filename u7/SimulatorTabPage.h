#pragma once
#include "MainTabPage.h"
#include "./Simulator/SimulatorWidget.h"

//
//
// SimulatorTabPage
//
//
class SimulatorTabPage : public MainTabPage
{
	Q_OBJECT

public:
	SimulatorTabPage(DbController* dbc, QWidget* parent);
	virtual ~SimulatorTabPage();

	// Public methods
	//
public:

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
private:
	SimulatorWidget* m_simultorWidget = nullptr;
};


