#ifndef SERVICETABLEMODEL_H
#define SERVICETABLEMODEL_H

#include <QAbstractTableModel>
#include <QHostAddress>
#include "../include/SocketIO.h"

const quint32   SS_MF_UNDEFINED = 10,
                SS_MF_UNAVAILABLE = 11;

class UdpClientSocket;

struct ServiceData
{
    ServiceInformation information;

    UdpClientSocket* clientSocket;
    QWidget* statusWidget;

    ServiceData();
};

struct HostInfo
{
    quint32 ip;
    ServiceData servicesData[RQSTP_COUNT];

    HostInfo() : ip(0) {}
};

class ServiceTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ServiceTableModel(QObject *parent = 0);
    ~ServiceTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void checkAddress(QString connectionAddress);
    void addAddress(QString connectionAddress);

signals:
    void serviceStateChanged(int row);

public slots:
    void serviceAckReceived(RequestHeader header, QByteArray data);
    void serviceNotFound();
    void checkServiceStates();

    void sendCommand(int row, int col, int command);
    void removeHost(int row);

private:
    QVector<HostInfo> m_hostsInfo;
    bool m_freezeUpdate;

    void setServiceState(quint32 ip, quint16 port, int state);
    QPair<int,int> getServiceState(quint32 ip, quint16 port);
    void checkForDeletingSocket(UdpClientSocket* socket);
};

#endif // SERVICETABLEMODEL_H
