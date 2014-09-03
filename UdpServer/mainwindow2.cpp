#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	m_baseServiceController = new BaseServiceController(RQSTP_BASE);
}

MainWindow::~MainWindow()
{
	delete m_baseServiceController;

	delete ui;
}
