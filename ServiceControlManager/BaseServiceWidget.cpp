#include "BaseServiceWidget.h"

#include "../UtilsLib/Ui/WidgetUtils.h"
#include "../UtilsLib/WUtils.h"

BaseServiceWidget::BaseServiceWidget(	ServiceTableModel* srvTableModel,
										const SoftwareInfo& softwareInfo,
										const ServiceData& serviceData,
										quint32 udpIp, quint16 udpPort,
										QWidget* parent) :
	QMainWindow(parent),
	m_srvTableModel(srvTableModel),
	m_udpIp(udpIp),
	m_udpPort(udpPort),
	m_serviceData(serviceData),
	m_softwareInfo(softwareInfo)
{
	TEST_PTR_RETURN(m_srvTableModel);

	m_serviceData.protoServiceInfo.mutable_softwareinfo()->set_softwaretype(softwareInfo.softwareType());
	m_serviceData.protoServiceInfo.set_servicestate(TO_INT(E::ServiceState::Undefined));

	//

	setWindowFlag(Qt::Dialog, true);

	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	//

	QToolBar* toolBar = addToolBar("Service actions");

	m_startServiceButton = toolBar->addAction("Start", this, SLOT(startService()));
	m_stopServiceButton = toolBar->addAction("Stop", this, SLOT(stopService()));
	m_restartServiceButton = toolBar->addAction("Restart", this, SLOT(restartService()));

	//

	statusBar()->addWidget(m_connectionStateStatus = new QLabel(this));
	statusBar()->addWidget(m_uptimeStatus = new QLabel(this));
	statusBar()->addWidget(m_runningStatus = new QLabel(this));

	m_connectionStateStatus->setMargin(5);
	m_uptimeStatus->setMargin(5);
	m_runningStatus->setMargin(5);

	//

	setWindowPosition(this, QString("Service_%1_%2").arg(QHostAddress(udpIp).toString()).arg(udpPort));

	addGeneralTab();
	addClientsTab();

	//

	m_udpSocketThread = new UdpSocketThread();

	m_baseClientSocket = new UdpClientSocket(QHostAddress(udpIp), udpPort);

	connect(m_baseClientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceWidget::serviceAckTimeout);
	connect(m_baseClientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceWidget::serviceAckReceived);

	m_udpSocketThread->addWorker(m_baseClientSocket);
	m_udpSocketThread->start();

	//

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &BaseServiceWidget::askServiceState);
	m_timer->start(500);

	askServiceState();
	updateSrvStatus();
}

BaseServiceWidget::~BaseServiceWidget()
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

void BaseServiceWidget::updateSrvStatusWidgets()
{
	updateWindowTitle();
	updateSrvControlButtons();
	updateSrvStatus();
	updateBuildInfo();
	updateStatusBar();
	updateBaseSettings();
	updateClients();
}

void BaseServiceWidget::updateWindowTitle()
{
	switch (m_serviceData.serviceState())
	{
	case E::Undefined:
	case E::Unavailable:
		setWindowTitle(m_serviceData.serviceName + " - No connection");
		break;

	case E::Stopped:
	case E::Starts:
	case E::Work:
	case E::Stops:
		{
			const Network::SoftwareInfo& softwareInfo = m_serviceData.protoServiceInfo.softwareinfo();

			setWindowTitle(m_serviceData.serviceName +
						   QString(" v%1.%2.%3 - %4 (%5:%6)").
						   arg(softwareInfo.majorversion()).
						   arg(softwareInfo.minorversion()).
						   arg(softwareInfo.patchversion()).
						   arg(QString::fromStdString(softwareInfo.equipmentid())).
						   arg(QHostAddress(m_udpIp).toString()).
						   arg(m_udpPort));
		}
		break;

	default:
		Q_ASSERT(false);
	}
}

void BaseServiceWidget::updateSrvControlButtons()
{
	switch (m_serviceData.serviceState())
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
}

