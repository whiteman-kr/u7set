#include "ConfigurationServiceWidget.h"
#include "TcpConfigServiceClient.h"
#include <../lib/SocketIO.h>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

ConfigurationServiceWidget::ConfigurationServiceWidget(quint32 ip, int portIndex, QWidget *parent) :
	BaseServiceStateWidget(ip, portIndex, parent)
{
	connect(this, &BaseServiceStateWidget::connectionStatisticChanged, this, &ConfigurationServiceWidget::updateStateInfo);

	QTableView* stateTableView = new QTableView(this);

	stateTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(stateTableView->fontMetrics().height() * 1.4));
	stateTableView->verticalHeader()->hide();

	stateTableView->horizontalHeader()->setDefaultSectionSize(250);
	stateTableView->horizontalHeader()->setStretchLastSection(true);

	stateTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	stateTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	stateTableView->setAlternatingRowColors(true);

	m_stateTabModel = new QStandardItemModel(1, 2, this);
	stateTableView->setModel(m_stateTabModel);

	m_stateTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_stateTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_stateTabModel->setData(m_stateTabModel->index(0, 0), "Connected to service");
	m_stateTabModel->setData(m_stateTabModel->index(0, 1), "No");

	addTab(stateTableView, "State");

	QTableView* clientsTableView = new QTableView(this);

	clientsTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(clientsTableView->fontMetrics().height() * 1.4));
	clientsTableView->verticalHeader()->hide();

	clientsTableView->horizontalHeader()->setDefaultSectionSize(150);
	clientsTableView->horizontalHeader()->setStretchLastSection(true);

	clientsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	clientsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	clientsTableView->setAlternatingRowColors(true);

	m_clientsTabModel = new QStandardItemModel(0, 7, this);
	clientsTableView->setModel(m_clientsTabModel);

	m_clientsTabModel->setHeaderData(0, Qt::Horizontal, "Software type");
	m_clientsTabModel->setHeaderData(1, Qt::Horizontal, "Equipment ID");
	m_clientsTabModel->setHeaderData(2, Qt::Horizontal, "IPv4");
	m_clientsTabModel->setHeaderData(3, Qt::Horizontal, "Connection time");
	m_clientsTabModel->setHeaderData(4, Qt::Horizontal, "Uptime");
	m_clientsTabModel->setHeaderData(5, Qt::Horizontal, "State");
	m_clientsTabModel->setHeaderData(6, Qt::Horizontal, "Packet counter");

	addTab(clientsTableView, "Clients");

	QTableView* buildInfoTableView = new QTableView(this);

	buildInfoTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(buildInfoTableView->fontMetrics().height() * 1.4));
	buildInfoTableView->verticalHeader()->hide();

	buildInfoTableView->horizontalHeader()->setDefaultSectionSize(250);
	buildInfoTableView->horizontalHeader()->setStretchLastSection(true);

	buildInfoTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	buildInfoTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	buildInfoTableView->setAlternatingRowColors(true);

	m_buildTabModel = new QStandardItemModel(1, 2, this);
	buildInfoTableView->setModel(m_buildTabModel);

	m_buildTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_buildTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_buildTabModel->setData(m_buildTabModel->index(0, 0), "Build status");
	m_buildTabModel->setData(m_buildTabModel->index(0, 1), "Not loaded");

	addTab(buildInfoTableView, "Build Info");

	QTableView* settingsTableView = new QTableView(this);

	settingsTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(settingsTableView->fontMetrics().height() * 1.4));
	settingsTableView->verticalHeader()->hide();

	settingsTableView->horizontalHeader()->setDefaultSectionSize(250);
	settingsTableView->horizontalHeader()->setStretchLastSection(true);

	settingsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	settingsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	settingsTableView->setAlternatingRowColors(true);

	m_settingsTabModel = new QStandardItemModel(4, 2, this);
	settingsTableView->setModel(m_settingsTabModel);

	m_settingsTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_settingsTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 0), "EquipmentID");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 0), "AutoloadBuildPath");
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 0), "ClientRequestIP");
	m_settingsTabModel->setData(m_settingsTabModel->index(3, 0), "WorkDirectory");

	addTab(settingsTableView, "Settings");

	addTab(new QTableView(this), "Log");
}

ConfigurationServiceWidget::~ConfigurationServiceWidget()
{
	dropTcpConnection();
}

