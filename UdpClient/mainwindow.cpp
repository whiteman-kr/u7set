#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	UdpClientSocket* m_clientSocket = new UdpClientSocket(QHostAddress("127.0.0.1"), PORT_BASE_SERVICE);

	connect(this, &MainWindow::clientSendRequest, m_clientSocket, &UdpClientSocket::sendRequest);

	m_clientSocketThread.run(m_clientSocket);

	m_ServiceController = new BaseServiceController(STP_CONFIG);
}

MainWindow::~MainWindow()
{
	delete ui;

	delete m_ServiceController;
}


void MainWindow::on_pushButton_clicked()
{
	UdpRequest request;

	request.setID(RQID_GET_SERVICE_INFO);

	emit clientSendRequest(request);
}

void MainWindow::on_sendFileButton_clicked()
{
	emit m_ServiceController->sendFile(QHostAddress("127.0.0.1"), PORT_BASE_SERVICE, "d:/base.css");
}
