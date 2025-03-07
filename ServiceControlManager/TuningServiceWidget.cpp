#include "TuningServiceWidget.h"
#include "TcpTuningServiceClient.h"
#include "TuningSourceWidget.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QMessageBox>

TuningServiceWidget::TuningServiceWidget(const SoftwareInfo& softwareInfo, const ServiceData& service, quint32 udpIp, quint16 udpPort, QWidget *parent) :
	BaseServiceWidget(softwareInfo, service, udpIp, udpPort, parent)
{
	//----------------------------------------------------------------------------------------------------
//	addClientsTab();

	//----------------------------------------------------------------------------------------------------
	QTableView* parametersTableView = addTabWithTableView(250, "Parameters");

	m_parametersTabModel = new QStandardItemModel(3, 2, this);
	parametersTableView->setModel(m_parametersTabModel);

	m_parametersTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_parametersTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_parametersTabModel->setData(m_parametersTabModel->index(0, 0), "Equipment ID");
	m_parametersTabModel->setData(m_parametersTabModel->index(1, 0), "Configuration IP 1");
	m_parametersTabModel->setData(m_parametersTabModel->index(2, 0), "Configuration IP 2");

	//----------------------------------------------------------------------------------------------------
	QTableView* settingsTableView = addTabWithTableView(250, "Settings");

	m_settingsTabModel = new QStandardItemModel(8, 2, this);
	settingsTableView->setModel(m_settingsTabModel);

	m_settingsTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_settingsTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 0), "Equipment ID");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 0), "LAN Equipment ID");

	m_settingsTabModel->setData(m_settingsTabModel->index(2, 0), "Client Request IP");
	m_settingsTabModel->setData(m_settingsTabModel->index(3, 0), "Client Request NetMask");

	m_settingsTabModel->setData(m_settingsTabModel->index(4, 0), "Tuning Data IP");
	m_settingsTabModel->setData(m_settingsTabModel->index(5, 0), "Tuning Data NetMask");

	m_settingsTabModel->setData(m_settingsTabModel->index(6, 0), "Signle LM Control");
	m_settingsTabModel->setData(m_settingsTabModel->index(7, 0), "Disable modules Type checking");

	m_settingsTabModel->setData(m_settingsTabModel->index(8, 0), "Tuning Sim IP");

	//----------------------------------------------------------------------------------------------------
	QTableView* tuningSourcesTableView = addTabWithTableView(125, "Tuning Sources");

	QStringList tuningSourceHeaderLabels;

	for (const QString& headerLabel : tuningSourceStaticFieldsHeaderLabels)
	{
		tuningSourceHeaderLabels << tr(qPrintable(headerLabel));
	}

	for (const QString& headerLabel : tuningSourceDynamicFieldsHeaderLabels)
	{
		tuningSourceHeaderLabels << tr(qPrintable(headerLabel));
	}

	m_tuningSourcesTabModel = new QStandardItemModel(0, static_cast<int>(tuningSourceHeaderLabels.size()), this);
	tuningSourcesTableView->setModel(m_tuningSourcesTabModel);

	m_tuningSourcesTabModel->setHorizontalHeaderLabels(tuningSourceHeaderLabels);

	tuningSourcesTableView->setColumnWidth(0, 250);
	tuningSourcesTableView->setColumnWidth(1, 250);

	connect(tuningSourcesTableView, &QTableView::doubleClicked, this, &TuningServiceWidget::onTuningSourceDoubleClicked);

	//----------------------------------------------------------------------------------------------------
	QTableView* tuningSignalsTableView = addTabWithTableView(125, "Tuning Signals");

	QStringList tuningSignalsHeaderLabels;

	for (const QString& headerLabel : tuningSignalsStaticFieldsHeaderLabels)
	{
		tuningSignalsHeaderLabels << tr(qPrintable(headerLabel));
	}

	for (const QString& headerLabel : tuningSignalsDynamicFieldsHeaderLabels)
	{
		tuningSignalsHeaderLabels << tr(qPrintable(headerLabel));
	}

	m_tuningSignalsTabModel = new QStandardItemModel(0, static_cast<int>(tuningSignalsHeaderLabels.size()), this);
	tuningSignalsTableView->setModel(m_tuningSignalsTabModel);

	tuningSignalsTableView->setColumnWidth(0, 175);
	tuningSignalsTableView->setColumnWidth(1, 250);

	m_tuningSignalsTabModel->setHorizontalHeaderLabels(tuningSignalsHeaderLabels);


	//----------------------------------------------------------------------------------------------------
	addTabWithTableView(250, "Log");
}