void ConfigurationServiceWidget::updateStateInfo()
{
	auto state = m_serviceInfo.servicestate();
	if (state != TO_INT(ServiceState::Unavailable) && state != TO_INT(ServiceState::Undefined))
	{
		m_stateTabModel->setData(m_stateTabModel->index(0, 1), "Yes");

		m_stateTabModel->setRowCount(state == ServiceState::Work ? 9 : 3);

		m_stateTabModel->setData(m_stateTabModel->index(1, 0), "Uptime");
		m_stateTabModel->setData(m_stateTabModel->index(2, 0), "Runing state");

		quint32 time = m_serviceInfo.uptime();

		int s = time % 60; time /= 60;
		int m = time % 60; time /= 60;
		int h = time % 24; time /= 24;

		m_stateTabModel->setData(m_stateTabModel->index(1, 1), QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
	}
	else
	{
		m_stateTabModel->setData(m_stateTabModel->index(0, 1), "No");
		m_stateTabModel->setRowCount(1);
		return;
	}

	QString runningStateStr;

	switch (state)
	{
		case ServiceState::Work:
			{
				runningStateStr = tr("Running");

				m_stateTabModel->setData(m_stateTabModel->index(3, 0), "Runing time");

				quint32 time = m_serviceInfo.serviceuptime();

				int s = time % 60; time /= 60;
				int m = time % 60; time /= 60;
				int h = time % 24; time /= 24;

				m_stateTabModel->setData(m_stateTabModel->index(3, 1), QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

				quint32 ip = m_serviceInfo.clientrequestip();
				quint16 port = m_serviceInfo.clientrequestport();
				QString address = QHostAddress(ip).toString() + QString(":%1").arg(port);

				m_stateTabModel->setData(m_stateTabModel->index(4, 0), "Client request address");
				m_stateTabModel->setData(m_stateTabModel->index(4, 1), address);

				m_stateTabModel->setData(m_stateTabModel->index(5, 0), "Current work build directory");
				m_stateTabModel->setData(m_stateTabModel->index(6, 0), "Check build attempt quantity");
				m_stateTabModel->setData(m_stateTabModel->index(7, 0), "Status of build updating");
				m_stateTabModel->setData(m_stateTabModel->index(8, 0), "Connected client quantity");

				if (m_tcpClientSocket == nullptr || m_tcpClientSocket->serviceStateIsReady() == false)
				{
					m_stateTabModel->setData(m_stateTabModel->index(5, 1), "???");
					m_stateTabModel->setData(m_stateTabModel->index(6, 1), "???");
					m_stateTabModel->setData(m_stateTabModel->index(7, 1), "???");
					m_stateTabModel->setData(m_stateTabModel->index(8, 1), "???");
				}

				m_stateTabModel->setData(m_stateTabModel->index(8, 1), m_clientsTabModel->rowCount());

				m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), address);

				if (m_tcpClientSocket != nullptr)
				{
					HostAddressPort&& curAddress = m_tcpClientSocket->serverAddressPort(0);
					if (curAddress.address32() != ip || curAddress.port() != port)
					{
						dropTcpConnection();
					}
				}

				if (m_tcpClientSocket == nullptr)
				{
					createTcpConnection(ip, port);
				}
			}

			break;

		case ServiceState::Stopped:
			runningStateStr = tr("Stopped");
			break;

		case ServiceState::Unavailable:
			runningStateStr = tr("Unavailable");
			break;

		case ServiceState::Undefined:
			runningStateStr = tr("Undefined");
			break;

		case ServiceState::Starts:
			runningStateStr = tr("Starts");
			break;

		case ServiceState::Stops:
			runningStateStr = tr("Stops");
			break;

		default:
			assert(false);
			runningStateStr = tr("Unknown state");
			break;
	}

	m_stateTabModel->setData(m_stateTabModel->index(2, 1), runningStateStr);
}

void ConfigurationServiceWidget::updateServiceState()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->serviceStateIsReady() == false)
	{
		m_stateTabModel->setData(m_stateTabModel->index(5, 1), "???");
		m_stateTabModel->setData(m_stateTabModel->index(6, 1), "???");
		m_stateTabModel->setData(m_stateTabModel->index(7, 1), "???");
		m_stateTabModel->setData(m_stateTabModel->index(8, 1), "???");
	}

	const Network::ConfigurationServiceState& s = m_tcpClientSocket->serviceState();

	m_stateTabModel->setData(m_stateTabModel->index(5, 1), QString::fromStdString(s.currentbuilddirectory()));
	m_stateTabModel->setData(m_stateTabModel->index(6, 1), s.checkbuildattemptquantity());
	m_stateTabModel->setData(m_stateTabModel->index(7, 1), E::valueToString<E::ConfigCheckerState>(s.buildcheckerstate()));
}

void ConfigurationServiceWidget::updateClients()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->clientsIsReady() == false)
	{
		m_clientsTabModel->setRowCount(0);
	}

	const Network::ConfigurationServiceClients& cs = m_tcpClientSocket->clients();

	m_clientsTabModel->setRowCount(cs.clients_size());
	m_stateTabModel->setData(m_stateTabModel->index(8, 1), cs.clients_size());

	for (int i = 0; i < cs.clients_size(); i++)
	{
		const Network::ConfigurationServiceClientInfo& ci = cs.clients(i);

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 0), ci.softwaretype());

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 1), QString::fromStdString(ci.equipmentid()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 2), QHostAddress(ci.ip()).toString());

		quint64 uptime = ci.uptime();

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 3), QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch() - uptime));

		int ms = uptime % 1000; uptime /= 1000;
		int s = uptime % 60; uptime /= 60;
		int m = uptime % 60; uptime /= 60;
		int h = uptime % 24; uptime /= 24;

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 4), QString("(%1d %2:%3:%4.%5)").arg(uptime).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(ms, 3, 10, QChar('0')));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 5), ci.isactual() ? "Actual" : "Non actual");

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 6), static_cast<qint64>(ci.replyquantity()));
	}
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

	m_clientsTabModel->setRowCount(0);

	for (int i = 0; i < m_settingsTabModel->rowCount(); i++)
	{
		m_settingsTabModel->setData(m_settingsTabModel->index(i, 1), "???");
	}
}

void ConfigurationServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpConfigServiceClient(HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	connect(m_tcpClientSocket, &TcpConfigServiceClient::serviceStateLoaded, this, &ConfigurationServiceWidget::updateServiceState);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::clientsLoaded, this, &ConfigurationServiceWidget::updateClients);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::buildInfoLoaded, this, &ConfigurationServiceWidget::updateBuildInfo);
	connect(m_tcpClientSocket, &TcpConfigServiceClient::settingsLoaded, this, &ConfigurationServiceWidget::updateServiceSettings);

	connect(m_tcpClientSocket, &TcpConfigServiceClient::disconnected, this, &ConfigurationServiceWidget::clearServiceData);

	m_tcpClientThread->start();
}

void ConfigurationServiceWidget::dropTcpConnection()
{
	m_tcpClientThread->quitAndWait();

	m_tcpClientSocket = nullptr;
}
