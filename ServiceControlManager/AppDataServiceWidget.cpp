#include "AppDataServiceWidget.h"
#include "ScmTcpAppDataClient.h"
#include "AppDataSourceWidget.h"
#include <QTableView>
#include <QAction>
#include <QHeaderView>
#include <QStandardItemModel>
#include "../lib/WidgetUtils.h"

const int DSC_EQUIPMENT_ID = 0,
DSC_DATA_ID = 1,
DSC_IP = 2,
DSC_ENABLE_DATA = 3,
DSC_STATE = 4,
DSC_UPTIME = 5,
DSC_RECEIVED = 6,
DSC_SPEED = 7,
DSC_CAPTION = 8,
DSC_PORT = 9,
DSC_RUP_FRAMES_QUANTITY = 10,
DSC_DATA_TYPE = 11,
DSC_MODULE_NUMBER = 12,
DSC_MODULE_TYPE = 13,
DSC_SUBSYSTEM_ID = 14,
DSC_SUBSYSTEM_CAPTION = 15,
DSC_SUBSYSTEM_CHANNEL = 16,
DSC_ADAPTER_ID = 17,
DSC_UNIQUE_ID = 18,

DSC_RECEIVES_DATA = 19,
DSC_RECEIVED_DATA_ID = 20,
DSC_RUP_FRAMES_QUEUE_SIZE = 21,
DSC_RUP_FRAMES_QUEUE_MAX_SIZE = 22,
DSC_RECEIVED_FRAMES_COUNT = 23,
DSC_RECEIVED_PACKET_COUNT = 24,
DSC_DATA_PROCESSING_ENABLED = 25,
DSC_PROCESSED_PACKET_COUNT = 26,
DSC_LAST_PACKET_SYSTEM_TIME = 27,
DSC_RUP_FRAME_PLANT_TIME = 28,
DSC_RUP_FRAME_NUMERATOR = 29,
DSC_SIGNAL_STATES_QUEUE_SIZE = 30,
DSC_SIGNAL_STATES_QUEUE_MAX_SIZE = 31,
DSC_ACQUIRED_SIGNALS_COUNT = 32,

DSC_ERROR_PROTOCOL_VERSION = 33,
DSC_ERROR_FRAMES_QUANTITY = 34,
DSC_ERROR_FRAME_NOMBER = 35,
DSC_LOST_PACKET_COUNT = 36,
DSC_ERROR_DATA_ID = 37,
DSC_ERROR_BAD_FRAME_SIZE = 38,
DSC_ERROR_DUPLICATE_PLANT_TIME = 39,
DSC_ERROR_NONMONOTONIC_PLANT_TIME = 40,
DSC_COUNT = 41;

const int dataSourceStateColumn[] =
{
	DSC_STATE,
	DSC_UPTIME,
	DSC_RECEIVED,
	DSC_SPEED,
	DSC_ERROR_PROTOCOL_VERSION,
	DSC_ERROR_FRAMES_QUANTITY,
	DSC_ERROR_FRAME_NOMBER,
	DSC_LOST_PACKET_COUNT,
};

const int DATA_SOURCE_STATE_COLUMN_COUNT = sizeof(dataSourceStateColumn) / sizeof(dataSourceStateColumn[0]);

const char* const dataSourceColumnStr[] =
{
	"Equipment ID",
	"Data ID",
	"IP",
	"Enable data",
	"State",
	"Uptime",
	"Received",
	"Speed",
	"Caption",
	"Port",
	"Frames count",
	"Data type",
	"Module number",
	"Module type",
	"Subsystem ID",
	"Subsystem caption",
	"Subsystem channel",
	"Adapter ID",
	"Unique ID",

	"Receives data",
	"Received data ID",
	"RUP frames queue size",
	"RUP frames queue max size",
	"Received frames count",
	"Received packet count",
	"Data processing enabled",
	"Processed packet count",
	"Last packet system time",
	"RUP frame plant time",
	"RUP frame numerator",
	"Signal states queue size",
	"Signal states queue max size",
	"Acquired signals count",

	"Error Protocol version",
	"Error Frames quantity",
	"Error Frame nomber",
	"Lost packet count",
	"Error Data ID",
	"Error Bad frame size",
	"Error Duplicate plant time",
	"Error nonmonotonic plant time",
};