void BaseServiceWidget::updateSrvStatus()
{
	E::ServiceState srvState = m_serviceData.serviceState();

	//

	m_srvStatusModel->setData(m_srvStatusModel->index(0, 0), "Connected to service");

	if (srvState == E::ServiceState::Unavailable ||
		srvState == E::ServiceState::Undefined)
	{
		m_srvStatusModel->setData(m_srvStatusModel->index(0, 1), "No");
		m_srvStatusModel->setRowCount(1);
		return;
	}

	m_srvStatusModel->setData(m_srvStatusModel->index(0, 1), "Yes");

	//

	m_srvStatusModel->setData(m_srvStatusModel->index(1, 0), "Uptime");
	m_srvStatusModel->setData(m_srvStatusModel->index(1, 1),
							  formatUptime(m_serviceData.protoServiceInfo.uptime()));

	//

	m_srvStatusModel->setData(m_srvStatusModel->index(2, 0), "Running state");

	switch (srvState)
	{
	case E::ServiceState::Work:
		m_srvStatusModel->setData(m_srvStatusModel->index(2, 1), getRunningStateStr());
		break;

	case E::ServiceState::Stopped:
	case E::ServiceState::Unavailable:
	case E::ServiceState::Undefined:
	case E::ServiceState::Starts:
	case E::ServiceState::Stops:
		m_srvStatusModel->setData(m_srvStatusModel->index(2, 1), E::valueToString(srvState));
		break;

	default:
		Q_ASSERT(false);
		m_srvStatusModel->setData(m_srvStatusModel->index(2, 1), tr("Unknown state"));
		break;
	}

	m_srvStatusModel->setData(m_srvStatusModel->index(3, 0), "Runing time");
	m_srvStatusModel->setData(m_srvStatusModel->index(3, 1),
							  formatUptime(m_serviceData.protoServiceInfo.serviceruntime()));

	m_srvStatusModel->setRowCount(4);

	updateColumnsWidth(m_srvStatusModel);
}

void BaseServiceWidget::updateBuildInfo()
{
	const OnlineLib::BuildInfo& b = m_serviceData.buildInfo;

	m_buildInfoModel->setData(m_buildInfoModel->index(0, 0), "Build status");

	if (b.project.isEmpty())
	{
		m_buildInfoModel->setRowCount(1);
		m_buildInfoModel->setData(m_buildInfoModel->index(0, 1), "Not loaded");
		return;
	}

	m_buildInfoModel->setData(m_buildInfoModel->index(0, 1), "Loaded");

	m_buildInfoModel->setData(m_buildInfoModel->index(1, 0), "Project name");
	m_buildInfoModel->setData(m_buildInfoModel->index(1, 1), b.project);

	m_buildInfoModel->setData(m_buildInfoModel->index(2, 0), "Build No");
	m_buildInfoModel->setData(m_buildInfoModel->index(2, 1), b.buildNo);

	m_buildInfoModel->setData(m_buildInfoModel->index(3, 0), "Build date");
	m_buildInfoModel->setData(m_buildInfoModel->index(3, 1), b.dateTimeStr());

	m_buildInfoModel->setData(m_buildInfoModel->index(4, 0), "Changeset");
	m_buildInfoModel->setData(m_buildInfoModel->index(4, 1), b.changeset);

	m_buildInfoModel->setData(m_buildInfoModel->index(5, 0), "User name");
	m_buildInfoModel->setData(m_buildInfoModel->index(5, 1), b.user);

	m_buildInfoModel->setData(m_buildInfoModel->index(6, 0), "Workstation");
	m_buildInfoModel->setData(m_buildInfoModel->index(6, 1), b.workstation);

	m_buildInfoModel->setRowCount(7);

	updateColumnsWidth(m_buildInfoModel);
}

