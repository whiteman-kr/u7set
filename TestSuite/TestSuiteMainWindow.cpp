#include "TestSuiteMainWindow.h"
#include "ui_TestSuiteMainWindow.h"

TestSuiteMainWindow::TestSuiteMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::TestSuiteMainWindow)
{
	ui->setupUi(this);
}

TestSuiteMainWindow::~TestSuiteMainWindow()
{
	delete ui;
}

