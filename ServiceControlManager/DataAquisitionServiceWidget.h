#ifndef DATAAQUISITIONSERVICEWIDGET_H
#define DATAAQUISITIONSERVICEWIDGET_H

#include "BaseServiceStateWidget.h"
#include "../include/OrderedHash.h"
#include <QAbstractTableModel>
#include "../include/DataSource.h"

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
	void invalidateData();
	void parseData(UdpRequest udpRequest);

signals:
	void dataClientSendRequest(const UdpRequest& request);
	void changedSourceInfo();
	void changedSourceState();

private:
	UdpClientSocket* m_clientSocket;
	UdpSocketThread m_clientSocketThread;
	bool m_active = false;
	int m_currentDataRequestType = RQID_GET_DATA_SOURCES_IDS;
	int m_currentDataSourceIndex = -1;
	PtrOrderedHash<int, AppDataSource> m_dataSource;
	QTimer* m_periodicTimer;

	void sendDataRequest(int requestType);
};

class DataAquisitionServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	DataAquisitionServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);

public slots:
	void updateSourceInfo();
	void updateSourceState();
	void checkVisibility();

private:
	DataSourcesStateModel* m_model = nullptr;
	QTableView* m_view = nullptr;
};

#endif // DATAAQUISITIONSERVICEWIDGET_H
