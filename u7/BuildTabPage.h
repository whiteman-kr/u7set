#pragma once
#include "MainTabPage.h"
#include "ProjectBuilder.h"
#include "../include/DeviceObject.h"
#include "../include/OutputLog.h"

class DbController;
class QCheckBox;


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
	//QTableWidget* m_taskTable = nullptr;

	QWidget* m_rightSideWidget = nullptr;
	QTextEdit* m_outputWidget = nullptr;
	QPushButton* m_buildButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	QSplitter* m_vsplitter = nullptr;
	QSplitter* m_hsplitter = nullptr;

	QWidget* m_settingsWidget = nullptr;
	QCheckBox* m_debugCheckBox = nullptr;

	OutputLog m_outputLog;
	int m_logTimerId = -1;

	ProjectBuilder m_builder;				// In constructor it receives pointer to m_outputLog, so m_outputLog must be created already!
};


