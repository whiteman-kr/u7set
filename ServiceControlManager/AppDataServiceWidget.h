#ifndef DATAAQUISITIONSERVICEWIDGET_H
#define DATAAQUISITIONSERVICEWIDGET_H

#include <QAbstractTableModel>
#include "../lib/OrderedHash.h"
#include "../lib/AppDataSource.h"
#include "../lib/Tcp.h"
#include "BaseServiceStateWidget.h"


class QTableView;
class TcpAppDataClient;
class SimpleThread;
class QActionGroup;

class DataSourcesStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit DataSourcesStateModel(QObject *parent = 0);
	~DataSourcesStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void updateData(int firstRow, int lastRow, int firstColumn, int lastColumn);
	void updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight);

	void setClient(TcpAppDataClient* clientSocket) { m_clientSocket = clientSocket; }

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
	explicit SignalStateModel(QObject *parent = 0);
	~SignalStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void updateData(int firstRow, int lastRow, int firstColumn, int lastColumn);
	void updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight);

	void setClient(TcpAppDataClient* clientSocket) { m_clientSocket = clientSocket; }

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
	AppDataServiceWidget(const SoftwareInfo& softwareInfo, quint32 udpIp, quint16 udpPort, QWidget *parent = 0);
	~AppDataServiceWidget();

public slots:
	void updateServiceState();

	void updateStateInfo();

	void updateSourceInfo();
	void updateSourceStateColumns();

	void updateSignalInfo();
	void updateSignalStateColumns();

	void updateClientsInfo();

	void updateSettings();

	void clearServiceData();

	void changeSourceColumnVisibility(QAction* action);

protected:
	void createTcpConnection(quint32 ip, quint16 port) override;
	void dropTcpConnection() override;

private:
	DataSourcesStateModel* m_dataSourcesStateModel = nullptr;
	SignalStateModel* m_signalStateModel = nullptr;
	QStandardItemModel* m_settingsTabModel = nullptr;

	QTableView* m_dataSourcesView = nullptr;
	QTableView* m_signalsView = nullptr;

	QActionGroup* m_sourceTableHeadersContextMenuActions = nullptr;

	TcpAppDataClient* m_tcpClientSocket;
	SimpleThread *m_tcpClientThread;

	void saveSourceColumnVisibility(int index, bool visible);

private slots:
	void saveSourceColumnWidth(int index);
};

#endif // DATAAQUISITIONSERVICEWIDGET_H
