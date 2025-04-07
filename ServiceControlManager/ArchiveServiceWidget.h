#pragma once

#pragma once

#include "BaseServiceWidget.h"

class ArchiveServiceWidget : public BaseServiceWidget
{
	Q_OBJECT
public:
	ArchiveServiceWidget(ServiceTableModel* srvTableModel,
					 const SoftwareInfo& softwareInfo,
					 const ServiceData& serviceData,
					 quint32 ip, quint16 tcpPort,
					 QWidget* parent = 0);
	virtual ~ArchiveServiceWidget();

	virtual void initWidget() override;

public slots:
	int updateSrvStatus(int rowCount) override;
	int updateSettings(int rowCount) override;
};
