#include "AppDataServiceWidget.h"
#include "ScmTcpAppDataClient.h"
#include "AppDataSourceWidget.h"
#include "../UtilsLib/Ui/WidgetUtils.h"

#include <QTableView>
#include <QMenu>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

/*
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
DSC_RECEIVED_FRAMES_COUNT = 21,
DSC_RECEIVED_PACKET_COUNT = 22,
DSC_DATA_PROCESSING_ENABLED = 23,
DSC_PROCESSED_PACKET_COUNT = 24,
DSC_LAST_PACKET_SYSTEM_TIME = 25,
DSC_RUP_FRAME_PLANT_TIME = 26,
DSC_RUP_FRAME_NUMERATOR = 27,
DSC_SIGNAL_STATES_QUEUE_SIZE = 28,
DSC_SIGNAL_STATES_QUEUE_MAX_SIZE = 29,
DSC_ACQUIRED_SIGNALS_COUNT = 30,

DSC_ERROR_PROTOCOL_VERSION = 31,
DSC_ERROR_FRAMES_QUANTITY = 32,
DSC_ERROR_FRAME_NOMBER = 33,
DSC_LOST_PACKET_COUNT = 34,
DSC_ERROR_DATA_ID = 35,
DSC_ERROR_PLANT_TIME_FORMAT = 36,
DSC_ERROR_DUPLICATE_PLANT_TIME = 37,
DSC_ERROR_NONMONOTONIC_PLANT_TIME = 38,
DSC_COUNT = 39;

const int dataSourceStateColumn[] =
{
	DSC_STATE,
	DSC_RECEIVES_DATA,
	DSC_UPTIME,
	DSC_RECEIVED_DATA_ID,
	DSC_RUP_FRAME_PLANT_TIME,
	DSC_RECEIVED,
	DSC_SPEED,
	DSC_RECEIVED_FRAMES_COUNT,
	DSC_RECEIVED_PACKET_COUNT,
	DSC_DATA_PROCESSING_ENABLED,
	DSC_PROCESSED_PACKET_COUNT,
	DSC_LAST_PACKET_SYSTEM_TIME,
	DSC_RUP_FRAME_PLANT_TIME,
	DSC_RUP_FRAME_NUMERATOR,
	DSC_SIGNAL_STATES_QUEUE_SIZE,
	DSC_SIGNAL_STATES_QUEUE_MAX_SIZE,
	DSC_ACQUIRED_SIGNALS_COUNT,
	DSC_ERROR_PROTOCOL_VERSION,
	DSC_ERROR_FRAMES_QUANTITY,
	DSC_ERROR_FRAME_NOMBER,
	DSC_ERROR_DATA_ID,
	DSC_ERROR_PLANT_TIME_FORMAT,
	DSC_ERROR_DUPLICATE_PLANT_TIME,
	DSC_ERROR_NONMONOTONIC_PLANT_TIME,
	DSC_LOST_PACKET_COUNT,
};

const int DATA_SOURCE_STATE_COLUMN_COUNT = sizeof(dataSourceStateColumn) / sizeof(dataSourceStateColumn[0]);

const char* const dataSourceColumnStr[] =
{
	"Equipment ID",
	"Data UID",
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

	"Error protocol version",
	"Error frames quantity",
	"Error frame number",
	"Lost packet count",
	"Error Data UID",
	"Error plant time format",
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
SC_APERTURE_TYPE = 22,
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

const int signalStateColumn[] =
{
	SC_VALUE,
	SC_IS_VALID,
	SC_IS_FINE_APERTURE,
	SC_IS_COARSE_APERTURE,
	SC_IS_AUTO_POINT,
	SC_IS_VALIDITY_CHANGE,
	SC_SYSTEM_TIME,
	SC_LOCAL_TIME,
	SC_PLANT_TIME,
};

const int SIGNAL_STATE_COLUMN_COUNT = sizeof(signalStateColumn) / sizeof(signalStateColumn[0]);


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
	return static_cast<int>(m_dataSources.size());
}

int DataSourcesStateModel::columnCount(const QModelIndex&) const
{
	return DATA_SOURCE_COLUMN_COUNT;
}


QVariant DataSourcesStateModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	if (row < 0 || row > rowCount())
	{
		return QVariant();
	}

	const OnlineLib::DataSource& source = m_dataSources[row].first;
	const Network::AppDataSourceState& state = m_dataSources[row].second;

	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column())
			{
				// DataSourceInfo
				//
				case DSC_CAPTION: return source.moduleCaption();
				case DSC_IP: return source.lanControllersInfo()[0].appDataIP;
				case DSC_PORT: return source.lanControllersInfo()[0].appDataPort;
				case DSC_RUP_FRAMES_QUANTITY: return source.lanControllersInfo()[0].appDataFramesQuantity;
				case DSC_DATA_TYPE: return E::valueToString(source.lanControllersInfo()[0].lanControllerType);
				case DSC_EQUIPMENT_ID: return source.moduleEquipmentID();
				case DSC_MODULE_NUMBER: return source.lmNumber();
				case DSC_MODULE_TYPE: return source.moduleType();
				case DSC_SUBSYSTEM_ID: return source.subsystemKey();
			    case DSC_SUBSYSTEM_CAPTION: return source.subsystemID();
				case DSC_SUBSYSTEM_CHANNEL: return source.subsystemChannel();
				case DSC_ADAPTER_ID: return source.lanControllersInfo()[0].equipmentID;
				case DSC_ENABLE_DATA: return source.lanControllersInfo()[0].appDataEnable;
				case DSC_DATA_ID: return "0x" + QString("%1").
										arg(source.lanControllersInfo()[0].rupAppDataUID,
											sizeof(source.lanControllersInfo()[0].rupAppDataUID) * 2, 16, QChar('0')).toUpper();
				case DSC_UNIQUE_ID: return "0x" + QString("%1").arg(source.moduleUniqueID(), sizeof(source.moduleUniqueID()) * 2, 16, QChar('0')).toUpper();
				case DSC_STATE: return state.receivesdata() ? "Receive data" : "No data";

				// DataSourceState
				//
				case DSC_UPTIME: return formatUptime(state.uptime());
				case DSC_RECEIVED: return static_cast<qint64>(state.receiveddatasize());
				case DSC_SPEED: return state.datareceivingspeed();
				case DSC_RECEIVES_DATA: return state.receivesdata();
				case DSC_RECEIVED_DATA_ID: return "0x" + QString("%1").arg(state.receiveddataid(),
													sizeof(state.receiveddataid()) * 2, 16, QChar('0')).toUpper();

				//

				case DSC_RECEIVED_FRAMES_COUNT: return static_cast<qint64>(state.receivedframescount());
				case DSC_RECEIVED_PACKET_COUNT: return static_cast<qint64>(state.receivedpacketcount());
				case DSC_DATA_PROCESSING_ENABLED: return state.dataprocessingenabled();
				case DSC_PROCESSED_PACKET_COUNT: return 0;
				case DSC_LAST_PACKET_SYSTEM_TIME: return 0;
				case DSC_RUP_FRAME_PLANT_TIME: return source.getTimeStr(state.lmtime());
				case DSC_RUP_FRAME_NUMERATOR: return state.rupframenumerator();
				case DSC_SIGNAL_STATES_QUEUE_SIZE: return state.signalstatesqueuecursize();
				case DSC_SIGNAL_STATES_QUEUE_MAX_SIZE: return state.signalstatesqueuecurmaxsize();
				case DSC_ACQUIRED_SIGNALS_COUNT: return source.appSignalsCount();

				case DSC_ERROR_PROTOCOL_VERSION: return static_cast<qint64>(state.errorprotocolversion());
				case DSC_ERROR_FRAMES_QUANTITY: return static_cast<qint64>(state.errorframesquantity());
				case DSC_ERROR_FRAME_NOMBER: return static_cast<qint64>(state.errorframeno());
				case DSC_LOST_PACKET_COUNT: return static_cast<qint64>(state.lostpacketcount());
				case DSC_ERROR_DATA_ID: return static_cast<qint64>(state.errordataid());
				case DSC_ERROR_PLANT_TIME_FORMAT: return static_cast<qint64>(state.errorplanttimeformat());
				case DSC_ERROR_DUPLICATE_PLANT_TIME: return static_cast<qint64>(state.errorduplicateplanttime());
				case DSC_ERROR_NONMONOTONIC_PLANT_TIME: return static_cast<qint64>(state.errornonmonotonicplanttime());
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
		if (orientation == Qt::Vertical && section < TO_INT(m_dataSources.size()))
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

void DataSourcesStateModel::setClient(TcpAppDataClient* clientSocket)
{
	m_clientSocket = clientSocket;
}

const OnlineLib::DataSource& DataSourcesStateModel::getDataSource(int row) const
{
	return m_dataSources[row].first;
}

void DataSourcesStateModel::updateStates()
{
	for(auto& [ds, state] : m_dataSources)
	{
		m_clientSocket->getDataSourceState(ds.moduleEquipmentID(), &state);
	}
}

void DataSourcesStateModel::invalidateData()
{
	beginResetModel();
	m_dataSources.clear();
	endResetModel();
}

void DataSourcesStateModel::reloadList()
{
	beginResetModel();

	if (m_clientSocket != nullptr)
	{
		std::vector<OnlineLib::DataSource> dss = m_clientSocket->getDataSources();

		m_dataSources.clear();
		m_dataSources.reserve(dss.size());

		for(const OnlineLib::DataSource& ds : dss)
		{
			m_dataSources.emplace_back(ds, Network::AppDataSourceState());
		}
	}

	endResetModel();
}*/

