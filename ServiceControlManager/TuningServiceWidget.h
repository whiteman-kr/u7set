#pragma once

#include "BaseServiceWidget.h"
#include "TuningSourcesModel.h"
#include "TuningSourceWidget.h"

class TuningServiceWidget : public BaseServiceWidget
{
	Q_OBJECT
public:
	TuningServiceWidget(ServiceTableModel* srvTableModel,
					 const SoftwareInfo& softwareInfo,
					 const ServiceData& serviceData,
					 quint32 ip, quint16 tcpPort,
					 QWidget* parent = 0);
	virtual ~TuningServiceWidget();

	virtual void initWidget() override;

	virtual void updateDerivedWidgets(const Network::ServiceInfo& srvInfo) override;
	virtual void clearDerivedWidgets() override;

public slots:
	int updateSrvStatus(int rowCount) override;
	int updateSettings(int rowCount) override;

private slots:
	void forgetWidget(QString dataSourceID);

private:
	void addTuningSourcesTab();

	void onSourceDoubleClicked(const QModelIndex& index);

	void updateModels(const Network::ServiceInfo& srvInfo);

private:
	TuningSourcesModel* m_sourcesModel = nullptr;
	QTableView* m_sourcesView = nullptr;

	std::map<QString, TuningSourceWidget*> m_sourceWidgets;
};
