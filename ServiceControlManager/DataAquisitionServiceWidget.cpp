#include "DataAquisitionServiceWidget.h"
#include <QTableView>

const int DSC_NAME = 0,
DSC_IP = 1,
DSC_PART_COUNT = 2,
DSC_STATE = 3,
DSC_UPTIME = 4,
DSC_RECEIVED = 5,
DSC_SPEED = 6;


const char* const dataSourceColumnStr[] =
{
	"Name",
	"IP",
	"Part count",
	"State",
	"Uptime",
	"Received",
	"Speed"
};

const int DATA_SOURCE_COLUMN_COUNT = sizeof(dataSourceColumnStr) / sizeof(dataSourceColumnStr[0]);


DataSourcesStateModel::DataSourcesStateModel(QHostAddress ip, QObject* parent) :
	QAbstractTableModel(parent)
{
	m_clientSocket = new UdpClientSocket(ip, PORT_DATA_AQUISITION_SERVICE_INFO);

	connect(m_clientSocket, &UdpClientSocket::ackTimeout, this, &DataSourcesStateModel::ackTimeout);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &DataSourcesStateModel::ackReceived);
	connect(this, &DataSourcesStateModel::dataClientSendRequest, m_clientSocket, &UdpClientSocket::sendRequest);

	m_clientSocketThread.run(m_clientSocket);

	m_periodicTimer = new QTimer(this);
	connect(m_periodicTimer, &QTimer::timeout, this, &DataSourcesStateModel::onGetStateTimer);

	sendDataRequest(RQID_GET_DATA_SOURCES_IDS);
}

DataSourcesStateModel::~DataSourcesStateModel()
{
	m_clientSocket->deleteLater();
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
		const DataSource& d = m_dataSource[row];
		switch (index.column())
		{
		case DSC_NAME: return d.name();
		case DSC_IP: return d.hostAddress().toString();
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
			return m_dataSource[section].ID();
		}
	}
	return QVariant();
}

void DataSourcesStateModel::setActive(bool active)
{
	m_active = active;
}

void DataSourcesStateModel::onGetStateTimer()
{
	if (!m_active || m_currentDataRequestType != RQID_GET_DATA_SOURCES_STATISTICS || m_clientSocket->isWaitingForAck())
	{
		return;
	}

	sendDataRequest(RQID_GET_DATA_SOURCES_STATISTICS);
}

void DataSourcesStateModel::ackTimeout()
{
	beginResetModel();
	m_dataSource.clear();
	endResetModel();
	sendDataRequest(RQID_GET_DATA_SOURCES_IDS);
}

void DataSourcesStateModel::ackReceived(UdpRequest udpRequest)
{
	quint32 sourceCount = 0;

	switch (udpRequest.ID())
	{
	case RQID_GET_DATA_SOURCES_IDS:
		sourceCount = udpRequest.readDword();

		if (sourceCount == 0)
		{
			beginResetModel();
			m_dataSource.clear();
			endResetModel();
		}
		else
		{
			beginResetModel();
			for (quint32 i = 0; i < sourceCount; i++)
			{
				quint32 sourceID = udpRequest.readDword();

				DataSource* dataSource = new DataSource;
				dataSource->setID(sourceID);
				m_dataSource.append(static_cast<int>(sourceID), dataSource);
			}
			endResetModel();

			sendDataRequest(RQID_GET_DATA_SOURCES_INFO);
		}

		break;

	case RQID_GET_DATA_SOURCES_INFO:
		sourceCount = udpRequest.readDword();

		assert((int)sourceCount == m_dataSource.count());
		for (quint32 i = 0; i < sourceCount; i++)
		{
			DataSourceInfo dsi;

			udpRequest.readStruct(&dsi);

			if (m_dataSource.contains(dsi.ID))
			{
				m_dataSource[m_dataSource.keyIndex(dsi.ID)].setInfo(dsi);
			}
		}
		sendDataRequest(RQID_GET_DATA_SOURCES_STATISTICS);
		emit changedSourceInfo();
		m_periodicTimer->start(500);
		break;

	case RQID_GET_DATA_SOURCES_STATISTICS:
		sourceCount = udpRequest.readDword();

		assert((int)sourceCount == m_dataSource.count());
		for (quint32 i = 0; i < sourceCount; i++)
		{
			DataSourceStatistics dss;

			udpRequest.readStruct(&dss);

			if (m_dataSource.contains(dss.ID))
			{
				m_dataSource[m_dataSource.keyIndex(dss.ID)].setStatistics(dss);
			}
		}
		emit changedSourceState();
		break;

	default:
		assert(false);
	}
}

void DataSourcesStateModel::sendDataRequest(int requestType)
{
	m_currentDataRequestType = requestType;
	UdpRequest getInfoRequest;
	getInfoRequest.setID(requestType);

	if (requestType != RQID_GET_DATA_SOURCES_IDS)
	{
		if (m_dataSource.count() == 0)
		{
			assert(false);
			return;
		}
		getInfoRequest.writeDword(m_dataSource.count());
		for (int i = 0; i < m_dataSource.count(); i++)
		{
			getInfoRequest.writeDword(m_dataSource[i].ID());
		}
	}

	assert(getInfoRequest.ID() != 0);
	dataClientSendRequest(getInfoRequest);
}


DataAquisitionServiceWidget::DataAquisitionServiceWidget(quint32 ip, int portIndex, QWidget *parent) :
	BaseServiceStateWidget(ip, portIndex, parent)
{
	m_model = new DataSourcesStateModel(QHostAddress(ip), this);
	m_view = new QTableView;
	m_view->setModel(m_model);

	m_view->resizeColumnsToContents();

	connect(m_model, &DataSourcesStateModel::changedSourceInfo, this, &DataAquisitionServiceWidget::updateSourceInfo);
	connect(m_model, &DataSourcesStateModel::changedSourceState, this, &DataAquisitionServiceWidget::updateSourceState);

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &DataAquisitionServiceWidget::checkVisibility);
	timer->start(5);

	addTab(m_view, tr("Data Sources"));
}

void DataAquisitionServiceWidget::updateSourceInfo()
{
	m_view->resizeColumnToContents(DSC_NAME);
	m_view->resizeColumnToContents(DSC_IP);
	m_view->resizeColumnToContents(DSC_PART_COUNT);
}

void DataAquisitionServiceWidget::updateSourceState()
{
	m_view->resizeColumnToContents(DSC_STATE);
	m_view->resizeColumnToContents(DSC_UPTIME);
	m_view->resizeColumnToContents(DSC_RECEIVED);
	m_view->resizeColumnToContents(DSC_SPEED);
}

void DataAquisitionServiceWidget::checkVisibility()
{
	if (isVisible() && !m_model->isActive())
	{
		m_model->setActive(true);
	}

	if (!isVisible() && m_model->isActive())
	{
		m_model->setActive(false);
	}
}
