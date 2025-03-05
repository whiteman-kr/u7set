#pragma once

class QStandardItemModel;
class TcpConfigServiceClient;

#include "../OnlineLib/Tcp.h"
#include "BaseServiceWidget.h"

class CfigServiceWidget : public BaseServiceWidget
{
	Q_OBJECT
public:
	CfigServiceWidget(	const SoftwareInfo& softwareInfo,
								const ServiceData& service,
								quint32 udpIp, quint16 udpPort,
								QWidget *parent = 0);
	~CfigServiceWidget();

public slots:
	void updateStateInfo();
	void updateClientsInfo();
	void updateServiceParameters();

	int updateSettings(int rowCount) override;

	void clearServiceData();

protected:
	void createTcpConnection(quint32 ip, quint16 port) override;
	void dropTcpConnection() override;

private:
	QStandardItemModel* m_settingsTabModel = nullptr;
	QStandardItemModel* m_parametersTabModel = nullptr;
	TcpConfigServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
};
