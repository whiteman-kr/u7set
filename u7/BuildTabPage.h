#pragma once
#include "MainTabPage.h"

class DbController;
class QCheckBox;
class QTextEdit;
class QPushButton;
class QSplitter;
class QComboBox;
class QTextBrowser;

namespace Builder
{
	class Builder;
}

class OutputLogItem;

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

	void warningsLevelChanged(int index);

	void prevIssue();
	void nextIssue();

	void search();

signals:
	void buildStarted();					// Just retranslate signal from Builder
	void buildFinished(int errorCount);		// Just retranslate signal from Builder

private:
	void getProjectBuildPath(QString* buildCurrentPath, QString* buildLastPath) const;

	void appendMessagesToOutputLog(const std::vector<OutputLogItem>& messages);

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
	QTextBrowser* m_outputWidget = nullptr;

	QPushButton* m_prevIssueButton = nullptr;
	QPushButton* m_nextIssueButton = nullptr;

	QLineEdit* m_findTextEdit = nullptr;
	QPushButton* m_findTextButton = nullptr;

	QPushButton* m_buildButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	QSplitter* m_vsplitter = nullptr;

	QWidget* m_settingsWidget = nullptr;

	QLabel* m_buildLabel[2] = {nullptr, nullptr};
	QComboBox* m_warningsLevelComboBox = nullptr;

	QComboBox* m_generateAppLogicDrawings = nullptr;
	QComboBox* m_generateAppSignalsXml = nullptr;
	QComboBox* m_generateAppSignalsExtXml = nullptr;
	QComboBox* m_generateExtraDebugInfo = nullptr;
	QComboBox* m_runSimTestsOnBuild = nullptr;

	int m_logTimerId = -1;

	std::unique_ptr<Builder::Builder> m_builder;		// In constructor it receives pointer to m_outputLog, so m_outputLog must be created already!

	std::vector<OutputLogItem> m_messages;
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


