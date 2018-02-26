#include "ConfigurationServiceWidget.h"
#include "TcpConfigServiceClient.h"
#include <../lib/SocketIO.h>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

ConfigurationServiceWidget::ConfigurationServiceWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, quint16 udpPort, QWidget *parent) :
	BaseServiceStateWidget(softwareInfo, udpIp, udpPort, parent)
{
	connect(this, &BaseServiceStateWidget::connectionStatisticChanged, this, &ConfigurationServiceWidget::updateStateInfo);

	setStateTabMaxRowQuantity(9);

	//----------------------------------------------------------------------------------------------------
	addClientsTab();

	//----------------------------------------------------------------------------------------------------
	QTableView* buildInfoTableView = addTabWithTableView(250, "Build Info");

	m_buildTabModel = new QStandardItemModel(1, 2, this);
	buildInfoTableView->setModel(m_buildTabModel);

	m_buildTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_buildTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_buildTabModel->setData(m_buildTabModel->index(0, 0), "Build status");
	m_buildTabModel->setData(m_buildTabModel->index(0, 1), "Not loaded");

	//----------------------------------------------------------------------------------------------------
	QTableView* settingsTableView = addTabWithTableView(250, "Settings");

	m_settingsTabModel = new QStandardItemModel(4, 2, this);
	settingsTableView->setModel(m_settingsTabModel);

	m_settingsTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_settingsTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 0), "EquipmentID");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 0), "AutoloadBuildPath");
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 0), "ClientRequestIP");
	m_settingsTabModel->setData(m_settingsTabModel->index(3, 0), "WorkDirectory");

	//----------------------------------------------------------------------------------------------------
	addTabWithTableView(250, "Log");
}

ConfigurationServiceWidget::~ConfigurationServiceWidget()
{
	dropTcpConnection();
}

void ConfigurationServiceWidget::updateStateInfo()
{
	if (m_serviceInfo.servicestate() == ServiceState::Work)
	{
		stateTabModel()->setData(stateTabModel()->index(5, 0), "Current work build directory");
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

		stateTabModel()->setData(stateTabModel()->index(8, 1), clientsTabModel()->rowCount());

		quint32 ip = m_serviceInfo.clientrequestip();
		quint16 port = m_serviceInfo.clientrequestport();
		QString address = QHostAddress(ip).toString() + QString(":%1").arg(port);

		if (ip != getWorkingClientRequestIp())
		{
			address = QHostAddress(ip).toString() + QString(":%1").arg(port) + " => " + QHostAddress(getWorkingClientRequestIp()).toString() + QString(":%1").arg(port);
		}

		m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), address);

		if (m_tcpClientSocket != nullptr)
		{
			HostAddressPort&& curAddress = m_tcpClientSocket->currentServerAddressPort();
			if (curAddress.address32() != ip || curAddress.port() != port)
			{
				dropTcpConnection();
			}
		}

		if (m_tcpClientSocket == nullptr)
		{
			createTcpConnection(getWorkingClientRequestIp(), port);
		}
	}
}

void ConfigurationServiceWidget::updateServiceState()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->serviceStateIsReady() == false)
	{
		stateTabModel()->setData(stateTabModel()->index(5, 1), "???");
		stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
		stateTabModel()->setData(stateTabModel()->index(7, 1), "???");
		stateTabModel()->setData(stateTabModel()->index(8, 1), "???");
	}

	const Network::ConfigurationServiceState& s = m_tcpClientSocket->serviceState();

	stateTabModel()->setData(stateTabModel()->index(5, 1), QString::fromStdString(s.currentbuilddirectory()));
	stateTabModel()->setData(stateTabModel()->index(6, 1), s.checkbuildattemptquantity());
	stateTabModel()->setData(stateTabModel()->index(7, 1), E::valueToString<E::ConfigCheckerState>(s.buildcheckerstate()));
}

void ConfigurationServiceWidget::updateClientsInfo()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->clientsIsReady() == false)
	{
		clientsTabModel()->setRowCount(0);
		return;
	}

	updateClientsModel(m_tcpClientSocket->clients());
}

void ConfigurationServiceWidget::updateBuildInfo()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->buildInfoIsReady() == false)
	{
		m_buildTabModel->setData(m_buildTabModel->index(0, 1), "Not loaded");
		m_buildTabModel->setRowCount(1);
		return;
	}

	const Builder::BuildInfo& b = m_tcpClientSocket->buildInfo();

	m_buildTabModel->setData(m_buildTabModel->index(0, 1), "Loaded");

	m_buildTabModel->setRowCount(8);

	m_buildTabModel->setData(m_buildTabModel->index(1, 0), "Project name");
	m_buildTabModel->setData(m_buildTabModel->index(1, 1), b.project);

	m_buildTabModel->setData(m_buildTabModel->index(2, 0), "Build type");
	m_buildTabModel->setData(m_buildTabModel->index(2, 1), b.typeStr());

	m_buildTabModel->setData(m_buildTabModel->index(3, 0), "Build No");
	m_buildTabModel->setData(m_buildTabModel->index(3, 1), b.id);

	m_buildTabModel->setData(m_buildTabModel->index(4, 0), "Build date");
	m_buildTabModel->setData(m_buildTabModel->index(4, 1), b.dateStr());

	m_buildTabModel->setData(m_buildTabModel->index(5, 0), "Changeset");
	m_buildTabModel->setData(m_buildTabModel->index(5, 1), b.changeset);

	m_buildTabModel->setData(m_buildTabModel->index(6, 0), "User name");
	m_buildTabModel->setData(m_buildTabModel->index(6, 1), b.user);

	m_buildTabModel->setData(m_buildTabModel->index(7, 0), "Workstation");
	m_buildTabModel->setData(m_buildTabModel->index(7, 1), b.workstation);
}

void ConfigurationServiceWidget::updateServiceSettings()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->settingsIsReady() == false)
	{
		for (int i = 0; i < m_settingsTabModel->rowCount(); i++)
		{
			m_settingsTabModel->setData(m_settingsTabModel->index(i, 1), "???");
		}
		return;
	}

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 1), m_tcpClientSocket->equipmentID());
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 1), m_tcpClientSocket->autoloadBuildPath());
	m_settingsTabModel->setData(m_settingsTabModel->index(3, 1), m_tcpClientSocket->workDirectory());
}

void ConfigurationServiceWidget::clearServiceData()
{
	m_buildTabModel->setData(m_buildTabModel->index(0, 1), "Not loaded");
	m_buildTabModel->setRowCount(1);

	clientsTabModel()->setRowCount(0);

	for (int i = 0; i < m_settingsTabModel->rowCount(); i++)
	{
		m_settingsTabModel->setData(m_settingsTabModel->index(i, 1), "???");
	}
}

void ConfigurationServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpConfigServiceClient(softwareInfo(), HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	connect(m_tcpClientSocket, &TcpConfigServiceClient::serviceStateLoaded, this, &ConfigurationServiceWidget::updateServiceState);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::clientsLoaded, this, &ConfigurationServiceWidget::updateClientsInfo);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::buildInfoLoaded, this, &ConfigurationServiceWidget::updateBuildInfo);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::settingsLoaded, this, &ConfigurationServiceWidget::updateServiceSettings);

	connect(m_tcpClientSocket, &TcpConfigServiceClient::disconnected, this, &ConfigurationServiceWidget::clearServiceData);

	m_tcpClientThread->start();
}

void ConfigurationServiceWidget::dropTcpConnection()
{
	m_tcpClientThread->quitAndWait();
	delete m_tcpClientThread;
	m_tcpClientThread = nullptr;

	m_tcpClientSocket = nullptr;
}
