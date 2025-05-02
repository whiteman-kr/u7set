#pragma once

#include <ServiceLib/Service.h>
#include "../OnlineLib/CfgLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "TcpAppDataServer.h"
#include "TcpArchiveClient.h"
#include "RtTrendsServer.h"
#include "DynamicAppSignalState.h"
#include "AppDataSource.h"
#include "ApertureFile.h"

class TcpArchiveClient;
class AppDataReceiver;

namespace RtTrends
{
	class ServerThread;
}

#pragma pack(push, 1)

union RecordsPerMin
{
	struct
	{
		qint32 recordsCount;
		qint32 dynamicStateIndex : 31;
		qint32 overrided : 1;
	};

	qint64 int64Value = 0;
};

#pragma pack(pop)

inline bool operator == (const RecordsPerMin& a, const RecordsPerMin& b)
{
	return a.int64Value == b.int64Value;
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
						 int argc,
						 char** argv,
						 CircularLoggerShared logger);

	AppDataServiceWorker(const AppDataServiceWorker* worker);

	~AppDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;

	virtual void processGetServiceInfoRequest(const Network::GetServiceInfoRequest& rq) override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port) const;
	bool isConnectedToArchiveService(quint32 &ip, quint16 &port) const;

	const AppDataSources& appDataSources() const { return m_appDataSources; }
	AppDataSources& appDataSources() { return m_appDataSources; }

	const AppSignals& appSignals() const { return m_appSignals; }

	const DynamicAppSignalStates& appSignalStates() const { return m_appSignalStates; }
	DynamicAppSignalStates& appSignalStates() { return m_appSignalStates; }

	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, bool isArchivingQueue, const QString& description);
	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

	void registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
										const std::set<Hash>& hashes);
	void unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue);

	void fillAppDataReceiveState(Network::AppDataReceiveState* adrs);

	const std::vector<QString>& acquiredAppSignalIDs() const { return m_acquiredAppSignalIDs; }
	int acquiredAppSignalIDsCount() const { return static_cast<int>(m_acquiredAppSignalIDs.size()); }

signals:
	void restartArchSignalsTimer();

private:
	virtual void initServiceSpecificCmdLineArgs() override;
	virtual void loadServiceSpecificSettings() override;

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

	void createTimeErrLog();
	void shutdownTimeErrLog();

	void createAndInitSignalStates();
	void buildAcuiredAppSignalIDs();
	void prepareAppDataSources();

	void applyNewConfiguration();
	void clearConfiguration();

	void runAppDataReceiverThread();
	void stopAppDataReceiverThread();

	void runTcpAppDataServer();
	void stopTcpAppDataServer();

	void runTcpArchiveClientThread();
	void stopTcpArchiveClientThread();

	void runRtTrendsServerThread();
	void stopRtTrendsServerThread();

	void onGetDataSourcesIDs(UdpRequest& request);
	void onGetDataSourcesInfo(UdpRequest& request);
	void onGetDataSourcesState(UdpRequest& request);

	void getRecordsPerMin(std::vector<RecordsPerMin>* recordsPerMin,
						  int count, double* updateStatus) const;

	void onRestartArchSignalsTimer();
	void onArchSignalsTimer();

	void copyArchSignalsInfo(Network::ServiceInfo& serviceInfo) const;

private:
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	AppDataServiceSettings m_curSettingsProfile;

	int m_appDataProcessingThreadCount = 0;
	QString m_strCmdLineAppDataReceivingIP;
	HostAddressPort m_cmdLineAppDataReceivingIP;
	bool m_logRupTimeErrors = false;

	mutable QMutex m_startStopMutex;

	QTimer* m_archSignalsUpdateTimer = nullptr;
	qint64 m_archSignalsTimerStartMs = 0;

	const qint64 ARCH_SIGNALS_UPDATE_INTERVAL = 60 * 1000;

	CircularLoggerShared m_timeErrLog;

	int m_autoArchivingGroupsCount = 0;

	AppSignals m_appSignals;

	AppDataSources m_appDataSources;

	DynamicAppSignalStates m_appSignalStates;

	ApertureFile m_apertureFile;

	mutable QMutex m_recordsPerMinMutex;
	std::vector<RecordsPerMin> m_recordsPerMin;
	mutable std::vector<RecordsPerMin> m_cachedRecordsPerMin;
	mutable int m_archSignalsRequestCtr = 0;
	mutable bool m_updateArchSignalsAnyway = false;

	std::vector<QString> m_acquiredAppSignalIDs;

	//

	AppDataReceiver* m_appDataReceiver = nullptr;

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;

	TcpArchiveClientThread* m_tcpArchiveClientThread = nullptr;

	RtTrends::ServerThread* m_rtTrendsServerThread = nullptr;
};
