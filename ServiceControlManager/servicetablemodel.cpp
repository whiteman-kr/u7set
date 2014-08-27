#include "servicetablemodel.h"
#include <QBrush>

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
    setActive(0x7f000001, 4510, true);
    setActive(0x7f000001, 4530, true);
    setActive(0xc0a80001, 4520, true);
    setActive(0xc0a80002, 4540, true);
    setHeaderData(0, Qt::Horizontal, tr("IP Address"));
    for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
    {
        setHeaderData(i + 1, Qt::Horizontal, QString(serviceTypesInfo[i].name));
    }
}

int ServiceTableModel::rowCount(const QModelIndex &parent) const
{
    return hostsInfo.count();
}

int ServiceTableModel::columnCount(const QModelIndex &parent) const
{
    return SERVICE_TYPE_COUNT + 1;
}

QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    switch(role)
    {
    case Qt::DisplayRole:
        if (col == 0)
        {
            return QHostAddress(hostsInfo[index.row()].ip).toString();
        }
        return hostsInfo[row].servicesInfo[col - 1].active ? tr("Yes") : tr("No");
        break;
    case Qt::BackgroundRole:
        if (col > 0)  //change background only for cell(1,2)
        {
            if (hostsInfo[row].servicesInfo[col - 1].active)
            {
                return QBrush(Qt::green);
            }
            else
            {
                return QBrush(Qt::red);
            }
        }
        return QVariant();
        break;
    default:
        return QVariant();
    }
}

void ServiceTableModel::setActive(quint32 ip, quint16 port, bool active)
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
        if (hostsInfo[i].ip != ip)
        {
            continue;
        }
        hostInfo& hi = hostsInfo[i];
        hi.servicesInfo[portIndex].active = active;
    }
    hostInfo hi;
    hi.ip = ip;
    hi.servicesInfo[portIndex].active = active;
    hostsInfo.append(hi);
}