TuningServiceWidget::~TuningServiceWidget()
{
	for (auto* widget : m_tuningSourceWidgetList)
	{
		widget->deleteLater();
	}
	m_tuningSourceWidgetList.clear();

	dropTcpConnection();
}

void TuningServiceWidget::updateStateInfo()
{
	if (m_serviceData.serviceState() == E::ServiceState::Work)
	{
		HostAddressPort workingIp = getWorkingClientRequestIp();

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

	auto tuningSettings = std::dynamic_pointer_cast<TuningServiceSettings>(m_serviceData.settings);

	if (tuningSettings == nullptr)
	{
		return;
	}

	// TO DO 2ch tuning!
	//
	TuningServiceSettings::ChannelSettings ch = tuningSettings->channelSettings[0];

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 1), tuningSettings->equipmentID);

	m_settingsTabModel->setData(m_settingsTabModel->index(1, 1), tuningSettings->clientRequestIP.addressStr());
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), tuningSettings->clientRequestNetmask.toString());

	m_settingsTabModel->setData(m_settingsTabModel->index(3, 1), ch.tuningDataIP.addressStr());
	m_settingsTabModel->setData(m_settingsTabModel->index(4, 1), ch.tuningDataNetmask.toString());

	m_settingsTabModel->setData(m_settingsTabModel->index(5, 1), tuningSettings->singleLmControl ? tr("True") : tr("False"));
	m_settingsTabModel->setData(m_settingsTabModel->index(6, 1), tuningSettings->disableModulesTypeChecking ? tr("True") : tr("False"));

	m_settingsTabModel->setData(m_settingsTabModel->index(7, 1), ch.tuningSimIP.addressStr());
}

void TuningServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpTuningServiceClient(softwareInfo(), HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	connect(m_tcpClientSocket, &TcpTuningServiceClient::clientsLoaded, this, &TuningServiceWidget::updateClientsInfo);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::settingsLoaded, this, &TuningServiceWidget::updateServiceParameters);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSourcesInfoLoaded, this, &TuningServiceWidget::reloadTuningSourcesList);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSoursesStateUpdated, this, &TuningServiceWidget::updateTuningSourcesState);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSignalsInfoLoaded, this, &TuningServiceWidget::reloadTuningSignalsList);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSignalsStateUpdated, this, &TuningServiceWidget::updateTuningSignalsState);

	connect(m_tcpClientSocket, &TcpTuningServiceClient::socketDisconnected, this, &TuningServiceWidget::clearServiceData);

	m_tcpClientThread->start();

	emit newTcpClientSocket(m_tcpClientSocket);
}

void TuningServiceWidget::dropTcpConnection()
{
	emit clearTcpClientSocket();

	if (m_tcpClientThread != nullptr)
	{
		m_tcpClientThread->quitAndWait();
		delete m_tcpClientThread;
		m_tcpClientThread = nullptr;

		m_tcpClientSocket = nullptr;	// Should be deleted on m_tcpClientThread->quitAndWait();
	}
}

void TuningServiceWidget::updateClientsInfo()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->clientsIsReady() == false)
	{
//		clientsTabModel()->setRowCount(0);
		return;
	}

//	updateClientsModel(m_tcpClientSocket->clients());
}

void TuningServiceWidget::updateServiceParameters()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->settingsIsReady() == false)
	{
		for (int i = 0; i < m_parametersTabModel->rowCount(); i++)
		{
			m_parametersTabModel->setData(m_parametersTabModel->index(i, 1), "???");
		}
		return;
	}

	m_parametersTabModel->setData(m_parametersTabModel->index(0, 1), m_tcpClientSocket->equipmentID());
	m_parametersTabModel->setData(m_parametersTabModel->index(1, 1), m_tcpClientSocket->configIP1());
	m_parametersTabModel->setData(m_parametersTabModel->index(2, 1), m_tcpClientSocket->configIP2());
}

