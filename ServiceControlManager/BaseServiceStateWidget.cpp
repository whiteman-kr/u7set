#include "BaseServiceStateWidget.h"
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QToolBar>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include "../lib/Types.h"
#include "../lib/WidgetUtils.h"


BaseServiceStateWidget::BaseServiceStateWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, qint32 udpPort, QWidget *parent) :
	QMainWindow(parent),
	m_udpIp(udpIp),
	m_udpPort(udpPort),
	m_softwareInfo(softwareInfo)
{
	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	m_serviceInfo.mutable_softwareinfo()->set_softwaretype(softwareInfo.softwareType());
	m_serviceInfo.set_servicestate(TO_INT(ServiceState::Undefined));

	QToolBar* toolBar = addToolBar("Service actions");
	m_startServiceButton = toolBar->addAction("Start", this, SLOT(startService()));
	m_stopServiceButton = toolBar->addAction("Stop", this, SLOT(stopService()));
	m_restartServiceButton = toolBar->addAction("Restart", this, SLOT(restartService()));

	statusBar()->addWidget(m_connectionStateStatus = new QLabel(this));
	statusBar()->addWidget(m_uptimeStatus = new QLabel(this));
	statusBar()->addWidget(m_runningStatus = new QLabel(this));

	m_connectionStateStatus->setMargin(5);
	m_uptimeStatus->setMargin(5);
	m_runningStatus->setMargin(5);

	m_socketThread = new UdpSocketThread();

	m_baseClientSocket = new UdpClientSocket(QHostAddress(udpIp), udpPort);
	connect(m_baseClientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceStateWidget::serviceNotFound);
	connect(m_baseClientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceStateWidget::serviceAckReceived);

	m_socketThread->addWorker(m_baseClientSocket);
	m_socketThread->start();

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &BaseServiceStateWidget::askServiceState);
	m_timer->start(500);

	setWindowPosition(this, QString("Service_%1_%2/geometry").arg(QHostAddress(udpIp).toString()).arg(udpPort));

	addStateTab();

	updateServiceState();
}

BaseServiceStateWidget::~BaseServiceStateWidget()
{
	m_timer->stop();

	if (m_socketThread)
	{
		m_socketThread->quitAndWait();
		delete m_socketThread;
		m_socketThread = nullptr;
	}

	QSettings settings;
	QString settingName = QString("Service_%1_%2/geometry").arg(QHostAddress(m_udpIp).toString()).arg(m_udpPort);
	settings.setValue(settingName, geometry());
}

