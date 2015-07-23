#pragma once

#include <QMainWindow>

#include "../include/BaseService.h"
#include "../include/SocketIO.h"
#include "FscDataSource.h"
#include "ClientSocket.h"
#include "../include/ProtoUdp.h"
#include "../include/Tcp.h"

namespace Ui {
class MainWindow;
}


class MyClient : public Tcp::Client
{
	Q_OBJECT

private:
	QTimer m_timer;

	void onTimer();

public:
	MyClient();

	virtual void onSocketThreadStarted() override;

	virtual void onConnection() override;
};


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

	QVector<FscDataSource*> m_fscDataSources;

	void runFscDataSources();
	void stopFscDataSources();

	//	ProtoUdp::ClientThread* m_protoUdpClientThread = nullptr;

	Tcp::ClientThread<MyClient>* m_tcpClientThread;
};


