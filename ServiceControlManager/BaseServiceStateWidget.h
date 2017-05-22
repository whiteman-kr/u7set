#ifndef BASESERVICESTATEWIDGET_H
#define BASESERVICESTATEWIDGET_H

#include <QMainWindow>
#include "../lib/UdpSocket.h"
#include "../lib/Service.h"

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
	void needToReloadData();
	void invalidateData();

public slots:
	void updateServiceState();
	void askServiceState();

	void startService();
	void stopService();
	void restartService();

	void serviceAckReceived(const UdpRequest udpRequest);
	void serviceNotFound();

protected:
	UdpSocketThread* m_socketThread = nullptr;

private:
	void sendCommand(int command);

	quint32 m_ip = 0;
	int m_portIndex = 0;
	int m_udpAckQuantity = 0;

	QAction* m_startServiceButton;
	QAction* m_stopServiceButton;
	QAction* m_restartServiceButton;

	QTimer* m_timer = nullptr;
	UdpClientSocket* m_baseClientSocket = nullptr;
	QLabel* m_connectionStateStatus = nullptr;
	QLabel* m_uptimeStatus = nullptr;
	QLabel* m_runningStatus = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	Network::ServiceInfo m_serviceInfo;
};

#endif // BASESERVICESTATEWIDGET_H
