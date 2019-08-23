#include "ServiceTableModel.h"
#include <QBrush>
#include <QDebug>
#include "../lib/UdpSocket.h"
#include "../lib/SocketIO.h"
#include <QSettings>
#include <QWidget>
#include <QApplication>
#include <QBuffer>
#include "AppDataServiceWidget.h"
#include "ConfigurationServiceWidget.h"
#include "TuningServiceWidget.h"
#include "../lib/Types.h"

HostInfo::HostInfo() : ip(0)
{
	servicesData.resize(servicesInfo.count());

	for (int i = 0; i < servicesInfo.count(); i++)
	{
		servicesData[i].information.mutable_softwareinfo()->set_softwaretype(servicesInfo[i].softwareType);
	}
}

ServiceData::ServiceData() :
	clientSocket(nullptr),
	statusWidget(nullptr)
{
	information.mutable_softwareinfo()->set_softwaretype(E::SoftwareType::BaseService);
	information.set_servicestate(TO_INT(ServiceState::Undefined));
}

ServiceTableModel::ServiceTableModel(const SoftwareInfo& softwareInfo, QWidget *parent) :
	QAbstractTableModel(parent),
	m_softwareInfo(softwareInfo),
	m_freezeUpdate(false),
	m_parrentWidget(parent),
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

		for (int j = 0; j < servicesInfo.count(); j++)
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
		for (int j = 0; j < servicesInfo.count(); j++)
		{
			UdpClientSocket*& clientSocket = m_hostsInfo[i].servicesData[j].clientSocket;

			assert(clientSocket == nullptr);

			clientSocket = new UdpClientSocket(QHostAddress(m_hostsInfo[i].ip), servicesInfo[j].port);

			connect(clientSocket, &UdpClientSocket::ackTimeout, this, &ServiceTableModel::serviceNotFound);
			connect(clientSocket, &UdpClientSocket::ackReceived, this, &ServiceTableModel::serviceAckReceived);

//			m_hostsInfo[i].servicesData[j].clientSocket = clientSocket;

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
		for (int j = 0; j < servicesInfo.count(); j++)
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
	return servicesInfo.count();
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();

	const Network::ServiceInfo& si = m_hostsInfo[row].servicesData[col].information;

	ServiceState serviceState = static_cast<ServiceState>(si.servicestate());

	switch(role)
	{
		case Qt::DisplayRole:
		{
			QString str;
			bool serviceFound = false;

			for (int i = 0; i < servicesInfo.count(); i++)
			{
				if (servicesInfo[i].softwareType == static_cast<E::SoftwareType>(si.softwareinfo().softwaretype()))
				{
					str = servicesInfo[i].name;
					serviceFound = true;
					break;
				}
			}
			if (serviceFound)
			{
				str += QString(" v%1.%2.%3(0x%4)\n").arg(si.softwareinfo().majorversion()).
													 arg(si.softwareinfo().minorversion()).
													 arg(si.softwareinfo().commitno()).
													 arg(si.softwareinfo().crc(), 0, 16, QChar('0'));
			}

			if (serviceState != ServiceState::Undefined &&
				serviceState != ServiceState::Unavailable)
			{
				qint64 time = si.uptime();
				qint64 s = time % 60; time /= 60;
				qint64 m = time % 60; time /= 60;
				qint64 h = time % 24; time /= 24;
				str += tr("Uptime") + QString(" (%1d %2:%3:%4)\n").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
			}
			switch(serviceState)
			{
				case ServiceState::Work:
				{
					qint64 time = si.serviceuptime();
					qint64 s = time % 60; time /= 60;
					qint64 m = time % 60; time /= 60;
					qint64 h = time % 24; time /= 24;
					str += tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
				} break;
				case ServiceState::Stopped: str += tr("Stopped"); break;
				case ServiceState::Unavailable: str += tr("Unavailable"); break;
				case ServiceState::Undefined: str += tr("Undefined"); break;
				case ServiceState::Starts: str += tr("Starts"); break;
				case ServiceState::Stops: str += tr("Stops"); break;
				default: str += tr("Unknown state"); break;
			}
			if (serviceState != ServiceState::Undefined &&
				serviceState != ServiceState::Unavailable)
			{
				str += tr("\nListening clients on %1:%2").arg(QHostAddress(si.clientrequestip()).toString()).arg(si.clientrequestport());
			}
			return str;
		}
			break;
		case Qt::BackgroundRole:
			switch(serviceState)
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
			return servicesInfo[section].name;
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

	for (int i = 0; i < servicesInfo.count(); i++)
	{
		if (servicesInfo[i].port == port)
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
			Network::ServiceInfo& si = m_hostsInfo[i].servicesData[portIndex].information;

			if (static_cast<ServiceState>(si.servicestate()) != state)
			{
				si.set_servicestate(TO_INT(state));
				emit serviceStateChanged(i);
			}
			QModelIndex changedIndex = index(i, portIndex);
			emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::DisplayRole);
			return;
		}
	}

	HostInfo hi;
	hi.ip = ip;
	hi.servicesData[portIndex].information.set_servicestate(TO_INT(state));
	beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
	m_hostsInfo.append(hi);

	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(m_hostsInfo.count() - 1);
}

