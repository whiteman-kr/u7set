#pragma once

#include "../include/BaseService.h"
#include "../include/DataSource.h"


class DataServiceMainFunctionWorker : public MainFunctionWorker
{
	Q_OBJECT

private:
	QHash<quint32, DataSource> m_dataSources;

	UdpSocketThread* m_infoSocketThread = nullptr;

	void initDataSources();
	void runUdpThreads();
	void stopUdpThreads();

	void onGetDataSourcesIDs(UdpRequest& request);
	void onGetDataSourcesInfo(UdpRequest& request);
	void onGetDataSourcesState(UdpRequest& request);

public:
	virtual void initialize() override;
	virtual void shutdown() override;

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};


class DataAquisitionService : public BaseService
{
public:
	DataAquisitionService(int argc, char ** argv);
	~DataAquisitionService();
};
