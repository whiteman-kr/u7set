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
#include "SignalStatesProcessingThread.h"
#include "RtTrendsServer.h"


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

	void runSignalStatesProcessingThread();
	void stopSignalStatesProcessingThread();

	void runAppDataProcessingThreads();
	void stopAppDataProcessingThreads();

	void runTcpAppDataServer();
	void stopTcpAppDataServer();

	void runTcpArchiveClientThread();
	void stopTcpArchiveClientThread();

	void runRtTrendsServerThread();
	void stopRtTrendsServerThread();

	void runTimer();
	void stopTimer();

	void onGetDataSourcesIDs(UdpRequest& request);
	void onGetDataSourcesInfo(UdpRequest& request);
	void onGetDataSourcesState(UdpRequest& request);

	void onTimer();

private:
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	AppDataServiceSettings m_cfgSettings;
	int m_appDataProcessingThreadCount = 0;

	int m_autoArchivingGroupsCount = 0;

	AppSignals m_appSignals;

	AppDataSources m_appDataSources;				// all data sources
	AppDataSourcesIP m_appDataSourcesIP;

	SignalsToSources m_signalsToSources;

	AppSignalStates m_signalStates;

	AppDataProcessingThreadsPool m_appDataProcessingThreadsPool;

	AppDataReceiverThread* m_appDataReceiverThread = nullptr;

	SignalStatesProcessingThread* m_signalStatesProcessingThread = nullptr;

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;

	TcpArchiveClientThread* m_tcpArchiveClientThread = nullptr;

	RtTrends::ServerThread* m_rtTrendsServerThread = nullptr;

	static const int APP_SIGNAL_EVENTS_QUEUE_MAX_SIZE = 1024 * 1024;

	QTimer m_timer;
};