AppDataServiceWidget::AppDataServiceWidget(ServiceTableModel* srvTableModel,
	const SoftwareInfo& softwareInfo,
	const ServiceData& serviceData,
	quint32 ip, quint16 tcpPort,
	QWidget* parent) :
	BaseServiceWidget(srvTableModel, softwareInfo, serviceData, ip, tcpPort, parent)
{
	// Data Sources
/*	m_dataSourcesStateModel = new DataSourcesStateModel(this);
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
//	addClientsTab(false);

	// Parameters
	QTableView* parametersTableView = addTabWithTableView(250, "Parameters");

	m_parametersTabModel = new QStandardItemModel(3, 2, this);
	parametersTableView->setModel(m_parametersTabModel);

	m_parametersTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_parametersTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_parametersTabModel->setData(m_parametersTabModel->index(0, 0), "Equipment ID");
	m_parametersTabModel->setData(m_parametersTabModel->index(1, 0), "Configuration IP 1");
	m_parametersTabModel->setData(m_parametersTabModel->index(2, 0), "Configuration IP 2");

	// Settings
	QTableView* settingsTableView = addTabWithTableView(250, "Settings");

	m_settingsTabModel = new QStandardItemModel(12, 2, this);
	settingsTableView->setModel(m_settingsTabModel);

	m_settingsTabModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_settingsTabModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 0), "ConfigService1 ID");
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 0), "ConfigService1 IP");

	m_settingsTabModel->setData(m_settingsTabModel->index(2, 0), "ConfigService2 ID");
	m_settingsTabModel->setData(m_settingsTabModel->index(3, 0), "ConfigService2 IP");

	m_settingsTabModel->setData(m_settingsTabModel->index(4, 0), "AppData Receiving IP");
	m_settingsTabModel->setData(m_settingsTabModel->index(5, 0), "AppData Receiving NetMask");

	m_settingsTabModel->setData(m_settingsTabModel->index(6, 0), "AutoArchive Interval");

	m_settingsTabModel->setData(m_settingsTabModel->index(7, 0), "ArchService ID");
	m_settingsTabModel->setData(m_settingsTabModel->index(8, 0), "ArchService IP");

	auto appDataSettings = std::dynamic_pointer_cast<AppDataServiceSettings>(m_serviceData.settings);

	if (appDataSettings == nullptr)
	{
		return;
	}

	int row = 9;

	for(const RqCtrlSettings& rcs : appDataSettings->rcSettings )
	{
		m_settingsTabModel->setData(m_settingsTabModel->index(row++, 0),
									QString("RC%1.Client Request IP").arg(rcs.ID(), 2, 10, QChar('0')));

		m_settingsTabModel->setData(m_settingsTabModel->index(row++, 0),
									QString("RC%1.Client Request NetMask").arg(rcs.ID(), 2, 10, QChar('0')));

		m_settingsTabModel->setData(m_settingsTabModel->index(row++, 0),
									QString("RC%1.Realtime Trends Request IP").arg(rcs.ID(), 2, 10, QChar('0')));
	}

	// Log
	addTab(new QTableView(this), tr("Log")); */
}

