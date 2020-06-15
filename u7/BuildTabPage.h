#pragma once
#include "MainTabPage.h"
#include "../Builder/Builder.h"
#include "../Builder/IssueLogger.h"
#include "../lib/DeviceObject.h"
#include "../lib/OutputLog.h"

class DbController;
class QCheckBox;
class QTextEdit;
class QPushButton;
class QSplitter;
class QComboBox;

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
	bool isBuildRunning() const;

	const std::map<QUuid, OutputMessageLevel>* itemsIssues() const;
	void cancelBuild();

	int progress() const;

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
	void buildWasFinished(int errorCount);

	void prevIssue();
	void nextIssue();

	void search();

signals:
	void buildStarted();					// Just retranslate signal from Builder
	void buildFinished(int errorCount);		// Just retranslate signal from Builder

	// Data
	//
private:
	enum class WarningShowLevel
	{
		ShowAll,
		Middle,
		Important,
		HideAll
	};

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
	QComboBox* m_warningsLevelComboBox = nullptr;

	int m_logTimerId = -1;

	QFile m_logFile;
	static const char* m_buildLogFileName;

	Builder::Builder m_builder;		// In constructor it receives pointer to m_outputLog, so m_outputLog must be created already!

	std::map<QUuid, OutputMessageLevel> m_itemsIssues;		// contains QUuid of all schemes items with issues

	// Issue navigation
	//
	QTextCursor m_lastNavCursor;
	bool m_lastNavIsPrevIssue = false;
	bool m_lastNavIsNextIssue = false;

	// Actions
	//
	QAction* m_findNextAction = nullptr;
	QAction* m_prevIssueAction = nullptr;
	QAction* m_nextIssueAction = nullptr;
};


