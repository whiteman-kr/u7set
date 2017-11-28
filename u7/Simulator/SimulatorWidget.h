#ifndef SIMULATORWIDGET_H
#define SIMULATORWIDGET_H

#include <QMainWindow>
#include <QtWidgets>
#include "../lib/DbController.h"

class SimulatorProjectWidget;

class SimulatorWidget : public QMainWindow, HasDbController
{
	Q_OBJECT
public:
	explicit SimulatorWidget(DbController* db, QWidget* parent = nullptr);
	virtual ~SimulatorWidget();

protected:
	void createDocks();

protected slots:
	void loadBuild(QString buildPath);

private:
	QDockWidget* m_projectDock = nullptr;
	SimulatorProjectWidget* m_projectWidget = nullptr;
};


// Widget for selection build and module
//
class SimulatorProjectWidget : public QWidget, HasDbController
{
	Q_OBJECT

public:
	explicit SimulatorProjectWidget(DbController* db, QWidget* parent = nullptr);
	virtual ~SimulatorProjectWidget();

protected:
	QString buildsPath();

protected slots:
	virtual void showEvent(QShowEvent*) override;

	void fillBuildList();
	void debugReleaseChanged(const QString &);
	void loadButtonClicked();

	void buildListSelectionChanged(int currentRow);
	void buildListItemDoubleClicked(QListWidgetItem* item);

signals:
	void loadBuild(QString buildPath);

private:
	QComboBox* m_debugReleaseCombo = nullptr;

	QPushButton* m_refreshButton = nullptr;
	QPushButton* m_loadButton = nullptr;

	QLabel* m_buildLabel = nullptr;
	QListWidget* m_buildList = nullptr;

	QSplitter* m_splitter = nullptr;

	QLabel* m_equipmentLabel = nullptr;
	QTreeWidget* m_equipmentTree = nullptr;
};

#endif // SIMULATORWIDGET_H
