#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_clientSocket = new ClientSocket(QHostAddress("192.168.14.85"), 4000);
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_clientSocket;
}

void MainWindow::on_pushButton_clicked()
{
}