AppDataServiceWidget::~AppDataServiceWidget()
{
	// for (auto* widget : m_appDataSourceWidgetList)
	// {
	// 	widget->deleteLater();
	// }
	// m_appDataSourceWidgetList.clear();
	// dropTcpConnection();
}

void AppDataServiceWidget::initWidget()
{
	addGeneralTab();
	addClientsTab();
	addAppDataSourcesTab();
	addArchiveSignalsTab();
}

void AppDataServiceWidget::updateDerivedWidgets(const Network::ServiceInfo& srvInfo)
{
	updateModels(srvInfo);
}

void AppDataServiceWidget::clearDerivedWidgets()
{
	Network::ServiceInfo clearSrvInfo;

	clearSrvInfo.set_archsignalsupdated(true);

	updateModels(clearSrvInfo);
}

void AppDataServiceWidget::forgetWidget(QString dataSourceID)
{
	m_sourceWidgets.erase(dataSourceID);
}

void AppDataServiceWidget::updateModels(const Network::ServiceInfo& srvInfo)
{
	if (m_sourcesModel != nullptr)
	{
		m_sourcesModel->updateData(srvInfo);
	}

	if (m_sourceWidgets.size() > 0)
	{
		int srcCount = srvInfo.appdatasourcesstates_size();

		for(int i = 0; i < srcCount; i++)
		{
			const Network::AppDataSourceState& dsState = srvInfo.appdatasourcesstates(i);

			AppDataSourceWidget* w = getValueOrNullptr(m_sourceWidgets, QString::fromStdString(dsState.lancontrollerid()));

			if (w != nullptr)
			{
				w->updateData(dsState);
			}
		}
	}

	if (m_archSignalsModel != nullptr )
	{
		if (srvInfo.archsignalsupdated() == true)
		{
			m_archSignalsModel->updateData(srvInfo);
		}

		m_archSignalsProgressBar->setValue(srvInfo.archsignalsupdateprogress());
	}
}

