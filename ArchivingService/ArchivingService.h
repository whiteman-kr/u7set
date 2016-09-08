#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"


class ArchivingServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	ArchivingServiceSettings m_settings;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	void clearConfiguration();
	void applyNewConfiguration();

	bool readConfiguration(const QByteArray& fileData);

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) override;

public:
	ArchivingServiceWorker(	const QString& serviceEquipmentID,
							const QString& cfgServiceIP1,
							const QString& cfgServiceIP2,
							const QString& buildPath);
	~ArchivingServiceWorker();

	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override
	{
		return new ArchivingServiceWorker(serviceEquipmentID(), cfgServiceIP1(), cfgServiceIP2(), buildPath());
	}
};

