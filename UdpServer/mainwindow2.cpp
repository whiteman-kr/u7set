#include "mainwindow2.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	m_baseServiceController = new BaseServiceController(STP_BASE);
}

MainWindow::~MainWindow()
{
	delete m_baseServiceController;

	delete ui;
}


void MainWindow::on_pushButton_clicked()
{
	m_baseServiceController->sendFile(QHostAddress("127.0.0.1"), PORT_BASE_SERVICE, "d:\base.css");
}