/*void AppDataServiceWidget::updateSrvStatus()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->stateIsReady() == false)
	{
		assert(false);
		return;
	}
	stateTabModel()->setData(stateTabModel()->index(6, 1), m_tcpClientSocket->configServiceConnectionState());
	stateTabModel()->setData(stateTabModel()->index(7, 1), m_tcpClientSocket->archiveServiceConnectionState());

	auto state = m_tcpClientSocket->serviceState().appdatareceivestate();

	stateTabModel()->setData(stateTabModel()->index(8, 1), static_cast<qint64>(state.receivingspeed()));
	stateTabModel()->setData(stateTabModel()->index(9, 1), static_cast<qint64>(state.rupframesreceivingspeed()));

	stateTabModel()->setData(stateTabModel()->index(10, 1), static_cast<qint64>(state.rupframescount()));
	stateTabModel()->setData(stateTabModel()->index(11, 1), static_cast<qint64>(state.simframescount()));

	stateTabModel()->setData(stateTabModel()->index(12, 1), static_cast<qint64>(state.errdatagramsize()));
	stateTabModel()->setData(stateTabModel()->index(13, 1), static_cast<qint64>(state.errsimversion()));
	stateTabModel()->setData(stateTabModel()->index(14, 1), static_cast<qint64>(state.errunknownappdatasourceip()));
	stateTabModel()->setData(stateTabModel()->index(15, 1), static_cast<qint64>(state.errrupframecrc()));

	stateTabModel()->setData(stateTabModel()->index(16, 1), static_cast<qint64>(state.errnotexpectedsimpacket()));

	auto appDataSettings = std::dynamic_pointer_cast<AppDataServiceSettings>(m_serviceData.settings);

	if (appDataSettings == nullptr)
	{
		return;
	}

	m_settingsTabModel->setData(m_settingsTabModel->index(0, 1), appDataSettings->cfgServiceID1);
	m_settingsTabModel->setData(m_settingsTabModel->index(1, 1), appDataSettings->cfgServiceIP1.addressStr());

	m_settingsTabModel->setData(m_settingsTabModel->index(2, 1), appDataSettings->cfgServiceID2);
	m_settingsTabModel->setData(m_settingsTabModel->index(3, 1), appDataSettings->cfgServiceIP2.addressStr());

	m_settingsTabModel->setData(m_settingsTabModel->index(4, 1), appDataSettings->appDataReceivingIP.addressStr());
	m_settingsTabModel->setData(m_settingsTabModel->index(5, 1), appDataSettings->appDataReceivingNetmask.toString());

	m_settingsTabModel->setData(m_settingsTabModel->index(6, 1), QString::number(appDataSettings->autoArchiveInterval));

	m_settingsTabModel->setData(m_settingsTabModel->index(7, 1), appDataSettings->archServiceID);
	m_settingsTabModel->setData(m_settingsTabModel->index(8, 1), appDataSettings->archServiceIP.addressStr());

	int row = 9;

	for(const RqCtrlSettings& rcs : appDataSettings->rcSettings )
	{
		m_settingsTabModel->setData(m_settingsTabModel->index(row++, 1), rcs.clientRequestIP().addressPortStr());
		m_settingsTabModel->setData(m_settingsTabModel->index(row++, 1), rcs.clientRequestNetmask().toString());
		m_settingsTabModel->setData(m_settingsTabModel->index(row++, 1), rcs.rtTrendsRequestIP().addressPortStr());
	}
}

void AppDataServiceWidget::updateStateInfo()
{
	if (m_serviceData.protoServiceInfo.servicestate() == E::ServiceState::Work)
	{
		stateTabModel()->setData(stateTabModel()->index(5, 0), "Connected client quantity");
		stateTabModel()->setData(stateTabModel()->index(6, 0), "Connected to CfgService");
		stateTabModel()->setData(stateTabModel()->index(7, 0), "Connected to ArchiveService");

		stateTabModel()->setData(stateTabModel()->index(8, 0), "App data receiving speed, bytes/s");
		stateTabModel()->setData(stateTabModel()->index(9, 0), "RUP frames receiving speed, frames/s");

		stateTabModel()->setData(stateTabModel()->index(10, 0), "RUP frames count");
		stateTabModel()->setData(stateTabModel()->index(11, 0), "Simulated frames count");

		stateTabModel()->setData(stateTabModel()->index(12, 0), "Datagram size errors");
		stateTabModel()->setData(stateTabModel()->index(13, 0), "Simulation version errors");
		stateTabModel()->setData(stateTabModel()->index(14, 0), "Unknown AppDataSource IP errors");
		stateTabModel()->setData(stateTabModel()->index(15, 0), "RUP frames CRC errors");

		stateTabModel()->setData(stateTabModel()->index(16, 0), "Not expected Simulated packets");

		if (m_tcpClientSocket == nullptr || m_tcpClientSocket->stateIsReady() == false)
		{
			stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
			stateTabModel()->setData(stateTabModel()->index(7, 1), "???");
		}
		else
		{
			updateSrvStatus();
		}
	}

	HostAddressPort workingIp = getWorkingClientRequestIp();

	if (m_tcpClientSocket != nullptr)
	{
		HostAddressPort&& curAddress = m_tcpClientSocket->serverAddressPort(0);

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
	m_dataSourcesStateModel->updateStates();

	int firstVisibleRow = m_dataSourcesView->rowAt(0);

	int lastVisibleRow = m_dataSourcesView->rowAt(m_dataSourcesView->height());

	if (lastVisibleRow == -1)
	{
		lastVisibleRow = m_dataSourcesStateModel->rowCount() - 1;
	}

	int firstVisibleColumn = m_dataSourcesView->columnAt(0);

	int lastVisibleColumn = m_dataSourcesView->columnAt(m_dataSourcesView->width());

	if (lastVisibleColumn == -1)
	{
		lastVisibleColumn = DATA_SOURCE_STATE_COLUMN_COUNT - 1;
	}

	m_dataSourcesStateModel->updateData(firstVisibleRow,
										lastVisibleRow,
										firstVisibleColumn,
										lastVisibleColumn);
}

void AppDataServiceWidget::updateSignalStateColumns()
{
	int firstRow = m_signalsView->rowAt(0);

	int lastRow = m_signalsView->rowAt(m_signalsView->height());
	if (lastRow == -1)
	{
		lastRow = m_signalStateModel->rowCount() - 1;
	}

	for (int i = 0; i < SIGNAL_STATE_COLUMN_COUNT; i++)
	{
		int currentColumn = signalStateColumn[i];
		m_signalStateModel->updateData(firstRow,
									   lastRow,
									   currentColumn,
									   currentColumn);
	}
}

void AppDataServiceWidget::updateClientsInfo()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->clientsIsReady() == false)
	{
//		clientsTabModel()->setRowCount(0);
		return;
	}

//	updateClientsModel(m_tcpClientSocket->clients());
}

void AppDataServiceWidget::updateServiceParameters()
{
	if (m_tcpClientSocket == nullptr || m_tcpClientSocket->settingsIsReady() == false)
	{
		assert(false);
		return;
	}
	m_parametersTabModel->setData(m_parametersTabModel->index(0, 1), m_tcpClientSocket->equipmentID());
	m_parametersTabModel->setData(m_parametersTabModel->index(1, 1), m_tcpClientSocket->configIP1());
	m_parametersTabModel->setData(m_parametersTabModel->index(2, 1), m_tcpClientSocket->configIP2());
}

void AppDataServiceWidget::clearServiceData()
{
//	stateTabModel()->setData(stateTabModel()->index(6, 1), "???");
//	stateTabModel()->setData(stateTabModel()->index(7, 1), "???");

	m_parametersTabModel->setData(m_parametersTabModel->index(0, 1), "???");
	m_parametersTabModel->setData(m_parametersTabModel->index(1, 1), "???");
	m_parametersTabModel->setData(m_parametersTabModel->index(2, 1), "???");
}

void AppDataServiceWidget::onAppDataSourceDoubleClicked(const QModelIndex &index)
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	int row = index.row();
	const OnlineLib::DataSource& ads = m_dataSourcesStateModel->getDataSource(row);

	for (auto& sourceWidget : m_appDataSourceWidgetList)
	{
		TEST_PTR_CONTINUE(sourceWidget);

		if (sourceWidget->id() == ads.moduleUniqueID() && sourceWidget->equipmentId() == ads.moduleEquipmentID())
		{
			sourceWidget->show();
			sourceWidget->raise();
			sourceWidget->activateWindow();

			return;
		}
	}

	AppDataSourceWidget* newWidget = new AppDataSourceWidget(ads.moduleUniqueID(), ads.moduleEquipmentID(), this);
	newWidget->setClientSocket(m_tcpClientSocket);

	newWidget->show();
	newWidget->raise();
	newWidget->activateWindow();

	m_appDataSourceWidgetList.append(newWidget);

	connect(this, &AppDataServiceWidget::newTcpClientSocket, newWidget, &AppDataSourceWidget::setClientSocket);
	connect(this, &AppDataServiceWidget::clearTcpClientSocket, newWidget, &AppDataSourceWidget::unsetClientSocket);

	connect(newWidget, &AppDataSourceWidget::forgetMe, this, &AppDataServiceWidget::forgetWidget);
}

*/

