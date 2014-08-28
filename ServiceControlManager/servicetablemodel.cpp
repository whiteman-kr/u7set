#include "servicetablemodel.h"
#include <QBrush>
#include <QDebug>
#include "../include/UdpSocket.h"
#include "../include/SocketIO.h"

serviceTypeInfo serviceTypesInfo[SERVICE_TYPE_COUNT] =
{
    {4510, "Configuration Service"},
    {4520, "FSC Data Acquisition Service"},
    {4530, "FSC Tuning Service"},
    {4540, "Data Archiving Service"},
};

ServiceTableModel::ServiceTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    /*setServiceState(0x7f000001, 4510, SERVICE_STATE_RUNNING);
    setServiceState(0x7f000001, 4530, SERVICE_STATE_RUNNING);
    setServiceState(0xc0a80001, 4520, SERVICE_STATE_RUNNING);
    setServiceState(0xc0a80002, 4540, SERVICE_STATE_RUNNING);*/
}

int ServiceTableModel::rowCount(const QModelIndex&) const
{
    return hostsInfo.count();
}

int ServiceTableModel::columnCount(const QModelIndex&) const
{
    return SERVICE_TYPE_COUNT;
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    switch(role)
    {
    case Qt::DisplayRole:
        switch(hostsInfo[row].servicesInfo[col].state)
        {
        case SERVICE_STATE_RUNNING: return tr("Running");
        case SERVICE_STATE_STOPPED: return tr("Stopped");
        case SERVICE_STATE_UNAVAILABLE: return tr("Unavailable");
        default: return QVariant();
        }
        break;
    case Qt::BackgroundRole:
        switch(hostsInfo[row].servicesInfo[col].state)
        {
        case SERVICE_STATE_RUNNING: return QBrush(Qt::green);
        case SERVICE_STATE_STOPPED: return QBrush(Qt::yellow);
        case SERVICE_STATE_UNAVAILABLE: return QBrush(Qt::lightGray);
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
            return tr(serviceTypesInfo[section].name);
        }
        if (orientation == Qt::Vertical)
        {
            return QHostAddress(hostsInfo[section].ip).toString();
        }
    }
    return QVariant();
}

void ServiceTableModel::setServiceState(quint32 ip, quint16 port, int state)
{
    int portIndex = -1;
    for (int j = 0; j < SERVICE_TYPE_COUNT; j++)
    {
        if (serviceTypesInfo[j].port == port)
        {
            portIndex = j;
            break;
        }
    }
    if (portIndex == -1)
    {
        return;
    }
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        if (hostsInfo[i].ip == ip)
        {
            hostInfo& hi = hostsInfo[i];
            hi.servicesInfo[portIndex].state = state;
            QModelIndex changedIndex = index(i, portIndex);
            emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::DisplayRole);
            return;
        }
    }
    hostInfo hi;
    hi.ip = ip;
    hi.servicesInfo[portIndex].state = state;
    hostsInfo.append(hi);
    QModelIndex changedIndex = index(hostsInfo.count() - 1, portIndex);
    emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::DisplayRole);
}

void ServiceTableModel::checkAddress(QString connectionAddress)
{
    QHostAddress ha(connectionAddress);
    quint32 ip = ha.toIPv4Address();
    if (ha.protocol() != QAbstractSocket::IPv4Protocol)
    {
        return;
    }
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        if (hostsInfo[i].ip == ip)
        {
            return;
        }
    }
    for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
    {
        UdpClientSocket* socket = new UdpClientSocket(QHostAddress(connectionAddress), serviceTypesInfo[i].port);
        connect(socket, SIGNAL(ackTimeout()), socket, SLOT(deleteLater()));
        connect(socket, SIGNAL(ackReceived(REQUEST_HEADER,QByteArray)), this, SLOT(serviceFound(REQUEST_HEADER,QByteArray)));
        socket->sendRequest(RQID_GET_SERVICE_STATE, nullptr, 0);
    }
}

void ServiceTableModel::serviceFound(REQUEST_HEADER header, QByteArray data)
{
    UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());
    if (socket == nullptr || header.ID != RQID_GET_SERVICE_STATE || data.count() < 8)
    {
        return;
    }
    quint64 time = *(quint64*)data.constData();
    setServiceState(socket->serverAddress().toIPv4Address(), socket->port(), time == 0 ? SERVICE_STATE_STOPPED : SERVICE_STATE_RUNNING);
    socket->deleteLater();
}
