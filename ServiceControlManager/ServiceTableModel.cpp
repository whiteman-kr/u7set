#include "ServiceTableModel.h"
#include <QBrush>
#include <QDebug>
#include "../include/UdpSocket.h"
#include "../include/SocketIO.h"
#include <QSettings>
#include <QWidget>
#include <QApplication>
#include <QBuffer>
#include "DataAquisitionServiceWidget.h"
#include "ConfigurationServiceWidget.h"
#include "../include/Types.h"


ServiceData::ServiceData() :
	clientSocket(nullptr),
	statusWidget(nullptr)
{
	information.type = ServiceType::BaseService;
	information.serviceState = ServiceState::Undefined;
}


ServiceTableModel::ServiceTableModel(QObject *parent) :
	QAbstractTableModel(parent),
	m_freezeUpdate(false),
	m_timer(parent)
{
	QSettings settings;

	int size = settings.beginReadArray("ServiceTableModel/ServerList");

	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		HostInfo hi;
		hi.ip = settings.value("IP").toUInt();
		m_hostsInfo.append(hi);
	}

	connect(&m_timer, &QTimer::timeout, this, &ServiceTableModel::checkServiceStates);

	m_timer.start(500);
}


ServiceTableModel::~ServiceTableModel()
{
	m_timer.stop();

	QSettings settings;
	settings.beginWriteArray("ServiceTableModel/ServerList", m_hostsInfo.count());

	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("IP", m_hostsInfo[i].ip);
		for (uint j = 0; j < SERVICE_TYPE_COUNT; j++)
		{
			if (m_hostsInfo[i].servicesData[j].statusWidget != nullptr)
			{
				delete m_hostsInfo[i].servicesData[j].statusWidget;
			}
		}
	}
	settings.endArray();

	finishtUdpSocketThread();
}


void ServiceTableModel::startUdpSocketThread()
{
	if (m_socketThread != nullptr)
	{
		return;
	}

	m_socketThread = new UdpSocketThread();

	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		for (uint j = 0; j < SERVICE_TYPE_COUNT; j++)
		{
			UdpClientSocket* clientSocket = m_hostsInfo[i].servicesData[j].clientSocket;

			assert(clientSocket == nullptr);

			clientSocket = new UdpClientSocket(QHostAddress(m_hostsInfo[i].ip), serviceInfo[j].port);
			connect(clientSocket, &UdpClientSocket::ackTimeout, this, &ServiceTableModel::serviceNotFound);
			connect(clientSocket, &UdpClientSocket::ackReceived, this, &ServiceTableModel::serviceAckReceived);
			m_hostsInfo[i].servicesData[j].clientSocket = clientSocket;

			m_socketThread->addWorker(clientSocket);

			if (!clientSocket->isWaitingForAck())
			{
				clientSocket->sendRequest(RQID_SERVICE_GET_INFO);
			}
		}
	}

	m_socketThread->start();
}


void ServiceTableModel::finishtUdpSocketThread()
{
	if (m_socketThread == nullptr)
	{
		return;
	}

	m_socketThread->quitAndWait();
	delete m_socketThread;
	m_socketThread = nullptr;

	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		for (uint j = 0; j < SERVICE_TYPE_COUNT; j++)
		{
			m_hostsInfo[i].servicesData[j].clientSocket = nullptr;
		}
	}
}


void ServiceTableModel::restartUdpSocketThread()
{
	finishtUdpSocketThread();
	startUdpSocketThread();
}


int ServiceTableModel::rowCount(const QModelIndex&) const
{
	return m_hostsInfo.count();
}

