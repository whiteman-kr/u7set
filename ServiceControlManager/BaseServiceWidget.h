#pragma once

#include <QMainWindow>

#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "ServiceTableModel.h"
#include "ScmServiceClient.h"
#include "Columns.h"

class BaseServiceWidget : public QMainWindow
{
	Q_OBJECT
public:
	explicit BaseServiceWidget(	ServiceTableModel* srvTableModel,
								const SoftwareInfo& softwareInfo,
								const ServiceData& service,
								quint32 ip, quint16 tcpPort,
								QWidget* parent = nullptr);
	virtual ~BaseServiceWidget();

	virtual void initWidget() = 0;

	int addTab(QWidget* page, const QString& label);
	QTableView* addTabWithTableView(int defaultSectionSize, const QString& label);

	QTableView* createTableView(QAbstractItemModel* model,
								const Columns& columns);

	HostAddressPort getWorkingClientRequestIp();

	const SoftwareInfo& softwareInfo() { return m_softwareInfo; }

	virtual void onServiceInfoUpdated(QByteArray replyData);

	virtual void updateDerivedWidgets(const Network::ServiceInfo& srvInfo);
	virtual void clearDerivedWidgets();

	void overrideApertures(const std::vector<ApertureRecord>& apertures);

signals:
	void invalidateServiceData();

public slots:
	void updateSrvStatusWidgets();
	void updateWindowTitle();
	void updateSrvControlButtons();

	void updateBaseSrvStatus();
	virtual int updateSrvStatus(int rowCount);

	void updateBuildInfo();
	void updateStatusBar();

	void updateBaseSettings();
	virtual int updateSettings(int rowCount);

	void updateClients();

	void updateColumnsWidth(QStandardItemModel* model);

	QString getRunningStateStr() const;

	void startService();
	void stopService();
	void restartService();

protected:
	void addGeneralTab();
	void addClientsTab();

	void createTcpConnection(quint32 ip, quint16 tcpPort);
	void dropTcpConnection();
	void onScmServiceClientDisconnected();

	QString rqCtrlInfoStr(const RqCtrlSettings& rcs);
	QString clientRequestIpInfoStr(E::SecurityLevel sLevel, const HostAddressPort ip, QHostAddress netmask);

	void closeEvent(QCloseEvent *event) override;

private:
	void clearServiceData();

	void enqueueRequest(int request);

	void onSectionResized(int index, int oldSize, int newSize);

protected:
	UdpSocketThread* m_udpSocketThread = nullptr;

	quint32 m_ip = 0;
	quint16 m_tcpPort = 0;

	ServiceData m_serviceData;
	SoftwareInfo m_softwareInfo;

private:
	void sendCommand(int command);

protected:
	QStandardItemModel* m_srvStatusModel = nullptr;
	QStandardItemModel* m_buildInfoModel = nullptr;
	QStandardItemModel* m_settingsModel = nullptr;
	QStandardItemModel* m_clientsModel = nullptr;

	inline static const QString m_cfgServiceEquipmentID1 = QStringLiteral("CfgServiceEquipmentID1");
	inline static const QString m_cfgServiceEquipmentID2 = QStringLiteral("CfgServiceEquipmentID2");

	inline static const QString m_cfgServiceIP1 = QStringLiteral("CfgServiceIP1");
	inline static const QString m_cfgServiceIP2 = QStringLiteral("CfgServiceIP2");

private:
	ServiceTableModel* m_srvTableModel = nullptr;

	int m_udpAckQuantity = 0;

	QAction* m_startServiceButton = nullptr;
	QAction* m_stopServiceButton = nullptr;
	QAction* m_restartServiceButton = nullptr;

	QTimer* m_timer = nullptr;
	QLabel* m_connectionStateStatus = nullptr;
	QLabel* m_uptimeStatus = nullptr;
	QLabel* m_runningStatus = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	ScmServiceClient* m_scmSrvClient = nullptr;
	SimpleThread* m_scmSrvClientThread = nullptr;

	std::map<QAbstractItemModel*, std::pair<QTableView*, Columns>> m_modelTableViewColumns;
};
