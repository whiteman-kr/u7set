#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"


class ArchivingServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ArchivingServiceWorker(const QString &serviceName, int &argc, char **argv, const VersionInfo &versionInfo);
	~ArchivingServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

private:
	virtual void initCmdLineParser() override;
	virtual void processCmdLineSettings() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void clearConfiguration();
	void applyNewConfiguration();

	bool readConfiguration(const QByteArray& fileData);
	bool loadConfigurationFromFile(const QString& fileName);

private slots:
	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	QString m_equipmentID;
	QString m_cfgServiceIP1Str;
	QString m_cfgServiceIP2Str;

	HostAddressPort m_cfgServiceIP1;
	HostAddressPort m_cfgServiceIP2;

	QSettings m_settings;

	ArchivingServiceSettings m_cfgSettings;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

};