int ServiceTableModel::columnCount(const QModelIndex&) const
{
	return SERVICE_TYPE_COUNT;
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	const ServiceInformation& si = m_hostsInfo[row].servicesData[col].information;
	switch(role)
	{
		case Qt::DisplayRole:
		{
			QString str;
			bool serviceFound = false;

			for (uint i = 0; i < SERVICE_TYPE_COUNT; i++)
			{
				if (serviceInfo[i].serviceType == si.type)
				{
					str = serviceInfo[i].name;
					serviceFound = true;
					break;
				}
			}
			if (serviceFound)
			{
				str += QString(" v%1.%2.%3(0x%4)\n").arg(si.majorVersion).arg(si.minorVersion).arg(si.buildNo).arg(si.crc, 0, 16, QChar('0'));
			}
			if (si.serviceState != ServiceState::Undefined && si.serviceState != ServiceState::Unavailable)
			{
				quint32 time = si.uptime;
				int s = time % 60; time /= 60;
				int m = time % 60; time /= 60;
				int h = time % 24; time /= 24;
				str += tr("Uptime") + QString(" (%1d %2:%3:%4)\n").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
			}
			switch(si.serviceState)
			{
				case ServiceState::Work:
				{
					quint32 time = si.serviceUptime;
					int s = time % 60; time /= 60;
					int m = time % 60; time /= 60;
					int h = time % 24; time /= 24;
					str += tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
				} break;
				case ServiceState::Stopped: str += tr("Stopped"); break;
				case ServiceState::Unavailable: str += tr("Unavailable"); break;
				case ServiceState::Undefined: str += tr("Undefined"); break;
				case ServiceState::Starts: str += tr("Starts"); break;
				case ServiceState::Stops: str += tr("Stops"); break;
				default: str += tr("Unknown state"); break;
			}
			return str;
		}
			break;
		case Qt::BackgroundRole:
			switch(si.serviceState)
			{
				case ServiceState::Work:
					return QBrush(QColor(0x7f,0xff,0x7f));
				case ServiceState::Starts:
				case ServiceState::Stops:
				case ServiceState::Stopped:
					return QBrush(QColor(0xff,0xff,0x7f));
				case ServiceState::Unavailable:
					return QBrush(Qt::lightGray);
				default: return QBrush(Qt::red);
			}
			break;
		default:
			return QVariant();
	}
}

QVariant ServiceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			return tr(serviceInfo[section].name);
		}
		if (orientation == Qt::Vertical)
		{
			return QHostAddress(m_hostsInfo[section].ip).toString();
		}
	}
	return QVariant();
}

void ServiceTableModel::setServiceState(quint32 ip, quint16 port, ServiceState state)
{
	int portIndex = -1;
	for (uint i = 0; i < SERVICE_TYPE_COUNT; i++)
	{
		if (serviceInfo[i].port == port)
		{
			portIndex = i;
			break;
		}
	}
	if (portIndex == -1)
	{
		return;
	}
	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		if (m_hostsInfo[i].ip == ip)
		{
			ServiceInformation& si = m_hostsInfo[i].servicesData[portIndex].information;
			if (si.serviceState != state)
			{
				si.serviceState = state;
				emit serviceStateChanged(i);
			}
			QModelIndex changedIndex = index(i, portIndex);
			emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::DisplayRole);
			return;
		}
	}
	HostInfo hi;
	hi.ip = ip;
	hi.servicesData[portIndex].information.serviceState = state;
	beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
	m_hostsInfo.append(hi);
	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(m_hostsInfo.count() - 1);
}

QPair<int,int> ServiceTableModel::getServiceState(quint32 ip, quint16 port)
{
	int portIndex = -1;
	for (uint i = 0; i < SERVICE_TYPE_COUNT; i++)
	{
		if (serviceInfo[i].port == port)
		{
			portIndex = i;
			break;
		}
	}
	if (portIndex == -1)
	{
		return QPair<int,int>(-1, -1);
	}
	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		if (m_hostsInfo[i].ip == ip)
		{
			return QPair<int,int>(i, portIndex);
		}
	}
	return QPair<int, int>(-1, portIndex);
}

void ServiceTableModel::addAddress(QString connectionAddress)
{
	QHostAddress ha(connectionAddress);
	if (ha.protocol() != QAbstractSocket::IPv4Protocol)
	{
		return;
	}
	quint32 ip = ha.toIPv4Address();
	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		if (m_hostsInfo[i].ip == ip)
		{
			return;
		}
	}
	HostInfo hi;
	hi.ip = ip;
	beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
	m_hostsInfo.append(hi);
	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(m_hostsInfo.count() - 1);
}


