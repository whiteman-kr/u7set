#include "TcpClientMainWindow.h"
#include "ui_TcpClientMainWindow.h"
#include "../include/OrderedHash.h"

TcpClientMainWindow::TcpClientMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TcpClientMainWindow)
{
	ui->setupUi(this);

	/*m_testClient = new TestClient(HostAddressPort("192.168.11.254", PORT_CONFIG_SERVICE_REQUEST),
								  HostAddressPort("127.0.0.1", PORT_CONFIG_SERVICE_REQUEST));
	m_tcpThread = new Tcp::Thread(m_testClient);

	connect(m_testClient, &TestClient::signal_changeCount, this, &TcpClientMainWindow::slot_changeCount);

	m_tcpThread->start();*/

	m_cfgLoader = new CfgLoader("SYSTEMID_RACKID_WS00_DACQSERVICE", 1, HostAddressPort("127.0.0.1", PORT_CONFIG_SERVICE_REQUEST), HostAddressPort("227.33.0.1", PORT_CONFIG_SERVICE_REQUEST));
	m_fileClientThread = new Tcp::Thread(m_cfgLoader);

	connect(m_cfgLoader, &CfgLoader::signal_configurationReady, this, &TcpClientMainWindow::onConfigurationReady);

	m_fileClientThread->start();

	//connect(m_fileClient, &Tcp::FileClient::signal_endDownloadFile, this, &TcpClientMainWindow::onEndDownloadFile);
}


void TcpClientMainWindow::onEndDownloadFile(const QString filename, Tcp::FileTransferResult errorCode)
{
	qDebug() << "End download file " << filename << " error = " << static_cast<int>(errorCode);
}



TcpClientMainWindow::~TcpClientMainWindow()
{
	m_fileClientThread->quit();
	delete m_fileClientThread;
//	m_tcpThread->quit();
//	delete m_tcpThread;
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


void TestClient::processReply(quint32 /*requestID*/, const char* /*replyData*/, quint32 /*replyDataSize*/)
{
	sendRandomRequest();
}


void TcpClientMainWindow::on_pushButton_clicked()
{
	QByteArray fileData;
	QString errorStr;

	bool result = m_cfgLoader->downloadCfgFile("/SYSTEMID_RACKID_WS00_DACQSERVICE/equipment.xml", &fileData, &errorStr);

	if (result == false)
	{
		qDebug() << errorStr;
	}

	result = m_cfgLoader->downloadCfgFile("/SYSTEMID_RACKID_WS00_DACQSERVICE/appSignals.xml", &fileData, &errorStr);

	if (result == false)
	{
		qDebug() << errorStr;
	}
}




void TcpClientMainWindow::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	int a = 0;
	a++;

	dnFile = buildFileInfoArray[1].pathFileName;
}