void TuningServiceWidget::reloadTuningSourcesList()
{
	m_tuningSourcesTabModel->setRowCount(0);

	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSourcesInfoIsReady() == false)
	{
		return;
	}

	const QList<ClientLib::TuningSource>& tsList = m_tcpClientSocket->tuningSources();

	int sourcesLanCount = 0;

	for (const ClientLib::TuningSource& ts : tsList)
	{
		sourcesLanCount += ts.info().lancontrollerinfo_size();
	}

	m_tuningSourcesTabModel->setRowCount(sourcesLanCount);
	int row = 0;

	for (const ClientLib::TuningSource& ts : tsList)
	{
		const ::Network::DataSourceInfo& info = ts.info();

		int lanControllersCount = ts.info().lancontrollerinfo_size();

		for (int c = 0; c < lanControllersCount; c++)
		{
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 0), ts.equipmentId());
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 1), QString::fromStdString(info.lancontrollerinfo(c).equipmentid()));
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 2), QString::fromStdString(info.modulecaption()));
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 3), QString::fromStdString(info.lancontrollerinfo(c).tuningip()));
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 4), info.lancontrollerinfo(c).tuningport());
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 5), QString::fromStdString(info.subsystemchannel()));
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 6), info.subsystemkey());
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 7), QString::fromStdString(info.subsystemid()));
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 8), info.lmnumber());

			for (int j = static_cast<int>(tuningSourceStaticFieldsHeaderLabels.count()); j < m_tuningSourcesTabModel->columnCount(); j++)
			{
				m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, j), "-");
			}

			row++;
		}
	}
}

void TuningServiceWidget::updateTuningSourcesState()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSourcesStateIsReady() == false)
	{
		for (int i = 0; i < m_tuningSourcesTabModel->rowCount(); i++)
		{
			for (int j = static_cast<int>(tuningSourceStaticFieldsHeaderLabels.count()); j < m_tuningSourcesTabModel->columnCount(); j++)
			{
				m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(i, j), "???");
			}
		}
		return;
	}

	int firstColumn = static_cast<int>(tuningSourceStaticFieldsHeaderLabels.count());

	int row = 0;

	for (const ClientLib::TuningSource& ts : m_tcpClientSocket->tuningSources())
	{
		for (int c = 0; c < ts.controllersCount(); c++)
		{
			// Find a state to display
			//
			for (int s = 0; s < ts.statesCount(); s++)
			{
				const ::Network::TuningSourceState& state = ts.state(s);

				if (QString::fromStdString(state.lanequipmentid()) == ts.controllerEquipmentId(c))
				{
					m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 0), state.isreply() ? tr("Yes") : tr("No"));
					m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 1), state.controlisactive() ? tr("Yes") : tr("No"));
					m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 2), state.setsor() ? tr("Yes") : tr("No"));
					m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 3), static_cast<qint64>(state.requestcount()));
					m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 4), static_cast<qint64>(state.replycount()));
				}
			}
			//

			row++;
		}
	}
}

void TuningServiceWidget::reloadTuningSignalsList()
{
	m_tuningSignalsTabModel->setRowCount(0);

	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSignalsInfoIsReady() == false)
	{
		return;
	}

	const QVector<AppSignal>& tspVector = m_tcpClientSocket->tuningSignalParams();

	m_tuningSignalsTabModel->setRowCount(static_cast<int>(tspVector.count()));
	int row = 0;

	for (const AppSignal& tsp : tspVector)
	{
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 0), tsp.customAppSignalID());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 1), tsp.equipmentID());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 2), tsp.appSignalID());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 3), tsp.caption());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 4), tsp.unit());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 5), E::valueToString<E::SignalType>(tsp.signalType()));
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 6), tsp.tuningDefaultValue().toString());

		for (int j = static_cast<int>(tuningSignalsStaticFieldsHeaderLabels.count()); j < m_tuningSignalsTabModel->columnCount(); j++)
		{
			m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, j), "???");
		}

		row++;
	}
}

