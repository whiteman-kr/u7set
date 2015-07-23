#include "servermainwindow.h"
#include "ui_servermainwindow.h"

namespace Tcp
{

	void MyServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		return;
	}
}


ServerMainWindow::ServerMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ServerMainWindow)
{
	ui->setupUi(this);

	/*m_protoUdpServerThread = new ProtoUdp::ServerThread(HostAddressPort("192.168.75.85", PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST));

	m_protoUdpServerThread->run();*/

	Tcp::MyServer* ms = new Tcp::MyServer;

	m_tcpServerThread = new Tcp::ServerThread(HostAddressPort("192.168.11.254", PORT_CONFIG_SERVICE_REQUEST), ms);

	m_tcpServerThread->start();
}


ServerMainWindow::~ServerMainWindow()
{
	m_tcpServerThread->quit();

	delete m_tcpServerThread;

	/*m_protoUdpServerThread->quit();

	delete m_protoUdpServerThread;*/

	delete ui;

	//delete m_ServiceController;
}