void BaseServiceWidget::updateStatusBar()
{
	switch (m_serviceData.serviceState())
	{
	case E::Undefined:
	case E::Unavailable:
		m_connectionStateStatus->setText("No connection with service");
		m_uptimeStatus->setHidden(true);
		m_runningStatus->setHidden(true);
		break;

	case E::Stopped:
	case E::Starts:
	case E::Work:
	case E::Stops:
		m_connectionStateStatus->setText("Connected to service" + QString(" - %1").arg(m_udpAckQuantity));
		m_uptimeStatus->setText(tr("Uptime ") + formatUptime(m_serviceData.protoServiceInfo.uptime()));
		m_runningStatus->setText(getRunningStateStr());

		if (m_serviceData.serviceState() == E::ServiceState::Work)
		{
			if (m_serviceData.sessionParams.softwareRunMode == E::SoftwareRunMode::Normal)
			{
				m_runningStatus->setStyleSheet("background-color: rgb(127, 255, 127);");
			}
			else
			{
				m_runningStatus->setStyleSheet("background-color: rgb(255, 255, 127);");
			}
		}
		else
		{
			m_runningStatus->setStyleSheet("background-color: rgb(255, 127, 127);");
		}

		m_uptimeStatus->setHidden(false);
		m_runningStatus->setHidden(false);
		break;

	default:
		assert(false);
	}
}

void BaseServiceWidget::updateBaseSettings()
{
	if (m_serviceData.settings == nullptr)
	{
		m_settingsModel->setRowCount(0);
		return;
	}

	const SoftwareInfo& swInfo = m_serviceData.swInfo;

	m_settingsModel->setData(m_settingsModel->index(0, 0), "EquipementID");
	m_settingsModel->setData(m_settingsModel->index(0, 1), swInfo.equipmentID());

	m_settingsModel->setData(m_settingsModel->index(1, 0), "SoftwareType");
	m_settingsModel->setData(m_settingsModel->index(1, 1), swInfo.softwareType());

	m_settingsModel->setData(m_settingsModel->index(2, 0), "Host name");
	m_settingsModel->setData(m_settingsModel->index(2, 1), swInfo.hostname());

	m_settingsModel->setData(m_settingsModel->index(3, 0), "OS username");
	m_settingsModel->setData(m_settingsModel->index(3, 1), swInfo.osUsername());

	int rowCount = 4;

	rowCount = updateSettings(rowCount);

	m_settingsModel->setRowCount(rowCount);

	updateColumnsWidth(m_settingsModel);
}

int BaseServiceWidget::updateSettings(int rowCount)
{
	Q_UNUSED(rowCount);
	return 0;
}

void BaseServiceWidget::updateClients()
{
	QStandardItemModel* cm = m_clientsModel;

	const Network::ServiceClients& clients = m_serviceData.protoServiceInfo.clients();

	int clientsCount = clients.clients_size();

	cm->setRowCount(clientsCount);

	if (clientsCount == 0)
	{
		return;
	}

	for(int i = 0; i < clientsCount; i++)
	{
		const Network::ServiceClientInfo& client = clients.clients(i);
		const Network::SoftwareInfo& sw = client.softwareinfo();

		cm->setData(cm->index(i, 0), HostAddressPort(client.requestip(), client.requestport()).toString());
		cm->setData(cm->index(i, 1), QString::fromStdString(sw.equipmentid()));
		cm->setData(cm->index(i, 2), HostAddressPort(client.clientip(), client.clientport()).toString());
		cm->setData(cm->index(i, 3), E::valueToString(static_cast<E::SoftwareType>(sw.softwaretype())));
		cm->setData(cm->index(i, 4), formatUptime(client.uptime() / 1000));
		cm->setData(cm->index(i, 5), client.replyquantity());
	}

	updateColumnsWidth(m_clientsModel);
}

void BaseServiceWidget::updateColumnsWidth(QStandardItemModel* model)
{
	TEST_PTR_RETURN(model);

	auto it = m_modelTableViewColumns.find(model);

	if (it == m_modelTableViewColumns.end())
	{
		return;
	}

	QTableView* tableView = it->second.first;
	const Columns& columns = it->second.second;

	int index = 0;

	for(const Column& column : columns)
	{
		tableView->setColumnWidth(index++, column.width);
	}
}

QString BaseServiceWidget::getRunningStateStr() const
{
	return QString(tr("Running in %1 mode with %2 profile").
					arg(E::valueToString(m_serviceData.sessionParams.softwareRunMode)).
					arg(m_serviceData.sessionParams.currentSettingsProfile));
}

