#ifndef SERVERMAINWINDOW_H
#define SERVERMAINWINDOW_H

#include <QMainWindow>
#include "../include/BaseService.h"
#include "../include/ProtoUdp.h"

namespace Ui {
class ServerMainWindow;
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
};

#endif // SERVERMAINWINDOW_H
