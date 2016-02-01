#ifndef BASESERVICESTATEWIDGET_H
#define BASESERVICESTATEWIDGET_H

#include <QMainWindow>
#include "../include/UdpSocket.h"
#include "../include/Service.h"

const quint32	SS_MF_UNDEFINED = 10,
SS_MF_UNAVAILABLE = 11;

class QAction;
class QLabel;

class BaseServiceStateWidget : public QMainWindow
{
	Q_OBJECT
public:
	explicit BaseServiceStateWidget(quint32 ip, int portIndex, QWidget *parent = 0);
	virtual ~BaseServiceStateWidget();

	int addTab(QWidget* page, const QString& label);

signals:

public slots:
	void updateServiceState();
	void askServiceState();

	void startService();
	void stopService();
	void restartService();

	void serviceAckReceived(const UdpRequest udpRequest);
	void serviceNotFound();

private:
	void sendCommand(int command);

	QAction* startServiceButton;
	QAction* stopServiceButton;
	QAction* restartServiceButton;

	QTimer* m_timer = nullptr;
	UdpClientSocket* m_clientSocket = nullptr;
	QLabel* m_whoIsLabel = nullptr;
	QLabel* m_uptimeLabel = nullptr;
	QLabel* m_runningLabel = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	ServiceInformation serviceState;
};

#endif // BASESERVICESTATEWIDGET_H
