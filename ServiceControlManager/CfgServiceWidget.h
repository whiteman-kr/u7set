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
					 quint32 ip, quint16 tcpPort,
					 QWidget* parent = 0);
	virtual ~CfgServiceWidget();

public slots:
	int updateSrvStatus(int rowCount) override;
	int updateSettings(int rowCount) override;
};
