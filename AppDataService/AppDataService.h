#pragma once

#include "../include/Service.h"
#include "../include/DataSource.h"
#include "../include/Signal.h"
#include "../include/CfgServerLoader.h"
#include "../include/ServiceSettings.h"
#include "AppSignalState.h"

#include "DataChannel.h"

namespace Hardware
{
	class DeviceRoot;
}



class AppDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	UdpSocketThread* m_infoSocketThread = nullptr;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	std::shared_ptr<Hardware::DeviceRoot> m_deviceRoot;
	SignalSet m_signalSet;
	UnitList m_unitInfo;

	QVector<Signal> m_appSignals;
	QHash<QString, int> m_appSignalID2IndexMap;

	AppSignalState* m_signalStates = nullptr;

	QTimer m_timer;

	AppDataServiceSettings m_settings;

	DataChannelThread* m_appDataChannelThread[AppDataServiceSettings::DATA_CHANNEL_COUNT];

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

	void stopDataChannels();
	void initDataChannels();
	void runDataChannels();


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

