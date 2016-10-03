#pragma once
#include "MainTabPage.h"
#include "./Builder/Builder.h"
#include "./Builder/IssueLogger.h"
#include "../lib/DeviceObject.h"
#include "../lib/OutputLog.h"
#include <QTextCursor>
#include <QLineEdit>

class DbController;
class QCheckBox;
class QTextEdit;
class QPushButton;
class QSplitter;

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

	bool isBuildRunning() const;

	const std::map<QUuid, OutputMessageLevel>* itemsIssues() const;
	void cancelBuild();

protected:
	void CreateActions();

signals:

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void timerEvent(QTimerEvent* event) override;

public slots:
	void projectOpened();
	void projectClosed();

	void build();
	void cancel();

protected slots:
	void buildWasStarted();
	void buildWasFinished();

	void hideWaringsStateChanged(int state);

	void prevIssue();
	void nextIssue();

	void search();

	// Data
	//
private:
	QWidget* m_rightSideWidget = nullptr;
	QTextEdit* m_outputWidget = nullptr;

	QPushButton* m_prevIssueButton = nullptr;
	QPushButton* m_nextIssueButton = nullptr;

	QLineEdit* m_findTextEdit = nullptr;
	QPushButton* m_findTextButton = nullptr;

	QPushButton* m_buildButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	QSplitter* m_vsplitter = nullptr;
	QSplitter* m_hsplitter = nullptr;

	QWidget* m_settingsWidget = nullptr;

	QCheckBox* m_debugCheckBox = nullptr;
	QCheckBox* m_hideWarningsCheckBox = nullptr;

	Builder::IssueLogger m_outputLog;
	int m_logTimerId = -1;

	QFile m_logFile;
	static const char* m_buildLogFileName;

	Builder::Builder m_builder;		// In constructor it receives pointer to m_outputLog, so m_outputLog must be created already!

	std::map<QUuid, OutputMessageLevel> m_itemsIssues;		// contains QUuid of all schame items with issues

	// Issue navigation
	//
	QTextCursor m_lastNavCursor;
	bool m_lastNavIsPrevIssue = false;
	bool m_lastNavIsNextIssue = false;
};


