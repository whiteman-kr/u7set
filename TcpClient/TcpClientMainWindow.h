#ifndef TCPCLIENTMAINWINDOW_H
#define TCPCLIENTMAINWINDOW_H

#include <QMainWindow>
#include "../lib/Tcp.h"
#include "../lib/TcpFileTransfer.h"
#include "../lib/CfgServerLoader.h"

#include "../AppDataService/TcpAppDataClient.h"

namespace Ui {
class TcpClientMainWindow;
}

class TestClient : public Tcp::Client
{
	Q_OBJECT
private:
	char* m_data = nullptr;
	int count = 0;

public:
	TestClient(const HostAddressPort &serverAddressPort1, const HostAddressPort &serverAddressPort2);
	~TestClient();

	void sendRandomRequest();

signals:
	void signal_changeCount(int count);

private:
	virtual void onConnection() override;
	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;
};


class TcpClientMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TcpClientMainWindow(QWidget *parent = 0);
	~TcpClientMainWindow();

	void slot_changeCount(int count);

private slots:
	void on_pushButton_clicked();
	void onEndDownloadFile(const QString fileName, Tcp::FileTransferResult errorCode);


	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	QString dnFile = "/SYSTEMID_RACKID_WS00_DACQSERVICE/equipment.xml";
	Ui::TcpClientMainWindow *ui;

	TestClient* m_testClient = nullptr;
	Tcp::Thread* m_tcpThread = nullptr;

	Tcp::FileClient* m_fileClient = nullptr;
	Tcp::Thread* m_fileClientThread = nullptr;

	SimpleThread* m_appDataClientTread = nullptr;

	CfgLoader* m_cfgLoader = nullptr;
};

#endif // TCPCLIENTMAINWINDOW_H
