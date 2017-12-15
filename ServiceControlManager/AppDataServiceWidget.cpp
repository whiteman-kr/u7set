#include "AppDataServiceWidget.h"
#include "TcpAppDataClient.h"
#include <QTableView>
#include <QAction>
#include <QHeaderView>
#include <QStandardItemModel>

const int DSC_CAPTION = 0,
DSC_IP = 1,
DSC_PORT = 2,
DSC_PART_COUNT = 3,
DSC_CHANNEL = 4,
DSC_DATA_TYPE = 5,
DSC_EQUIPMENT_ID = 6,
DSC_MODULE_NUMBER = 7,
DSC_MODULE_TYPE = 8,
DSC_SUBSYSTEM_ID = 9,
DSC_SUBSYSTEM_CAPTION = 10,
DSC_ADAPTER_ID = 11,
DSC_ENABLE_DATA = 12,
DSC_DATA_ID = 13,
DSC_FIRST_STATE_COLUMN = 14,
DSC_STATE = 14,
DSC_UPTIME = 15,
DSC_RECEIVED = 16,
DSC_SPEED = 17,
DSC_ERROR_PROTOCOL_VERSION = 18,
DSC_ERROR_FRAMES_QUANTITY = 19,
DSC_ERROR_FRAME_NOMBER = 20,
DSC_LOSTED_FRAMES_COUNT = 21,
DSC_ERROR_DATA_ID = 22,
DSC_ERROR_BAD_FRAME_SIZE = 23,
DSC_COUNT = 24;

const char* const dataSourceColumnStr[] =
{
	"Caption",
	"IP",
	"Port",
	"Part count",
	"Channel",
	"Data type",
	"Equipment ID",
	"Module number",
	"Module type",
	"Subsystem ID",
	"Subsystem caption",
	"Adapter ID",
	"Enable data",
	"Data ID",
	"State",
	"Uptime",
	"Received",
	"Speed",
	"Error Protocol version",
	"Error Frames quantity",
	"Error Frame nomber",
	"Losted frames count",
	"Error Data ID",
	"Error Bad frame size",
};

const int DATA_SOURCE_COLUMN_COUNT = sizeof(dataSourceColumnStr) / sizeof(dataSourceColumnStr[0]);

const QVector<int> defaultSourceColumnVisibility =
{
	DSC_CAPTION,
	DSC_IP,
	DSC_PORT,
	DSC_PART_COUNT,
	DSC_STATE,
	DSC_UPTIME,
	DSC_RECEIVED,
	DSC_SPEED
};


const int SC_ID = 0,
SC_CAPTION = 1,
SC_FIRST_STATE_COLUMN = 2,
SC_VALUE = 2,
SC_VALID = 3,
SC_UNIT = 4,
SC_COUNT = 5;

const char* const signalColumnStr[] =
{
	"ID",
	"Caption",
	"Value",
	"Valid",
	"Unit"
};

const int SIGNAL_COLUMN_COUNT = sizeof(signalColumnStr) / sizeof(signalColumnStr[0]);



DataSourcesStateModel::DataSourcesStateModel(TcpAppDataClient* clientSocket, QObject* parent) :
	QAbstractTableModel(parent),
	m_clientSocket(clientSocket)
{
	static_assert(DSC_COUNT == DATA_SOURCE_COLUMN_COUNT, "Data source column count error");
}

DataSourcesStateModel::~DataSourcesStateModel()
{
}

int DataSourcesStateModel::rowCount(const QModelIndex&) const
{
	return m_dataSource.count();
}

int DataSourcesStateModel::columnCount(const QModelIndex&) const
{
	return DATA_SOURCE_COLUMN_COUNT;
}


