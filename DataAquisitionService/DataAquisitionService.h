#pragma once

#include "../include/Service.h"
#include "../include/DataSource.h"
#include "../include/Signal.h"
#include "../include/CfgServerLoader.h"

#include "FscDataAcquisitionThread.h"

namespace Hardware {
	class DeviceRoot;
}


class DataServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	QHash<quint32, DataSource> m_dataSources;
	QVector<HostAddressPort> m_fscDataAcquisitionAddressPorts;

	QVector<FscDataAcquisitionThread*> m_fscDataAcquisitionThreads;

	UdpSocketThread* m_infoSocketThread = nullptr;

    CfgLoaderThread* m_cfgLoaderThread = nullptr;

	std::shared_ptr<Hardware::DeviceRoot> m_deviceRoot;
	SignalSet m_signalSet;
	UnitList m_unitInfo;

	void initDataSources();
	void initListeningPorts();
	void readConfigurationFiles();

	void runUdpThreads();
	void stopUdpThreads();

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void runFscDataReceivingThreads();
	void stopFscDataReceivingThreads();

	void onGetDataSourcesIDs(UdpRequest& request);
	void onGetDataSourcesInfo(UdpRequest& request);
	void onGetDataSourcesState(UdpRequest& request);

	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

public:
	DataServiceWorker() : ServiceWorker(ServiceType::DataAcquisition) {}
	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override { return new DataServiceWorker; }

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};


/*
class DataAquisitionService : public Service
{
public:
	DataAquisitionService(int argc, char ** argv);
	~DataAquisitionService();
};
*/
