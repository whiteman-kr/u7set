#include "servicetablemodel.h"
#include <QBrush>
#include <QDebug>
#include "../include/UdpSocket.h"
#include "../include/SocketIO.h"
#include <QSettings>
#include <QWidget>
#include <QApplication>
#include <QBuffer>


ServiceData::ServiceData() :
    clientSocket(nullptr),
    statusWidget(nullptr)
{
    information.type = -1;
    information.mainFunctionState = SS_MF_UNDEFINED;
}


ServiceTableModel::ServiceTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_freezeUpdate(false)
{
    QSettings settings;
    int size = settings.beginReadArray("server list");
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        HostInfo hi;
        hi.ip = settings.value("IP").toUInt();
        m_hostsInfo.append(hi);
    }
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkServiceStates()));
    timer->start(500);
}

ServiceTableModel::~ServiceTableModel()
{
    QSettings settings;
    settings.beginWriteArray("server list", m_hostsInfo.count());
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("IP", m_hostsInfo[i].ip);
        for (int j = 0; j < RQSTP_COUNT; j++)
        {
            if (m_hostsInfo[i].servicesData[j].clientSocket != nullptr)
            {
                delete m_hostsInfo[i].servicesData[j].clientSocket;
            }
            if (m_hostsInfo[i].servicesData[j].statusWidget != nullptr)
            {
                delete m_hostsInfo[i].servicesData[j].statusWidget;
            }
        }
    }
    settings.endArray();
}

int ServiceTableModel::rowCount(const QModelIndex&) const
{
    return m_hostsInfo.count();
}

int ServiceTableModel::columnCount(const QModelIndex&) const
{
    return RQSTP_COUNT;
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    const ServiceInformation& si = m_hostsInfo[row].servicesData[col].information;
    switch(role)
    {
    case Qt::DisplayRole:
        {
            QString str;
            bool serviceFound = false;

            for (int i = 0; i < RQSTP_COUNT; i++)
            {
                if (serviceTypesInfo[i].serviceType == si.type)
                {
                    str = serviceTypesInfo[i].name;
                    serviceFound = true;
                    break;
                }
            }
            if (serviceFound)
            {
                str += QString(" v%1.%2.%3(0x%4)\n").arg(si.majorVersion).arg(si.minorVersion).arg(si.buildNo).arg(si.crc, 0, 16, QChar('0'));
            }
            if (si.mainFunctionState != SS_MF_UNDEFINED && si.mainFunctionState != SS_MF_UNAVAILABLE)
            {
                quint32 time = si.uptime;
                int s = time % 60; time /= 60;
                int m = time % 60; time /= 60;
                int h = time % 24; time /= 24;
                str += tr("Uptime") + QString(" (%1d %2:%3:%4)\n").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
            }
            switch(si.mainFunctionState)
            {
                case SS_MF_WORK:
                {
                    quint32 time = si.mainFunctionUptime;
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
        switch(si.mainFunctionState)
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
            return QHostAddress(m_hostsInfo[section].ip).toString();
        }
    }
    return QVariant();
}

void ServiceTableModel::setServiceState(quint32 ip, quint16 port, int state)
{
    int portIndex = -1;
    for (int i = 0; i < RQSTP_COUNT; i++)
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
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        if (m_hostsInfo[i].ip == ip)
        {
            ServiceInformation& si = m_hostsInfo[i].servicesData[portIndex].information;
            if (si.mainFunctionState != state)
            {
                si.mainFunctionState = state;
                emit serviceStateChanged(i);
            }
            QModelIndex changedIndex = index(i, portIndex);
            emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::DisplayRole);
            return;
        }
    }
    HostInfo hi;
    hi.ip = ip;
    hi.servicesData[portIndex].information.mainFunctionState = state;
    beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
    m_hostsInfo.append(hi);
    endInsertRows();
    emit serviceStateChanged(m_hostsInfo.count() - 1);
}

QPair<int,int> ServiceTableModel::getServiceState(quint32 ip, quint16 port)
{
    int portIndex = -1;
    for (int i = 0; i < RQSTP_COUNT; i++)
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
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        if (m_hostsInfo[i].ip == ip)
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
    for (int i = 0; i < RQSTP_COUNT; i++)
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
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        if (m_hostsInfo[i].ip != socket->serverAddress().toIPv4Address())
        {
            continue;
        }
        UdpClientSocket** clientSocket = &m_hostsInfo[i].servicesData[portIndex].clientSocket;
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
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        if (m_hostsInfo[i].ip == ip)
        {
            return;
        }
    }
    for (int i = 0; i < RQSTP_COUNT; i++)
    {
        UdpClientSocket* socket = new UdpClientSocket(QHostAddress(connectionAddress), serviceTypesInfo[i].port);
        connect(socket, SIGNAL(ackTimeout()), this, SLOT(serviceNotFound()));
        connect(socket, SIGNAL(ackReceived(RequestHeader,QByteArray)), this, SLOT(serviceAckReceived(RequestHeader,QByteArray)));
        socket->sendRequest(RQID_GET_SERVICE_INFO, nullptr, 0);
    }
}

