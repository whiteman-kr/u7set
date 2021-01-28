#include "TuningServiceWidget.h"
#include "TcpTuningServiceClient.h"
#include "TuningSourceWidget.h"
#include <QStandardItemModel>
#include <QTableView>

TuningServiceWidget::TuningServiceWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, quint16 udpPort, QWidget *parent) :
	BaseServiceStateWidget(softwareInfo, udpIp, udpPort, parent)
{
	connect(this, &BaseServiceStateWidget::connectionStatisticChanged, this, &TuningServiceWidget::updateStateInfo);

	//----------------------------------------------------------------------------------------------------
	addClientsTab();

	//----------------------------------------------------------------------------------------------------
	QTableView* settingsTableView = addTabWithTableView(250, "Settings");

	m_settingsTabModel = new QStandardItemModel(3, 2, this);
	settingsTableView->setModel(m_settingsTabModel);

	m_settingsTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_settingsTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 0), "Equipment ID");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 0), "Configuration IP 1");
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 0), "Configuration IP 2");

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

	m_tuningSourcesTabModel = new QStandardItemModel(0, tuningSourceHeaderLabels.size(), this);
	tuningSourcesTableView->setModel(m_tuningSourcesTabModel);

	m_tuningSourcesTabModel->setHorizontalHeaderLabels(tuningSourceHeaderLabels);

	tuningSourcesTableView->setColumnWidth(0, 175);
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

	m_tuningSignalsTabModel = new QStandardItemModel(0, tuningSignalsHeaderLabels.size(), this);
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
	if (m_serviceInfo.servicestate() == ServiceState::Work)
	{
		//		quint32 ip = m_serviceInfo.clientrequestip();
		//qint32 port = m_serviceInfo.clientrequestport();

		// Work Here
		Q_ASSERT(false);

		quint32 ip = 0;
		qint32 port = 0;

		quint32 workingIp = getWorkingClientRequestIp();

		if (ip != workingIp)
		{
			//address = QHostAddress(ip).toString() + QString(":%1").arg(port) + " => " + QHostAddress(workingIp).toString() + QString(":%1").arg(port);
			ip = workingIp;
		}

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
			createTcpConnection(getWorkingClientRequestIp(), static_cast<quint16>(port));
		}
	}
}

void TuningServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpTuningServiceClient(softwareInfo(), HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	connect(m_tcpClientSocket, &TcpTuningServiceClient::clientsLoaded, this, &TuningServiceWidget::updateClientsInfo);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::settingsLoaded, this, &TuningServiceWidget::updateServiceSettings);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSourcesInfoLoaded, this, &TuningServiceWidget::reloadTuningSourcesList);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSoursesStateUpdated, this, &TuningServiceWidget::updateTuningSourcesState);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSignalsInfoLoaded, this, &TuningServiceWidget::reloadTuningSignalsList);
	connect(m_tcpClientSocket, &TcpTuningServiceClient::tuningSignalsStateUpdated, this, &TuningServiceWidget::updateTuningSignalsState);

	connect(m_tcpClientSocket, &TcpTuningServiceClient::disconnected, this, &TuningServiceWidget::clearServiceData);

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
		clientsTabModel()->setRowCount(0);
		return;
	}

	updateClientsModel(m_tcpClientSocket->clients());
}

void TuningServiceWidget::updateServiceSettings()
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
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 1), m_tcpClientSocket->configIP1());
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), m_tcpClientSocket->configIP2());
}

