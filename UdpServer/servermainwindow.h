#ifndef SERVERMAINWINDOW_H
#define SERVERMAINWINDOW_H

#include <QMainWindow>
#include "../include/BaseService.h"
#include "../include/ProtoUdp.h"
#include "../include/Tcp.h"
#include "../include/TcpFileTransfer.h"

namespace Ui
{
	class ServerMainWindow;
}

namespace Tcp
{
	class MyServer : public Server
	{
		Q_OBJECT

	public:
		MyServer() {}

		virtual Server* getNewInstance() override { return new MyServer; }

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;
	};
}



class ServerMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit ServerMainWindow(QWidget *parent = 0);
	~ServerMainWindow();

private:
	Ui::ServerMainWindow *ui;

	BaseServiceController* m_ServiceController = nullptr;

	ProtoUdp::ServerThread* m_protoUdpServerThread = nullptr;

	Tcp::ServerThread* m_tcpServerThread = nullptr;
};

#endif // SERVERMAINWINDOW_H
