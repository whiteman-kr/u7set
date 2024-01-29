#pragma once

#include "../ServiceLib/Service.h"
#include "../lib/DataSource.h"
#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../HardwareLib/DiagSignalType.h"

#include "DiagDataSource.h"

class DiagDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	DiagDataServiceWorker(const SoftwareInfo& softwareInfo,
						  const QString& serviceInstanceName,
						  int argc,
						  char** argv,
						  CircularLoggerShared logger);

	DiagDataServiceWorker(const DiagDataServiceWorker* worker);

	virtual ~DiagDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

private:
	void initServiceSpecificCmdLineArgs() override;
	virtual void loadServiceSpecificSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData,
							  const BuildFileInfoArray buildFileInfoArray,
							  SessionParams sessionParams,
							  std::shared_ptr<const SoftwareSettings> currentSettingsProfile);

	bool readDiagDataSources(const QByteArray& fileData, const QString& profile);
	bool readDiagSignals(const QByteArray& fileData);
	bool readDiagSignalTypes(const QByteArray& fileData);

	void applyNewConfiguration();
	void clearConfiguration();

	void runDiagDataReceiverThread();
	void stopDiagDataReceiverThread();

	void runTcpDiagDataServer();
	void stopTcpDiagDataServer();

private:
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	DiagDataServiceSettings m_curSettingsProfile;

	std::shared_ptr<const DiagDataServiceSettings> m_serviceSettings;

	Hardware::DiagSignalTypes m_diagSignalTypes;

	int m_diagDataProcessingThreadCount = 0;
	QString m_strCmdLineDiagDataReceivingIP;
	HostAddressPort m_cmdLineDiagDataReceivingIP;

	DiagDataSources m_diagDataSources;
	DynamicDiagSignalStates m_diagSignalStates;

	DiagDataReceiver* m_diagDataReceiver = nullptr;

};

