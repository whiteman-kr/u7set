#include "CfgServiceWidget.h"
#include "TcpConfigServiceClient.h"
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

void CfgServiceWidget::updateStateInfo()
{
	if (m_serviceData.serviceState() == E::ServiceState::Work)
	{
/*		stateTabModel()->setData(stateTabModel()->index(5, 0), "Current work build directory");
		stateTabModel()->setData(stateTabModel()->index(6, 0), "Check build attempt quantity");
		stateTabModel()->setData(stateTabModel()->index(7, 0), "Status of build updating");
		stateTabModel()->setData(stateTabModel()->index(8, 0), "Connected client quantity");

		if (m_tcpClientSocket == nullptr || m_tcpClientSocket->serviceStateIsReady() == false)
		{
			stateTabModel()->setData(stateTabModel()->index(5, 1), "???");
			stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
			stateTabModel()->setData(stateTabModel()->index(7, 1), "???");
			stateTabModel()->setData(stateTabModel()->index(8, 1), "???");
		}

		stateTabModel()->setData(stateTabModel()->index(8, 1), clientsTabModel()->rowCount());*/

		HostAddressPort workingIp = getWorkingClientRequestIp();

		m_parametersTabModel->setData(m_parametersTabModel->index(2, 1), workingIp.addressPortStr());

		if (m_tcpClientSocket != nullptr)
		{
			HostAddressPort&& curAddress = m_tcpClientSocket->currentServerAddressPort();

			if (curAddress != workingIp)
			{
				dropTcpConnection();
			}
		}

		if (m_tcpClientSocket == nullptr)
		{
			createTcpConnection(workingIp.address32(), workingIp.port());
		}
	}
}

void CfgServiceWidget::updateClientsInfo()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->clientsIsReady() == false)
	{
//		clientsTabModel()->setRowCount(0);
		return;
	}

//	updateClientsModel(m_tcpClientSocket->clients());
}

void CfgServiceWidget::updateServiceParameters()
{
/*	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->settingsIsReady() == false)
	{
		for (int i = 0; i < m_parametersTabModel->rowCount(); i++)
		{
			m_parametersTabModel->setData(m_parametersTabModel->index(i, 1), "???");
		}
		return;
	}

	m_parametersTabModel->setData(m_parametersTabModel->index(0, 1), m_tcpClientSocket->equipmentID());
	m_parametersTabModel->setData(m_parametersTabModel->index(1, 1), m_tcpClientSocket->autoloadBuildPath());
	m_parametersTabModel->setData(m_parametersTabModel->index(3, 1), m_tcpClientSocket->workDirectory());*/
}

int CfgServiceWidget::updateSettings(int rowCount)
{
	QStandardItemModel* sm = m_settingsModel;

	if (m_serviceData.settings == nullptr)
	{
		sm->setRowCount(0);
		return 0;
	}

	std::shared_ptr<CfgServiceSettings> st = std::dynamic_pointer_cast<CfgServiceSettings>(m_serviceData.settings);

	TEST_PTR_RETURN_VALUE(st, 0);

	sm->setData(sm->index(rowCount, 0), QStringLiteral("Check hostname"));
	sm->setData(sm->index(rowCount, 1), st->checkHostname ? QStringLiteral("True") : QStringLiteral("False"));

	rowCount++;

	for(const RqCtrlSettings& rcs : st->rcSettings)
	{
		sm->setData(sm->index(rowCount, 0), QString("Request Controller %1").arg(rcs.ID()));
		sm->setData(sm->index(rowCount, 1), rqCtrlInfoStr(rcs));
		rowCount++;
	}

	sm->setData(sm->index(rowCount, 0), QStringLiteral("Auto load build path"));
	sm->setData(sm->index(rowCount, 1), QString::fromStdString(m_serviceData.protoServiceInfo.autoloadbuildpath()));

	rowCount++;

	sm->setData(sm->index(rowCount, 0), QStringLiteral("Work directory"));
	sm->setData(sm->index(rowCount, 1), QString::fromStdString(m_serviceData.protoServiceInfo.workdirectory()));

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

void CfgServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpConfigServiceClient(softwareInfo(), HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	connect(m_tcpClientSocket, &TcpConfigServiceClient::serviceStateLoaded, this, &CfgServiceWidget::updateSrvStatus);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::clientsLoaded, this, &CfgServiceWidget::updateClientsInfo);
//	connect(m_tcpClientSocket, &TcpConfigServiceClient::buildInfoLoaded, this, &ConfigurationServiceWidget::updateBuildInfo);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::settingsLoaded, this, &CfgServiceWidget::updateServiceParameters);

	connect(m_tcpClientSocket, &TcpConfigServiceClient::socketDisconnected, this, &CfgServiceWidget::clearServiceData);

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
}