void AppDataServiceWidget::addAppDataSourcesTab()
{
	m_sourcesModel = new AppDataSourcesModel(this);
	m_sourcesView = createTableView(m_sourcesModel, m_sourcesModel->columns());

	connect(m_sourcesView, &QTableView::doubleClicked, this, &AppDataServiceWidget::onSourceDoubleClicked);

	addTab(m_sourcesView, "AppData sources");
}

void AppDataServiceWidget::addArchiveSignalsTab()
{
	QWidget* archSignalsWidget = new QWidget;

	//

	m_archSignalsModel = new ArchiveSignalsModel(this);
	m_archSignalsView = createTableView(m_archSignalsModel, m_archSignalsModel->columns());

	m_archSignalsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_archSignalsView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_archSignalsView, &QTableView::customContextMenuRequested,
			this, &AppDataServiceWidget::onCustomContextMenuRequested);

	//

	m_archSignalsProgressBar = new QProgressBar;

	m_archSignalsProgressBar->setRange(0, 100);
	m_archSignalsProgressBar->setStyleSheet(R"(
									QProgressBar {
										border: 2px solid grey;
										border-radius: 5px;
										text-align: center;
										background-color: #eee;
									}

									QProgressBar::chunk {
										background-color: #3498db;
										width: 10px;  /* ширина одного блока */
										margin: 1px;  /* расстояние между блоками */
									})");

	m_archSignalsProgressBar->setFixedHeight(15);

	//

	QVBoxLayout* vBoxLayout = new QVBoxLayout;

	vBoxLayout->addWidget(m_archSignalsView);
	vBoxLayout->addWidget(m_archSignalsProgressBar);

	archSignalsWidget->setLayout(vBoxLayout);

	addTab(archSignalsWidget, "TOP-500 archive signals");
}