void ServiceTableModel::addAddress(QString connectionAddress)
{
    QHostAddress ha(connectionAddress);
    quint32 ip = ha.toIPv4Address();
    if (ha.protocol() != QAbstractSocket::IPv4Protocol)
    {
        return;
    }
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        if (m_hostsInfo[i].ip == ip)
        {
            return;
        }
    }
    HostInfo hi;
    hi.ip = ip;
    beginInsertRows(QModelIndex(), m_hostsInfo.count(), m_hostsInfo.count());
    m_hostsInfo.append(hi);
    endInsertRows();
    emit serviceStateChanged(m_hostsInfo.count() - 1);
}

void ServiceTableModel::serviceAckReceived(RequestHeader header, QByteArray data)
{
    UdpClientSocket* socket = dynamic_cast<UdpClientSocket*>(sender());
    if (socket == nullptr)
    {
        return;
    }
    switch (header.id)
    {
    case RQID_GET_SERVICE_INFO:
    {
        if (data.count() != 32)
        {
            return;
        }
        quint32 ip = socket->serverAddress().toIPv4Address();
        QPair<int, int> place = getServiceState(ip, socket->port());
        if (place.first == -1 || place.first >= m_hostsInfo.count() || place.second == -1 || place.second >= RQSTP_COUNT)
        {
            return;
        }
        ServiceInformation& info = m_hostsInfo[place.first].servicesData[place.second].information;
        ServiceInformation& newInfo = *(ServiceInformation*)data.constData();
        if (info.mainFunctionState != newInfo.mainFunctionState)
        {
            info = newInfo;
            emit serviceStateChanged(place.first);
        }
        else
        {
            info = newInfo;
        }
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
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        if (m_hostsInfo[i].ip == ip)
        {
            setServiceState(socket->serverAddress().toIPv4Address(), socket->port(), SS_MF_UNAVAILABLE);
            checkForDeletingSocket(socket);
            return;
        }
    }
}

void ServiceTableModel::checkServiceStates()
{
    if (m_freezeUpdate)
    {
        return;
    }
    for (int i = 0; i < m_hostsInfo.count(); i++)
    {
        for (int j = 0; j < RQSTP_COUNT; j++)
        {
            UdpClientSocket* clientSocket = m_hostsInfo[i].servicesData[j].clientSocket;
            if (clientSocket == nullptr)
            {
                clientSocket = new UdpClientSocket(QHostAddress(m_hostsInfo[i].ip), serviceTypesInfo[j].port);
                connect(clientSocket, SIGNAL(ackTimeout()), this, SLOT(serviceNotFound()));
                connect(clientSocket, SIGNAL(ackReceived(RequestHeader,QByteArray)), this, SLOT(serviceAckReceived(RequestHeader,QByteArray)));
                m_hostsInfo[i].servicesData[j].clientSocket = clientSocket;
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
    UdpClientSocket* clientSocket = m_hostsInfo[row].servicesData[col].clientSocket;
    int state = m_hostsInfo[row].servicesData[col].information.mainFunctionState;
    if (clientSocket == nullptr)
    {
        return;
    }
    if (!(state == SS_MF_WORK && (command == RQID_SERVICE_MF_STOP || command == RQID_SERVICE_MF_RESTART)) &&
            !(state == SS_MF_STOPPED && command == RQID_SERVICE_MF_START || command == RQID_SERVICE_MF_RESTART))
    {
        return;
    }
    m_freezeUpdate = true;
    while (clientSocket->isWaitingForAck())
    {
        qApp->processEvents();
    }
    clientSocket->sendRequest(command, nullptr, 0);
    m_freezeUpdate = false;
}

void ServiceTableModel::removeHost(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    for (int j = 0; j < RQSTP_COUNT; j++)
    {
        if (m_hostsInfo[row].servicesData[j].clientSocket != nullptr)
        {
            delete m_hostsInfo[row].servicesData[j].clientSocket;
        }
        if (m_hostsInfo[row].servicesData[j].statusWidget != nullptr)
        {
            delete m_hostsInfo[row].servicesData[j].statusWidget;
        }
    }
    m_hostsInfo.removeAt(row);
    endRemoveRows();
}