QVariant DataSourcesStateModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	if (row < 0 || row > m_dataSource.count())
	{
		return QVariant();
	}

	const DataSource& d = *m_dataSource[row];

	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column())
			{
				case DSC_CAPTION: return d.lmCaption();
				case DSC_IP: return d.lmAddressStr();
				case DSC_PORT: return d.lmPort();
				case DSC_PART_COUNT: return d.partCount();
				case DSC_CHANNEL: return d.lmChannel();
				case DSC_DATA_TYPE: return d.lmDataTypeStr();
				case DSC_EQUIPMENT_ID: return d.lmEquipmentID();
				case DSC_MODULE_NUMBER: return d.lmNumber();
				case DSC_MODULE_TYPE: return d.lmModuleType();
				case DSC_SUBSYSTEM_ID: return d.lmSubsystemID();
				case DSC_SUBSYSTEM_CAPTION: return d.lmSubsystem();
				case DSC_ADAPTER_ID: return d.lmAdapterID();
				case DSC_ENABLE_DATA: return d.lmDataEnable();
				case DSC_DATA_ID: return d.lmDataID();
				case DSC_STATE: return E::valueToString<E::DataSourceState>(TO_INT(d.state()));
				case DSC_UPTIME:
				{
					auto time = d.uptime();
					int s = time % 60; time /= 60;
					int m = time % 60; time /= 60;
					int h = time % 24; time /= 24;
					return QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
				}
				case DSC_RECEIVED: return d.receivedDataSize();
				case DSC_SPEED: return d.dataReceivingRate();
				case DSC_ERROR_PROTOCOL_VERSION: return d.errorProtocolVersion();
				case DSC_ERROR_FRAMES_QUANTITY: return d.errorFramesQuantity();
				case DSC_ERROR_FRAME_NOMBER: return d.errorFrameNo();
				case DSC_LOSTED_FRAMES_COUNT: return d.lostedFramesCount();
				case DSC_ERROR_DATA_ID: return d.errorDataID();
				case DSC_ERROR_BAD_FRAME_SIZE: return d.errorBadFrameSize();
				default:
					assert(false);
				return QVariant();
			}
		break;
		case Qt::BackgroundRole:
			if (d.hasErrors())
			{
				return QBrush(QColor(0xff,0xee,0xee));
			}
		break;
		default:
			return QVariant();
	}

	return QVariant();
}

QVariant DataSourcesStateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal && section < DATA_SOURCE_COLUMN_COUNT)
		{
			return dataSourceColumnStr[section];
		}
		if (orientation == Qt::Vertical && section < m_dataSource.count())
		{
			return section + 1;
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

void DataSourcesStateModel::updateData(int firstRow, int lastRow, int firstColumn, int lastColumn)
{
	emit dataChanged(index(firstRow, firstColumn), index(lastRow, lastColumn), QVector<int>() << Qt::DisplayRole);
}

void DataSourcesStateModel::updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
}

void DataSourcesStateModel::invalidateData()
{
	beginResetModel();
	m_dataSource.clear();
	endResetModel();
}

void DataSourcesStateModel::reloadList()
{
	beginResetModel();
	m_dataSource = m_clientSocket->dataSources();
	qSort(m_dataSource.begin(), m_dataSource.end(), [](const DataSource* ds1, const DataSource* ds2) {
		return ds1->lmAddress32() < ds2->lmAddress32();
	});
	endResetModel();
}


AppDataServiceWidget::AppDataServiceWidget(const SoftwareInfo& softwareInfo, quint32 ip, int portIndex, QWidget *parent) :
	BaseServiceStateWidget(softwareInfo, ip, portIndex, parent)
{
	setStateTabMaxRowQuantity(8);

	assert(false);			// WM: const PORT_APP_DATA_SERVICE_CLIENT_REQUEST in next code should be replaced by port from RPCT configueration!!!!
	m_tcpClientSocket = new TcpAppDataClient(softwareInfo, HostAddressPort(getWorkingClientRequestIp(), PORT_APP_DATA_SERVICE_CLIENT_REQUEST));
	m_appDataClientTread = new SimpleThread(m_tcpClientSocket);

	// Data Sources
	m_dataSourcesStateModel = new DataSourcesStateModel(m_tcpClientSocket, this);
	m_dataSourcesView = addTabWithTableView(100, tr("AppData Sources"));;
	m_dataSourcesView->setModel(m_dataSourcesStateModel);

	connect(m_tcpClientSocket, &TcpAppDataClient::dataSourcesInfoLoaded, m_dataSourcesStateModel, &DataSourcesStateModel::reloadList);
	connect(m_tcpClientSocket, &TcpAppDataClient::dataSoursesStateUpdated, this, &AppDataServiceWidget::updateSourceStateColumns);
	connect(m_tcpClientSocket, &TcpAppDataClient::disconnected, m_dataSourcesStateModel, &DataSourcesStateModel::invalidateData);

	QSettings settings;
	QHeaderView* horizontalHeader = m_dataSourcesView->horizontalHeader();
	for (int i = 0; i < DSC_COUNT; i++)
	{
		m_dataSourcesView->setColumnWidth(i, settings.value(QString("DataAquisitionServiceWidget/SourceColumnWidth/%1").arg(QString(dataSourceColumnStr[i]).replace("/", "|")).replace("\n", " "),
															m_dataSourcesView->columnWidth(i)).toInt());
	}

	// Data Source columns actions
	m_sourceTableHeadersContextMenuActions = new QActionGroup(this);
	m_sourceTableHeadersContextMenuActions->setExclusive(false);
	QAction* columnAction = m_sourceTableHeadersContextMenuActions->addAction("All columns");
	columnAction->setCheckable(true);
	columnAction->setChecked(settings.value("DataAquisitionServiceWidget/SourceColumnVisibility/All columns", true).toBool());

	for (int i = 0; i < DSC_COUNT; i++)
	{
		columnAction = m_sourceTableHeadersContextMenuActions->addAction(QString(dataSourceColumnStr[i]).replace("\n", " "));
		columnAction->setCheckable(true);
		bool checked = settings.value(QString("DataAquisitionServiceWidget/SourceColumnVisibility/%1").arg(QString(dataSourceColumnStr[i]).replace("/", "|")).replace("\n", " "),
									  defaultSourceColumnVisibility.contains(i)).toBool();
		columnAction->setChecked(checked);
		m_dataSourcesView->setColumnHidden(i, !checked);
	}

	horizontalHeader->setContextMenuPolicy(Qt::ActionsContextMenu);
	horizontalHeader->addActions(m_sourceTableHeadersContextMenuActions->actions());
	connect(m_sourceTableHeadersContextMenuActions, &QActionGroup::triggered, this, &AppDataServiceWidget::changeSourceColumnVisibility);
	connect(horizontalHeader, &QHeaderView::sectionResized, this, &AppDataServiceWidget::saveSourceColumnWidth);

	// Signals
	m_signalStateModel = new SignalStateModel(m_tcpClientSocket, this);
	m_signalsView = addTabWithTableView(250, tr("Signals"));;
	m_signalsView->setModel(m_signalStateModel);

	connect(m_tcpClientSocket, &TcpAppDataClient::appSignalListLoaded, m_signalStateModel, &SignalStateModel::reloadList);
	connect(m_tcpClientSocket, &TcpAppDataClient::appSignalsStateUpdated, this, &AppDataServiceWidget::updateSignalStateColumns);
	connect(m_tcpClientSocket, &TcpAppDataClient::disconnected, m_signalStateModel, &SignalStateModel::invalidateData);

	addTab(new QTableView(this), tr("Clients"));

	addTab(new QTableView(this), tr("Settings"));

	addTab(new QTableView(this), tr("Log"));

	m_appDataClientTread->start();
}

