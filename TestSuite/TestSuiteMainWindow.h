#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class TestSuiteMainWindow; }
QT_END_NAMESPACE

class TestSuiteMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TestSuiteMainWindow(QWidget *parent = nullptr);
	~TestSuiteMainWindow();

private:
	Ui::TestSuiteMainWindow *ui;
};
#endif // TESTSUITEMAINWINDOW_H
