#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ServerSocket* m_serverSocket = new ServerSocket(QHostAddress("192.168.14.85"), 4000);

    m_socketThread.run(m_serverSocket);
}

MainWindow::~MainWindow()
{
    delete ui;
}
