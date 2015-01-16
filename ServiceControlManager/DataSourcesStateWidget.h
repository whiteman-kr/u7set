#ifndef DATASOURCESSTATEWIDGET_H
#define DATASOURCESSTATEWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QHostAddress>
#include "../include/UdpSocket.h"
#include "../include/OrderedHash.h"
#include "../include/DataSource.h"


class QTimer;
class QTableView;


class DataSourcesStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit DataSourcesStateModel(QHostAddress ip, QObject *parent = 0);
	~DataSourcesStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	bool isActive() { return m_active; }
	void setActive(bool active);

public slots:
	void onGetStateTimer();
	void ackTimeout();
	void ackReceived(UdpRequest udpRequest);

signals:
	void dataClientSendRequest(UdpRequest request);
	void changedSourceInfo();
	void changedSourceState();

private:
	UdpClientSocket* m_clientSocket = nullptr;
	UdpSocketThread m_clientSocketThread;
	bool m_active = false;
	int m_currentDataRequestType = RQID_GET_DATA_SOURCES_IDS;
	int m_currentDataSourceIndex = -1;
	OrderedHash<int, DataSource> m_dataSource;
	QTimer* m_periodicTimer;

	void sendDataRequest(int requestType);
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
	void updateSourceInfo();
	void updateSourceState();

private:
	QTimer* m_timer = nullptr;
	DataSourcesStateModel* m_model = nullptr;
	QTableView* m_view = nullptr;
};


#endif // DATASOURCESSTATEWIDGET_H