void ServiceTableModel::serviceAckReceived(const UdpRequest udpRequest)
{
	UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());
	if (socket == nullptr)
	{
		return;
	}
	switch (udpRequest.ID())
	{
		case RQID_SERVICE_GET_INFO:
		{
			quint32 ip = socket->serverAddress().toIPv4Address();
			QPair<int, int> place = getServiceState(ip, socket->port());

			if (place.first == -1)
			{
				const QHostAddress& sa = socket->serverAddress();
				if (sa.protocol() != QAbstractSocket::IPv4Protocol)
				{
					return;
				}
				HostInfo hi;
				hi.ip = sa.toIPv4Address();
				hi.servicesData[place.second].information = *(ServiceInformation*)udpRequest.data();
				beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
				m_hostsInfo.append(hi);
				endInsertRows();

				restartUdpSocketThread();

				return;
			}

			ServiceInformation& info = m_hostsInfo[place.first].servicesData[place.second].information;
			ServiceInformation& newInfo = *(ServiceInformation*)udpRequest.data();

			if (info.serviceState != newInfo.serviceState)
			{
				info = newInfo;
				emit serviceStateChanged(place.first);
			}
			else
			{
				info = newInfo;
			}
			QModelIndex changedIndex = index(place.first, place.second);
			emit dataChanged(changedIndex, changedIndex);
		}
		case RQID_SERVICE_START:
		case RQID_SERVICE_STOP:
		case RQID_SERVICE_RESTART:
			break;
		default:
			qDebug() << "Unknown packet ID";
	}
}

void ServiceTableModel::serviceNotFound()
{
	UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());
	if (socket == nullptr)
	{
		return;
	}
	quint32 ip = socket->serverAddress().toIPv4Address();
	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		if (m_hostsInfo[i].ip == ip)
		{
			setServiceState(socket->serverAddress().toIPv4Address(), socket->port(), ServiceState::Unavailable);
			return;
		}
	}
	socket->deleteLater();
}

void ServiceTableModel::checkServiceStates()
{
	if (m_freezeUpdate)
	{
		return;
	}

	if (m_socketThread == nullptr)
	{
		startUdpSocketThread();
	}

	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		for (uint j = 0; j < SERVICE_TYPE_COUNT; j++)
		{
			UdpClientSocket* clientSocket = m_hostsInfo[i].servicesData[j].clientSocket;

			assert(clientSocket != nullptr);

			if (clientSocket == nullptr)
			{
				continue;
			}

			if (!clientSocket->isWaitingForAck())
			{
				clientSocket->sendRequest(RQID_SERVICE_GET_INFO);
			}
		}
	}
}

void ServiceTableModel::removeHost(int row)
{
	beginRemoveRows(QModelIndex(), row, row);
	for (uint j = 0; j < SERVICE_TYPE_COUNT; j++)
	{
		if (m_hostsInfo[row].servicesData[j].statusWidget != nullptr)
		{
			delete m_hostsInfo[row].servicesData[j].statusWidget;
		}
	}
	m_hostsInfo.removeAt(row);
	endRemoveRows();

	restartUdpSocketThread();
}

void ServiceTableModel::openServiceStatusWidget(const QModelIndex& index)
{
	ServiceData& serviceData = m_hostsInfo[index.row()].servicesData[index.column()];
	if (serviceData.statusWidget == nullptr)
	{
		quint32 serviceType = TO_INT(serviceData.information.type);
		if (serviceType >= SERVICE_TYPE_COUNT)
		{
			serviceType = index.column();
		}
		switch (serviceType)
		{
			case ServiceType::AppDataService:
				serviceData.statusWidget = new DataAquisitionServiceWidget(m_hostsInfo[index.row()].ip, index.column());
				break;
			case ServiceType::ConfigurationService:
				serviceData.statusWidget = new ConfigurationServiceWidget(m_hostsInfo[index.row()].ip, index.column());
				break;
			default:
				serviceData.statusWidget = new BaseServiceStateWidget(m_hostsInfo[index.row()].ip, index.column());
				break;
		}
	}

	serviceData.statusWidget->showNormal();
	serviceData.statusWidget->raise();
	serviceData.statusWidget->activateWindow();
}

void ServiceTableModel::setServiceInformation(quint32 ip, quint16 port, ServiceInformation serviceInfo)
{
	QPair<int, int> place = getServiceState(ip, port);

	if (place.first >= m_hostsInfo.count() || place.second == -1 || place.second >= (int)SERVICE_TYPE_COUNT)
	{
		return;
	}

	if (place.first == -1)
	{
		HostInfo hi;
		hi.ip = ip;
		hi.servicesData[place.second].information = serviceInfo;
		beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
		m_hostsInfo.append(hi);
		endInsertRows();

		restartUdpSocketThread();
	}
	else
	{
		ServiceInformation& info = m_hostsInfo[place.first].servicesData[place.second].information;

		if (info.serviceState != serviceInfo.serviceState)
		{
			info = serviceInfo;
			emit serviceStateChanged(place.first);
		}
		else
		{
			info = serviceInfo;
		}
		QModelIndex changedIndex = index(place.first, place.second);
		emit dataChanged(changedIndex, changedIndex);
	}
}
