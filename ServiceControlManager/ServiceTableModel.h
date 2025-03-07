#pragma once

#include <QAbstractTableModel>

#include "ServiceData.h"

// For QueuedConnection (scan network)
//
Q_DECLARE_METATYPE(Network::ServiceInfo)

struct Host
{
	quint32 hostIP = 0;
	std::vector<ServiceData> servicesData;

	Host();

	int availableServicesCount();
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

	void addAddress(const QString& connectionAddress);

signals:
	void serviceStateChanged(int row);

public slots:
	void serviceAckReceived(const UdpRequest udpRequest);
	void serviceNotAck();
	void checkServiceStates();
	void removeHost(int row);
	void setServiceInformation(quint32 ip, quint16 port, Network::ServiceInfo sInfo);
	void openServiceStatusWidget(const QModelIndex& index);

private:
	void startUdpSocketThread();
	void finishtUdpSocketThread();
	void restartUdpSocketThread();

	void setServiceState(quint32 ip, quint16 port, E::ServiceState state);
	void getServiceState(quint32 ip, quint16 port, int& hostIndex, int& serviceIndex);

	int hostsCount() const;
	int serviceCount() const;
	int serviceColumn(quint16 port) const;

private:
	std::vector<Host> m_hosts;

	SoftwareInfo m_softwareInfo;

	std::map<quint16, int> m_serviceColumn;

	QWidget* m_parentWidget = nullptr;

	QTimer m_timer;

	UdpSocketThread* m_socketThread = nullptr;
};
