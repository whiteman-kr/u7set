#include "ArchiveServiceWidget.h"

ArchiveServiceWidget::ArchiveServiceWidget(ServiceTableModel* srvTableModel,
	const SoftwareInfo& softwareInfo,
	const ServiceData& serviceData,
	quint32 ip, quint16 tcpPort,
	QWidget* parent) :
	BaseServiceWidget(srvTableModel, softwareInfo, serviceData, ip, tcpPort, parent)
{
}

ArchiveServiceWidget::~ArchiveServiceWidget()
{
}

void ArchiveServiceWidget::initWidget()
{
	addGeneralTab();
	addClientsTab();
}

int ArchiveServiceWidget::updateSrvStatus(int rowCount)
{
/*	m_srvStatusModel->setData(m_srvStatusModel->index(rowCount, 0), QStringLiteral("CfgCheckerState"));
	m_srvStatusModel->setData(m_srvStatusModel->index(rowCount, 1),
							  E::valueToString(static_cast<E::ConfigCheckerState>(m_serviceData.protoServiceInfo.cfgcheckerstate())));
	rowCount++;*/

	return rowCount;
}

int ArchiveServiceWidget::updateSettings(int rowCount)
{
	if (m_serviceData.settings == nullptr)
	{
		return rowCount;
	}

	std::shared_ptr<ArchivingServiceSettings> st = std::dynamic_pointer_cast<ArchivingServiceSettings>(m_serviceData.settings);

	TEST_PTR_RETURN_VALUE(st, rowCount);

	const Network::ServiceInfo& protoInfo = m_serviceData.protoServiceInfo;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceEquipmentID1);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->cfgServiceID1);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceIP1);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->cfgServiceID1.isEmpty() ? Separator::EMPTY_STR :
								 HostAddressPort(protoInfo.cfgserviceip1(), protoInfo.cfgserviceport1()).toString());
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceEquipmentID2);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->cfgServiceID2);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceIP2);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->cfgServiceID2.isEmpty() ? Separator::EMPTY_STR :
								 HostAddressPort(protoInfo.cfgserviceip2(), protoInfo.cfgserviceport2()).toString());
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("AppDataReceivingIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 QString("%1, Netmask = %2").
							 arg(st->appDataReceivingIP.toString()).arg(st->appDataReceivingNetmask.toString()));
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("DiagDataReceivingIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 QString("%1, Netmask = %2").
							 arg(st->diagDataReceivingIP.toString()).arg(st->diagDataReceivingNetmask.toString()));
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ClientRequestIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 QString("%1, Netmask = %2").
							 arg(st->clientRequestIP.toString()).arg(st->clientRequestNetmask.toString()));
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("SecurityLevel"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), E::valueToString(st->securityLevel));

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ShortTermArchivePeriod (days)"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->shortTermArchivePeriod);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("LongTermArchivePeriod (days)"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->longTermArchivePeriod);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ArchiveLocation"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->archiveLocation);

	rowCount++;

	return rowCount;
}
