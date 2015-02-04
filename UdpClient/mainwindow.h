#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../include/BaseService.h"
#include "ClientSocket.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
	void on_sendFileButton_clicked();

	void onAckReceived(UdpRequest udpRequest);

signals:
	void clientSendRequest(UdpRequest request);

private:
    Ui::MainWindow *ui;

    UdpSocketThread m_clientSocketThread;

	BaseServiceController* m_ServiceController = nullptr;
};

#endif // MAINWINDOW_H
