#ifndef TESTLOGTABPAGE_H
#define TESTLOGTABPAGE_H

#include "../TestSuiteLib/TestLog.h"
#include "TestSuiteLog.h"

class TestLogTabPage : public QWidget
{
	Q_OBJECT
public:
	TestLogTabPage(TestSuite::TestLog& testLog, TestSuiteTestLogOutput& testLogOutput, QWidget* parent);

	void clearOutputWidget();

protected slots:
	void testingWasStarted();
	void testingWasFinished(int errorCount);

private slots:
	void prevIssue();
	void nextIssue();

	void search();
	void slot_typeChanged(int index);

private:
	void createActions();
	void timerEvent(QTimerEvent* event) override;
	void appendLogMessages(const std::vector<TestSuite::TestLogItem>& messages);

private:
	QTextBrowser* m_outputWidget = nullptr;

	QComboBox* m_typeCombo = nullptr;

	QPushButton* m_prevIssueButton = nullptr;
	QPushButton* m_nextIssueButton = nullptr;

	QLineEdit* m_findTextEdit = nullptr;
	QPushButton* m_findTextButton = nullptr;


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

	//

	TestSuite::TestLog& m_testLog;
	TestSuiteTestLogOutput& m_testLogOutput;

	int m_logTimerId = -1;
};

#endif // TESTLOGTABPAGE_H
