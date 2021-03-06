#ifndef CONFIGURATIONSERVICEWIDGET_H
#define CONFIGURATIONSERVICEWIDGET_H

class QStandardItemModel;
class TcpConfigServiceClient;

#include "../OnlineLib/Tcp.h"
#include "BaseServiceStateWidget.h"


class ConfigurationServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	ConfigurationServiceWidget(const SoftwareInfo& softwareInfo, const ServiceData& service, quint32 udpIp, quint16 udpPort, QWidget *parent = 0);
	~ConfigurationServiceWidget();

public slots:
	void updateStateInfo();
	void updateServiceState();
	void updateClientsInfo();
	void updateBuildInfo();
	void updateServiceParameters();

	void clearServiceData();

protected:
	void createTcpConnection(quint32 ip, quint16 port) override;
	void dropTcpConnection() override;

private:
	QStandardItemModel* m_buildTabModel = nullptr;
	QStandardItemModel* m_settingsTabModel = nullptr;
	QStandardItemModel* m_parametersTabModel = nullptr;
	TcpConfigServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
};

#endif // CONFIGURATIONSERVICEWIDGET_H