AppDataServiceWidget::~AppDataServiceWidget()
{
	m_appDataClientTread->quitAndWait();
}

void AppDataServiceWidget::updateServiceState()
{
}

void AppDataServiceWidget::updateStateInfo()
{
	if (m_serviceInfo.servicestate() == ServiceState::Work)
	{
		stateTabModel()->setData(stateTabModel()->index(5, 0), "Connected client quantity");
		stateTabModel()->setData(stateTabModel()->index(6, 0), "Connected to CfgService");
		stateTabModel()->setData(stateTabModel()->index(7, 0), "Connected to ArchiveService");

		stateTabModel()->setData(stateTabModel()->index(5, 1), "???");
		stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
		stateTabModel()->setData(stateTabModel()->index(7, 1), "???");
	}
}

void AppDataServiceWidget::updateSourceInfo()
{
	m_dataSourcesStateModel->updateData(m_dataSourcesView->indexAt(QPoint(0, 0)),
										m_dataSourcesView->indexAt(QPoint(m_dataSourcesView->width(), m_dataSourcesView->height())));
}

void AppDataServiceWidget::updateSignalInfo()
{
	m_signalStateModel->updateData(m_signalsView->indexAt(QPoint(0, 0)),
								   m_signalsView->indexAt(QPoint(m_signalsView->width(), m_signalsView->height())));
}

void AppDataServiceWidget::updateSourceStateColumns()
{
	int firstRow = m_dataSourcesView->rowAt(0);
	int lastRow = m_dataSourcesView->rowAt(m_signalsView->height());
	if (lastRow == -1)
	{
		lastRow = m_dataSourcesStateModel->rowCount() - 1;
	}

	int firstColumn = m_dataSourcesView->columnAt(0);
	int lastColumn = m_dataSourcesView->columnAt(m_dataSourcesView->width());
	if (lastColumn == -1)
	{
		lastColumn = m_dataSourcesStateModel->columnCount() - 1;
	}
	if (lastColumn < DSC_FIRST_STATE_COLUMN)
	{
		return;
	}

	m_dataSourcesStateModel->updateData(firstRow,
										lastRow,
										std::max(firstColumn, DSC_FIRST_STATE_COLUMN),
										lastColumn);
}

void AppDataServiceWidget::updateSignalStateColumns()
{
	int firstRow = m_signalsView->rowAt(0);
	int lastColumn = m_signalsView->columnAt(m_signalsView->width());
	if (lastColumn == -1)
	{
		lastColumn = m_signalStateModel->columnCount() - 1;
	}

	int firstColumn = m_signalsView->columnAt(0);
	if (lastColumn < SC_FIRST_STATE_COLUMN)
	{
		return;
	}
	int lastRow = m_signalsView->rowAt(m_signalsView->height());
	if (lastRow == -1)
	{
		lastRow = m_signalStateModel->rowCount() - 1;
	}

	m_signalStateModel->updateData(firstRow,
								   lastRow,
								   std::max(firstColumn, SC_FIRST_STATE_COLUMN),
								   lastColumn);
}

