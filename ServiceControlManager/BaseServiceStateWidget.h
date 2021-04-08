#ifndef BASESERVICESTATEWIDGET_H
#define BASESERVICESTATEWIDGET_H

#include <QMainWindow>
#include "../OnlineLib/UdpSocket.h"
#include "../ServiceLib/Service.h"
#include "../OnlineLib/Tcp.h"
#include "ServiceTableModel.h"

class QAction;
class QLabel;
class QTableView;
class QStandardItemModel;

class BaseServiceStateWidget : public QMainWindow
{
	Q_OBJECT
public:
	explicit BaseServiceStateWidget(const SoftwareInfo& softwareInfo, const ServiceData& service, quint32 udpIp, quint16 udpPort, QWidget* parent = nullptr);
	virtual ~BaseServiceStateWidget();

	int addTab(QWidget* page, const QString& label);
	QTableView* addTabWithTableView(int defaultSectionSize, const QString& label);
	void addStateTab();
	void addClientsTab(bool showStateColumn = true);
	QStandardItemModel* stateTabModel() { return m_stateTabModel; }
	QStandardItemModel* clientsTabModel() { return m_clientsTabModel; }

	void setStateTabMaxRowQuantity(int rowQuantity) { m_stateTabMaxRowQuantity = rowQuantity; }
	void setClientQuantityRowIndexOnStateTab(int index) { m_clientQuantityRowIndex = index; }

	quint32 getWorkingClientRequestIp();

	const SoftwareInfo& softwareInfo() { return m_softwareInfo; }

signals:
	void needToReloadData();
	void invalidateData();
	void connectionStatisticChanged();

public slots:
	void updateServiceState();
	void updateClientsModel(const Network::ServiceClients& serviceClients);
	void askServiceState();

	void startService();
	void stopService();
	void restartService();

	void serviceAckReceived(const UdpRequest udpRequest);
	void serviceNotFound();

protected:
	virtual void createTcpConnection(quint32 ip, quint16 port);
	virtual void dropTcpConnection() {}

	UdpSocketThread* m_socketThread = nullptr;

	quint32 m_udpIp = 0;
	int m_udpPort = -1;

	ServiceData m_service;

	SoftwareInfo m_softwareInfo;

private:
	void sendCommand(int command);

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

	int m_stateTabMaxRowQuantity = 5;
	int m_clientQuantityRowIndex = -1;
	QStandardItemModel* m_stateTabModel = nullptr;
	QStandardItemModel* m_clientsTabModel = nullptr;
};

#endif // BASESERVICESTATEWIDGET_H
