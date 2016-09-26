#pragma once

#include <QMainWindow>

#include "../lib/Service.h"
#include "../lib/SocketIO.h"
#include "FscDataSource.h"
#include "ClientSocket.h"
#include "../lib/ProtoUdp.h"
#include "../lib/Tcp.h"
#include "../TuningService/TcpTuningClient.h"


namespace Ui {
class MainWindow;
}

/*
class MyClient : public Tcp::Client
{
	Q_OBJECT

private:
	QTimer m_timer;
	QByteArray m_data;

	void onTimer();

//	void sendRandomRequest();

public:
	MyClient();

	virtual void onClientThreadStarted() override;

	virtual void onConnection() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;
};*/


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

	SimpleThread* m_thread = nullptr;


/*    UdpSocketThread m_clientSocketThread;

	BaseServiceController* m_ServiceController = nullptr;

	QVector<FscDataSource*> m_fscDataSources;

	void runFscDataSources();
	void stopFscDataSources();

	//	ProtoUdp::ClientThread* m_protoUdpClientThread = nullptr;*/

	//Tcp::ClientThread* m_tcpClientThread;
};


