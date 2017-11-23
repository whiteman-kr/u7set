#ifndef DATAAQUISITIONSERVICEWIDGET_H
#define DATAAQUISITIONSERVICEWIDGET_H

#include <QAbstractTableModel>
#include "../lib/OrderedHash.h"
#include "../lib/AppDataSource.h"
#include "BaseServiceStateWidget.h"


class QTableView;
class TcpAppDataClient;
class SimpleThread;
class QActionGroup;

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

	void updateData(int firstRow, int lastRow, int firstColumn, int lastColumn);
	void updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight);

public slots:
	void invalidateData();
	void reloadList();

private:
	TcpAppDataClient* m_clientSocket;
	QList<AppDataSource*> m_dataSource;
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

	void updateData(int firstRow, int lastRow, int firstColumn, int lastColumn);
	void updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight);

public slots:
	void invalidateData();
	void reloadList();

private:
	TcpAppDataClient* m_clientSocket;
};

class AppDataServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	AppDataServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);
	~AppDataServiceWidget();

public slots:
	void updateServiceState();

	void updateStateInfo();

	void updateSourceInfo();
	void updateSourceStateColumns();

	void updateSignalInfo();
	void updateSignalStateColumns();

	void checkVisibility();

	void changeSourceColumnVisibility(QAction* action);

private:
	DataSourcesStateModel* m_dataSourcesStateModel = nullptr;
	SignalStateModel* m_signalStateModel = nullptr;
	QTableView* m_dataSourcesView = nullptr;
	QTableView* m_signalsView = nullptr;
	QActionGroup* m_sourceTableHeadersContextMenuActions = nullptr;

	TcpAppDataClient* m_tcpClientSocket;
	SimpleThread* m_appDataClientTread;

	void saveSourceColumnVisibility(int index, bool visible);

private slots:
	void saveSourceColumnWidth(int index);
};

#endif // DATAAQUISITIONSERVICEWIDGET_H
