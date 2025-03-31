#pragma once

class QStandardItemModel;
class ScmServiceClient;

#include "../OnlineLib/Tcp.h"
#include "BaseServiceWidget.h"

class CfgServiceWidget : public BaseServiceWidget
{
	Q_OBJECT
public:
	CfgServiceWidget(ServiceTableModel* srvTableModel,
					const SoftwareInfo& softwareInfo,
					const ServiceData& service,
					quint32 udpIp, quint16 udpPort,
					QWidget* parent = 0);
	~CfgServiceWidget();

public slots:
	int updateSettings(int rowCount) override;

	void clearServiceData();

private:
	QStandardItemModel* m_settingsTabModel = nullptr;
	QStandardItemModel* m_parametersTabModel = nullptr;
	ScmServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
};