const int DATA_SOURCE_COLUMN_COUNT = sizeof(dataSourceColumnStr) / sizeof(dataSourceColumnStr[0]);

const QVector<int> defaultSourceColumnVisibility =
{
	DSC_CAPTION,
	DSC_IP,
	DSC_PORT,
	DSC_RUP_FRAMES_QUANTITY,
	DSC_STATE,
	DSC_UPTIME,
	DSC_RECEIVED,
	DSC_SPEED
};


const int SC_APP_SIGNAL_ID = 0,
SC_CUSTOM_APP_SIGNAL_ID = 1,
SC_CAPTION = 2,
SC_EQUIPMENT_ID = 3,
SC_LM_EQUIPMENT_ID = 4,
SC_BUS_TYPE_ID = 5,
SC_CHANNEL = 6,
SC_SIGNAL_TYPE = 7,
SC_IN_OUT_TYPE = 8,
SC_DATA_SIZE = 9,
SC_BYTE_ORDER = 10,
SC_ANALOG_SIGNAL_FORMAT = 11,
SC_UNIT = 12,
SC_ENABLE_TUNING = 13,
SC_TUNING_DEFAULT_VALUE = 14,
SC_TUNING_LOW_BOUND = 15,
SC_TUNING_HIGH_BOUND = 16,
SC_ACQUIRE = 17,
SC_ARCHIVE = 18,
SC_DECIMAL_PLACES = 19,
SC_COARSE_APERTURE = 20,
SC_FINE_APERTURE = 21,
SC_ADAPTIVE_APERTURE = 22,
SC_IO_BUF_ADDR = 23,
SC_TUNING_ADDR = 24,
SC_UAL_ADDR = 25,
SC_REG_BUF_ADDR = 26,
SC_REG_VALUE_ADDR = 27,
SC_REG_VALIDITY_ADDR = 28,

// Spec prop struct
//
SC_LOW_VALID_RANGE = 29,
SC_HIGH_VALID_RANGE = 30,

SC_FIRST_STATE_COLUMN = 31,
SC_VALUE = 31,
SC_IS_VALID = 32,
SC_IS_FINE_APERTURE = 33,
SC_IS_COARSE_APERTURE = 34,
SC_IS_AUTO_POINT = 35,
SC_IS_VALIDITY_CHANGE = 36,
SC_SYSTEM_TIME = 37,
SC_LOCAL_TIME = 38,
SC_PLANT_TIME = 39,
SC_COUNT = 40;

const char* const signalColumnStr[] =
{
	"ID",
	"CustomID",
	"Caption",
	"EquipmentID",
	"LmEquipmentID",
	"BusTypeID",
	"Channel",
	"SignalType",
	"InOutType",
	"DataSize",
	"ByteOrder",
	"AnalogSignalFormat",
	"Unit",
	"EnableTuning",
	"TuningDefaultValue",
	"TuningLowBound",
	"TuningHighBound",
	"Acquire",
	"Archive",
	"DecimalPlaces",
	"CoarseAperture",
	"FineAperture",
	"AdaptiveAperture",
	"IoBufAddr",
	"TuningAddr",
	"UalAddr",
	"RegBufAddr",
	"RegValueAddr",
	"RegValidityAddr",

	"LowValidRange",
	"HighValidRange",

	"Value",
	"IsValid",
	"IsFineAperture",
	"IsCoarseAperture",
	"IsAutoPoint",
	"IsValidityChange",
	"SystemTime",
	"LocalTime",
	"PlantTime",
};

const int SIGNAL_COLUMN_COUNT = sizeof(signalColumnStr) / sizeof(signalColumnStr[0]);

const QVector<int> defaultSignalColumnVisibility =
{
	SC_APP_SIGNAL_ID,
	SC_CAPTION,
	SC_FIRST_STATE_COLUMN,
	SC_VALUE,
	SC_IS_VALID,
	SC_UNIT,
	SC_LOW_VALID_RANGE,
	SC_HIGH_VALID_RANGE,
	SC_COUNT
};

