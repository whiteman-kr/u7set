#ifndef SERVICETABLEMODEL_H
#define SERVICETABLEMODEL_H

#include <QAbstractTableModel>
#include <QHostAddress>
#include "../include/SocketIO.h"

const int SERVICE_TYPE_COUNT = 4;

const int SERVICE_STATE_UNDEFINED = 0,
    SERVICE_STATE_UNAVAILABLE = 1,
    SERVICE_STATE_STOPPED = 2,
    SERVICE_STATE_RUNNING = 3,
    SERVICE_STATE_COUNT = 4;

class UdpClientSocket;

struct serviceTypeInfo
{
    quint16 port;
    char* name;
};

struct serviceInfo
{
    int state;
    UdpClientSocket* clientSocket;
    QWidget* statusWidget;

    serviceInfo() : state(SERVICE_STATE_UNDEFINED), clientSocket(nullptr), statusWidget(nullptr) {}
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

public slots:
    void serviceAckReceived(REQUEST_HEADER header, QByteArray data);
    void serviceNotFound();
    void checkServiceStates();

    void sendStart();
    void sendStop();
    void sendRestart();

private:
    QVector<hostInfo> hostsInfo;

    void setServiceState(quint32 ip, quint16 port, int state);
    void checkForDeletingSocket(UdpClientSocket* socket);
};

#endif // SERVICETABLEMODEL_H
