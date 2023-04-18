#pragma once

#include "../ServiceLib/Service.h"
#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/Queue.h"
#include "../lib/DataSource.h"
#include "../AppSignalLib/AppSignal.h"

class GatewayServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	static const int m_majorVersion = 0;
	static const int m_minorVersion = 5;

public:
	GatewayServiceWorker(const SoftwareInfo& softwareInfo,
						 const QString& serviceInstanceName,
						 int& argc,
						 char** argv,
						 CircularLoggerShared logger,
						 E::ServiceRunMode runMode);
	~GatewayServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port) const;

	const AppSignals& appSignals() const { return m_appSignals; }

/*	const DynamicAppSignalStates& appSignalStates() const { return m_appSignalStates; }
	DynamicAppSignalStates& appSignalStates() { return m_appSignalStates; }*/

	const std::set<Hash>& acquiredAppSignalIDs() const { return m_acquiredAppSignalIDs; }
	int acquiredAppSignalIDsCount() const { return static_cast<int>(m_acquiredAppSignalIDs.size()); }

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	bool processCustomCmdLineSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	//

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData,
							  const BuildFileInfoArray buildFileInfoArray,
							  SessionParams sessionParams,
							  std::shared_ptr<const SoftwareSettings> currentSettingsProfile);

	bool readAppSignals(const QString& fileName, const QByteArray& fileData);
	bool readGatewayDescription(const QString& fileName, const QByteArray& fileData);

	void createAndInitSignalStates();
	void buildAcuiredAppSignalIDs();

	void applyNewConfiguration();
	void clearConfiguration();

	void runTimer();
	void stopTimer();

	void onTimer();

	void parseGatewayDescription(const QString& filePathName, const QString& gwDesc);

	const AppSignal* getSignalFunc(const QString& signalID);

private:
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	GatewayServiceSettings m_curSettingsProfile;
	CircularLoggerShared m_timeErrLog;

	int m_autoArchivingGroupsCount = 0;

	AppSignals m_appSignals;

	//DynamicAppSignalStates m_appSignalStates;

	std::set<Hash> m_acquiredAppSignalIDs;

	//

	QTimer m_timer;
};