void AppDataServiceWidget::checkVisibility()
{
	/*if (isVisible() && !m_dataSourcesStateModel->isActive())
	{
		m_dataSourcesStateModel->setActive(true);
	}

	if (!isVisible() && m_dataSourcesStateModel->isActive())
	{
		m_dataSourcesStateModel->setActive(false);
	}*/
}

void AppDataServiceWidget::saveSourceColumnWidth(int index)
{
	QSettings settings;
	settings.setValue(QString("DataAquisitionServiceWidget/SourceColumnWidth/%1").arg(QString(dataSourceColumnStr[index]).replace("/", "|")).replace("\n", " "), m_dataSourcesView->columnWidth(index));
}

void AppDataServiceWidget::saveSourceColumnVisibility(int index, bool visible)
{
	QSettings settings;
	settings.setValue(QString("DataAquisitionServiceWidget/SourceColumnVisibility/%1").arg(QString(dataSourceColumnStr[index]).replace("/", "|")).replace("\n", " "), visible);
}

void AppDataServiceWidget::changeSourceColumnVisibility(QAction* action)
{
	int actionIndex = m_sourceTableHeadersContextMenuActions->actions().indexOf(action);
	if (actionIndex == 0)
	{
		for (int i = 0; i < DSC_COUNT; i++)
		{
			if (!action->isChecked())
			{
				saveSourceColumnWidth(i);
			}
			saveSourceColumnVisibility(i, action->isChecked());
			m_dataSourcesView->setColumnHidden(i, !action->isChecked());
			m_sourceTableHeadersContextMenuActions->actions()[i + 1]->setChecked(action->isChecked());
		}
	}
	else
	{
		if (!action->isChecked())
		{
			saveSourceColumnWidth(actionIndex - 1);
		}
		saveSourceColumnVisibility(actionIndex - 1, action->isChecked());
		m_dataSourcesView->setColumnHidden(actionIndex - 1, !action->isChecked());
	}
	if (m_dataSourcesView->horizontalHeader()->hiddenSectionCount() == DSC_COUNT)
	{
		m_dataSourcesView->showColumn(DSC_CAPTION);
		m_sourceTableHeadersContextMenuActions->actions()[DSC_CAPTION + 1]->setChecked(true);
		saveSourceColumnVisibility(DSC_CAPTION, true);
	}
}

SignalStateModel::SignalStateModel(TcpAppDataClient* clientSocket, QObject* parent) :
	QAbstractTableModel(parent),
	m_clientSocket(clientSocket)
{
}

SignalStateModel::~SignalStateModel()
{
}

int SignalStateModel::rowCount(const QModelIndex&) const
{
	return m_clientSocket->signalParams().count();
}

int SignalStateModel::columnCount(const QModelIndex&) const
{
	return SIGNAL_COLUMN_COUNT;
}

QVariant SignalStateModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	if (row < 0 || row > m_clientSocket->signalParams().count())
	{
		return QVariant();
	}
	if (role == Qt::DisplayRole)
	{
		const Signal& s = m_clientSocket->signalParams()[row];
		switch (index.column())
		{
			case SC_ID: return s.appSignalID();
			case SC_CAPTION: return s.caption();
			case SC_VALUE:
			{
				if (row < 0 || row > m_clientSocket->signalStates().count())
				{
					return QVariant();
				}
				const AppSignalState& ass = m_clientSocket->signalStates()[row];
				return ass.m_value;
			}
			case SC_VALID:
			{
				if (row < 0 || row > m_clientSocket->signalStates().count())
				{
					return QVariant();
				}
				const AppSignalState& ass = m_clientSocket->signalStates()[row];
				return ass.m_flags.valid ? tr("Yes") : tr("No");
			}
			case SC_UNIT: return s.unit();
			default:
				assert(false);
		}
	}
	return QVariant();
}

QVariant SignalStateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal && section < DATA_SOURCE_COLUMN_COUNT)
		{
			return signalColumnStr[section];
		}
		if (orientation == Qt::Vertical)
		{
			return section + 1;
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

void SignalStateModel::updateData(int firstRow, int lastRow, int firstColumn, int lastColumn)
{
	emit dataChanged(index(firstRow, firstColumn), index(lastRow, lastColumn), QVector<int>() << Qt::DisplayRole);
}

void SignalStateModel::updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
}

void SignalStateModel::invalidateData()
{
	beginResetModel();
	endResetModel();
}

void SignalStateModel::reloadList()
{
	beginResetModel();
	endResetModel();
}
