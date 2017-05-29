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

	m_stateTabModel = new QStandardItemModel(3, 2, this);
	stateTableView->setModel(m_stateTabModel);

	m_stateTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_stateTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_stateTabModel->setData(m_stateTabModel->index(0, 0), "Connected to CfgSrv");

	addTab(stateTableView, "State");
	addTab(new QTableView(this), "Clients");
	addTab(new QTableView(this), "Build Info");
	addTab(new QTableView(this), "Settings");

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

		m_stateTabModel->setRowCount(state == ServiceState::Work ? 5 /*9*/ : 3);

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
				m_stateTabModel->setData(m_stateTabModel->index(4, 0), "Client request address");
				m_stateTabModel->setData(m_stateTabModel->index(4, 1), QHostAddress(ip).toString() + QString(":%1").arg(port));

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

void ConfigurationServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpConfigServiceClient(HostAddressPort(ip, PORT_APP_DATA_SERVICE_CLIENT_REQUEST));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	m_tcpClientThread->start();
}

void ConfigurationServiceWidget::dropTcpConnection()
{
	m_tcpClientThread->quitAndWait();
}
