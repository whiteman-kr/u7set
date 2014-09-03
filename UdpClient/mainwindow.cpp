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
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    emit clientSendRequest(RQID_GET_SERVICE_INFO, 0, 0);
}