void BaseServiceStateWidget::updateServiceState()
{
	ServiceState serviceState = static_cast<ServiceState>(m_serviceInfo.servicestate());

	if (serviceState != TO_INT(ServiceState::Unavailable) && serviceState != TO_INT(ServiceState::Undefined))
	{
		m_stateTabModel->setData(m_stateTabModel->index(0, 1), "Yes");

		m_stateTabModel->setRowCount(serviceState == ServiceState::Work ? m_stateTabMaxRowQuantity : 3);

		m_stateTabModel->setData(m_stateTabModel->index(1, 0), "Uptime");
		m_stateTabModel->setData(m_stateTabModel->index(2, 0), "Runing state");
	}
	else
	{
		m_stateTabModel->setData(m_stateTabModel->index(0, 1), "No");
		m_stateTabModel->setRowCount(1);
		return;
	}

	QString serviceName = "Unknown Service";
	QString serviceShortName = "???";

	for (int i = 0; i < serviceInfo.count(); i++)
	{
		if (static_cast<E::SoftwareType>(m_serviceInfo.softwareinfo().softwaretype()) == serviceInfo[i].softwareType)
		{
			serviceName = serviceInfo[i].name;
			serviceShortName = serviceInfo[i].shortName;
			break;
		}
	}

	assert(serviceShortName != "???");

	switch (serviceState)
	{
		case Stopped:
		case Starts:
		case Work:
		case Stops:
			{
				const Network::SoftwareInfo& softwareInfo = m_serviceInfo.softwareinfo();
				setWindowTitle(serviceName +
							   QString(" v%1.%2.%3 - %4:%5 (%6)")
							   .arg(softwareInfo.majorversion())
							   .arg(softwareInfo.minorversion())
							   .arg(softwareInfo.commitno())
							   .arg(QHostAddress(m_udpIp).toString())
							   .arg(m_udpPort)
							   .arg(QString::fromStdString(softwareInfo.equipmentid())));

				m_connectionStateStatus->setText("Connected to service" + QString(" - %1").arg(m_udpAckQuantity));

				quint32 time = m_serviceInfo.uptime();

				int s = time % 60; time /= 60;
				int m = time % 60; time /= 60;
				int h = time % 24; time /= 24;

				QString&& uptimeStr = QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));

				m_stateTabModel->setData(m_stateTabModel->index(1, 1), uptimeStr);

				m_uptimeStatus->setText(tr("Uptime ") + uptimeStr);
				m_uptimeStatus->setHidden(false);

				m_runningStatus->setHidden(false);
			}

			break;

		case Undefined:
		case Unavailable:
			setWindowTitle(serviceName + " - No connection");

			m_connectionStateStatus->setText("No connection with service");

			m_uptimeStatus->setHidden(true);
			m_runningStatus->setHidden(true);

			break;

		default:
			assert(false);
	}

	QString runningStateStr;
	QString serviceUptimeStr;
	switch (serviceState)
	{
		case ServiceState::Work:
			{
				runningStateStr = tr("Running");

				m_stateTabModel->setData(m_stateTabModel->index(3, 0), "Runing time");

				quint32 time = m_serviceInfo.serviceuptime();

				int s = time % 60; time /= 60;
				int m = time % 60; time /= 60;
				int h = time % 24; time /= 24;

				QString&& serviceUptimeStr = QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));

				m_stateTabModel->setData(m_stateTabModel->index(3, 1), serviceUptimeStr);

				quint32 ip = m_serviceInfo.clientrequestip();
				quint16 port = m_serviceInfo.clientrequestport();
				QString address = QHostAddress(ip).toString() + QString(":%1").arg(port);
				if (ip != getWorkingClientRequestIp())
				{
					address = QHostAddress(ip).toString() + QString(":%1").arg(port) + " => " + QHostAddress(getWorkingClientRequestIp()).toString() + QString(":%1").arg(port);
				}

				m_stateTabModel->setData(m_stateTabModel->index(4, 0), "Client request address");
				m_stateTabModel->setData(m_stateTabModel->index(4, 1), address);
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

	m_runningStatus->setText(runningStateStr + " " + serviceUptimeStr);
	m_stateTabModel->setData(m_stateTabModel->index(2, 1), runningStateStr);

	switch(serviceState)
	{
		case ServiceState::Work:
			m_runningStatus->setStyleSheet("background-color: rgb(192, 255, 192);");
			break;
		case ServiceState::Starts:
		case ServiceState::Stops:
		case ServiceState::Stopped:
			m_runningStatus->setStyleSheet("background-color: rgb(255, 255, 192);");
			break;
		case ServiceState::Unavailable:
			m_runningStatus->setStyleSheet("background-color: lightGray;");
			break;

		default:
			m_runningStatus->setStyleSheet("background-color: red;");
	}

	switch (serviceState)
	{
		case ServiceState::Work:
			m_startServiceButton->setEnabled(false);
			m_stopServiceButton->setEnabled(true);
			m_restartServiceButton->setEnabled(true);
			break;
		case ServiceState::Stopped:
			m_startServiceButton->setEnabled(true);
			m_stopServiceButton->setEnabled(false);
			m_restartServiceButton->setEnabled(true);
			break;
		case ServiceState::Unavailable:
		case ServiceState::Undefined:
		case ServiceState::Starts:
		case ServiceState::Stops:
			m_startServiceButton->setEnabled(false);
			m_stopServiceButton->setEnabled(false);
			m_restartServiceButton->setEnabled(false);
			break;
		default:
			assert(false);
			break;
	}

	emit connectionStatisticChanged();
}

void BaseServiceStateWidget::updateClientsModel(const Network::ServiceClients& serviceClients)
{
	m_clientsTabModel->setRowCount(serviceClients.clients_size());
	stateTabModel()->setData(stateTabModel()->index(8, 1), serviceClients.clients_size());

	for (int i = 0; i < serviceClients.clients_size(); i++)
	{
		const Network::ServiceClientInfo& ci = serviceClients.clients(i);
		const Network::SoftwareInfo& si = ci.softwareinfo();

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 0),
								   E::valueToString<E::SoftwareType>(si.softwaretype()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 1),
								   QString("%1.%2.%3 (%4)")
								   .arg(si.majorversion())
								   .arg(si.minorversion())
								   .arg(si.commitno())
								   .arg(QString::fromStdString(si.buildbranch())));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 2), QString::fromStdString(si.equipmentid()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 3), QString::fromStdString(si.username()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 4), QHostAddress(ci.ip()).toString());

		quint64 uptime = ci.uptime();

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 5), QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch() - uptime));

		uptime /= 1000;
		int s = uptime % 60; uptime /= 60;
		int m = uptime % 60; uptime /= 60;
		int h = uptime % 24; uptime /= 24;

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 6), QString("(%1d %2:%3:%4)").arg(uptime).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 7), si.buildno() == SoftwareInfo::UNDEFINED_BUILD_NO ? "Non actual" : QString::number(si.buildno()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 8), static_cast<qint64>(ci.replyquantity()));
	}
}

void BaseServiceStateWidget::askServiceState()
{
	if (!m_baseClientSocket->isWaitingForAck())
	{
		m_baseClientSocket->sendRequest(RQID_SERVICE_GET_INFO);
	}
}

void BaseServiceStateWidget::startService()
{
	sendCommand(RQID_SERVICE_START);
}

void BaseServiceStateWidget::stopService()
{
	sendCommand(RQID_SERVICE_STOP);
}

void BaseServiceStateWidget::restartService()
{
	sendCommand(RQID_SERVICE_RESTART);
}