DataSourcesStateModel::DataSourcesStateModel(QObject* parent) :
	QAbstractTableModel(parent),
	m_clientSocket(nullptr)
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

	const AppDataSource& source = *m_dataSource[row];

	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column())
			{
				// DataSourceInfo
				//
				case DSC_CAPTION: return source.lmCaption();
				case DSC_IP: return source.lmAddressStr();
				case DSC_PORT: return source.lmPort();
				case DSC_RUP_FRAMES_QUANTITY: return source.lmRupFramesQuantity();
				case DSC_DATA_TYPE: return source.lmDataTypeStr();
				case DSC_EQUIPMENT_ID: return source.lmEquipmentID();
				case DSC_MODULE_NUMBER: return source.lmNumber();
				case DSC_MODULE_TYPE: return source.lmModuleType();
				case DSC_SUBSYSTEM_ID: return source.lmSubsystemKey();
				case DSC_SUBSYSTEM_CAPTION: return source.lmSubsystem();
				case DSC_SUBSYSTEM_CHANNEL: return source.lmSubsystemChannel();
				case DSC_ADAPTER_ID: return source.lmAdapterID();
				case DSC_ENABLE_DATA: return source.lmDataEnable();
				case DSC_DATA_ID: return "0x" + QString("%1").arg(source.lmDataID(), sizeof(source.lmDataID()) * 2, 16, QChar('0')).toUpper();
				case DSC_UNIQUE_ID: return "0x" + QString("%1").arg(source.lmUniqueID(), sizeof(source.lmUniqueID()) * 2, 16, QChar('0')).toUpper();
				case DSC_STATE: return E::valueToString<E::DataSourceState>(TO_INT(source.state()));

				// DataSourceState
				//
				case DSC_UPTIME:
				{
					auto time = source.uptime();
					int s = time % 60; time /= 60;
					int m = time % 60; time /= 60;
					int h = time % 24; time /= 24;
					return QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
				}
				case DSC_RECEIVED: return source.receivedDataSize();
				case DSC_SPEED: return source.dataReceivingRate();
				case DSC_RECEIVES_DATA: return source.dataReceives();
				case DSC_RECEIVED_DATA_ID: return "0x" + QString("%1").arg(source.receivedDataID(), sizeof(source.receivedDataID()) * 2, 16, QChar('0')).toUpper();
				case DSC_RUP_FRAMES_QUEUE_SIZE: return source.rupFramesQueueCurSize();
				case DSC_RUP_FRAMES_QUEUE_MAX_SIZE: return source.rupFramesQueueCurMaxSize();
				case DSC_RECEIVED_FRAMES_COUNT: return source.receivedFramesCount();
				case DSC_RECEIVED_PACKET_COUNT: return source.receivedPacketCount();
				case DSC_DATA_PROCESSING_ENABLED: return source.dataProcessingEnabled();
				case DSC_PROCESSED_PACKET_COUNT: return source.processedPacketCount();
				case DSC_LAST_PACKET_SYSTEM_TIME: return QDateTime::fromMSecsSinceEpoch(source.lastPacketSystemTime());
				case DSC_RUP_FRAME_PLANT_TIME: return QDateTime::fromMSecsSinceEpoch(source.rupFramePlantTime());
				case DSC_RUP_FRAME_NUMERATOR: return source.rupFrameNumerator();
				case DSC_SIGNAL_STATES_QUEUE_SIZE: return QString("%1 (%2%%)").arg(source.signalStatesQueueCurSize()).arg(0.01 * source.signalStatesQueueCurSize() / source.signalStatesQueueSize());
				case DSC_SIGNAL_STATES_QUEUE_MAX_SIZE: return QString("%1 (%2%%)").arg(source.signalStatesQueueCurMaxSize()).arg(0.01 * source.signalStatesQueueCurMaxSize() / source.signalStatesQueueSize());
				case DSC_ACQUIRED_SIGNALS_COUNT: return source.acquiredSignalsCount();

				case DSC_ERROR_PROTOCOL_VERSION: return source.errorProtocolVersion();
				case DSC_ERROR_FRAMES_QUANTITY: return source.errorFramesQuantity();
				case DSC_ERROR_FRAME_NOMBER: return source.errorFrameNo();
				case DSC_LOST_PACKET_COUNT: return source.lostPacketCount();
				case DSC_ERROR_DATA_ID: return source.errorDataID();
				case DSC_ERROR_BAD_FRAME_SIZE: return source.errorFrameSize();
				case DSC_ERROR_DUPLICATE_PLANT_TIME: return source.errorDuplicatePlantTime();
				case DSC_ERROR_NONMONOTONIC_PLANT_TIME: return source.errorNonmonotonicPlantTime();
				default:
					assert(false);
				return QVariant();
			}
			break;

		case Qt::BackgroundRole:
