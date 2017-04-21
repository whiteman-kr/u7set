#include "ServerMainWindow.h"
#include "ui_ServerMainWindow.h"

#include "../lib/CircularLogger.h"

namespace Tcp
{

	void MyServer::processRequest(quint32 /*requestID*/, const char* /*requestData*/, quint32 /*requestDataSize*/)
	{
		QByteArray reply;

//		currentThread->sleep(15);

		reply.resize(1024);

		sendReply(reply);

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

//	Tcp::MyServer* ms = new Tcp::MyServer;

/*	Tcp::FileServer* m_fileServer = new Tcp::FileServer("d:/temp/bbb-debug-000023");

	m_tcpServerThread = new Tcp::ServerThread(HostAddressPort("127.0.0.1", PORT_CONFIG_SERVICE_REQUEST), m_fileServer);

	m_tcpServerThread->start();*/

	//m_cfgServer = new CfgServer("d:/temp/build");

	m_logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(m_logger);

	m_cfgServer = new CfgServer("/home/serhiy/temp/udpserver", m_logger);

	m_tcpServerThread = new Tcp::ServerThread(HostAddressPort("127.0.0.1", PORT_CONFIGURATION_SERVICE_REQUEST), m_cfgServer, m_logger);

	m_tcpServerThread->start();
}


ServerMainWindow::~ServerMainWindow()
{

	m_tcpServerThread->quit();

	delete m_tcpServerThread;

	/*m_protoUdpServerThread->quit();

	delete m_protoUdpServerThread;*/

	delete ui;

	LOGGER_SHUTDOWN(m_logger);

	//delete m_ServiceController;
}

