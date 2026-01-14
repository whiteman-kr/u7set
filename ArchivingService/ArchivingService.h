#pragma once

#include <ServiceLib/Service.h>
#include "../OnlineLib/GrpcCfgLoader.h"
#include "../OnlineLib/SoftwareSettings.h"

#include "Archive.h"


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

	void startAllThreads();
	void stopAllThreads();

	void startArchive();
	void stopArchive();

	void startTcpAppDataServerThread();
	void stopTcpAppDataServerThread();

	void startTcpArchRequestsServerThread();
	void stopTcpArchiveRequestsServerThread();

	void onTimer1min();

	void logFileLoadResult(bool loadOk, const QString& fileName);

private slots:
	virtual void onConfigurationReady(const QByteArray configurationXmlData,
							  const BuildFileInfoArray buildFileInfoArray,
							  SessionParams sessionParams,
							  std::shared_ptr<const SoftwareSettings> curSettingsProfile) override;
private:
	QString m_overwriteArchiveLocation;
	int m_minQueueSizeForFlushing = 0;
	QString m_readOnlyArchivePath;

	ArchivingServiceSettings m_serviceSettings;
	QByteArray m_archInfoFileData;

	mutable QMutex m_startStopMutex;

	Tcp::ListenerThread* m_tcpAppDataServerThread = nullptr;
	Tcp::ListenerThread* m_tcpArchRequestsServerThread = nullptr;

	Archive* m_archive = nullptr;

	QTimer* m_timer = nullptr;
};
