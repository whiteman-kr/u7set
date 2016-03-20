#pragma once
#include "MainTabPage.h"
#include "./Builder/Builder.h"
#include "./Builder/IssueLogger.h"
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

	// Public methods
	//
public:
	//static BuildTabPage* instance();

	const std::map<QUuid, OutputMessageLevel>* itemsIssues() const;

protected:
	void CreateActions();

	void writeOutputLog(const OutputLogItem& logItem);

signals:

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

	void buildWasStarted();
	void buildWasFinished();

	// Data
	//
private:
	//static BuildTabPage* m_this;

	//QTableWidget* m_taskTable = nullptr;

	QWidget* m_rightSideWidget = nullptr;
	QTextEdit* m_outputWidget = nullptr;
	QPushButton* m_buildButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	QSplitter* m_vsplitter = nullptr;
	QSplitter* m_hsplitter = nullptr;

	QWidget* m_settingsWidget = nullptr;
	QCheckBox* m_debugCheckBox = nullptr;

	Builder::IssueLogger m_outputLog;
	int m_logTimerId = -1;

	QFile m_logFile;
	static const char* m_buildLogFileName;

	Builder::Builder m_builder;		// In constructor it receives pointer to m_outputLog, so m_outputLog must be created already!

	std::map<QUuid, OutputMessageLevel> m_itemsIssues;		// contains QUuid of all scheme items with issues
};


