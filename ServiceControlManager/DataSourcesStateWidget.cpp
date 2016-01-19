#include "DataSourcesStateWidget.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>


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


DataSourcesStateWidget::DataSourcesStateWidget(quint32 ip, int portIndex, QWidget *parent) :
	QWidget(parent)
{
	QHostAddress host = QHostAddress(ip);
	setWindowTitle(QString(serviceTypesInfo[portIndex].name) + " - " + host.toString());

	serviceState.mainFunctionState = SS_MF_UNDEFINED;

	m_stateLabel = new QLabel(this);

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(m_stateLabel);

	startServiceButton = new QPushButton(tr("Start service"), this);
	connect(startServiceButton, &QPushButton::clicked, this, &DataSourcesStateWidget::startService);
	hl->addWidget(startServiceButton);

	stopServiceButton = new QPushButton(tr("Stop service"), this);
	connect(stopServiceButton, &QPushButton::clicked, this, &DataSourcesStateWidget::stopService);
	hl->addWidget(stopServiceButton);

	restartServiceButton = new QPushButton(tr("Restart service"), this);
	connect(restartServiceButton, &QPushButton::clicked, this, &DataSourcesStateWidget::restartService);
	hl->addWidget(restartServiceButton);

	m_model = new DataSourcesStateModel(host, this);
	m_view = new QTableView(this);
	m_view->setModel(m_model);

	m_view->resizeColumnsToContents();

	connect(m_model, &DataSourcesStateModel::changedSourceInfo, this, &DataSourcesStateWidget::updateSourceInfo);
	connect(m_model, &DataSourcesStateModel::changedSourceState, this, &DataSourcesStateWidget::updateSourceState);

	QVBoxLayout* vl = new QVBoxLayout;

	vl->addLayout(hl);
	vl->addWidget(m_view);
	setLayout(vl);

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &DataSourcesStateWidget::checkVisibility);
	m_timer->start(5);

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &DataSourcesStateWidget::askServiceState);
	timer->start(500);

	m_clientSocket = new UdpClientSocket(QHostAddress(ip), serviceTypesInfo[portIndex].port);
	connect(m_clientSocket, &UdpClientSocket::ackTimeout, this, &DataSourcesStateWidget::serviceNotFound);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &DataSourcesStateWidget::serviceAckReceived);

	updateServiceState();
}

DataSourcesStateWidget::~DataSourcesStateWidget()
{
}

void DataSourcesStateWidget::checkVisibility()
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

void DataSourcesStateWidget::updateSourceInfo()
{
	m_view->resizeColumnToContents(DSC_NAME);
	m_view->resizeColumnToContents(DSC_IP);
	m_view->resizeColumnToContents(DSC_PART_COUNT);
}

void DataSourcesStateWidget::updateSourceState()
{
	m_view->resizeColumnToContents(DSC_STATE);
	m_view->resizeColumnToContents(DSC_UPTIME);
	m_view->resizeColumnToContents(DSC_RECEIVED);
	m_view->resizeColumnToContents(DSC_SPEED);
}

void DataSourcesStateWidget::updateServiceState()
{
	QString str = serviceTypeStr[SERVICE_DATA_ACQUISITION];
	str += QString(" v%1.%2.%3(0x%4)\n")
			.arg(serviceState.majorVersion)
			.arg(serviceState.minorVersion)
			.arg(serviceState.buildNo)
			.arg(serviceState.crc, 0, 16, QChar('0'));
	if (serviceState.mainFunctionState != SS_MF_UNDEFINED && serviceState.mainFunctionState != SS_MF_UNAVAILABLE)
	{
		quint32 time = serviceState.uptime;
		int s = time % 60; time /= 60;
		int m = time % 60; time /= 60;
		int h = time % 24; time /= 24;
		str += tr("Uptime") + QString(" (%1d %2:%3:%4)\n").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
	}
	switch (serviceState.mainFunctionState)
	{
		case SS_MF_WORK:
		{
			quint32 time = serviceState.mainFunctionUptime;
			int s = time % 60; time /= 60;
			int m = time % 60; time /= 60;
			int h = time % 24; time /= 24;
			str += tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
		} break;
		case SS_MF_STOPPED: str += tr("Stopped"); break;
		case SS_MF_UNAVAILABLE: str += tr("Unavailable"); break;
		case SS_MF_UNDEFINED: str += tr("Undefined"); break;
		case SS_MF_STARTS: str += tr("Starts"); break;
		case SS_MF_STOPS: str += tr("Stops"); break;
		default: str += tr("Unknown state"); break;
	}
	switch (serviceState.mainFunctionState)
	{
		case SS_MF_WORK:
			startServiceButton->setEnabled(false);
			stopServiceButton->setEnabled(true);
			restartServiceButton->setEnabled(true);
			break;
		case SS_MF_STOPPED:
			startServiceButton->setEnabled(true);
			stopServiceButton->setEnabled(false);
			restartServiceButton->setEnabled(true);
			break;
		case SS_MF_UNAVAILABLE:
		case SS_MF_UNDEFINED:
		case SS_MF_STARTS:
		case SS_MF_STOPS:
			startServiceButton->setEnabled(false);
			stopServiceButton->setEnabled(false);
			restartServiceButton->setEnabled(false);
			break;
		default:
			assert(false);
			break;
	}

	m_stateLabel->setText(str);
}

void DataSourcesStateWidget::askServiceState()
{
	m_clientSocket->sendShortRequest(RQID_GET_SERVICE_INFO);
}

void DataSourcesStateWidget::startService()
{
	sendCommand(RQID_SERVICE_MF_START);
}

void DataSourcesStateWidget::stopService()
{
	sendCommand(RQID_SERVICE_MF_STOP);
}

void DataSourcesStateWidget::restartService()
{
	sendCommand(RQID_SERVICE_MF_RESTART);
}

void DataSourcesStateWidget::serviceAckReceived(const UdpRequest udpRequest)
{
	switch (udpRequest.ID())
	{
		case RQID_GET_SERVICE_INFO:
		{
			serviceState = *(ServiceInformation*)udpRequest.data();
			updateServiceState();
		}
		case RQID_SERVICE_MF_START:
		case RQID_SERVICE_MF_STOP:
		case RQID_SERVICE_MF_RESTART:
			break;
		default:
			qDebug() << "Unknown packet ID";
	}
}

void DataSourcesStateWidget::serviceNotFound()
{
	if (serviceState.mainFunctionState != SS_MF_UNAVAILABLE)
	{
		serviceState.mainFunctionState = SS_MF_UNAVAILABLE;
		updateServiceState();
	}
}

void DataSourcesStateWidget::sendCommand(int command)
{
	int state = serviceState.mainFunctionState;
	if (!(state == SS_MF_WORK && (command == RQID_SERVICE_MF_STOP || command == RQID_SERVICE_MF_RESTART)) &&
		!(state == SS_MF_STOPPED && (command == RQID_SERVICE_MF_START || command == RQID_SERVICE_MF_RESTART)))
	{
		return;
	}
	m_clientSocket->sendShortRequest(command);
}


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
