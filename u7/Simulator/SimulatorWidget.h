#ifndef SIMULATORWIDGET_H
#define SIMULATORWIDGET_H

#include <QMainWindow>
#include <QtWidgets>
#include "../lib/DbController.h"
#include "../Simulator/Simulator.h"

class SimulatorProjectWidget;
class SimulatorOutputWidget;
class SimulatorMemoryWidget;
class SimulatorToolBar;

class SimulatorWidget : public QMainWindow, HasDbController
{
	Q_OBJECT
public:
	explicit SimulatorWidget(DbController* db, QWidget* parent = nullptr);
	virtual ~SimulatorWidget();

protected:
	void createToolBar();
	void createDocks();
	QDockWidget* createMemoryDock(QString caption);

	virtual void showEvent(QShowEvent*) override;

protected slots:
	void loadBuild(QString buildPath);

private:
	SimulatorToolBar* m_toolBar = nullptr;
	SimulatorProjectWidget* m_projectWidget = nullptr;
	std::vector<SimulatorMemoryWidget*> m_memoryWidgets;

	Sim::Simulator m_simulator;
};


class SimulatorToolBar : public QToolBar
{
	Q_OBJECT
public:
	explicit SimulatorToolBar(const QString& title, QWidget* parent = nullptr);
	virtual ~SimulatorToolBar();

private:
};

#endif // SIMULATORWIDGET_H
