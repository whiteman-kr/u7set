#pragma once

#include "../lib/Service.h"
#include "../lib/DataSource.h"
#include "../lib/Signal.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"
#include "../lib/DataChannel.h"

#include "AppDataChannel.h"
#include "AppSignalStateEx.h"
#include "TcpAppDataServer.h"


/*namespace Hardware
{
	class DeviceRoot;
}*/


class AppDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	AppDataServiceWorker(const QString& serviceName,
						 int& argc,
						 char** argv,
						 const VersionInfo& versionInfo,
						 std::shared_ptr<CircularLogger> logger);
	~AppDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

private:
	virtual void initCmdLineParser() override;
	virtual void processCmdLineSettings() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	//

	void readConfigurationFiles();

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
	void applyNewConfiguration();

private:
	QString m_equipmentID;
	QString m_cfgServiceIP1Str;
	QString m_cfgServiceIP2Str;

	HostAddressPort m_cfgServiceIP1;
	HostAddressPort m_cfgServiceIP2;

	std::shared_ptr<CircularLogger> m_logger;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	AppDataServiceSettings m_cfgSettings;

	UnitList m_unitInfo;

	AppSignals m_appSignals;

	AppDataSources m_appDataSources;				// all data sources
	AppDataSourcesIP m_enabledAppDataSources;		// only enabled data sources

	AppSignalStates m_signalStates;

	AppDataChannelThread* m_appDataChannelThread[AppDataServiceSettings::DATA_CHANNEL_COUNT];

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;

	QTimer m_timer;
};

