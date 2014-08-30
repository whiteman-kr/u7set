#include "servicetablemodel.h"
#include <QBrush>
#include <QDebug>
#include "../include/UdpSocket.h"
#include "../include/SocketIO.h"
#include <QSettings>
#include <QWidget>

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
    QSettings settings;
    int size = settings.beginReadArray("server list");
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        hostInfo hi;
        hi.ip = settings.value("IP").toUInt();
        hostsInfo.append(hi);
    }
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkServiceStates()));
    timer->start(500);
}

ServiceTableModel::~ServiceTableModel()
{
    QSettings settings;
    settings.beginWriteArray("server list", hostsInfo.count());
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("IP", hostsInfo[i].ip);
        for (int j = 0; j < SERVICE_TYPE_COUNT; j++)
        {
            if (hostsInfo[i].servicesInfo[j].clientSocket != nullptr)
            {
                hostsInfo[i].servicesInfo[j].clientSocket->deleteLater();
            }
            if (hostsInfo[i].servicesInfo[j].statusWidget != nullptr)
            {
                hostsInfo[i].servicesInfo[j].statusWidget->deleteLater();
            }
        }
    }
    settings.endArray();
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
    for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
    {
        if (serviceTypesInfo[i].port == port)
        {
            portIndex = i;
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
    beginInsertRows(QModelIndex(), hostsInfo.count(), hostsInfo.count());
    hostsInfo.append(hi);
    endInsertRows();
}

void ServiceTableModel::checkForDeletingSocket(UdpClientSocket *socket)
{
    // Socket should be remembered or deleted, if we are scanning.
    int portIndex = -1;
    for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
    {
        if (serviceTypesInfo[i].port == socket->port())
        {
            portIndex = i;
            break;
        }
    }
    if (portIndex == -1)
    {
        return;
    }
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        if (hostsInfo[i].ip != socket->serverAddress().toIPv4Address())
        {
            continue;
        }
        UdpClientSocket** clientSocket = &hostsInfo[i].servicesInfo[portIndex].clientSocket;
        if (*clientSocket == nullptr)
        {
            *clientSocket = socket;
        }
        if (*clientSocket != socket)
        {
            socket->deleteLater();
        }
    }
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
        connect(socket, SIGNAL(ackTimeout()), this, SLOT(serviceNotFound()));
        connect(socket, SIGNAL(ackReceived(REQUEST_HEADER,QByteArray)), this, SLOT(serviceAckReceived(REQUEST_HEADER,QByteArray)));
        socket->sendRequest(RQID_GET_SERVICE_STATE, nullptr, 0);
    }
}

void ServiceTableModel::serviceAckReceived(REQUEST_HEADER header, QByteArray data)
{
    UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());
    if (socket == nullptr)
    {
        return;
    }
    switch (header.ID)
    {
    case RQID_GET_SERVICE_STATE:
    {
        if (data.count() < 8)
        {
            return;
        }
        quint64 time = *(quint64*)data.constData();
        quint64 ip = socket->serverAddress().toIPv4Address();
        setServiceState(ip, socket->port(), time == 0 ? SERVICE_STATE_STOPPED : SERVICE_STATE_RUNNING);

        checkForDeletingSocket(socket);
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
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        if (hostsInfo[i].ip == ip)
        {
            setServiceState(socket->serverAddress().toIPv4Address(), socket->port(), SERVICE_STATE_UNAVAILABLE);
            checkForDeletingSocket(socket);
            return;
        }
    }
}

void ServiceTableModel::checkServiceStates()
{
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        for (int j = 0; j < SERVICE_TYPE_COUNT; j++)
        {
            UdpClientSocket* clientSocket = hostsInfo[i].servicesInfo[j].clientSocket;
            if (clientSocket == nullptr)
            {
                clientSocket = new UdpClientSocket(QHostAddress(hostsInfo[i].ip), serviceTypesInfo[j].port);
                connect(clientSocket, SIGNAL(ackTimeout()), this, SLOT(serviceNotFound()));
                connect(clientSocket, SIGNAL(ackReceived(REQUEST_HEADER,QByteArray)), this, SLOT(serviceAckReceived(REQUEST_HEADER,QByteArray)));
                hostsInfo[i].servicesInfo[j].clientSocket = clientSocket;
            }
            clientSocket->sendRequest(RQID_GET_SERVICE_STATE, nullptr, 0);
        }
    }
}

void ServiceTableModel::sendStart()
{

}

void ServiceTableModel::sendStop()
{

}

void ServiceTableModel::sendRestart()
{

}
