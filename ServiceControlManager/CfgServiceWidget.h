#pragma once

#include "BaseServiceWidget.h"

class CfgServiceWidget : public BaseServiceWidget
{
	Q_OBJECT
public:
	CfgServiceWidget(ServiceTableModel* srvTableModel,
					 const SoftwareInfo& softwareInfo,
					 const ServiceData& serviceData,
					 quint32 ip, quint16 tcpPort,
					 QWidget* parent = 0);
	virtual ~CfgServiceWidget();

	virtual void initWidget() override;

public slots:
	int updateSrvStatus(int rowCount) override;
	int updateSettings(int rowCount) override;
};