//			if (d.hasErrors())
//			{
//				return QBrush(QColor(0xff,0xee,0xee));
//			}
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

const AppDataSource* DataSourcesStateModel::getDataSource(int row) const
{
	return m_dataSource[row];
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
	if (m_clientSocket != nullptr)
	{
		m_dataSource = m_clientSocket->dataSources();
		qSort(m_dataSource.begin(), m_dataSource.end(), [](const DataSource* ds1, const DataSource* ds2) {
			return ds1->lmAddress32() < ds2->lmAddress32();
		});
	}
	endResetModel();
}


AppDataServiceWidget::AppDataServiceWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, quint16 udpPort, QWidget *parent) :
	BaseServiceStateWidget(softwareInfo, udpIp, udpPort, parent),
	m_tcpClientSocket(nullptr),
	m_tcpClientThread(nullptr)
{
	connect(this, &BaseServiceStateWidget::connectionStatisticChanged, this, &AppDataServiceWidget::updateStateInfo);

	setStateTabMaxRowQuantity(17);
	setClientQuantityRowIndexOnStateTab(5);

	// Data Sources
	m_dataSourcesStateModel = new DataSourcesStateModel(this);
	m_dataSourcesView = addTabWithTableView(100, tr("AppData Sources"));;
	m_dataSourcesView->setModel(m_dataSourcesStateModel);

	connect(m_dataSourcesView, &QTableView::doubleClicked, this, &AppDataServiceWidget::onAppDataSourceDoubleClicked);

	new TableDataVisibilityController(m_dataSourcesView, "AppDataServiceWidget/DataSources", defaultSourceColumnVisibility);

	// Signals
	m_signalStateModel = new SignalStateModel(this);

	QSortFilterProxyModel* sortModel = new QSortFilterProxyModel(this);
	sortModel->setSourceModel(m_signalStateModel);

	m_signalsView = addTabWithTableView(250, tr("Signals"));;
	m_signalsView->setModel(sortModel);
	m_signalsView->setSortingEnabled(true);

	new TableDataVisibilityController(m_signalsView, "AppDataServiceWidget/Signals", defaultSignalColumnVisibility);

	// Clients
	addClientsTab(false);

	// Settings
	QTableView* settingsTableView = addTabWithTableView(250, "Settings");

	m_settingsTabModel = new QStandardItemModel(3, 2, this);
	settingsTableView->setModel(m_settingsTabModel);

	m_settingsTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_settingsTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 0), "Equipment ID");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 0), "Configuration IP 1");
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 0), "Configuration IP 2");

	// Log
	addTab(new QTableView(this), tr("Log"));
}

AppDataServiceWidget::~AppDataServiceWidget()
{
	for (auto* widget : m_appDataSourceWidgetList)
	{
		widget->deleteLater();
	}
	m_appDataSourceWidgetList.clear();
	dropTcpConnection();
}

void AppDataServiceWidget::updateServiceState()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->stateIsReady() == false)
	{
		assert(false);
		return;
	}
	stateTabModel()->setData(stateTabModel()->index(6, 1), m_tcpClientSocket->configServiceConnectionState());
	stateTabModel()->setData(stateTabModel()->index(7, 1), m_tcpClientSocket->archiveServiceConnectionState());

	auto state = m_tcpClientSocket->serviceState().appdatareceivestate();

	stateTabModel()->setData(stateTabModel()->index(8, 1), static_cast<qint64>(state.receivingrate()));
	stateTabModel()->setData(stateTabModel()->index(9, 1), static_cast<qint64>(state.udpreceivingrate()));
	stateTabModel()->setData(stateTabModel()->index(10, 1), static_cast<qint64>(state.rupframesreceivingrate()));

	stateTabModel()->setData(stateTabModel()->index(11, 1), static_cast<qint64>(state.rupframescount()));
	stateTabModel()->setData(stateTabModel()->index(12, 1), static_cast<qint64>(state.simframescount()));

	stateTabModel()->setData(stateTabModel()->index(13, 1), static_cast<qint64>(state.errdatagramsize()));
	stateTabModel()->setData(stateTabModel()->index(14, 1), static_cast<qint64>(state.errsimversion()));
	stateTabModel()->setData(stateTabModel()->index(15, 1), static_cast<qint64>(state.errunknownappdatasourceip()));
	stateTabModel()->setData(stateTabModel()->index(16, 1), static_cast<qint64>(state.errrupframecrc()));
}

