#pragma once
#include "MainTabPage.h"
#include "ProjectBuilder.h"
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

	void buildStarted();
	void buildFinished();

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

	OutputLog m_outputLog;
	int m_logTimerId = -1;

	std::vector<BuildTask> m_tasks;
	ProjectBuilder m_builder;				// In constructor it receives pointer to m_outputLog, so m_outputLog must be created already!
};


