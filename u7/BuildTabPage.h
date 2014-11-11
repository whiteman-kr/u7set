#pragma once
#include "MainTabPage.h"
#include "../include/DeviceObject.h"
#include "../include/OutputLog.h"

class DbController;

class BuildTask
{
public:
	BuildTask(const QString& taskName, bool buildTask) : name(taskName), build(buildTask){}

	QString name;
	bool build = true;
};



//
//
// BuildTabPage
//
//
class BuildTabPage : public MainTabPage
{
	Q_OBJECT

public:
	BuildTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~BuildTabPage();

protected:
	void CreateActions();

	void writeOutputLog(const OutputLogItem& logItem);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void timerEvent(QTimerEvent* event) override;

public slots:
	void projectOpened();
	void projectClosed();

protected slots:
	void build();
	void cancel();

	// Data
	//
private:
	QTableWidget* m_taskTable = nullptr;

	QWidget* m_leftSideWidget = nullptr;
	QTextEdit* m_outputWidget = nullptr;
	QPushButton* m_buildButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	QSplitter* m_vsplitter = nullptr;
	QSplitter* m_hsplitter = nullptr;

	bool m_buildEquipmentConfiguration = false;

	std::vector<BuildTask> m_tasks;

	OutputLog m_outputLog;
	int m_logTimerId = -1;
};