void AppDataServiceWidget::updateStateInfo()
{
	if (m_serviceInfo.servicestate() == ServiceState::Work)
	{
		stateTabModel()->setData(stateTabModel()->index(5, 0), "Connected client quantity");
		stateTabModel()->setData(stateTabModel()->index(6, 0), "Connected to CfgService");
		stateTabModel()->setData(stateTabModel()->index(7, 0), "Connected to ArchiveService");

		stateTabModel()->setData(stateTabModel()->index(8, 0), "Receiving rate");
		stateTabModel()->setData(stateTabModel()->index(9, 0), "UDP receiving rate");
		stateTabModel()->setData(stateTabModel()->index(10, 0), "RUP frames receiving rate");

		stateTabModel()->setData(stateTabModel()->index(11, 0), "RUP frames count");
		stateTabModel()->setData(stateTabModel()->index(12, 0), "Simulated frames count");

		stateTabModel()->setData(stateTabModel()->index(13, 0), "Datagram size errors");
		stateTabModel()->setData(stateTabModel()->index(14, 0), "Simulation version errors");
		stateTabModel()->setData(stateTabModel()->index(15, 0), "Unknown AppDataSource IP errors");
		stateTabModel()->setData(stateTabModel()->index(16, 0), "RUP frames CRC errors");

		if (m_tcpClientSocket == nullptr || m_tcpClientSocket->stateIsReady() == false)
		{
			stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
			stateTabModel()->setData(stateTabModel()->index(7, 1), "???");
		}
		else
		{
			updateServiceState();
		}
	}

	quint32 ip = m_serviceInfo.clientrequestip();
	qint32 port = m_serviceInfo.clientrequestport();

	quint32 workingIp = getWorkingClientRequestIp();

	if (ip != workingIp)
	{
		ip = workingIp;
	}

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
		createTcpConnection(getWorkingClientRequestIp(), static_cast<quint16>(port));
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
	int firstVisibleRow = m_dataSourcesView->rowAt(0);
	int lastVisibleRow = m_dataSourcesView->rowAt(m_signalsView->height());
	if (lastVisibleRow == -1)
	{
		lastVisibleRow = m_dataSourcesStateModel->rowCount() - 1;
	}

	int firstVisibleColumn = m_dataSourcesView->columnAt(0);
	int lastVisibleColumn = m_dataSourcesView->columnAt(m_dataSourcesView->width());
	if (lastVisibleColumn == -1)
	{
		lastVisibleColumn = m_dataSourcesStateModel->columnCount() - 1;
	}

	for (int i = 0; i < DATA_SOURCE_STATE_COLUMN_COUNT; i++)
	{
		int currentColumn = dataSourceStateColumn[i];
		if (currentColumn >= firstVisibleColumn && currentColumn <= lastVisibleColumn)
		{
			m_dataSourcesStateModel->updateData(firstVisibleRow,
												lastVisibleRow,
												currentColumn,
												currentColumn);
		}
	}
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

void AppDataServiceWidget::updateClientsInfo()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->clientsIsReady() == false)
	{
		clientsTabModel()->setRowCount(0);
		return;
	}

	updateClientsModel(m_tcpClientSocket->clients());
}

void AppDataServiceWidget::updateSettings()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->settingsIsReady() == false)
	{
		assert(false);
		return;
	}
	m_settingsTabModel->setData(m_settingsTabModel->index(0, 1), m_tcpClientSocket->equipmentID());
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 1), m_tcpClientSocket->configIP1());
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), m_tcpClientSocket->configIP2());
}

void AppDataServiceWidget::clearServiceData()
{
	stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
	stateTabModel()->setData(stateTabModel()->index(7, 1), "???");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 1), "???");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 1), "???");
	m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), "???");
}

