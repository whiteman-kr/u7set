#include "servermainwindow.h"
#include "ui_servermainwindow.h"

ServerMainWindow::ServerMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ServerMainWindow)
{
	ui->setupUi(this);

	m_protoUdpClientThread = new ProtoUdpClientThread(HostAddressPort("192.168.75.85", PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST));

	m_protoUdpClientThread->run();
}

ServerMainWindow::~ServerMainWindow()
{
	m_protoUdpClientThread->quit();

	delete m_protoUdpClientThread;

	delete ui;

	//delete m_ServiceController;
}

