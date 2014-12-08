#ifndef DATASOURCESSTATEWIDGET_H
#define DATASOURCESSTATEWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QHostAddress>
#include "../include/UdpSocket.h"
#include "../include/OrderedHash.h"


class QTimer;


struct DataSourceDescription
{
	DataSourceInfo info;
	DataSourceState state;
};


class DataSourcesStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit DataSourcesStateModel(QHostAddress ip, quint16 port, QObject *parent = 0);
	~DataSourcesStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	bool isActive() { return m_active; }
	void setActive(bool active);

public slots:
	void onTimer();
	void ackTimeout();
	void ackReceived(UdpRequest udpRequest);

private:
	UdpClientSocket* m_clientSocket = nullptr;
	bool m_active = false;
	int m_currentRequestType = RQID_GET_DATA_SOURCES_IDS;
	int m_currentDataSourceIndex = -1;
	OrderedHash<int, DataSourceDescription> m_dataSourceDescription;
};


class DataSourcesStateWidget : public QWidget
{
	Q_OBJECT
public:
	explicit DataSourcesStateWidget(quint32 ip, int portIndex, QWidget *parent = 0);
	~DataSourcesStateWidget();

signals:

public slots:
	void checkVisibility();

private:
	QTimer* m_timer = nullptr;
	DataSourcesStateModel* m_model = nullptr;
};


#endif // DATASOURCESSTATEWIDGET_H