void TuningServiceWidget::reloadTuningSourcesList()
{
	m_tuningSourcesTabModel->setRowCount(0);

	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSourcesInfoIsReady() == false)
	{
		return;
	}

	const QList<TuningSource>& tsList = m_tcpClientSocket->tuningSources();

	m_tuningSourcesTabModel->setRowCount(tsList.count());
	int row = 0;

	for (const TuningSource& ts : tsList)
	{
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 0), ts.equipmentId());
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 1), QString::fromStdString(ts.info.lmcaption()));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 2), QString::fromStdString(ts.info.lmip()));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 3), ts.info.lmport());
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 4), QString::fromStdString(ts.info.lmsubsystemchannel()));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 5), ts.info.lmsubsystemkey());
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 6), QString::fromStdString(ts.info.lmsubsystemid()));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, 7), ts.info.lmnumber());

		for (int j = tuningSourceStaticFieldsHeaderLabels.count(); j < m_tuningSourcesTabModel->columnCount(); j++)
		{
			m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, j), "???");
		}

		row++;
	}
}

void TuningServiceWidget::updateTuningSourcesState()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSourcesStateIsReady() == false)
	{
		for (int i = 0; i < m_tuningSourcesTabModel->rowCount(); i++)
		{
			for (int j = tuningSourceStaticFieldsHeaderLabels.count(); j < m_tuningSourcesTabModel->columnCount(); j++)
			{
				m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(i, j), "???");
			}
		}
		return;
	}

	int firstColumn = tuningSourceStaticFieldsHeaderLabels.count();
	int row = 0;

	for (const TuningSource& ts : m_tcpClientSocket->tuningSources())
	{
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 0), ts.state.isreply() ? tr("Yes") : tr("No"));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 1), ts.state.controlisactive() ? tr("Yes") : tr("No"));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 2), ts.state.setsor() ? tr("Yes") : tr("No"));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 3), static_cast<qint64>(ts.state.requestcount()));
		m_tuningSourcesTabModel->setData(m_tuningSourcesTabModel->index(row, firstColumn + 4), static_cast<qint64>(ts.state.replycount()));

		row++;
	}
}

void TuningServiceWidget::reloadTuningSignalsList()
{
	m_tuningSignalsTabModel->setRowCount(0);

	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->tuningSignalsInfoIsReady() == false)
	{
		return;
	}

	const QVector<Signal>& tspVector = m_tcpClientSocket->tuningSignalParams();

	m_tuningSignalsTabModel->setRowCount(tspVector.count());
	int row = 0;

	for (const Signal& tsp : tspVector)
	{
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 0), tsp.customAppSignalID());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 1), tsp.equipmentID());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 2), tsp.appSignalID());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 3), tsp.caption());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 4), tsp.unit());
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 5), E::valueToString<E::SignalType>(tsp.signalType()));
		m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(row, 6), tsp.tuningDefaultValue().toString());

		for (int j = tuningSignalsStaticFieldsHeaderLabels.count(); j < m_tuningSignalsTabModel->columnCount(); j++)
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
			for (int j = tuningSignalsStaticFieldsHeaderLabels.count(); j < m_tuningSignalsTabModel->columnCount(); j++)
			{
				m_tuningSignalsTabModel->setData(m_tuningSignalsTabModel->index(i, j), "???");
			}
		}
		return;
	}

	int firstColumn = tuningSignalsStaticFieldsHeaderLabels.count();
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
	clientsTabModel()->setRowCount(0);

	for (int i = 0; i < m_settingsTabModel->rowCount(); i++)
	{
		m_settingsTabModel->setData(m_settingsTabModel->index(i, 1), "???");
	}

	m_tuningSourcesTabModel->setRowCount(0);
	m_tuningSignalsTabModel->setRowCount(0);
}

void TuningServiceWidget::onTuningSourceDoubleClicked(const QModelIndex &index)
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	int row = index.row();
	const TuningSource& ts = m_tcpClientSocket->tuningSources()[row];

	for (auto& sourceWidget : m_tuningSourceWidgetList)
	{
		if (sourceWidget->id() == ts.id() && sourceWidget->equipmentId() == ts.equipmentId())
		{
			sourceWidget->show();
			sourceWidget->raise();
			sourceWidget->activateWindow();

			return;
		}
	}

	TuningSourceWidget* newWidget = new TuningSourceWidget(ts.id(), ts.equipmentId(), this);
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
