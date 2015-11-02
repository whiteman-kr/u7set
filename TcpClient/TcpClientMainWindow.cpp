#include "TcpClientMainWindow.h"
#include "ui_TcpClientMainWindow.h"


TcpClientMainWindow::TcpClientMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TcpClientMainWindow)
{
	ui->setupUi(this);

	m_testClient = new TestClient(HostAddressPort("192.168.11.254", PORT_CONFIG_SERVICE_REQUEST),
								  HostAddressPort("127.0.0.1", PORT_CONFIG_SERVICE_REQUEST));
	m_tcpThread = new Tcp::Thread(m_testClient);

	connect(m_testClient, &TestClient::signal_changeCount, this, &TcpClientMainWindow::slot_changeCount);

	m_tcpThread->start();

}

TcpClientMainWindow::~TcpClientMainWindow()
{
	m_tcpThread->quit();

	delete m_tcpThread;
	delete ui;
}


void TcpClientMainWindow::slot_changeCount(int count)
{
	ui->label->setText(QString("%1").arg(count));
}


TestClient::TestClient(const HostAddressPort &serverAddressPort1, const HostAddressPort &serverAddressPort2) :
	Tcp::Client(serverAddressPort1, serverAddressPort2)
{
	m_data = new char[Tcp::TCP_MAX_DATA_SIZE];
}


TestClient::~TestClient()
{
	delete [] m_data;
}


void TestClient::onConnection()
{
	if (!isConnected())
	{
		return;
	}

	sendRandomRequest();
}


void TestClient::sendRandomRequest()
{
	int randomDataSize = qrand();

	if (randomDataSize > Tcp::TCP_MAX_DATA_SIZE)
	{
		randomDataSize = Tcp::TCP_MAX_DATA_SIZE;
	}

	sendRequest(1, m_data, /*randomDataSize*/ Tcp::TCP_MAX_DATA_SIZE/40);

	count++;

	if ((count % 100) == 0)
	{
		emit signal_changeCount(count);
	}
}


void TestClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	sendRandomRequest();
}
