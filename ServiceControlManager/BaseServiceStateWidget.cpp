#include "BaseServiceStateWidget.h"

#include "../UtilsLib/Ui/WidgetUtils.h"
#include "../UtilsLib/WUtils.h"

BaseServiceStateWidget::BaseServiceStateWidget(	const SoftwareInfo& softwareInfo,
												const ServiceData& serviceData,
												quint32 udpIp, quint16 udpPort,
												QWidget* parent) :
	QMainWindow(parent),
	m_udpIp(udpIp),
	m_udpPort(udpPort),
	m_serviceData(serviceData),
	m_softwareInfo(softwareInfo)
{
	setWindowFlag(Qt::Dialog, true);

	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	m_serviceData.protoServiceInfo.mutable_softwareinfo()->set_softwaretype(softwareInfo.softwareType());
	m_serviceData.protoServiceInfo.set_servicestate(TO_INT(E::ServiceState::Undefined));

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

	m_udpSocketThread = new UdpSocketThread();

	m_baseClientSocket = new UdpClientSocket(QHostAddress(udpIp), udpPort);

	connect(m_baseClientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceStateWidget::serviceNotFound);
	connect(m_baseClientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceStateWidget::serviceAckReceived);

	m_udpSocketThread->addWorker(m_baseClientSocket);
	m_udpSocketThread->start();

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &BaseServiceStateWidget::askServiceState);
	m_timer->start(500);

	setWindowPosition(this, QString("Service_%1_%2").arg(QHostAddress(udpIp).toString()).arg(udpPort));

	addStateTab();

	updateServiceState();
}

BaseServiceStateWidget::~BaseServiceStateWidget()
{
	m_timer->stop();

	if (m_udpSocketThread)
	{
		m_udpSocketThread->quitAndWait();
		delete m_udpSocketThread;
		m_udpSocketThread = nullptr;
	}

	saveWindowPosition(this, QString("Service_%1_%2").arg(QHostAddress(m_udpIp).toString()).arg(m_udpPort));
}