int AppDataServiceWidget::updateSettings(int rowCount)
{
	if (m_serviceData.settings == nullptr)
	{
		return rowCount;
	}

	std::shared_ptr<AppDataServiceSettings> st = std::dynamic_pointer_cast<AppDataServiceSettings>(m_serviceData.settings);

	TEST_PTR_RETURN_VALUE(st, rowCount);

	const Network::ServiceInfo& protoInfo = m_serviceData.protoServiceInfo;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceEquipmentID1);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->cfgServiceID1);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceIP1);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->cfgServiceID1.isEmpty() ? Separator::EMPTY_STR :
							 HostAddressPort(protoInfo.cfgserviceip1(), protoInfo.cfgserviceport1()).toString());
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceEquipmentID2);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->cfgServiceID2);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceIP2);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->cfgServiceID2.isEmpty() ? Separator::EMPTY_STR :
							 HostAddressPort(protoInfo.cfgserviceip2(), protoInfo.cfgserviceport2()).toString());
	rowCount++;

	for(const RqCtrlSettings& rcs : st->rcSettings)
	{
		m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QString("Request Controller %1").arg(rcs.ID()));
		m_settingsModel->setData(m_settingsModel->index(rowCount, 1), rqCtrlInfoStr(rcs));
		rowCount++;
	}

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("AppDataReceivingIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 QString("%1, Netmask = %2").arg(st->appDataReceivingIP.toString()).arg(st->appDataReceivingNetmask.toString()));
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ArchiveServiceEquipmentID"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->archServiceID);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ArchiveServiceIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->archServiceIP.toString());

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("AutoArchiveInterval (min)"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->autoArchiveInterval);

	rowCount++;

	return rowCount;
}

