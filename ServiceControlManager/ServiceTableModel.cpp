#include "ServiceTableModel.h"
#include <QBrush>
#include <QDebug>
#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/SocketIO.h"
#include <QSettings>
#include <QWidget>
#include <QApplication>
#include <QBuffer>
#include "AppDataServiceWidget.h"
#include "ConfigurationServiceWidget.h"
#include "TuningServiceWidget.h"

HostInfo::HostInfo() : ip(0)
{
	servicesData.reserve(servicesInfo.size());

	for (const ServiceInfo& si : servicesInfo)
	{
		ServiceData sd;

		sd.information.mutable_softwareinfo()->set_softwaretype(si.softwareType);
		sd.type = si.softwareType;

		servicesData.push_back(sd);
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
	settings.beginWriteArray("ServiceTableModel/ServerList", hostsInfoCount());

	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("IP", m_hostsInfo[i].ip);

		for (int j = 0; j < servicesInfo.size(); j++)
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
		for (int j = 0; j < servicesInfo.size(); j++)
		{
			UdpClientSocket*& clientSocket = m_hostsInfo[i].servicesData[j].clientSocket;

			assert(clientSocket == nullptr);

			clientSocket = new UdpClientSocket(QHostAddress(m_hostsInfo[i].ip), servicesInfo[j].port);

			connect(clientSocket, &UdpClientSocket::ackTimeout, this, &ServiceTableModel::serviceNotFound);
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

	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		for (int j = 0; j < servicesInfo.size(); j++)
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
	return hostsInfoCount();
}

int ServiceTableModel::columnCount(const QModelIndex&) const
{
	return static_cast<int>(servicesInfo.size());
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();

	const ServiceData& service = m_hostsInfo[row].servicesData[col];
	const Network::ServiceInfo& si = service.information;

	ServiceState serviceState = static_cast<ServiceState>(si.servicestate());

	switch(role)
	{
		case Qt::DisplayRole:
		{
			QString str;
			bool serviceFound = false;

			for (int i = 0; i < servicesInfo.size(); i++)
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
				str += QString(" v%1.%2.%3\n").arg(si.softwareinfo().majorversion()).
												arg(si.softwareinfo().minorversion()).
												arg(si.softwareinfo().patchversion());
			}

			if (serviceState != ServiceState::Undefined &&
				serviceState != ServiceState::Unavailable)
			{
				str += QString("Uptime %1\n").arg(formatUptime(si.uptime()));
			}
			switch(serviceState)
			{
				case ServiceState::Work:
				{
					qint64 runtime = si.serviceruntime();
					str += tr("Running in ") + E::valueToString(service.sessionParams.softwareRunMode) + " mode " + formatUptime(runtime);
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
				str += tr("\nListening clients on %1:%2").arg(QHostAddress(service.clientRequestIp).toString()).arg(service.clientRequestPort);
			}
			return str;
		}
			break;
		case Qt::BackgroundRole:
			switch(serviceState)
			{
				case ServiceState::Work:
					return QBrush((service.sessionParams.softwareRunMode == E::SoftwareRunMode::Normal) ? QColor(0x7f,0xff,0x7f) : QColor(0x7f,0x7f,0xff)) ;
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

	for (int i = 0; i < servicesInfo.size(); i++)
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
	beginInsertRows(QModelIndex(), hostsInfoCount(), hostsInfoCount());
	m_hostsInfo.append(hi);

	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(hostsInfoCount() - 1);
}

void ServiceTableModel::getServiceState(quint32 ip, quint16 port, int& hostIndex, int& serviceIndex)
{
	serviceIndex = -1;
	hostIndex = -1;

	for (int i = 0; i < servicesInfo.size(); i++)
	{
		if (servicesInfo[i].port == port)
		{
			serviceIndex = i;
			break;
		}
	}
	if (serviceIndex == -1)
	{
		return;
	}
	for (int i = 0; i < m_hostsInfo.count(); i++)
	{
		if (m_hostsInfo[i].ip == ip)
		{
			hostIndex = i;
		}
	}
}

int ServiceTableModel::hostsInfoCount() const
{
	return static_cast<int>(m_hostsInfo.count());
}

void ServiceData::parseServiceInfo()
{
	sessionParams.loadFrom(information.sessionparams());
	QString settingsXml = QString::fromStdString(information.settingsxml());

	if (settings == nullptr)
	{
		settings = SoftwareSettingsSet::createAppropriateSettings(type);
	}
	SoftwareSettings* pSettings = settings.get();
	SoftwareSettingsSet::readSettingsFromXmlString(settingsXml, settings.get());

	switch (type)
	{
	case E::SoftwareType::ConfigurationService:
	{
		CfgServiceSettings* cfgServiceSettings = dynamic_cast<CfgServiceSettings*>(pSettings);
		Q_ASSERT(cfgServiceSettings);
		clientRequestIp = cfgServiceSettings->clientRequestIP.address32IfSet();
		clientRequestPort = cfgServiceSettings->clientRequestIP.portIfSet();
	}
		break;
	case E::SoftwareType::AppDataService:
	{
		AppDataServiceSettings* adsSettings = dynamic_cast<AppDataServiceSettings*>(pSettings);

		if (adsSettings != nullptr)
		{
			if (adsSettings->rcSettings.empty() == false)
			{

				clientRequestIp = adsSettings->rcSettings[0].clientRequestIP.address32IfSet();
				clientRequestPort = adsSettings->rcSettings[0].clientRequestIP.portIfSet();
			}
		}
		else
		{
			Q_ASSERT(adsSettings);
		}
	}
		break;
	case E::SoftwareType::DiagDataService:
	{
		DiagDataServiceSettings* diagDataServiceSettings = dynamic_cast<DiagDataServiceSettings*>(pSettings);
		Q_ASSERT(diagDataServiceSettings);
		clientRequestIp = diagDataServiceSettings->clientRequestIP.address32IfSet();
		clientRequestPort = diagDataServiceSettings->clientRequestIP.portIfSet();
	}
		break;
	case E::SoftwareType::TuningService:
	{
		TuningServiceSettings* tuningDataServiceSettings = dynamic_cast<TuningServiceSettings*>(pSettings);
		Q_ASSERT(tuningDataServiceSettings);

		// TO DO 2ch tuning!
		//
		clientRequestIp = tuningDataServiceSettings->clientRequestIP.address32IfSet();
		clientRequestPort = tuningDataServiceSettings->clientRequestIP.portIfSet();
	}
		break;
	case E::SoftwareType::ArchiveService:
	{
        ArchivingServiceSettings* archivingServiceSettings = dynamic_cast<ArchivingServiceSettings*>(pSettings);
        Q_ASSERT(archivingServiceSettings);
		clientRequestIp = archivingServiceSettings->clientRequestIP.address32IfSet();
		clientRequestPort = archivingServiceSettings->clientRequestIP.portIfSet();
	}
		break;
	default:
		Q_ASSERT(false);
	}
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
	beginInsertRows(QModelIndex(), hostsInfoCount(), hostsInfoCount());
	m_hostsInfo.append(hi);
	endInsertRows();

	restartUdpSocketThread();

	emit serviceStateChanged(hostsInfoCount() - 1);
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
			int hostIndex = -1;
			int serviceIndex = -1;
			getServiceState(ip, socket->port(), hostIndex, serviceIndex);

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

				HostInfo hi;
				hi.ip = sa.toIPv4Address();
				ServiceData& service = hi.servicesData[serviceIndex];
				service.information = newServiceInfo;

				service.parseServiceInfo();

				beginInsertRows(QModelIndex(), hostsInfoCount(), hostsInfoCount());

				m_hostsInfo.append(hi);

				endInsertRows();

				restartUdpSocketThread();

				return;
			}

			ServiceData& service = m_hostsInfo[hostIndex].servicesData[serviceIndex];
			Network::ServiceInfo& info = service.information;

			if (info.servicestate() != newServiceInfo.servicestate())
			{
				info = newServiceInfo;
				emit serviceStateChanged(hostIndex);
			}
			else
			{
				info = newServiceInfo;
			}
			service.parseServiceInfo();
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
		for (int j = 0; j < servicesInfo.size(); j++)
		{
			UdpClientSocket* clientSocket = m_hostsInfo[i].servicesData[j].clientSocket;

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

	for (int j = 0; j < servicesInfo.size(); j++)
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
			serviceData.statusWidget = new AppDataServiceWidget(m_softwareInfo, serviceData, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
			break;

		case E::SoftwareType::ConfigurationService:
			serviceData.statusWidget = new ConfigurationServiceWidget(m_softwareInfo, serviceData, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
			break;

		case E::SoftwareType::TuningService:
			serviceData.statusWidget = new TuningServiceWidget(m_softwareInfo, serviceData, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
			break;

		default:
			serviceData.statusWidget = new BaseServiceStateWidget(m_softwareInfo, serviceData, m_hostsInfo[index.row()].ip, udpPort, m_parrentWidget);
		}
	}

	serviceData.statusWidget->showNormal();
	serviceData.statusWidget->raise();
	serviceData.statusWidget->activateWindow();
}

void ServiceTableModel::setServiceInformation(quint32 ip, quint16 port, Network::ServiceInfo sInfo)
{
	int hostIndex = -1;
	int serviceIndex = -1;
	getServiceState(ip, port, hostIndex, serviceIndex);

	if (hostIndex >= m_hostsInfo.count() || serviceIndex == -1 || serviceIndex >= servicesInfo.size())
	{
		return;
	}

	if (hostIndex == -1)
	{
		HostInfo hi;
		hi.ip = ip;
		hi.servicesData[serviceIndex].information = sInfo;
		beginInsertRows(QModelIndex(), hostsInfoCount(), hostsInfoCount());
		m_hostsInfo.append(hi);
		endInsertRows();

		restartUdpSocketThread();
	}
	else
	{
		Network::ServiceInfo& info = m_hostsInfo[hostIndex].servicesData[serviceIndex].information;

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
