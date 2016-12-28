#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"


class ArchivingServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ArchivingServiceWorker();
	~ArchivingServiceWorker();

	virtual void initCmdLineParser() override;

	virtual void initialize() override;
	virtual void shutdown() override;

private:
	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	void clearConfiguration();
	void applyNewConfiguration();

	bool readConfiguration(const QByteArray& fileData);
	bool loadConfigurationFromFile(const QString& fileName);

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) override;

private:
	ArchivingServiceSettings m_settings;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	QString m_buildPath;
	QString m_cfgServiceIP1;
	QString m_cfgServiceIP2;
};