void AppDataServiceWidget::onSourceDoubleClicked(const QModelIndex& index)
{
	int row = index.row();

	QString lanControllerID = m_sourcesModel->getSourceLanControllerID(row);

	AppDataSourceWidget* srcWidget = getValueOrNullptr(m_sourceWidgets, lanControllerID);

	if (srcWidget == nullptr)
	{
		srcWidget = new AppDataSourceWidget(lanControllerID, this);

		m_sourceWidgets.emplace(lanControllerID, srcWidget);

		srcWidget->show();

		connect(srcWidget, &AppDataSourceWidget::forgetMe, this, &AppDataServiceWidget::forgetWidget);
	}
	else
	{
		srcWidget->activateWindow();
	}
}

void AppDataServiceWidget::onCustomContextMenuRequested(const QPoint& pos)
{
	m_selectedRows.clear();

	QModelIndex index = m_archSignalsView->indexAt(pos);

	if (index.isValid() == false)
	{
		return;
	}

	QModelIndexList selectedIndexes = m_archSignalsView->selectionModel()->selectedRows();

	m_selectedRows.reserve(selectedIndexes.size());

	for(const QModelIndex& indx : selectedIndexes)
	{
		m_selectedRows.push_back(indx.row());
	}

	QMenu menu;

	QAction* changeAperturesAction = new QAction("Change aperture(s)",&menu);

	menu.addAction(changeAperturesAction);

	connect(changeAperturesAction, &QAction::triggered,
			this, &AppDataServiceWidget::onChangeApertures);

	menu.exec(m_archSignalsView->viewport()->mapToGlobal(pos));
}

