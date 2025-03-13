#include "ServiceTableModel.h"
#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/SocketIO.h"
#include "AppDataServiceWidget.h"
#include "CfgServiceWidget.h"
#include "TuningServiceWidget.h"

// --------------------------------------------------------------------------------------
//
//	Host struct implementation
//
// --------------------------------------------------------------------------------------

Host::Host()
{
	servicesData.reserve(servicesInfo.size() - 1);

	for (const ServiceInfo& si : servicesInfo)
	{
		if (si.softwareType == E::SoftwareType::BaseService)
		{
			continue;
		}

		ServiceData& sd = servicesData.emplace_back(ServiceData{});

		sd.type = si.softwareType;
		sd.serviceName = si.name;
		sd.port = si.port;

		sd.protoServiceInfo.mutable_softwareinfo()->set_softwaretype(si.softwareType);
	}
}

// --------------------------------------------------------------------------------------
//
//	ServiceTableModel class implementation
//
// --------------------------------------------------------------------------------------

ServiceTableModel::ServiceTableModel(const SoftwareInfo& softwareInfo, QWidget *parent) :
	QAbstractTableModel(parent),
	m_parentWidget(parent),
	m_softwareInfo(softwareInfo),
	m_timer(parent)
{
	int serviceColumn = 0;

	for(const ServiceInfo& si : servicesInfo)
	{
		if (si.softwareType == E::SoftwareType::BaseService)
		{
			continue;

		}
		m_serviceColumn.emplace(si.port, serviceColumn);
		serviceColumn++;
	}

	QSettings settings;

	int size = settings.beginReadArray("ServiceTableModel/ServerList");

	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);

		Host& hi = m_hosts.emplace_back(Host{});

		hi.hostIP = settings.value("IP").toUInt();
	}

	connect(&m_timer, &QTimer::timeout, this, &ServiceTableModel::checkServiceStates);

	m_timer.start(500);
}

ServiceTableModel::~ServiceTableModel()
{
	m_timer.stop();

	for(BaseServiceWidget* srvWidget : m_srvWidgets)
	{
		DELETE_IF_NOT_NULL(srvWidget);
	}

	m_srvWidgets.clear();

	QSettings settings;

	int hstCount = hostsCount();

	settings.beginWriteArray("ServiceTableModel/ServerList", hstCount);

	for (int i = 0; i < hstCount; i++)
	{
		Host& host = m_hosts[i];

		settings.setArrayIndex(i);
		settings.setValue("IP", host.hostIP);
	}

	settings.endArray();

	finishtUdpSocketThread();
}

int ServiceTableModel::rowCount(const QModelIndex&) const
{
	return hostsCount();
}

int ServiceTableModel::columnCount(const QModelIndex&) const
{
	return serviceCount();
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
	int hostIndex = index.row();
	int serviceIndex = index.column();

	const ServiceData& sd = m_hosts[hostIndex].servicesData[serviceIndex];
	const Network::ServiceInfo& si = sd.protoServiceInfo;

	E::ServiceState srvState = serviceState(si);

	switch(role)
	{
	case Qt::DisplayRole:
		{
			QString str;

			if (sd.serviceState() == E::ServiceState::Undefined ||
				sd.serviceState() == E::ServiceState::Unavailable)
			{
				return QString("Not available");
			}

			str += sd.serviceName;
			str += QString(" v%1.%2.%3\n\n").arg(si.softwareinfo().majorversion()).
				   arg(si.softwareinfo().minorversion()).
				   arg(si.softwareinfo().patchversion());

			switch(srvState)
			{
			case E::ServiceState::Work:
					str += tr("Running in ") + E::valueToString(sd.sessionParams.softwareRunMode) + " mode";
					break;

			case E::ServiceState::Stopped:
			case E::ServiceState::Starts:
			case E::ServiceState::Stops:
					str += E::valueToString(srvState);
					break;

			case E::ServiceState::Unavailable:
			case E::ServiceState::Undefined:
			default:
				Q_ASSERT(false);
				str += tr("Unknown state");
			}

			str += QString("\n\nUptime %1").arg(formatUptime(si.uptime()));

			return str;
		}
		break;

	case Qt::BackgroundRole:
		switch(srvState)
		{
		case E::ServiceState::Work:
			return QBrush((sd.sessionParams.softwareRunMode == E::SoftwareRunMode::Normal) ? QColor(0x7f,0xff,0x7f) : QColor(0xff,0xff,0x7f)) ;
		case E::ServiceState::Starts:
		case E::ServiceState::Stops:
		case E::ServiceState::Stopped:
			return QBrush(QColor(0xff,0x7f,0x7f));
		case E::ServiceState::Unavailable:
		default:
			return QBrush(Qt::lightGray);
		}
		break;

	case Qt::TextAlignmentRole:
		return Qt::AlignCenter;

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
			return servicesInfo[section + 1].name;
		}

		if (orientation == Qt::Vertical)
		{
			return QHostAddress(m_hosts[section].hostIP).toString();
		}
	}

	return QVariant();
}

