#include "MonitorMainWindow.h"
#include "ui_MainWindow.h"

MonitorMainWindow::MonitorMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MonitorMainWindow::~MonitorMainWindow()
{
	delete ui;
}

void MonitorMainWindow::on_actionAbout_triggered()
{

}
