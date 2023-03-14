#pragma once

#include "../ServiceLib/Service.h"
#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/Queue.h"
#include "../lib/DataSource.h"

#include "AppDataReceiver.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"
#include "SignalStatesProcessingThread.h"
#include "RtTrendsServer.h"


class TcpArchiveClient;
class AsyncAppDataReceiver;

namespace RtTrends
{
	class ServerThread;
}

class AppDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	static const int m_majorVersion = 0;
	static const int m_minorVersion = 5;

public:
	AppDataServiceWorker(const SoftwareInfo& softwareInfo,
						 const QString& serviceInstanceName,
						 int& argc,
						 char** argv,
						 CircularLoggerShared logger,
						 E::ServiceRunMode runMode);
	~AppDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port) const;
	bool isConnectedToArchiveService(quint32 &ip, quint16 &port) const;

	const AppDataSources& appDataSources() const { return m_appDataSources; }
	AppDataSources& appDataSources() { return m_appDataSources; }

	DynamicAppSignalStates& signalStates() { return m_signalStates; }

	E::SecurityLevel securityLevel() const;

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	//

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData,
							  const BuildFileInfoArray buildFileInfoArray,
							  SessionParams sessionParams,
							  std::shared_ptr<const SoftwareSettings> currentSettingsProfile);

	bool readAppDataSources(const QByteArray& fileData, const QString& profile);
	bool readAppSignals(const QByteArray& fileData);

	void buildAppSignalID2IndexMap(bool signalsLoadResult);

	void createTimeErrLog();
	void shutdownTimeErrLog();

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

	AppDataServiceSettings m_curSettingsProfile;

	int m_appDataProcessingThreadCount = 0;
	QString m_strCmdLineAppDataReceivingIP;
	HostAddressPort m_cmdLineAppDataReceivingIP;
	bool m_logRupTimeErrors = false;

	CircularLoggerShared m_timeErrLog;

	int m_autoArchivingGroupsCount = 0;

	AppSignals m_appSignals;

	AppDataSources m_appDataSources;

	DynamicAppSignalStates m_signalStates;

	AsyncAppDataReceiver* m_asyncAppDataReceiver = nullptr;

	SignalStatesProcessingThread* m_signalStatesProcessingThread = nullptr;

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;

	TcpArchiveClientThread* m_tcpArchiveClientThread = nullptr;

	RtTrends::ServerThread* m_rtTrendsServerThread = nullptr;

	QTimer m_timer;
};

