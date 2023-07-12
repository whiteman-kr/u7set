#pragma once

#include "../ServiceLib/Service.h"
#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/Queue.h"

#include "TcpAppDataServer.h"
#include "Archive.h"
#include "TcpArchRequestsServer.h"
#include "ArchRequest.h"
#include "ArchWriterThread.h"

class ArchivingService : public ServiceWorker
{
	Q_OBJECT

public:
	ArchivingService(const SoftwareInfo& softwareInfo,
						   const QString &serviceInstanceName,
						   int argc,
						   char **argv,
						   std::shared_ptr<CircularLogger> logger);
	ArchivingService(const ArchivingService* worker);

	~ArchivingService();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

	bool isReadOnlyArchive() const;

private:
	virtual void initServiceSpecificCmdLineArgs() override;
	virtual void loadServiceSpecificSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void startAllThreads();
	void stopAllThreads();

	void startArchive();
	void stopArchive();

	void startTcpAppDataServerThread();
	void stopTcpAppDataServerThread();

	void startTcpArchRequestsServerThread();
	void stopTcpArchiveRequestsServerThread();

	void logFileLoadResult(bool loadOk, const QString& fileName);

private slots:
	void onConfigurationReady(const QByteArray configurationXmlData,
							  const BuildFileInfoArray buildFileInfoArray,
							  SessionParams sessionParams,
							  std::shared_ptr<const SoftwareSettings> curSettingsProfile);

private:
	QString m_overwriteArchiveLocation;
	int m_minQueueSizeForFlushing = 0;
	QString m_readOnlyArchivePath;

	ArchivingServiceSettings m_serviceSettings;
	OnlineLib::BuildInfo m_buildInfo;
	QByteArray m_archInfoFileData;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	Tcp::ServerThread* m_tcpAppDataServerThread = nullptr;
	Tcp::ServerThread* m_tcpArchRequestsServerThread = nullptr;

	Archive* m_archive = nullptr;
};
