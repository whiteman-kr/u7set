#pragma once

#include "../include/BaseService.h"
#include "../include/DataSource.h"
#include "../include/Signal.h"

#include "FscDataAcquisitionThread.h"

namespace Hardware {
	class DeviceRoot;
}


class DataServiceMainFunctionWorker : public MainFunctionWorker
{
	Q_OBJECT

private:
	QHash<quint32, DataSource> m_dataSources;
	QVector<HostAddressPort> m_fscDataAcquisitionAddressPorts;

	QVector<FscDataAcquisitionThread*> m_fscDataAcquisitionThreads;

	UdpSocketThread* m_infoSocketThread = nullptr;

	std::shared_ptr<Hardware::DeviceRoot> m_deviceRoot;
	SignalSet m_signalSet;
	UnitList m_unitInfo;

	void initDataSources();
	void initListeningPorts();
	void readConfigurationFiles();
	void readEquipmentConfig();
	void readApplicationSignalsConfig();

	void runUdpThreads();
	void stopUdpThreads();

	void runFscDataReceivingThreads();
	void stopFscDataReceivingThreads();

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