void BaseServiceWidget::askServiceState()
{
	if (!m_baseClientSocket->isWaitingForAck())
	{
		m_baseClientSocket->sendRequest(RQID_SERVICE_GET_INFO);
	}
}

void BaseServiceWidget::startService()
{
	sendCommand(RQID_SERVICE_START);
}

void BaseServiceWidget::stopService()
{
	sendCommand(RQID_SERVICE_STOP);
}

void BaseServiceWidget::restartService()
{
	sendCommand(RQID_SERVICE_RESTART);
}

void BaseServiceWidget::serviceAckReceived(const UdpRequest udpRequest)
{
//	qDebug() << "Service ACK received";

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
				emit invalidateServiceData();
			}

			m_serviceData.protoServiceInfo = newServiceState;
			m_serviceData.parseProtoServiceInfo();

			updateSrvStatusWidgets();
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

void BaseServiceWidget::serviceAckTimeout()
{
//	qDebug() << "Service ack TIMEOUT";

	if (m_serviceData.serviceState() != E::ServiceState::Unavailable)
	{
		clearServiceData();
		updateSrvStatusWidgets();
	}
}

void BaseServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	Q_UNUSED(ip);
	Q_UNUSED(port);
}

void BaseServiceWidget::dropTcpConnection()
{
}

QString BaseServiceWidget::rqCtrlInfoStr(const RqCtrlSettings& rcs)
{
	QString str;

	quint32 propsMask = rcs.propsMask();

	if (propsMask & RqCtrlSettings::PROP_EQUIPMENT_ID)
	{
		str += rcs.equipmentID();
	}

	if (propsMask & RqCtrlSettings::PROP_ENABLE)
	{
		str += Separator::COMMA_SPACE;
		str += rcs.enable() ? "Enabled" : "Disabled";
	}

	if (propsMask & RqCtrlSettings::PROP_SECURITY_LEVEL)
	{
		str += Separator::COMMA_SPACE;
		str += QString("Security = %1").arg(E::valueToString(rcs.securityLevel()));
	}

	if (propsMask & RqCtrlSettings::PROP_CLIENT_REQUEST_IP)
	{
		str += Separator::COMMA_SPACE;
		str += QString("IP = %1").arg(rcs.clientRequestIP().toString());
	}

	if (propsMask & RqCtrlSettings::PROP_CLIENT_REQUEST_NETMASK)
	{
		str += Separator::COMMA_SPACE;
		str += QString("Netmask = %1").arg(rcs.clientRequestNetmask().toString());
	}

	if (propsMask & RqCtrlSettings::PROP_RT_TRENDS_REQUEST_IP)
	{
		str += Separator::COMMA_SPACE;
		str += QString("RtTrendsIP = %1").arg(rcs.rtTrendsRequestIP().toString());
	}

	return str;
}

void BaseServiceWidget::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event);
	m_srvTableModel->deleteSrvWidget(this);
}

void BaseServiceWidget::addGeneralTab()
{
	QWidget* generalTabWidget = new QWidget();

	//

	m_srvStatusModel = new QStandardItemModel(0, TO_INT(propValueColumns.size()), this);
	QTableView* srvStateTableView = createTableView(m_srvStatusModel, propValueColumns);

	//

	m_buildInfoModel = new QStandardItemModel(0, TO_INT(propValueColumns.size()), this);
	m_buildInfoModel->setData(m_buildInfoModel->index(0, 0), "Build status");
	m_buildInfoModel->setData(m_buildInfoModel->index(0, 1), "Not loaded");

	QTableView* buildInfoTableView = createTableView(m_buildInfoModel, propValueColumns);

	//

	m_settingsModel = new QStandardItemModel(0, TO_INT(propValueColumns.size()), this);
	QTableView* settingsTableView = createTableView(m_settingsModel, propValueColumns);

	//

	QVBoxLayout* vBoxLayout = new QVBoxLayout(generalTabWidget);

	vBoxLayout->addWidget(new QLabel("Service status"));
	vBoxLayout->addWidget(srvStateTableView, 20);
	vBoxLayout->addWidget(new QLabel("Build Information"));
	vBoxLayout->addWidget(buildInfoTableView, 30);
	vBoxLayout->addWidget(new QLabel("Service Settings"));
	vBoxLayout->addWidget(settingsTableView, 50);

	generalTabWidget->setLayout(vBoxLayout);

	addTab(generalTabWidget, "General");
}