void BaseServiceStateWidget::serviceAckReceived(const UdpRequest udpRequest)
{
	m_udpAckQuantity++;

	switch (udpRequest.ID())
	{
		case RQID_SERVICE_GET_INFO:
		{
			Network::ServiceInfo newServiceState;

			bool result = newServiceState.ParseFromArray(udpRequest.data(), udpRequest.dataSize());

			assert(result == true);

			ServiceState oldState = static_cast<ServiceState>(m_serviceInfo.servicestate());
			ServiceState newState = static_cast<ServiceState>(newServiceState.servicestate());

			if (newState != ServiceState::Work && oldState == ServiceState::Work)
			{
				emit invalidateData();
			}

			if (newState == ServiceState::Work &&
					(oldState != ServiceState::Work || newServiceState.serviceuptime() < m_serviceInfo.serviceuptime()))
			{
				emit needToReloadData();
			}

			m_serviceInfo = newServiceState;

			updateServiceState();
		}
		break;
		case RQID_SERVICE_START:
		case RQID_SERVICE_STOP:
		case RQID_SERVICE_RESTART:
			break;
		default:
			qDebug() << "Unknown packet ID";
	}
}

void BaseServiceStateWidget::serviceNotFound()
{
	m_udpAckQuantity = 0;

	if (m_serviceInfo.servicestate() != TO_INT(ServiceState::Unavailable))
	{
		m_serviceInfo.set_servicestate(TO_INT(ServiceState::Unavailable));
		updateServiceState();
	}
}

int BaseServiceStateWidget::addTab(QWidget* page, const QString& label)
{
	return m_tabWidget->addTab(page, label);
}

QTableView* BaseServiceStateWidget::addTabWithTableView(int defaultSectionSize, const QString& label)
{
	QTableView* newTableView = new QTableView(this);

	newTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(newTableView->fontMetrics().height() * 1.4));
	newTableView->verticalHeader()->hide();

	newTableView->horizontalHeader()->setDefaultSectionSize(defaultSectionSize);
	newTableView->horizontalHeader()->setStretchLastSection(true);
	newTableView->horizontalHeader()->setHighlightSections(false);

	newTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	newTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	newTableView->setAlternatingRowColors(true);
	newTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	addTab(newTableView, label);

	return newTableView;
}

void BaseServiceStateWidget::addStateTab()
{
	QTableView* stateTableView = addTabWithTableView(250, "State");

	m_stateTabModel = new QStandardItemModel(1, 2, this);
	stateTableView->setModel(m_stateTabModel);

	m_stateTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_stateTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_stateTabModel->setData(m_stateTabModel->index(0, 0), "Connected to service");
	m_stateTabModel->setData(m_stateTabModel->index(0, 1), "No");
}

void BaseServiceStateWidget::addClientsTab(bool showStateColumn)
{
	QTableView* clientsTableView = addTabWithTableView(150, "Clients");

	m_clientsTabModel = new QStandardItemModel(0, 8, this);
	clientsTableView->setModel(m_clientsTabModel);

	if (showStateColumn == false)
	{
		clientsTableView->hideColumn(6);
	}

	m_clientsTabModel->setHeaderData(0, Qt::Horizontal, "Software type");
	m_clientsTabModel->setHeaderData(1, Qt::Horizontal, "Version");
	m_clientsTabModel->setHeaderData(2, Qt::Horizontal, "Equipment ID");
	m_clientsTabModel->setHeaderData(3, Qt::Horizontal, "User");
	m_clientsTabModel->setHeaderData(4, Qt::Horizontal, "IPv4");
	m_clientsTabModel->setHeaderData(5, Qt::Horizontal, "Connection time");
	m_clientsTabModel->setHeaderData(6, Qt::Horizontal, "Connection uptime");
	m_clientsTabModel->setHeaderData(7, Qt::Horizontal, "Build");
	m_clientsTabModel->setHeaderData(8, Qt::Horizontal, "Packet counter");

	clientsTableView->setColumnWidth(0, 200);
	clientsTableView->setColumnWidth(2, 250);
	clientsTableView->setColumnWidth(7, 100);
}

quint32 BaseServiceStateWidget::getWorkingClientRequestIp()
{
	QHostAddress address(m_serviceInfo.clientrequestip());

	if (address == QHostAddress::AnyIPv4)
	{
		address.setAddress(m_udpIp);
	}

	return address.toIPv4Address();
}

void BaseServiceStateWidget::sendCommand(int command)
{
	ServiceState state = static_cast<ServiceState>(m_serviceInfo.servicestate());

	if (!(state == ServiceState::Work && (command == RQID_SERVICE_STOP || command == RQID_SERVICE_RESTART)) &&
		!(state == ServiceState::Stopped && (command == RQID_SERVICE_START || command == RQID_SERVICE_RESTART)))
	{
		return;
	}
	if (m_baseClientSocket->isWaitingForAck())
	{
		QMessageBox::critical(this, tr("Command send error"), tr("Socket is waiting for ack, repeat your command later."));
		return;
	}
	m_baseClientSocket->sendRequest(command);
}
