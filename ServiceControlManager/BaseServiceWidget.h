#pragma once

#include <QMainWindow>

#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "ServiceTableModel.h"

class BaseServiceWidget : public QMainWindow
{
	struct Column
	{
		QString caption;
		int width = 0;
	};

	using Columns = std::vector<Column>;

	Q_OBJECT
public:
	explicit BaseServiceWidget(	ServiceTableModel* srvTableModel,
								const SoftwareInfo& softwareInfo,
								const ServiceData& service,
								quint32 udpIp, quint16 udpPort,
								QWidget* parent = nullptr);
	virtual ~BaseServiceWidget();

	int addTab(QWidget* page, const QString& label);
	QTableView* addTabWithTableView(int defaultSectionSize, const QString& label);

	QTableView* createTableView(QStandardItemModel* model,
								const Columns& columns);

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

	void updateClients();

	void updateColumnsWidth(QStandardItemModel* model);

	QString getRunningStateStr() const;

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

	void closeEvent(QCloseEvent *event) override;

private:
	void addGeneralTab();
	void addClientsTab();

	void clearServiceData();

	void onSectionResized(int index, int oldSize, int newSize);

protected:
	UdpSocketThread* m_udpSocketThread = nullptr;

	quint32 m_udpIp = 0;
	quint16 m_udpPort = 0;

	ServiceData m_serviceData;
	SoftwareInfo m_softwareInfo;

private:
	void sendCommand(int command);

protected:
	QStandardItemModel* m_srvStatusModel = nullptr;
	QStandardItemModel* m_buildInfoModel = nullptr;
	QStandardItemModel* m_settingsModel = nullptr;
	QStandardItemModel* m_clientsModel = nullptr;

private:
	ServiceTableModel* m_srvTableModel = nullptr;

	int m_udpAckQuantity = 0;

	QAction* m_startServiceButton = nullptr;
	QAction* m_stopServiceButton = nullptr;
	QAction* m_restartServiceButton = nullptr;

	QTimer* m_timer = nullptr;
	UdpClientSocket* m_udpSocket = nullptr;
	QLabel* m_connectionStateStatus = nullptr;
	QLabel* m_uptimeStatus = nullptr;
	QLabel* m_runningStatus = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	std::map<QStandardItemModel*, std::pair<QTableView*, Columns>> m_modelTableViewColumns;

	inline static const Columns propValueColumns =
	{
		{"Property", 200},
		{"Value", 300},
	};

	inline static const Columns clientTabColumns =
	{
		{"Request IP", 120},
		{"EquipmentID", 300},
		{"Client IP", 120},
		{"Software", 150},
		{"Connection time", 150},
		{"Packet counter", 150},
	};
};