QPair<int,int> ServiceTableModel::getServiceState(quint32 ip, quint16 port)
{
	int portIndex = -1;

	for (int i = 0; i < servicesInfo.count(); i++)
	{
		if (servicesInfo[i].port == port)
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

			Network::ServiceInfo newServiceInfo;

			if (newServiceInfo.ParseFromArray(udpRequest.data(),
											  static_cast<int>(udpRequest.dataSize())) == false)
			{
				assert(false);
				return;
			}

			if (place.first == -1)
			{
				const QHostAddress& sa = socket->serverAddress();

				if (sa.protocol() != QAbstractSocket::IPv4Protocol)
				{
					return;
				}

				HostInfo hi;
				hi.ip = sa.toIPv4Address();
				hi.servicesData[place.second].information = newServiceInfo;

				beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());

				m_hostsInfo.append(hi);

				endInsertRows();

				restartUdpSocketThread();

				return;
			}

			Network::ServiceInfo& info = m_hostsInfo[place.first].servicesData[place.second].information;

			if (info.servicestate() != newServiceInfo.servicestate())
			{
				info = newServiceInfo;
				emit serviceStateChanged(place.first);
			}
			else
			{
				info = newServiceInfo;
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
		for (int j = 0; j < servicesInfo.count(); j++)
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

	for (int j = 0; j < servicesInfo.count(); j++)
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
		E::SoftwareType serviceSoftwareType = static_cast<E::SoftwareType>(serviceData.information.softwareinfo().softwaretype());
		quint16 udpPort = servicesInfo[index.column()].port;

		switch (serviceSoftwareType)
		{
		case E::SoftwareType::AppDataService:
			serviceData.statusWidget = new AppDataServiceWidget(m_softwareInfo, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
			break;

		case E::SoftwareType::ConfigurationService:
			serviceData.statusWidget = new ConfigurationServiceWidget(m_softwareInfo, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
			break;

		case E::SoftwareType::TuningService:
			serviceData.statusWidget = new TuningServiceWidget(m_softwareInfo, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
			break;

		default:
			serviceData.statusWidget = new BaseServiceStateWidget(m_softwareInfo, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
		}
	}

	serviceData.statusWidget->showNormal();
	serviceData.statusWidget->raise();
	serviceData.statusWidget->activateWindow();
}

void ServiceTableModel::setServiceInformation(quint32 ip, quint16 port, Network::ServiceInfo sInfo)
{
	QPair<int, int> place = getServiceState(ip, port);

	if (place.first >= m_hostsInfo.count() || place.second == -1 || place.second >= servicesInfo.count())
	{
		return;
	}

	if (place.first == -1)
	{
		HostInfo hi;
		hi.ip = ip;
		hi.servicesData[place.second].information = sInfo;
		beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
		m_hostsInfo.append(hi);
		endInsertRows();

		restartUdpSocketThread();
	}
	else
	{
		Network::ServiceInfo& info = m_hostsInfo[place.first].servicesData[place.second].information;

		if (info.servicestate() != sInfo.servicestate())
		{
			info = sInfo;
			emit serviceStateChanged(place.first);
		}
		else
		{
			info = sInfo;
		}
		QModelIndex changedIndex = index(place.first, place.second);
		emit dataChanged(changedIndex, changedIndex);
	}
}
