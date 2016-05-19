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


class AppDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	UdpSocketThread* m_serviceInfoThread = nullptr;
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	AppDataServiceSettings m_settings;

	UnitList m_unitInfo;

	AppSignals m_appSignals;
	AppDataSources m_appDataSources;

	AppSignalStates m_signalStates;

	AppDataChannelThread* m_appDataChannelThread[AppDataServiceSettings::DATA_CHANNEL_COUNT];

	Tcp::ServerThread* m_tcpAddDataServerThread = nullptr;

	QTimer m_timer;

	//

	void readConfigurationFiles();

	void runServiceInfoThread();
	void stopServiceInfoThread();

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void runFscDataReceivingThreads();
	void stopFscDataReceivingThreads();

	void runTcpAppDataServer();
	void stopTcpAppDataServer();

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

