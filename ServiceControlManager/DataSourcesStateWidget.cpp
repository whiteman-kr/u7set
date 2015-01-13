#include "DataSourcesStateWidget.h"
#include <QTableView>
#include <QVBoxLayout>


const int DSC_IP = 0,
DSC_FRAME_COUNT = 1,
DSC_STATE = 2,
DSC_UPTIME = 3,
DSC_RECEIVED = 4,
DSC_SPEED = 5;


const char* const dataSourceColumnStr[] =
{
	"IP",
	"Frame count",
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

	m_model = new DataSourcesStateModel(host, serviceTypesInfo[portIndex].port, this);
	QTableView* view = new QTableView(this);
	view->setModel(m_model);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(view);
	setLayout(vl);

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &DataSourcesStateWidget::checkVisibility);
	m_timer->start(5);
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


DataSourcesStateModel::DataSourcesStateModel(QHostAddress ip, quint16 port, QObject* parent) :
	QAbstractTableModel(parent)
{
	m_clientSocket = new UdpClientSocket(ip, port);
	connect(m_clientSocket, &UdpClientSocket::ackTimeout, this, &DataSourcesStateModel::ackTimeout);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &DataSourcesStateModel::ackReceived);

	onTimer();
}

DataSourcesStateModel::~DataSourcesStateModel()
{
	if (m_clientSocket != nullptr)
	{
		m_clientSocket->deleteLater();
	}
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
			case DSC_IP: return d.hostAddress().toString();
			case DSC_FRAME_COUNT: return d.partCount();
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
			case DSC_SPEED: return 0;
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
			return m_dataSource[section].name();
		}
	}
	return QVariant();
}

void DataSourcesStateModel::setActive(bool active)
{
	m_active = active;
	if (active)
	{
		onTimer();
	}
}

void DataSourcesStateModel::onTimer()
{
	if (!m_active)
	{
		return;
	}

	if (m_clientSocket->isWaitingForAck())
	{
		QTimer::singleShot(500, this, SLOT(onTimer()));
	}

	m_clientSocket->sendShortRequest(RQID_GET_DATA_SOURCES_IDS);
}

void DataSourcesStateModel::ackTimeout()
{
	if (!m_active)
	{
		return;
	}
}

void DataSourcesStateModel::ackReceived(UdpRequest udpRequest)
{
	if (!m_active)
	{
		return;
	}
}
