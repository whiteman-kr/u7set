#include "CfgServiceWidget.h"
#include "ScmServiceClient.h"
#include "../OnlineLib/SocketIO.h"
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

CfgServiceWidget::CfgServiceWidget(ServiceTableModel* srvTableModel,
	const SoftwareInfo& softwareInfo,
	const ServiceData& service,
	quint32 ip, quint16 tcpPort,
	QWidget *parent) :
	BaseServiceWidget(srvTableModel, softwareInfo, service, ip, tcpPort, parent)
{
}

CfgServiceWidget::~CfgServiceWidget()
{
}

void CfgServiceWidget::initWidget()
{
	addGeneralTab();
	addClientsTab();
}

int CfgServiceWidget::updateSrvStatus(int rowCount)
{
	m_srvStatusModel->setData(m_srvStatusModel->index(rowCount, 0), QStringLiteral("CfgCheckerState"));
	m_srvStatusModel->setData(m_srvStatusModel->index(rowCount, 1),
							  E::valueToString(static_cast<E::ConfigCheckerState>(m_serviceData.protoServiceInfo.cfgcheckerstate())));
	rowCount++;

	return rowCount;
}

int CfgServiceWidget::updateSettings(int rowCount)
{
	if (m_serviceData.settings == nullptr)
	{
		return rowCount;
	}

	std::shared_ptr<CfgServiceSettings> st = std::dynamic_pointer_cast<CfgServiceSettings>(m_serviceData.settings);

	TEST_PTR_RETURN_VALUE(st, 0);

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("Check hostname"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->checkHostname ? QStringLiteral("True") : QStringLiteral("False"));

	rowCount++;

	for(const RqCtrlSettings& rcs : st->rcSettings)
	{
		m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QString("Request Controller %1").arg(rcs.ID()));
		m_settingsModel->setData(m_settingsModel->index(rowCount, 1), rqCtrlInfoStr(rcs));
		rowCount++;
	}

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("Auto load build path"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), QString::fromStdString(m_serviceData.protoServiceInfo.autoloadbuildpath()));

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("Work directory"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), QString::fromStdString(m_serviceData.protoServiceInfo.workdirectory()));

	rowCount++;

	return rowCount;
}