void AppDataServiceWidget::onAppDataSourceDoubleClicked(const QModelIndex &index)
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	int row = index.row();
	const AppDataSource* ads = m_dataSourcesStateModel->getDataSource(row);

	TEST_PTR_RETURN(ads);

	for (auto& sourceWidget : m_appDataSourceWidgetList)
	{
		TEST_PTR_CONTINUE(sourceWidget);
		if (sourceWidget->id() == ads->lmUniqueID() && sourceWidget->equipmentId() == ads->lmEquipmentID())
		{
			sourceWidget->show();
			sourceWidget->raise();
			sourceWidget->activateWindow();

			return;
		}
	}

	AppDataSourceWidget* newWidget = new AppDataSourceWidget(ads->lmUniqueID(), ads->lmEquipmentID(), this);
	newWidget->setClientSocket(m_tcpClientSocket);

	newWidget->show();
	newWidget->raise();
	newWidget->activateWindow();

	m_appDataSourceWidgetList.append(newWidget);

	connect(this, &AppDataServiceWidget::newTcpClientSocket, newWidget, &AppDataSourceWidget::setClientSocket);
	connect(this, &AppDataServiceWidget::clearTcpClientSocket, newWidget, &AppDataSourceWidget::unsetClientSocket);

	connect(newWidget, &AppDataSourceWidget::forgetMe, this, &AppDataServiceWidget::forgetWidget);
}

void AppDataServiceWidget::forgetWidget()
{
	AppDataSourceWidget *widget = dynamic_cast<AppDataSourceWidget*>(sender());
	TEST_PTR_RETURN(widget);
	m_appDataSourceWidgetList.removeAll(widget);
}

void AppDataServiceWidget::createTcpConnection(quint32 ip, quint16 port)
{
	m_tcpClientSocket = new TcpAppDataClient(softwareInfo(), HostAddressPort(ip, port));
	m_tcpClientThread = new SimpleThread(m_tcpClientSocket);

	m_dataSourcesStateModel->setClient(m_tcpClientSocket);
	m_signalStateModel->setClient(m_tcpClientSocket);

	connect(m_tcpClientSocket, &TcpAppDataClient::dataSourcesInfoLoaded, m_dataSourcesStateModel, &DataSourcesStateModel::reloadList);
	connect(m_tcpClientSocket, &TcpAppDataClient::dataSoursesStateUpdated, this, &AppDataServiceWidget::updateSourceStateColumns);
	connect(m_tcpClientSocket, &TcpAppDataClient::disconnected, m_dataSourcesStateModel, &DataSourcesStateModel::invalidateData);

	connect(m_tcpClientSocket, &TcpAppDataClient::appSignalListLoaded, m_signalStateModel, &SignalStateModel::reloadList);
	connect(m_tcpClientSocket, &TcpAppDataClient::appSignalsStateUpdated, this, &AppDataServiceWidget::updateSignalStateColumns);
	connect(m_tcpClientSocket, &TcpAppDataClient::disconnected, m_signalStateModel, &SignalStateModel::invalidateData);

	connect(m_tcpClientSocket, &TcpAppDataClient::clientsLoaded, this, &AppDataServiceWidget::updateClientsInfo);
	connect(m_tcpClientSocket, &TcpAppDataClient::disconnected, [this](){ clientsTabModel()->removeRows(0, clientsTabModel()->rowCount()); });

	connect(m_tcpClientSocket, &TcpAppDataClient::stateLoaded, this, &AppDataServiceWidget::updateServiceState);

	connect(m_tcpClientSocket, &TcpAppDataClient::settingsLoaded, this, &AppDataServiceWidget::updateSettings);

	connect(m_tcpClientSocket, &TcpAppDataClient::disconnected, this, &AppDataServiceWidget::clearServiceData);

	m_tcpClientThread->start();

	emit newTcpClientSocket(m_tcpClientSocket);
}

void AppDataServiceWidget::dropTcpConnection()
{
	emit clearTcpClientSocket();

	m_dataSourcesStateModel->setClient(nullptr);
	m_signalStateModel->setClient(nullptr);

	m_tcpClientThread->quitAndWait();
	delete m_tcpClientThread;
	m_tcpClientThread = nullptr;

	m_tcpClientSocket = nullptr;
}

SignalStateModel::SignalStateModel(QObject* parent) :
	QAbstractTableModel(parent),
	m_clientSocket(nullptr)
{
	static_assert(SC_COUNT == SIGNAL_COLUMN_COUNT, "Signal column count error");
}

SignalStateModel::~SignalStateModel()
{
}

