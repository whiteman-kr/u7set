#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>
#include "../TestSuiteLib/TestEngine.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TestSuiteMainWindow; }
QT_END_NAMESPACE

class TestSuiteMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TestSuiteMainWindow(QWidget *parent = nullptr);
	~TestSuiteMainWindow();

private slots:
	void on_m_run_clicked();
	void newLogItem(const TestLogItem& item);
	void testFinished(int result);

private:
	Ui::TestSuiteMainWindow *ui;

	TestEngine* m_testEngine = nullptr;
};

extern TestSuiteMainWindow* theMainWindow;

#endif // TESTSUITEMAINWINDOW_H
