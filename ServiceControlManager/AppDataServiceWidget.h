#pragma once

#include <QAbstractTableModel>
#include "../AppDataService/AppDataSource.h"
#include "../OnlineLib/Tcp.h"
#include "BaseServiceWidget.h"

#include "AppDataSourcesModel.h"
#include "ArchiveSignalsModel.h"

/*
class QTableView;
class TcpAppDataClient;
class SimpleThread;
class QActionGroup;
class AppDataSourceWidget;

class DataSourcesStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit DataSourcesStateModel(QObject *parent = nullptr);
	~DataSourcesStateModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void updateData(int firstRow, int lastRow, int firstColumn, int lastColumn);
	void updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight);

	void setClient(TcpAppDataClient* clientSocket);

	const OnlineLib::DataSource& getDataSource(int row) const;

	void updateStates();

public slots:
	void invalidateData();
	void reloadList();

private:
	TcpAppDataClient* m_clientSocket;
	std::vector<std::pair<OnlineLib::DataSource, Network::AppDataSourceState>> m_dataSources;
};

class SignalStateModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit SignalStateModel(QObject *parent = nullptr);
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
};*/

class AppDataServiceWidget : public BaseServiceWidget
{
	Q_OBJECT
public:
	AppDataServiceWidget(ServiceTableModel* srvTableModel,
						 const SoftwareInfo& softwareInfo,
						 const ServiceData& serviceData,
						 quint32 ip, quint16 tcpPort,
						 QWidget* parent = nullptr);
	~AppDataServiceWidget();

	virtual void initWidget() override;

	virtual void updateDerivedWidgets(const Network::ServiceInfo& srvInfo) override;

private:
	void addAppDataSourcesTab();
	void addArchiveSignalsTab();

	virtual int updateSettings(int rowCount) override;

private:
	AppDataSourcesModel* m_sourcesModel = nullptr;
	QTableView* m_sourcesView = nullptr;

	ArchiveSignalsModel* m_archSignalsModel = nullptr;
	QTableView* m_archSignalsView = nullptr;

/*	DataSourcesStateModel* m_dataSourcesStateModel = nullptr;
	SignalStateModel* m_signalStateModel = nullptr;
	QStandardItemModel* m_parametersTabModel = nullptr;
	QStandardItemModel* m_settingsTabModel = nullptr;

	QTableView* m_dataSourcesView = nullptr;
	QTableView* m_signalsView = nullptr;

	TcpAppDataClient* m_tcpClientSocket;
	SimpleThread* m_tcpClientThread;

	QList<AppDataSourceWidget*> m_appDataSourceWidgetList;*/
};
