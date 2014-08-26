#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_clientSocket(nullptr)
{
    ui->setupUi(this);

    m_clientSocket = new ClientSocket(QHostAddress("192.168.14.85"), 4000);

    connect(this, &MainWindow::clientSendRequest, m_clientSocket, &UdpClientSocket::sendRequest);

    m_clientSocketThread.runClientSocket(m_clientSocket);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    emit clientSendRequest(1, 0, 0);
}
