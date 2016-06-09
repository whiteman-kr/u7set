#ifndef DATAAQUISITIONSERVICEWIDGET_H
#define DATAAQUISITIONSERVICEWIDGET_H

#include "BaseServiceStateWidget.h"
#include "../lib/OrderedHash.h"
#include <QAbstractTableModel>
#include "../lib/DataSource.h"

class QTableView;
class TcpAppDataClient;
class SimpleThread;

class DataSourcesStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit DataSourcesStateModel(TcpAppDataClient* clientSocket, QObject *parent = 0);
	~DataSourcesStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
	void invalidateData();
	void reloadList();
	void updateStateColumns();

private:
	TcpAppDataClient* m_clientSocket;
	QList<DataSource*> m_dataSource;
};

class SignalStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit SignalStateModel(TcpAppDataClient* clientSocket, QObject *parent = 0);
	~SignalStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
	void invalidateData();
	void reloadList();
	void updateStateColumns();

private:
	TcpAppDataClient* m_clientSocket;
};

class DataAquisitionServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	DataAquisitionServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);
	~DataAquisitionServiceWidget();

public slots:
	void updateSourceInfo();

	void updateSignalInfo();

	void checkVisibility();

private:
	DataSourcesStateModel* m_dataSourcesStateModel = nullptr;
	SignalStateModel* m_signalStateModel = nullptr;
	QTableView* m_dataSourcesView = nullptr;
	QTableView* m_signalsView = nullptr;

	TcpAppDataClient* m_clientSocket;
	SimpleThread* m_appDataClientTread;
};

#endif // DATAAQUISITIONSERVICEWIDGET_H
