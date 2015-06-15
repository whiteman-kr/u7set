#include "servermainwindow.h"
#include "ui_servermainwindow.h"


ServerMainWindow::ServerMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ServerMainWindow)
{
	ui->setupUi(this);

	m_protoUdpServerThread = new ProtoUdp::ServerThread(HostAddressPort("192.168.75.85", PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST));

	m_protoUdpServerThread->run();
}


ServerMainWindow::~ServerMainWindow()
{
	m_protoUdpServerThread->quit();

	delete m_protoUdpServerThread;

	delete ui;

	//delete m_ServiceController;
}

