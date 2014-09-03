#ifndef SERVICETABLEMODEL_H
#define SERVICETABLEMODEL_H

#include <QAbstractTableModel>
#include <QHostAddress>
#include "../include/SocketIO.h"

const int SERVICE_TYPE_COUNT = 4;

/*const int SERVICE_STATE_UNDEFINED = 0,
    SERVICE_STATE_UNAVAILABLE = 1,
    SERVICE_STATE_STOPPED = 2,
    SERVICE_STATE_RUNNING = 3,
    SERVICE_STATE_COUNT = 4;*/

const quint32   SS_MF_UNDEFINED = 10,
                SS_MF_UNAVAILABLE = 11;

class UdpClientSocket;

struct serviceTypeInfo
{
    quint32 serviceType;
    quint16 port;
    char* name;
};

struct serviceInfo
{
    quint32 serviceType;
    quint32 majorVersion;
    quint32 minorVersion;
    quint32 buildNo;
    quint32 CRC;

    quint32 serviceTime;
    quint32 state;
    quint32 serviceFunctionTime;

    UdpClientSocket* clientSocket;
    QWidget* statusWidget;

    serviceInfo();
};

struct hostInfo
{
    quint32 ip;
    serviceInfo servicesInfo[SERVICE_TYPE_COUNT];

    hostInfo() : ip(0) {}
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

signals:
    void serviceStateChanged(int row);

public slots:
    void serviceAckReceived(REQUEST_HEADER header, QByteArray data);
    void serviceNotFound();
    void checkServiceStates();

    void sendCommand(int row, int col, int command);

private:
    QVector<hostInfo> hostsInfo;
    bool freezeUpdate;

    void setServiceState(quint32 ip, quint16 port, int state);
    QPair<int,int> getServiceState(quint32 ip, quint16 port);
    void checkForDeletingSocket(UdpClientSocket* socket);
};

#endif // SERVICETABLEMODEL_H
