#pragma once

#include "../include/Service.h"
#include "../include/DataSource.h"
#include "../include/Signal.h"
#include "../include/CfgServerLoader.h"
#include "../include/ServiceSettings.h"
#include "../include/Queue.h"
#include "../include/DataChannel.h"

#include "AppDataChannel.h"
#include "AppSignalState.h"


namespace Hardware
{
	class DeviceRoot;
}




//typedef HashedVector<quint32, DataSource*> AppSignals;

class AppDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	UdpSocketThread* m_infoSocketThread = nullptr;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

//	std::shared_ptr<Hardware::DeviceRoot> m_deviceRoot;
	//SignalSet m_signalSet;
	UnitList m_unitInfo;

	AppSignals m_appSignals;
	AppDataSources m_appDataSources;

	AppSignalStates m_signalStates;

	QTimer m_timer;

	AppDataServiceSettings m_settings;

	AppDataChannelThread* m_appDataChannelThread[AppDataServiceSettings::DATA_CHANNEL_COUNT];

	void readConfigurationFiles();

	void runUdpThreads();
	void stopUdpThreads();

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void runFscDataReceivingThreads();
	void stopFscDataReceivingThreads();

	void runTimer();
	void stopTimer();

	void onGetDataSourcesIDs(UdpRequest& request);
	void onGetDataSourcesInfo(UdpRequest& request);
	void onGetDataSourcesState(UdpRequest& request);

	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	void onTimer();

	bool readConfiguration(const QByteArray& fileData);
	bool readDataSources(QByteArray& fileData);
	bool readAppSignals(QByteArray& fileData);

	void buildAppSignalID2IndexMap(bool signalsLoadResult);
	void createAndInitSignalStates();

	void stopDataChannelThreads();
	void initDataChannelThreads();
	void runDataChannelThreads();

	void clearConfiguration();

public:
	AppDataServiceWorker(const QString& serviceStrID,
					  const QString& cfgServiceIP1,
					  const QString& cfgServiceIP2);
	~AppDataServiceWorker();

	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override
	{
		return new AppDataServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2());
	}

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};

