#ifndef SERVERMAINWINDOW_H
#define SERVERMAINWINDOW_H

#include <QMainWindow>
#include "../include/BaseService.h"

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
};

#endif // SERVERMAINWINDOW_H