void ServiceTableModel::addAddress(const QString &connectionAddress)
{
	QHostAddress ha(connectionAddress);

	if (ha.protocol() != QAbstractSocket::IPv4Protocol)
	{
		return;
	}

	quint32 ip = ha.toIPv4Address();

	for (int i = 0; i < hostsCount(); i++)
	{
		if (m_hosts[i].hostIP == ip)
		{
			return;
		}
	}

	Host hi;
	hi.hostIP = ip;

	beginInsertRows(QModelIndex(), hostsCount(), hostsCount());
	m_hosts.push_back(hi);
	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(hostsCount() - 1);
}

void ServiceTableModel::deleteSrvWidget(BaseServiceWidget* srvWidget)
{
	auto it = m_srvWidgets.find(srvWidget);

	if (it != m_srvWidgets.end())
	{
		DELETE_IF_NOT_NULL(srvWidget);
		m_srvWidgets.erase(it);
	}
}

void ServiceTableModel::serviceAckReceived(const UdpRequest udpRequest)
{
	UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());

	if (socket == nullptr)
	{
		return;
	}

	quint32 rqID = udpRequest.ID();

	switch (rqID)
	{
	case RQID_SERVICE_GET_INFO:
		Q_ASSERT(false);
		break;

	case RQID_SERVICE_GET_SHORT_INFO:
	{
		quint32 ip = socket->serverAddress().toIPv4Address();

		int hostIndex = -1;
		int serviceIndex = -1;

		getServiceState(ip, socket->port(), &hostIndex, &serviceIndex);

		Network::ServiceInfo newServiceInfo;

		if (newServiceInfo.ParseFromArray(udpRequest.data(),
										  static_cast<int>(udpRequest.dataSize())) == false)
		{
			qDebug() << Q_FUNC_INFO << "newServiceInfo.ParseFromArray failed";
			Q_ASSERT(false);
			return;
		}

		if (hostIndex == -1)
		{
			const QHostAddress& sa = socket->serverAddress();

			if (sa.protocol() != QAbstractSocket::IPv4Protocol)
			{
				return;
			}

			Host hi;

			hi.hostIP = sa.toIPv4Address();

			ServiceData& sd = hi.servicesData[serviceIndex];
			sd.protoServiceInfo = newServiceInfo;
			sd.parseProtoServiceInfo();

			beginInsertRows(QModelIndex(), hostsCount(), hostsCount());

			m_hosts.push_back(hi);

			endInsertRows();

			restartUdpSocketThread();

			return;
		}

		ServiceData& sd = m_hosts[hostIndex].servicesData[serviceIndex];
		Network::ServiceInfo& info = sd.protoServiceInfo;

		if (info.servicestate() != newServiceInfo.servicestate())
		{
			info = newServiceInfo;
			emit serviceStateChanged(hostIndex);
		}
		else
		{
			info = newServiceInfo;
		}

		sd.parseProtoServiceInfo();
		QModelIndex changedIndex = index(hostIndex, serviceIndex);
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

void ServiceTableModel::serviceNotAck()
{
	UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());

	if (socket == nullptr)
	{
		return;
	}
	quint32 ip = socket->serverAddress().toIPv4Address();

	for (int i = 0; i < hostsCount(); i++)
	{
		if (m_hosts[i].hostIP == ip)
		{
			setServiceState(socket->serverAddress().toIPv4Address(), socket->port(), E::ServiceState::Unavailable);
			return;
		}
	}

	socket->deleteLater();
}

void ServiceTableModel::checkServiceStates()
{
	if (m_socketThread == nullptr)
	{
		startUdpSocketThread();
	}

	for (Host& host : m_hosts)
	{
		for (ServiceData& sd : host.servicesData)
		{
			if (sd.clientSocket == nullptr)
			{
				continue;
			}

			if (!sd.clientSocket->isWaitingForAck())
			{
				sd.clientSocket->sendRequest(RQID_SERVICE_GET_SHORT_INFO);
			}
		}
	}
}

void ServiceTableModel::removeHost(int row)
{
	beginRemoveRows(QModelIndex(), row, row);

	m_hosts.erase(m_hosts.begin() + row);

	endRemoveRows();

	restartUdpSocketThread();
}

void ServiceTableModel::setServiceInformation(quint32 ip, quint16 port, Network::ServiceInfo sInfo)
{
	int hostIndex = -1;
	int serviceIndex = -1;

	getServiceState(ip, port, &hostIndex, &serviceIndex);

	if (hostIndex >= hostsCount() || serviceIndex == -1 || serviceIndex >= servicesInfo.size())
	{
		return;
	}

	if (hostIndex == -1)
	{
		Host hi;
		hi.hostIP = ip;
		hi.servicesData[serviceIndex].protoServiceInfo = sInfo;

		beginInsertRows(QModelIndex(), hostsCount(), hostsCount());

		m_hosts.push_back(hi);

		endInsertRows();

		restartUdpSocketThread();
	}
	else
	{
		Network::ServiceInfo& info = m_hosts[hostIndex].servicesData[serviceIndex].protoServiceInfo;

		if (info.servicestate() != sInfo.servicestate())
		{
			info = sInfo;
			emit serviceStateChanged(hostIndex);
		}
		else
		{
			info = sInfo;
		}
		QModelIndex changedIndex = index(hostIndex, serviceIndex);
		emit dataChanged(changedIndex, changedIndex);
	}
}

void ServiceTableModel::openServiceStatusWidget(const QModelIndex& index)
{
	ServiceData& sd = m_hosts[index.row()].servicesData[index.column()];

	E::SoftwareType serviceSoftwareType = static_cast<E::SoftwareType>(sd.protoServiceInfo.softwareinfo().softwaretype());

	BaseServiceWidget* srvWidget = nullptr;

	switch (serviceSoftwareType)
	{
	case E::SoftwareType::ConfigurationService:
		srvWidget = new CfgServiceWidget(this, m_softwareInfo, sd, m_hosts[index.row()].hostIP, sd.port, m_parentWidget);
		break;

	case E::SoftwareType::AppDataService:
		srvWidget = new AppDataServiceWidget(this, m_softwareInfo, sd, m_hosts[index.row()].hostIP, sd.port, m_parentWidget);
		break;

	case E::SoftwareType::TuningService:
		srvWidget = new TuningServiceWidget(this, m_softwareInfo, sd, m_hosts[index.row()].hostIP, sd.port, m_parentWidget);
		break;

	case E::SoftwareType::ArchiveService:
	case E::SoftwareType::DiagDataService:
	case E::SoftwareType::GatewayService:
	default:
		Q_ASSERT(false);		// To Do
		return;
	}

	if (srvWidget == nullptr)
	{
		return;
	}

	m_srvWidgets.insert(srvWidget);

	srvWidget->showNormal();
	srvWidget->raise();
	srvWidget->activateWindow();
}

void ServiceTableModel::startUdpSocketThread()
{
	if (m_socketThread != nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_socketThread = new UdpSocketThread();

	for(Host& host : m_hosts)
	{
		for(ServiceData& sd : host.servicesData)
		{
			UdpClientSocket*& clientSocket = sd.clientSocket;

			Q_ASSERT(clientSocket == nullptr);

			clientSocket = new UdpClientSocket(QHostAddress(host.hostIP), sd.port);

			connect(clientSocket, &UdpClientSocket::ackTimeout, this, &ServiceTableModel::serviceNotAck);
			connect(clientSocket, &UdpClientSocket::ackReceived, this, &ServiceTableModel::serviceAckReceived);

			m_socketThread->addWorker(clientSocket);
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

	for(Host& host : m_hosts)
	{
		for(ServiceData& sd : host.servicesData)
		{
			sd.clientSocket = nullptr;
		}
	}
}

void ServiceTableModel::restartUdpSocketThread()
{
	finishtUdpSocketThread();
	startUdpSocketThread();
}

void ServiceTableModel::setServiceState(quint32 ip, quint16 port, E::ServiceState state)
{
	int hostIndex = 0;

	for (Host& host : m_hosts)
	{
		if (host.hostIP != ip)
		{
			hostIndex++;
			continue;
		}

		int sdIndex = 0;

		for(ServiceData& sd : host.servicesData)
		{
			if (sd.port != port)
			{
				sdIndex++;
				continue;
			}

			Network::ServiceInfo& si = sd.protoServiceInfo;

			if (serviceState(si) != state)
			{
				si.set_servicestate(TO_INT(state));
				emit serviceStateChanged(hostIndex);
			}

			QModelIndex changedIndex = index(hostIndex, sdIndex);
			emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::DisplayRole);
			return;
		}
	}

	int sdIndex = serviceColumn(port);

	if (sdIndex == -1)
	{
		return;
	}

	Host hi;

	hi.hostIP = ip;
	hi.servicesData[sdIndex].protoServiceInfo.set_servicestate(TO_INT(state));

	beginInsertRows(QModelIndex(), hostsCount(), hostsCount());

	m_hosts.push_back(hi);

	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(hostsCount() - 1);
}

void ServiceTableModel::getServiceState(quint32 ip, quint16 port, int* hostIndex, int* serviceIndex)
{
	TEST_PTR_RETURN(hostIndex);
	TEST_PTR_RETURN(serviceIndex);

	*hostIndex = 0;
	*serviceIndex = serviceColumn(port);

	for(Host& host : m_hosts)
	{
		if (host.hostIP == ip)
		{
			return;
		}

		(*hostIndex)++;
	}
}

int ServiceTableModel::hostsCount() const
{
	return TO_INT(m_hosts.size());
}

int ServiceTableModel::serviceCount() const
{
	return TO_INT(servicesInfo.size() - 1);
}

int ServiceTableModel::serviceColumn(quint16 port) const
{
	auto it = m_serviceColumn.find(port);

	if (it == m_serviceColumn.end())
	{
		Q_ASSERT(false);
		return -1;
	}

	return it->second;
}