int SignalStateModel::rowCount(const QModelIndex&) const
{
	if (m_clientSocket == nullptr)
	{
		return 0;
	}
	return m_clientSocket->signalParams().count();
}

int SignalStateModel::columnCount(const QModelIndex&) const
{
	return SIGNAL_COLUMN_COUNT;
}

QVariant SignalStateModel::data(const QModelIndex& index, int role) const
{
	static QString yes = tr("Yes");
	static QString no = tr("No");

	if (m_clientSocket == nullptr)
	{
		return QVariant();
	}
	int row = index.row();
	if (row < 0 || row > m_clientSocket->signalParams().count())
	{
		return QVariant();
	}
	if (role == Qt::DisplayRole)
	{
		const Signal& s = m_clientSocket->signalParams()[row];
		const AppSignalState& ass = m_clientSocket->signalStates()[row];

		switch (index.column())
		{
		case SC_APP_SIGNAL_ID: return s.appSignalID();
		case SC_CUSTOM_APP_SIGNAL_ID: return s.customAppSignalID();
		case SC_CAPTION: return s.caption();
		case SC_EQUIPMENT_ID: return s.equipmentID();
		case SC_LM_EQUIPMENT_ID: return s.lmEquipmentID();
		case SC_BUS_TYPE_ID: return s.busTypeID();
		case SC_CHANNEL: return E::valueToString<E::Channel>(s.channel());
		case SC_SIGNAL_TYPE: return E::valueToString<E::SignalType>(s.signalType());
		case SC_IN_OUT_TYPE: return E::valueToString<E::SignalInOutType>(s.inOutType());
		case SC_DATA_SIZE: return s.dataSize();
		case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(s.byteOrder());
		case SC_ANALOG_SIGNAL_FORMAT: return E::valueToString<E::AnalogAppSignalFormat>(s.analogSignalFormat());
		case SC_ENABLE_TUNING: return s.enableTuning() ? yes : no;
		case SC_TUNING_DEFAULT_VALUE: return s.tuningDefaultValue().toString();
		case SC_TUNING_LOW_BOUND: return s.tuningLowBound().toString();
		case SC_TUNING_HIGH_BOUND: return s.tuningHighBound().toString();
		case SC_ACQUIRE: return s.acquire() ? yes : no;
		case SC_ARCHIVE: return s.archive() ? yes : no;
		case SC_DECIMAL_PLACES: return s.decimalPlaces();
		case SC_COARSE_APERTURE: return s.coarseAperture();
		case SC_FINE_APERTURE: return s.fineAperture();
		case SC_ADAPTIVE_APERTURE: return s.adaptiveAperture() ? yes : no;
		case SC_IO_BUF_ADDR: return s.ioBufAddr().toString();
		case SC_TUNING_ADDR: return s.tuningAddr().toString();
		case SC_UAL_ADDR: return s.ualAddr().toString();
		case SC_REG_BUF_ADDR: return s.regBufAddr().toString();
		case SC_REG_VALUE_ADDR: return s.regValueAddr().toString();
		case SC_REG_VALIDITY_ADDR: return s.regValidityAddr().toString();

		case SC_VALUE: return ass.m_value;
		case SC_IS_VALID: return ass.m_flags.valid ? yes : no;
		case SC_IS_FINE_APERTURE: return ass.m_flags.valid ? yes : no;;
		case SC_IS_COARSE_APERTURE: return ass.m_flags.coarseAperture ? yes : no;
		case SC_IS_AUTO_POINT: return ass.m_flags.autoPoint ? yes : no;
		case SC_IS_VALIDITY_CHANGE: return ass.m_flags.validityChange ? yes : no;
		case SC_SYSTEM_TIME: return ass.m_time.systemToDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		case SC_LOCAL_TIME: return ass.m_time.localToDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		case SC_PLANT_TIME: return ass.m_time.plantToDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		case SC_UNIT: return s.unit();
		case SC_LOW_VALID_RANGE: return s.lowValidRange();
		case SC_HIGH_VALID_RANGE: return s.highValidRange();
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
		if (orientation == Qt::Horizontal)
		{
			if (section >= 0 && section < SIGNAL_COLUMN_COUNT)
			{
				return signalColumnStr[section];
			}
			else
			{
				assert(false);
				return "???";
			}
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
