#include "servermainwindow.h"
#include "ui_servermainwindow.h"

ServerMainWindow::ServerMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ServerMainWindow)
{
	ui->setupUi(this);

	m_ServiceController = new BaseServiceController(STP_BASE, new MainFunctionWorker());
}

ServerMainWindow::~ServerMainWindow()
{
	delete ui;

	delete m_ServiceController;
}