void TuningServiceWidget::updateTuningSignalsState()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSignalsStateIsReady() == false)
	{
		for (int i = 0; i < m_tuningSignalsTabModel->rowCount(); i++)
		{
			for (int j = static_cast<int>(tuningSignalsStaticFieldsHeaderLabels.count()); j < m_tuningSignalsTabModel->columnCount(); j++)
			{
				m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(i, j), "???");
			}
		}
		return;
	}

	int firstColumn = static_cast<int>(tuningSignalsStaticFieldsHeaderLabels.count());
	int row = 0;

	for (const TuningSignalState& tss : m_tcpClientSocket->tuningSignalStates())
	{
		//int precision = m_tcpClientSocket->tuningSignalParams()[row].decimalPlaces();

		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, firstColumn + 0), tss.value().toString());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, firstColumn + 1), tss.lowBound().toString());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, firstColumn + 2), tss.highBound().toString());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, firstColumn + 3), tss.valid() ? tr("Yes") : tr("No"));
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, firstColumn + 4), (tss.outOfRange() && tss.value() < tss.lowBound())  ? tr("Yes") : tr("No"));
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, firstColumn + 5), (tss.outOfRange() && tss.value() > tss.highBound())  ? tr("Yes") : tr("No"));

		row++;
	}
}

void TuningServiceWidget::clearServiceData()
{
//	clientsTabModel()->setRowCount(0);

	// for (int i = 0; i < m_parametersTabModel->rowCount(); i++)
	// {
	// 	m_parametersTabModel->setData(m_parametersTabModel->index(i, 1), "???");
	// }

	// m_tuningSourcesTabModel->setRowCount(0);
	// m_tuningSignalsTabModel->setRowCount(0);
}

void TuningServiceWidget::onTuningSourceDoubleClicked(const QModelIndex &index)
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	int row = index.row();

	const ClientLib::TuningSource* tsClicked = nullptr;
	int lanIndexClicked = 0;
	QString lanEquipmentIdClicked;

	// Find TuningSource and controller from the row
	//
	{
		int rowCounter = 0;

		for (const ClientLib::TuningSource& ts : m_tcpClientSocket->tuningSources())
		{
			for (int c = 0; c < ts.controllersCount(); c++)
			{
				if (rowCounter == row)
				{
					tsClicked = &ts;
					lanIndexClicked = c;
					lanEquipmentIdClicked = ts.controllerEquipmentId(c);

					break;
				}

				rowCounter++;
			}

			// We have found a controller user clicked on
			//
			if (tsClicked != nullptr)
			{
				break;
			}
		}
	}

	if (tsClicked == nullptr)
	{
		return;
	}

	// Now check if we have state for this controller
	//
	{
		bool stateFound = false;

		for (int c = 0; c < tsClicked->statesCount(); c++)
		{
			if (QString::fromStdString(tsClicked->state(c).lanequipmentid()) == lanEquipmentIdClicked)
			{
				stateFound = true;
				break;
			}
		}

		if (stateFound == false)
		{
			QMessageBox::warning(this, qAppName(), tr("No state information for this source!"));
			return;
		}
	}

	// Create or show widget for clicked TuningSource
	//
	for (auto& sourceWidget : m_tuningSourceWidgetList)
	{
		if (sourceWidget->id() == tsClicked->id() &&
			sourceWidget->equipmentId() == tsClicked->equipmentId() &&
			sourceWidget->controllerEquipmentId() == lanEquipmentIdClicked)
		{
			sourceWidget->show();
			sourceWidget->raise();
			sourceWidget->activateWindow();

			return;
		}
	}

	TuningSourceWidget* newWidget = new TuningSourceWidget(tsClicked->id(),
														   tsClicked->equipmentId(),
														   lanEquipmentIdClicked,
														   lanIndexClicked,
														   this);
	newWidget->setClientSocket(m_tcpClientSocket);

	newWidget->show();
	newWidget->raise();
	newWidget->activateWindow();

	m_tuningSourceWidgetList.append(newWidget);

	connect(this, &TuningServiceWidget::newTcpClientSocket, newWidget, &TuningSourceWidget::setClientSocket);
	connect(this, &TuningServiceWidget::clearTcpClientSocket, newWidget, &TuningSourceWidget::unsetClientSocket);

	connect(newWidget, &TuningSourceWidget::forgetMe, this, &TuningServiceWidget::forgetWidget);
}

void TuningServiceWidget::forgetWidget()
{
	TuningSourceWidget *widget = dynamic_cast<TuningSourceWidget*>(sender());
	TEST_PTR_RETURN(widget);
	m_tuningSourceWidgetList.removeAll(widget);
}
