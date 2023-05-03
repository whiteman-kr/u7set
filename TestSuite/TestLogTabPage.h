#ifndef TESTLOGTABPAGE_H
#define TESTLOGTABPAGE_H

#include "../TestSuiteLib/TestLog.h"
#include "TestSuiteLog.h"

class TestLogTabPage : public QWidget
{
public:
	TestLogTabPage(TestSuiteTestLogOutput& testLogOutput, QWidget* parent);

	void clearOutputLog();

protected slots:
	void testingWasStarted();
	void testingWasFinished(int errorCount);

private slots:
	void prevIssue();
	void nextIssue();

	void search();

private:
	void createActions();
	void timerEvent(QTimerEvent* event) override;

private:
	QTextBrowser* m_outputWidget = nullptr;

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

	TestSuiteTestLogOutput& m_testLogOutput;

	int m_logTimerId = -1;
};

#endif // TESTLOGTABPAGE_H
