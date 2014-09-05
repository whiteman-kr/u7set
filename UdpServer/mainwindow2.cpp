#include "mainwindow.h"
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
