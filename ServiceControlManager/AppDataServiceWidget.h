#pragma once

#include <QAbstractTableModel>
#include <QProgressBar>
#include "../AppDataService/AppDataSource.h"
#include "../OnlineLib/Tcp.h"
#include "BaseServiceWidget.h"

#include "AppDataSourcesModel.h"
#include "ArchiveSignalsModel.h"
#include "AppDataSourceWidget.h"

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
	virtual void clearDerivedWidgets() override;

private slots:
	void forgetWidget(QString dataSourceID);

private:
	void updateModels(const Network::ServiceInfo& srvInfo);

	void addAppDataSourcesTab();
	void addArchiveSignalsTab();

	virtual int updateSettings(int rowCount) override;

	void onSourceDoubleClicked(const QModelIndex& index);
	void onCustomContextMenuRequested(const QPoint &pos);
	void onChangeApertures();

private:
	AppDataSourcesModel* m_sourcesModel = nullptr;
	QTableView* m_sourcesView = nullptr;

	ArchiveSignalsModel* m_archSignalsModel = nullptr;
	QTableView* m_archSignalsView = nullptr;
	QProgressBar* m_archSignalsProgressBar = nullptr;
	std::vector<int> m_selectedRows;

	std::map<QString, AppDataSourceWidget*> m_sourceWidgets;
};