void BaseServiceStateWidget::updateServiceState()
{
	emit onUpdateServiceState();

	E::ServiceState srvState = m_serviceData.serviceState();
	const SessionParams& session = m_serviceData.sessionParams;
	const Network::SoftwareInfo& softwareInfo = m_serviceData.protoServiceInfo.softwareinfo();

	if (srvState == E::ServiceState::Unavailable ||
		srvState == E::ServiceState::Undefined)
	{
		m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_CONNECTED, 1), "No");
		m_stateTabModel->setRowCount(1);
		return;
	}

	m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_CONNECTED, 1), "Yes");

	m_stateTabModel->setRowCount(srvState == E::ServiceState::Work ? m_stateTabMaxRowQuantity : 3);

	m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_UPTIME, 0), "Uptime");
	m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_RUNNING_STATE, 0), "Running state");

	QString serviceName = "Unknown Service";
	QString serviceShortName = "???";

	for (const ServiceInfo& si : servicesInfo)
	{
		if (m_serviceData.type == si.softwareType)
		{
			serviceName = si.name;
			serviceShortName = si.shortName;
			break;
		}
	}

	Q_ASSERT(serviceShortName != "???");

	switch (srvState)
	{
	case E::Undefined:
	case E::Unavailable:
		setWindowTitle(serviceName + " - No connection");

		m_connectionStateStatus->setText("No connection with service");
		m_uptimeStatus->setHidden(true);
		m_runningStatus->setHidden(true);

		break;

	case E::Stopped:
	case E::Starts:
	case E::Work:
	case E::Stops:
		{
			setWindowTitle(serviceName +
						   QString(" v%1.%2.%3 - %4 (%5:%6)").
								arg(softwareInfo.majorversion()).
								arg(softwareInfo.minorversion()).
								arg(softwareInfo.patchversion()).
								arg(QString::fromStdString(softwareInfo.equipmentid())).
								arg(QHostAddress(m_udpIp).toString()).
								arg(m_udpPort));

			m_connectionStateStatus->setText("Connected to service" + QString(" - %1").arg(m_udpAckQuantity));

			qint64 uptime = m_serviceData.protoServiceInfo.uptime();

			QString&& uptimeStr = formatUptime(uptime);

			m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_UPTIME, 1), uptimeStr);

			m_uptimeStatus->setText(tr("Uptime ") + uptimeStr);
			m_uptimeStatus->setHidden(false);

			m_runningStatus->setHidden(false);
		}

		break;

	default:
		assert(false);
	}

	QString runningStateStr;

	switch (srvState)
	{
	case E::ServiceState::Work:
			{
				runningStateStr = tr("Running in ") + E::valueToString(session.softwareRunMode) + tr(" mode with ") + session.currentSettingsProfile + tr(" profile");

				m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_RUNNING_TIME, 0), "Runing time");

				qint64 runtime = m_serviceData.protoServiceInfo.serviceruntime();

				m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_RUNNING_TIME, 1), formatUptime(runtime));

				HostAddressPort workingIp = getWorkingClientRequestIp();

				m_stateTabModel->setData(m_stateTabModel->index(4, 0), "Client request address");
				m_stateTabModel->setData(m_stateTabModel->index(4, 1), workingIp.addressPortStr());
			}
			break;

	case E::ServiceState::Stopped:
	case E::ServiceState::Unavailable:
	case E::ServiceState::Undefined:
	case E::ServiceState::Starts:
	case E::ServiceState::Stops:
			runningStateStr = E::valueToString(srvState);
			break;

	default:
		assert(false);
		runningStateStr = tr("Unknown state");
		break;
	}

	m_runningStatus->setText(runningStateStr);
	m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_RUNNING_STATE, 1), runningStateStr);

	switch(srvState)
	{
	case E::ServiceState::Work:
			if (session.softwareRunMode == E::SoftwareRunMode::Normal)
			{
				m_runningStatus->setStyleSheet("background-color: rgb(127, 255, 127);");
			}
			else
			{
				m_runningStatus->setStyleSheet("background-color: rgb(255, 255, 127);");
			}
			break;
	case E::ServiceState::Starts:
	case E::ServiceState::Stops:
	case E::ServiceState::Stopped:
			m_runningStatus->setStyleSheet("background-color: rgb(255, 127, 127);");
			break;
	case E::ServiceState::Unavailable:
			m_runningStatus->setStyleSheet("background-color: lightGray;");
			break;

	default:
		m_runningStatus->setStyleSheet("background-color: red;");
	}

	switch (srvState)
	{
	case E::ServiceState::Work:
			m_startServiceButton->setEnabled(false);
			m_stopServiceButton->setEnabled(true);
			m_restartServiceButton->setEnabled(true);
			break;
	case E::ServiceState::Stopped:
			m_startServiceButton->setEnabled(true);
			m_stopServiceButton->setEnabled(false);
			m_restartServiceButton->setEnabled(true);
			break;
	case E::ServiceState::Unavailable:
	case E::ServiceState::Undefined:
	case E::ServiceState::Starts:
	case E::ServiceState::Stops:
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

	if (m_clientQuantityRowIndex != -1)
	{
		stateTabModel()->setData(stateTabModel()->index(m_clientQuantityRowIndex, 1), serviceClients.clients_size());
	}

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
								   .arg(si.patchversion())
								   .arg(QString::fromStdString(si.branchname())));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 2), QString::fromStdString(si.equipmentid()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 3), QString::fromStdString(si.osusername()));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 4), QHostAddress(ci.ip()).toString());

		quint64 uptime = ci.uptime();

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 5), QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch() - uptime));

		uptime /= 1000;

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 6), formatUptime(uptime));

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 7), si.pipelineid());

		m_clientsTabModel->setData(m_clientsTabModel->index(i, 8), ci.replyquantity());
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

			bool result = newServiceState.ParseFromArray(udpRequest.data(),
														 static_cast<int>(udpRequest.dataSize()));

			if (result == false)
			{
				return;
			}

			E::ServiceState oldState = m_serviceData.serviceState();
			E::ServiceState newState = static_cast<E::ServiceState>(newServiceState.servicestate());

			if (newState != E::ServiceState::Work && oldState == E::ServiceState::Work)
			{
				emit invalidateData();
			}

			if (newState == E::ServiceState::Work &&
				(oldState != E::ServiceState::Work ||
					newServiceState.serviceruntime() < m_serviceData.protoServiceInfo.serviceruntime()))
			{
				emit needToReloadData();
			}

			m_serviceData.protoServiceInfo = newServiceState;
			m_serviceData.parseProtoServiceInfo();

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

	if (m_serviceData.serviceState() != E::ServiceState::Unavailable)
	{
		m_serviceData.protoServiceInfo.set_servicestate(TO_INT(E::ServiceState::Unavailable));
		updateServiceState();
	}
}

void BaseServiceStateWidget::createTcpConnection(quint32 ip, quint16 port)
{
	Q_UNUSED(ip);
	Q_UNUSED(port);
}

void BaseServiceStateWidget::dropTcpConnection()
{
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

	m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_CONNECTED, 0), "Connected to service");
	m_stateTabModel->setData(m_stateTabModel->index(SS_ROW_CONNECTED, 1), "No");
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

HostAddressPort BaseServiceStateWidget::getWorkingClientRequestIp()
{
	if (m_serviceData.clientRequestIPs.empty())
	{
		return HostAddressPort(m_udpIp, m_udpPort);
	}

	return m_serviceData.clientRequestIPs[0];
}

void BaseServiceStateWidget::sendCommand(int command)
{
	E::ServiceState state = m_serviceData.serviceState();

	if (!(state == E::ServiceState::Work && (command == RQID_SERVICE_STOP || command == RQID_SERVICE_RESTART)) &&
		!(state == E::ServiceState::Stopped && (command == RQID_SERVICE_START || command == RQID_SERVICE_RESTART)))
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
