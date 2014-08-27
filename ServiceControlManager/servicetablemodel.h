#ifndef SERVICETABLEMODEL_H
#define SERVICETABLEMODEL_H

#include <QAbstractTableModel>
#include <QHostAddress>

const int SERVICE_TYPE_COUNT = 4;

struct serviceTypeInfo
{
    quint16 port;
    char* name;
};

struct serviceInfo
{
    bool active;
    QWidget* statusWidget;

    serviceInfo() : active(false), statusWidget(nullptr) {}
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

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void setActive(quint32 ip, quint16 port, bool active);

signals:

public slots:

private:
    QVector<hostInfo> hostsInfo;
};

#endif // SERVICETABLEMODEL_H