void BaseServiceWidget::addClientsTab()
{
	m_clientsModel = new QStandardItemModel(0, TO_INT(clientTabColumns.size()), this);
	QTableView* clientsTableView = createTableView(m_clientsModel, clientTabColumns);

	addTab(clientsTableView, "Clients");
}

void BaseServiceWidget::clearServiceData()
{
	if (m_serviceData.serviceState() != E::ServiceState::Unavailable)
	{
		m_udpAckQuantity = 0;
		m_serviceData.protoServiceInfo.set_servicestate(TO_INT(E::ServiceState::Unavailable));
		m_serviceData.protoServiceInfo.clear_clients();
		m_serviceData.buildInfo.clear();
		m_serviceData.settings.reset();
	}
}

void BaseServiceWidget::onSectionResized(int index, int oldSize, int newSize)
{
	Q_UNUSED(oldSize);

	QHeaderView* hHeader = qobject_cast<QHeaderView*>(QObject::sender());

	TEST_PTR_RETURN(hHeader);

	QTableView* tableView = qobject_cast<QTableView*>(hHeader->parent());

	for(auto& [model, tableViewColumns] : m_modelTableViewColumns)
	{
		if (tableView == tableViewColumns.first)
		{
			if (index < tableViewColumns.second.size())
			{
				tableViewColumns.second[index].width = newSize;
			}
			break;
		}
	}
}

int BaseServiceWidget::addTab(QWidget* page, const QString& label)
{
	return m_tabWidget->addTab(page, label);
}

QTableView* BaseServiceWidget::addTabWithTableView(int defaultSectionSize, const QString& label)
{
	QTableView* tableView = new QTableView(this);

	QHeaderView* vHeader = tableView->verticalHeader();

	vHeader->setDefaultSectionSize(static_cast<int>(tableView->fontMetrics().height() * 1.4));
	vHeader->hide();

	QHeaderView* hHeader = tableView->horizontalHeader();

	hHeader->setDefaultSectionSize(defaultSectionSize);
	hHeader->setStretchLastSection(true);
	hHeader->setHighlightSections(false);

	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	tableView->setAlternatingRowColors(true);
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	addTab(tableView, label);

	return tableView;
}

QTableView* BaseServiceWidget::createTableView(QStandardItemModel* model,
												const Columns& columns)
{
	QTableView* tableView = new QTableView();

	QHeaderView* hHeader = tableView->horizontalHeader();

	hHeader->setSectionResizeMode(QHeaderView::Interactive);

	connect(hHeader, &QHeaderView::sectionResized, this, &BaseServiceWidget::onSectionResized);

	if (columns.empty() == false)
	{
		int index = 0;

		for(const auto& [caption, width] : columns)
		{
			model->setHeaderData(index, Qt::Horizontal, caption);
			index++;
		}
	}
	else
	{
		tableView->horizontalHeader()->hide();
		hHeader->setDefaultSectionSize(200);
	}

	hHeader->setStretchLastSection(true);
	hHeader->setHighlightSections(false);

	QHeaderView* vHeader = tableView->verticalHeader();

	vHeader->setDefaultSectionSize(static_cast<int>(tableView->fontMetrics().height() * 1.4));
	vHeader->hide();

	//

	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	tableView->setModel(model);

	m_modelTableViewColumns.insert({model, std::pair{tableView, columns}});

	return tableView;
}

HostAddressPort BaseServiceWidget::getWorkingClientRequestIp()
{
	if (m_serviceData.clientRequestIPs.empty())
	{
		return HostAddressPort(m_udpIp, m_udpPort);
	}

	return m_serviceData.clientRequestIPs[0];
}

void BaseServiceWidget::sendCommand(int command)
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
