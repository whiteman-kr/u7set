#include "CfgServiceWidget.h"
#include "ScmServiceClient.h"
#include "../OnlineLib/SocketIO.h"
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

CfgServiceWidget::CfgServiceWidget(	ServiceTableModel* srvTableModel,
									const SoftwareInfo& softwareInfo,
									const ServiceData& service,
									quint32 udpIp, quint16 udpPort,
									QWidget *parent) :
	BaseServiceWidget(srvTableModel, softwareInfo, service, udpIp, udpPort, parent)
{
}

CfgServiceWidget::~CfgServiceWidget()
{
	dropTcpConnection();
}


int CfgServiceWidget::updateSettings(int rowCount)
{
	if (m_serviceData.settings == nullptr)
	{
		m_settingsModel->setRowCount(0);
		return 0;
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

void CfgServiceWidget::clearServiceData()
{
//	clientsTabModel()->setRowCount(0);

	// for (int i = 0; i < m_settingsTabModel->rowCount(); i++)
	// {
	// 	m_parametersTabModel->setData(m_parametersTabModel->index(i, 1), "???");
	// }
}
/*
void CfgServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new ScmServiceClient(softwareInfo(), HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	connect(m_tcpClientSocket, &ScmServiceClient::serviceStateLoaded, this, &CfgServiceWidget::updateSrvStatus);
	connect(m_tcpClientSocket, &ScmServiceClient::clientsLoaded, this, &CfgServiceWidget::updateClientsInfo);
//	connect(m_tcpClientSocket, &TcpConfigServiceClient::buildInfoLoaded, this, &ConfigurationServiceWidget::updateBuildInfo);
	connect(m_tcpClientSocket, &ScmServiceClient::settingsLoaded, this, &CfgServiceWidget::updateServiceParameters);

	connect(m_tcpClientSocket, &ScmServiceClient::socketDisconnected, this, &CfgServiceWidget::clearServiceData);

	m_tcpClientThread->start();
}

void CfgServiceWidget::dropTcpConnection()
{
	if (m_tcpClientThread != nullptr)
	{
		m_tcpClientThread->quitAndWait();
		delete m_tcpClientThread;
		m_tcpClientThread = nullptr;
	}

	m_tcpClientSocket = nullptr;
}*/
