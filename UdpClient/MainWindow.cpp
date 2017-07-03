#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../lib/DataSource.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	/*UdpClientSocket* m_clientSocket = new UdpClientSocket(QHostAddress("127.0.0.1"), PORT_DATA_AQUISITION_SERVICE_INFO);

	connect(this, &MainWindow::clientSendRequest, m_clientSocket, &UdpClientSocket::sendRequest);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &MainWindow::onAckReceived);

	m_clientSocketThread.run(m_clientSocket);

	m_ServiceController = new BaseServiceController(STP_CONFIG, new MainFunctionWorker());*/

	/*m_protoUdpClientThread = new ProtoUdp::ClientThread(HostAddressPort("192.168.75.85", PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST));

	m_protoUdpClientThread->start();*/

	/*m_tcpClientThread = new Tcp::ClientThread<MyClient>(HostAddressPort("192.168.11.254", PORT_CONFIG_SERVICE_REQUEST));

	m_tcpClientThread->start();*/

//	runFscDataSources();

	Tuning::TcpTuningClient* tuningClient = new Tuning::TcpTuningClient(HostAddressPort("192.168.14.87", 13333), "");

	m_thread = new SimpleThread(tuningClient);

	m_thread->start();
}


MainWindow::~MainWindow()
{
	m_thread->quitAndWait();

	delete m_thread;

	/*m_tcpClientThread->quit();

	delete m_tcpClientThread;*/

	/*m_protoUdpClientThread->quit();
	delete m_protoUdpClientThread;*/

//	stopFscDataSources();
	delete ui;
//	delete m_ServiceController;
}
/*

void MainWindow::runFscDataSources()
{
	m_fscDataSources.append(new FscDataSource(HostAddressPort("192.168.11.254", PORT_DATA_AQUISITION), QHostAddress("192,168.11.1"), 10, 3));
	m_fscDataSources.append(new FscDataSource(HostAddressPort("192.168.11.254", PORT_DATA_AQUISITION), QHostAddress("192,168.11.2"), 10, 2));
}


void MainWindow::stopFscDataSources()
{
	foreach(FscDataSource* source, m_fscDataSources)
	{
		delete source;
	}

	m_fscDataSources.clear();
}
*/

void MainWindow::on_pushButton_clicked()
{
	/*UdpRequest request;

	request.setID(RQID_GET_DATA_SOURCES_IDS);

	emit clientSendRequest(request);*/

	QByteArray qq;

	qq.resize(100);

	//m_protoUdpClientThread->sendRequest(22, qq);
}

void MainWindow::on_sendFileButton_clicked()
{
//	emit m_ServiceController->sendFile(QHostAddress("127.0.0.1"), PORT_BASE_SERVICE, "d:/qqq.pdf");
}


void MainWindow::onAckReceived(UdpRequest udpRequest)
{
	UdpRequest getInfoRequest;

	quint32 count = 0;

	switch(udpRequest.ID())
	{
	case RQID_GET_DATA_SOURCES_IDS:


		getInfoRequest.setID(RQID_GET_DATA_SOURCES_INFO);

		count = udpRequest.readDword();

		getInfoRequest.writeDword(count);

		for(quint32 i = 0; i < count; i++)
		{
			quint32 sourceID = udpRequest.readDword();

			qDebug() << "Src ID = " << sourceID;

			getInfoRequest.writeDword(sourceID);
		}

		emit clientSendRequest(getInfoRequest);

		break;

	case RQID_GET_DATA_SOURCES_INFO:

		count = udpRequest.readDword();

		for(quint32 i = 0; i < count; i++)
		{
			DataSourceInfo dsi;

			udpRequest.readStruct(&dsi);

			qDebug() << "Src Name = " << QString(reinterpret_cast<QChar*>(dsi.name)) << " IP = " <<
						QHostAddress(dsi.ip).toString() << "part count = " << dsi.partCount;

		}

		break;
	}
}



