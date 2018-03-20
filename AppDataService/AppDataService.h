#pragma once

#include "../lib/Service.h"
#include "../lib/DataSource.h"
#include "../lib/Signal.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"

#include "AppDataReceiver.h"
#include "AppSignalStateEx.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"

class TcpArchiveClient;

class AppDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	static const int m_majorVersion = 0;
	static const int m_minorVersion = 5;

public:
	AppDataServiceWorker(const SoftwareInfo& softwareInfo,
						 const QString& serviceName,
						 int& argc,
						 char** argv,
						 CircularLoggerShared log);
	~AppDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port) const;
	bool isConnectedToArchiveService(quint32 &ip, quint16 &port) const;

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	//

	void readConfigurationFiles();

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void runAppDataReceiverThread();
	void stopAppDataReceiverlThread();

	void runTcpAppDataServer();
	void stopTcpAppDataServer();

	void runTcpArchiveClientThreads();
	void stopTcpArchiveClientThreads();

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

	void clearConfiguration();
	void applyNewConfiguration();

	void resizeAppSignalEventsQueue();

private:
	CircularLoggerShared m_log;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	AppDataServiceSettings m_cfgSettings;

	int m_autoArchivingGroupsCount = 0;

	AppSignals m_appSignals;

	AppDataSources m_appDataSources;				// all data sources
	AppDataSourcesIP m_appDataSourcesIP;

	AppSignalStates m_signalStates;

	AppDataReceiverThread* m_appDataReceiverThread = nullptr;

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;

	TcpArchiveClientThread* m_tcpArchiveClientThread = nullptr;

	static const int APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE = 1024 * 1024;

	AppSignalStatesQueue m_signalStatesQueue;

	QTimer m_timer;
};

