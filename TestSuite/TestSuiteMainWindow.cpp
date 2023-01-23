#include "TestSuiteMainWindow.h"
#include "ui_TestSuiteMainWindow.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

TestSuiteMainWindow::TestSuiteMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::TestSuiteMainWindow)
{
	ui->setupUi(this);

	m_testEngine = new TestEngine();
	connect(&m_testEngine->testResultLog(), &TestResultLog::newLogItem, this, &TestSuiteMainWindow::newLogItem);
	connect(m_testEngine, &TestEngine::finished, this, &TestSuiteMainWindow::testFinished);
}

TestSuiteMainWindow::~TestSuiteMainWindow()
{
	delete m_testEngine;

	delete ui;
}


void TestSuiteMainWindow::on_m_run_clicked()
{

	if (m_testEngine->isRunning() == false)
	{
		m_testEngine->start();
	}

}


void TestSuiteMainWindow::newLogItem(const TestLogItem& item)
{
	QString text = ui->m_resultsLog->toPlainText() + "\n" + item.toText();
	ui->m_resultsLog->setText(text);
}

void TestSuiteMainWindow::testFinished(int result)
{

}

TestSuiteMainWindow* theMainWindow = nullptr;