void AppDataServiceWidget::onChangeApertures()
{
	// init dialog parameters

	QListWidget* signalsList = new QListWidget;

	QStringList appSignalIDs;
	QStringList discreteAppSignalIDs;
	std::optional<int> apertureType;
	std::optional<double> coarseAperture;
	std::optional<double> fineAperture;

	int index = 0;

	for(int row : m_selectedRows)
	{
		const Network::ArchSignalInfo& asi = m_archSignalsModel->at(row);

		QString appSignalID = QString::fromStdString(asi.appsignalid());

		if (asi.signaltype() == TO_INT(E::SignalType::Discrete))
		{
			discreteAppSignalIDs.append(appSignalID);
			continue;
		}

		appSignalIDs.append(appSignalID);

		signalsList->addItem(appSignalID);

		if (index == 0)
		{
			apertureType = asi.aperturetype();
			coarseAperture = asi.coarseaperture();
			fineAperture = asi.fineaperture();
		}
		else
		{
			if (apertureType.has_value() && apertureType.value() != asi.aperturetype())
			{
				apertureType.reset();
			}

			if (coarseAperture.has_value() && coarseAperture.value() != asi.coarseaperture())
			{
				coarseAperture.reset();
			}

			if (fineAperture.has_value() && fineAperture.value() != asi.fineaperture())
			{
				fineAperture.reset();
			}
		}

		index++;
	}

	if (discreteAppSignalIDs.isEmpty() == false)
	{
		QString msg;

		msg.append("It is not possible to change the aperture of discrete signal(s):\n\n");

		int rest = 0;

		if (discreteAppSignalIDs.size() > 5)
		{
			rest = discreteAppSignalIDs.size() - 5;
			discreteAppSignalIDs.remove(5, rest);
		}

		msg.append(discreteAppSignalIDs.join(Separator::NEW_LINE));

		if (rest > 0)
		{
			msg.append(QString("\n\nand %1 more signal(s)").arg(rest));
		}

		if (QMessageBox::warning(this, "Warning", msg, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
		{
			return;
		}
	}

	if (appSignalIDs.isEmpty())
	{
		return;
	}

	QDialog dlg;

	QGridLayout* gridLayout = new QGridLayout(&dlg);

	// create dialog layout

	gridLayout->addWidget(signalsList, 0, 0, 1, 2);

	//

	gridLayout->addWidget(new QLabel("Aperture type"), 1, 0);

	QComboBox* apertureTypeList = new QComboBox;

	std::vector<std::pair<int, QString>> values = E::enumValues<E::ApertureType>();

	if (apertureType.has_value() == false)
	{
		apertureTypeList->addItem(QString(), -1);
		apertureTypeList->setCurrentText(QString());
	}

	for(const auto& [value, text] : values)
	{
		apertureTypeList->addItem(text, value);

		if (apertureType.has_value() && apertureType.value() == value)
		{
			apertureTypeList->setCurrentText(text);
		}
	}

	gridLayout->addWidget(apertureTypeList, 1, 1);

	//

	gridLayout->addWidget(new QLabel("Coarse aperture"), 2, 0);

	QLineEdit* coarseApertureEdit = new QLineEdit;

	if (coarseAperture.has_value())
	{
		coarseApertureEdit->setText(QString::number(coarseAperture.value()));
	}

	gridLayout->addWidget(coarseApertureEdit, 2, 1);

	//

	gridLayout->addWidget(new QLabel("Fine aperture"), 3, 0);

	QLineEdit* fineApertureEdit = new QLineEdit;

	if (fineAperture.has_value())
	{
		fineApertureEdit->setText(QString::number(fineAperture.value()));
	}

	gridLayout->addWidget(fineApertureEdit, 3, 1);

	// create dialog

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	gridLayout->addWidget(buttonBox, 4, 0, 1, 2);

	connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

	E::ApertureType resultApertureType = E::ApertureType::RangePercent;
	double resultCoarseAperture = 0;
	double resultFineAperture = 0;

	connect(buttonBox, &QDialogButtonBox::accepted, &dlg, [&]()
	{
		QString errStr;
		bool ok = false;

		// check aperture type

		QString valueStr = apertureTypeList->currentText();

		resultApertureType = E::stringToValue<E::ApertureType>(valueStr, &ok);

		if (ok == false)
		{
			errStr.append("The ApertureType don't set.\n\n");
		}

		// check coarse aperture

		valueStr = coarseApertureEdit->text();

		resultCoarseAperture = valueStr.toDouble(&ok);

		if (ok == false)
		{
			errStr.append("The CoarseAperture don't set.\n\n");
		}
		else
		{
			resultCoarseAperture = abs(resultCoarseAperture);
			coarseApertureEdit->setText(QString::number(resultCoarseAperture));

			if (resultCoarseAperture == 0)
			{
				errStr.append("The CoarseAperture can't be 0.\n\n");
			}
		}

		// check fine aperture

		valueStr = fineApertureEdit->text();

		resultFineAperture = valueStr.toDouble(&ok);

		if (ok == false)
		{
			errStr.append("The FineAperture don't set.\n\n");
		}
		else
		{
			resultFineAperture = abs(resultFineAperture);
			fineApertureEdit->setText(QString::number(resultFineAperture));

			if (resultFineAperture == 0)
			{
				errStr.append("The FineAperture can't be 0.\n\n");
			}
		}

		//

		if (errStr.isEmpty() && resultCoarseAperture <= resultFineAperture)
		{
			errStr.append("The CoarseAperture should be greate than the FineAperture.\n\n");
		}

		//

		if (errStr.isEmpty() == false)
		{
			QMessageBox::critical(&dlg, "Error", errStr);
			return;
		}

		dlg.accept();
	});

	//

	dlg.setLayout(gridLayout);
	dlg.resize(500, 400);

	int result = dlg.exec();

	if (result == QDialog::Rejected)
	{
		return;
	}

	//

	std::vector<ApertureRecord> apertures;

	apertures.reserve(appSignalIDs.size());

	for(const QString& appSignalID : appSignalIDs)
	{
		ApertureRecord ar;

		ar.signalID = appSignalID;
		ar.apertureType = resultApertureType;
		ar.coarseAperture = resultCoarseAperture;
		ar.fineAperture = resultFineAperture;

		apertures.push_back(ar);

	}

	overrideApertures(apertures);
}

/*

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
	return static_cast<int>(m_clientSocket->signalParams().count());
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
		const AppSignal& s = m_clientSocket->signalParams()[row];
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
		case SC_APERTURE_TYPE: return E::valueToString<E::ApertureType>(s.apertureType());
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
*/
