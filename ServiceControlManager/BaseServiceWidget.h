#pragma once

#include <QMainWindow>

#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "ServiceTableModel.h"

class BaseServiceWidget : public QMainWindow
{
	Q_OBJECT
public:
	explicit BaseServiceWidget(const SoftwareInfo& softwareInfo,
									const ServiceData& service,
									quint32 udpIp, quint16 udpPort,
									QWidget* parent = nullptr);

	virtual ~BaseServiceWidget();

	int addTab(QWidget* page, const QString& label);
	QTableView* addTabWithTableView(int defaultSectionSize, const QString& label);
	QTableView* createTableView(QStandardItemModel* model, int defaultSectionSize,
								const QStringList& columns = QStringList());
	void addClientsTab(bool showStateColumn = true);
	QStandardItemModel* clientsTabModel() { return m_clientsTabModel; }

	void setClientQuantityRowIndexOnStateTab(int index) { m_clientQuantityRowIndex = index; }

	HostAddressPort getWorkingClientRequestIp();

	const SoftwareInfo& softwareInfo() { return m_softwareInfo; }

signals:
	void invalidateServiceData();

public slots:
	void updateSrvStatusWidgets();
	void updateWindowTitle();
	void updateSrvControlButtons();
	void updateSrvStatus();
	void updateBuildInfo();
	void updateStatusBar();

	void updateBaseSettings();
	virtual int updateSettings(int rowCount);

	QString getRunningStateStr() const;

	void updateClientsModel(const Network::ServiceClients& serviceClients);
	void askServiceState();

	void startService();
	void stopService();
	void restartService();

	void serviceAckReceived(const UdpRequest udpRequest);
	void serviceAckTimeout();

protected:
	virtual void createTcpConnection(quint32 ip, quint16 port);
	virtual void dropTcpConnection();

	QString rqCtrlInfoStr(const RqCtrlSettings& rcs);

private:
	void addGeneralTab();
	void addParametersTab();

	void clearServiceData();

protected:
	UdpSocketThread* m_udpSocketThread = nullptr;

	quint32 m_udpIp = 0;
	quint16 m_udpPort = 0;

	ServiceData m_serviceData;
	SoftwareInfo m_softwareInfo;

private:
	void sendCommand(int command);

protected:
	QStandardItemModel* m_settingsModel = nullptr;
	QStandardItemModel* m_paramModel = nullptr;

private:
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

	int m_clientQuantityRowIndex = -1;

	QStandardItemModel* m_srvStatusModel = nullptr;
	QStandardItemModel* m_buildInfoModel = nullptr;

	QStandardItemModel* m_clientsTabModel = nullptr;

	static const int SS_ROW_CONNECTED = 0;
	static const int SS_ROW_UPTIME = 1;
	static const int SS_ROW_RUNNING_STATE = 2;
	static const int SS_ROW_RUNNING_TIME = 3;
};

