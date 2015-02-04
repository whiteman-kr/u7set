#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../include/DataSource.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	UdpClientSocket* m_clientSocket = new UdpClientSocket(QHostAddress("127.0.0.1"), PORT_DATA_AQUISITION_SERVICE_INFO);

	connect(this, &MainWindow::clientSendRequest, m_clientSocket, &UdpClientSocket::sendRequest);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &MainWindow::onAckReceived);

	m_clientSocketThread.run(m_clientSocket);

	m_ServiceController = new BaseServiceController(STP_CONFIG, new MainFunctionWorker());
}

MainWindow::~MainWindow()
{
	delete ui;

	delete m_ServiceController;
}


void MainWindow::on_pushButton_clicked()
{
	UdpRequest request;

	request.setID(RQID_GET_DATA_SOURCES_IDS);

	emit clientSendRequest(request);
}

void MainWindow::on_sendFileButton_clicked()
{
	emit m_ServiceController->sendFile(QHostAddress("127.0.0.1"), PORT_BASE_SERVICE, "d:/qqq.pdf");
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
