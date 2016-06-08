#include "DataAquisitionServiceWidget.h"
#include "TcpAppDataClient.h"
#include <QTableView>

const int DSC_CAPTION = 0,
DSC_IP = 1,
DSC_PORT = 2,
DSC_PART_COUNT = 3,
DSC_STATE = 4,
DSC_UPTIME = 5,
DSC_RECEIVED = 6,
DSC_SPEED = 7;

const char* const dataSourceColumnStr[] =
{
	"Caption",
	"IP",
	"Port",
	"Part count",
	"State",
	"Uptime",
	"Received",
	"Speed"
};

const int DATA_SOURCE_COLUMN_COUNT = sizeof(dataSourceColumnStr) / sizeof(dataSourceColumnStr[0]);


const int SC_ID = 0,
SC_CAPTION = 1,
SC_VALUE = 2,
SC_VALID = 3,
SC_UNIT = 4;

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
	if (role == Qt::DisplayRole)
	{
		const DataSource& d = *m_dataSource[row];
		switch (index.column())
		{
			case DSC_CAPTION: return d.lmCaption();
			case DSC_IP: return d.lmAddressStr();
			case DSC_PORT: return d.lmPort();
			case DSC_PART_COUNT: return d.partCount();
			case DSC_STATE: return d.state();
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
			default:
				assert(false);
		}
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


DataAquisitionServiceWidget::DataAquisitionServiceWidget(quint32 ip, int portIndex, QWidget *parent) :
	BaseServiceStateWidget(ip, portIndex, parent)
{
	m_clientSocket = new TcpAppDataClient(HostAddressPort(ip, PORT_APP_DATA_SERVICE_CLIENT_REQUEST));
	m_appDataClientTread = new SimpleThread(m_clientSocket);

	// Data Sources
	m_dataSourcesStateModel = new DataSourcesStateModel(m_clientSocket, this);
	m_dataSourcesView = new QTableView;
	m_dataSourcesView->setModel(m_dataSourcesStateModel);

	connect(m_clientSocket, &TcpAppDataClient::dataSourcesInfoLoaded, m_dataSourcesStateModel, &DataSourcesStateModel::reloadList);
	connect(m_clientSocket, &TcpAppDataClient::dataSourcesInfoLoaded, this, &DataAquisitionServiceWidget::updateSourceInfo);
	connect(m_clientSocket, &TcpAppDataClient::disconnected, m_dataSourcesStateModel, &DataSourcesStateModel::invalidateData);

	m_dataSourcesView->resizeColumnsToContents();

	addTab(m_dataSourcesView, tr("Data Sources"));

	// Signals
	m_signalStateModel = new SignalStateModel(m_clientSocket, this);
	m_signalsView = new QTableView;
	m_signalsView->setModel(m_signalStateModel);

	connect(m_clientSocket, &TcpAppDataClient::appSignalListLoaded, m_signalStateModel, &SignalStateModel::reloadList);
	connect(m_clientSocket, &TcpAppDataClient::appSignalsStateUpdated, m_signalStateModel, &SignalStateModel::updateStateColumns);
	connect(m_clientSocket, &TcpAppDataClient::appSignalListLoaded, this, &DataAquisitionServiceWidget::updateSignalInfo);
	connect(m_clientSocket, &TcpAppDataClient::disconnected, m_signalStateModel, &SignalStateModel::invalidateData);

	addTab(m_signalsView, tr("Signals"));

	//For pausing requests if window hided
	//
	/*QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &DataAquisitionServiceWidget::checkVisibility);
	timer->start(5);*/

	m_appDataClientTread->start();
}

DataAquisitionServiceWidget::~DataAquisitionServiceWidget()
{
	m_appDataClientTread->quitAndWait();
}

void DataAquisitionServiceWidget::updateSourceInfo()
{
	m_dataSourcesView->resizeColumnToContents(DSC_CAPTION);
	m_dataSourcesView->resizeColumnToContents(DSC_IP);
	m_dataSourcesView->resizeColumnToContents(DSC_PORT);
	m_dataSourcesView->resizeColumnToContents(DSC_PART_COUNT);
}

void DataAquisitionServiceWidget::updateSignalInfo()
{
	m_signalsView->resizeColumnToContents(SC_ID);
	m_signalsView->resizeColumnToContents(SC_CAPTION);
	m_signalsView->resizeColumnToContents(SC_UNIT);
}

void DataAquisitionServiceWidget::checkVisibility()
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
				return ass.value;
			}
			case SC_VALID:
			{
				if (row < 0 || row > m_clientSocket->signalStates().count())
				{
					return QVariant();
				}
				const AppSignalState& ass = m_clientSocket->signalStates()[row];
				return ass.flags.valid ? tr("Yes") : tr("No");
			}
			case SC_UNIT: return m_clientSocket->unit(s.unitID());
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

void SignalStateModel::updateStateColumns()
{
	emit dataChanged(index(0, SC_VALUE), index(rowCount() - 1, SC_VALID));
}
