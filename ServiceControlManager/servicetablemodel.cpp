#include "servicetablemodel.h"
#include <QBrush>
#include <QDebug>
#include "../include/UdpSocket.h"
#include "../include/SocketIO.h"
#include <QSettings>
#include <QWidget>
#include <QApplication>
#include <QBuffer>

serviceTypeInfo serviceTypesInfo[SERVICE_TYPE_COUNT] =
{
    {RQSTP_CONFIG, PORT_CONFIG_SERRVICE, "Configuration Service"},
    {RQSTP_FSC_AQUISION, PORT_FCS_AQUISION_SERVICE, "FSC Data Acquisition Service"},
    {RQSTP_FSC_TUNING, PORT_FCS_TUNING_SERVICE, "FSC Tuning Service"},
    {RQSTP_ARCHIVING, PORT_ARCHIVING_SERVICE, "Data Archiving Service"},
};

serviceInfo::serviceInfo() :
    serviceType(-1),
    majorVersion(0),
    minorVersion(0),
    buildNo(0),
    CRC(0),
    serviceTime(0),
    state(SS_MF_UNDEFINED),
    serviceFunctionTime(0),
    clientSocket(nullptr),
    statusWidget(nullptr)
{
}

ServiceTableModel::ServiceTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    freezeUpdate(false)
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
                delete hostsInfo[i].servicesInfo[j].clientSocket;
            }
            if (hostsInfo[i].servicesInfo[j].statusWidget != nullptr)
            {
                delete hostsInfo[i].servicesInfo[j].statusWidget;
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
        {
            QString str;
            const serviceInfo& si = hostsInfo[row].servicesInfo[col];
            bool serviceFound = false;

            for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
            {
                if (serviceTypesInfo[i].serviceType == si.serviceType)
                {
                    str = serviceTypesInfo[i].name;
                    serviceFound = true;
                    break;
                }
            }
            if (serviceFound)
            {
                str += QString(" v%1.%2.%3(0x%4)\n").arg(si.majorVersion).arg(si.minorVersion).arg(si.buildNo).arg(si.CRC, 0, 16, QChar('0'));
            }
            if (si.state != SS_MF_UNDEFINED && si.state != SS_MF_UNAVAILABLE)
            {
                quint32 time = si.serviceTime;
                int s = time % 60; time /= 60;
                int m = time % 60; time /= 60;
                int h = time % 24; time /= 24;
                str += tr("Uptime") + QString(" (%1d %2:%3:%4)\n").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
            }
            switch(si.state)
            {
                case SS_MF_WORK:
                {
                    quint32 time = si.serviceFunctionTime;
                    int s = time % 60; time /= 60;
                    int m = time % 60; time /= 60;
                    int h = time % 24; time /= 24;
                    str += tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
                } break;
                case SS_MF_STOPPED: str += tr("Stopped"); break;
                case SS_MF_UNAVAILABLE: str += tr("Unavailable"); break;
                case SS_MF_UNDEFINED: str += tr("Undefined"); break;
                case SS_MF_STARTS: str += tr("Starts"); break;
                case SS_MF_STOPS: str += tr("Stops"); break;
                default: str += tr("Unknown state"); break;
            }
            return str;
        }
        break;
    case Qt::BackgroundRole:
        switch(hostsInfo[row].servicesInfo[col].state)
        {
        case SS_MF_WORK: return QBrush(QColor(0x7f,0xff,0x7f));
        case SS_MF_STARTS:
        case SS_MF_STOPS:
        case SS_MF_STOPPED:
            return QBrush(QColor(0xff,0xff,0x7f));
        case SS_MF_UNAVAILABLE: return QBrush(Qt::lightGray);
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
            if (hi.servicesInfo[portIndex].state != state)
            {
                hi.servicesInfo[portIndex].state = state;
                emit serviceStateChanged(i);
            }
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

QPair<int,int> ServiceTableModel::getServiceState(quint32 ip, quint16 port)
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
        return QPair<int,int>(-1, -1);
    }
    for (int i = 0; i < hostsInfo.count(); i++)
    {
        if (hostsInfo[i].ip == ip)
        {
            return QPair<int,int>(i, portIndex);
        }
    }
    return QPair<int, int>(-1, -1);
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
        socket->sendRequest(RQID_GET_SERVICE_INFO, nullptr, 0);
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
    case RQID_GET_SERVICE_INFO:
    {
        if (data.count() != 32)
        {
            return;
        }
        quint32 ip = socket->serverAddress().toIPv4Address();
        QPair<int, int> place = getServiceState(ip, socket->port());
        if (place.first == -1 || place.first >= hostsInfo.count() || place.second == -1 || place.second >= SERVICE_TYPE_COUNT)
        {
            return;
        }
        serviceInfo& si = hostsInfo[place.first].servicesInfo[place.second];
        QBuffer buffer(&data);
        buffer.open(QBuffer::ReadOnly);
        QDataStream in(&buffer);
        in >> si.serviceType;
        in >> si.majorVersion;
        in >> si.minorVersion;
        in >> si.buildNo;
        in >> si.CRC;

        in >> si.serviceTime;
        quint32 state;
        in >> state;
        if (state != si.state)
        {
            si.state = state;
            emit serviceStateChanged(place.first);
        }
        in >> si.serviceFunctionTime;
        QModelIndex changedIndex = index(place.first, place.second);
        emit dataChanged(changedIndex, changedIndex);

        checkForDeletingSocket(socket);
    }
    case RQID_SERVICE_MF_START:
    case RQID_SERVICE_MF_STOP:
    case RQID_SERVICE_MF_RESTART:
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
            setServiceState(socket->serverAddress().toIPv4Address(), socket->port(), SS_MF_UNAVAILABLE);
            checkForDeletingSocket(socket);
            return;
        }
    }
}

void ServiceTableModel::checkServiceStates()
{
    if (freezeUpdate)
    {
        return;
    }
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
            while (clientSocket->isWaitingForAck())
            {
                qApp->processEvents();
            }
            clientSocket->sendRequest(RQID_GET_SERVICE_INFO, nullptr, 0);
        }
    }
}

void ServiceTableModel::sendCommand(int row, int col, int command)
{
    UdpClientSocket* clientSocket = hostsInfo[row].servicesInfo[col].clientSocket;
    int state = hostsInfo[row].servicesInfo[col].state;
    if (clientSocket == nullptr)
    {
        return;
    }
    if (!(state == SS_MF_WORK && (command == RQID_SERVICE_MF_STOP || command == RQID_SERVICE_MF_RESTART)) &&
            !(state == SS_MF_STOPPED && command == RQID_SERVICE_MF_START))
    {
        return;
    }
    freezeUpdate = true;
    while (clientSocket->isWaitingForAck())
    {
        qApp->processEvents();
    }
    clientSocket->sendRequest(command, nullptr, 0);
    freezeUpdate = false;
}
