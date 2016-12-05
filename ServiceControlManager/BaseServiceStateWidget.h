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

	QAction* startServiceButton;
	QAction* stopServiceButton;
	QAction* restartServiceButton;

	QTimer* m_timer = nullptr;
	UdpClientSocket* m_baseClientSocket = nullptr;
	QLabel* m_whoIsLabel = nullptr;
	QLabel* m_uptimeLabel = nullptr;
	QLabel* m_runningLabel = nullptr;
	QLabel* m_clientRequestAddressLabel = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	Network::ServiceInfo serviceState;
};

#endif // BASESERVICESTATEWIDGET_H
