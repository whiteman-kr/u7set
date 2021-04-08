#ifndef SERVICETABLEMODEL_H
#define SERVICETABLEMODEL_H

#include <QAbstractTableModel>
#include <QHostAddress>
#include "../OnlineLib/UdpSocket.h"
#include "../ServiceLib/Service.h"
#include "../lib/Types.h"
#include "../OnlineLib/Tcp.h"


// For QueuedConnection (scan network)
Q_DECLARE_METATYPE(Network::ServiceInfo)


class UdpClientSocket;

struct ServiceData
{
	Network::ServiceInfo information;

	quint32 clientRequestIp;
	quint16 clientRequestPort;
	SessionParams sessionParams;
	std::shared_ptr<SoftwareSettings> settings;
	E::SoftwareType type = E::SoftwareType::Unknown;

	UdpClientSocket* clientSocket = nullptr;
	QWidget* statusWidget = nullptr;

	ServiceData();
	void parseServiceInfo();
};

struct HostInfo
{
	QVector<ServiceData> servicesData;
	quint32 ip = 0;

	HostInfo();
};

class ServiceTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit ServiceTableModel(const SoftwareInfo& softwareInfo, QWidget* parent = 0);
	~ServiceTableModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void addAddress(QString connectionAddress);

signals:
	void serviceStateChanged(int row);

public slots:
	void serviceAckReceived(const UdpRequest udpRequest);
	void serviceNotFound();
	void checkServiceStates();

	void removeHost(int row);
	void openServiceStatusWidget(const QModelIndex& index);

	void setServiceInformation(quint32 ip, quint16 port, Network::ServiceInfo sInfo);

private:
	QVector<HostInfo> m_hostsInfo;
	SoftwareInfo m_softwareInfo;
	bool m_freezeUpdate;

	QWidget* m_parrentWidget = nullptr;

	QTimer m_timer;

	UdpSocketThread* m_socketThread = nullptr;

	void startUdpSocketThread();
	void finishtUdpSocketThread();
	void restartUdpSocketThread();

	void setServiceState(quint32 ip, quint16 port, ServiceState state);
	void getServiceState(quint32 ip, quint16 port, int& hostIndex, int& portIndex);
	void checkForDeletingSocket(UdpClientSocket* socket);
};

#endif // SERVICETABLEMODEL_H
