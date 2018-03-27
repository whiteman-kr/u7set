#pragma once

#include "../lib/Service.h"
#include "../lib/DataSource.h"
#include "../lib/Signal.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"

#include "AppDataReceiver.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"
#include "AppDataProcessingThread.h"


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
						 CircularLoggerShared logger);
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

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	bool readConfigurationSettings(const QByteArray& fileData);
	bool readDataSources(const QByteArray& fileData);
	bool readAppSignals(const QByteArray& fileData);

	void buildAppSignalID2IndexMap(bool signalsLoadResult);
	void createAndInitSignalStates();
	void prepareAppDataSources();

	void applyNewConfiguration();
	void clearConfiguration();

	void runAppDataReceiverThread();
	void stopAppDataReceiverlThread();

	void runAppDataProcessingThreads();
	void stopAppDataProcessingThreads();

	void runTcpAppDataServer();
	void stopTcpAppDataServer();

	void runTcpArchiveClientThreads();
	void stopTcpArchiveClientThreads();

	void runTimer();
	void stopTimer();

	void onGetDataSourcesIDs(UdpRequest& request);
	void onGetDataSourcesInfo(UdpRequest& request);
	void onGetDataSourcesState(UdpRequest& request);

	void resizeAppSignalEventsQueue();

	void onTimer();

private:
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	AppDataServiceSettings m_cfgSettings;
	int m_appDataProcessingThreadCount = 0;

	int m_autoArchivingGroupsCount = 0;

	AppSignals m_appSignals;

	AppDataSources m_appDataSources;				// all data sources
	AppDataSourcesIP m_appDataSourcesIP;

	AppSignalStates m_signalStates;

	AppDataProcessingThreadsPool m_appDataProcessingThreadsPool;

	AppDataReceiverThread* m_appDataReceiverThread = nullptr;

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;

	TcpArchiveClientThread* m_tcpArchiveClientThread = nullptr;

	static const int APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE = 1024 * 1024;

	AppSignalStatesQueue m_signalStatesQueue;

	QTimer m_timer;
};

