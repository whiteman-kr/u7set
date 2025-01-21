#pragma once

#include <ServiceLib/Service.h>
#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../GatewayLib/GatewayDescription.h"
#include "GatewayHandler.h"

class GatewayServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	static const int m_majorVersion = 0;
	static const int m_minorVersion = 5;

public:
	GatewayServiceWorker(const SoftwareInfo& softwareInfo,
						 const QString& serviceInstanceName,
						 int argc,
						 char** argv,
						 CircularLoggerShared logger);
	GatewayServiceWorker(const GatewayServiceWorker* worker);

	~GatewayServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port) const;

	const AppSignals& appSignals() const { return m_appSignals; }

private:
	virtual void initServiceSpecificCmdLineArgs() override;
	virtual void loadServiceSpecificSettings() override;

	bool processServiceSpecificCmdLineArgs() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	//

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void onConfigurationReady(const QByteArray configurationXmlData,
							  const BuildFileInfoArray buildFileInfoArray,
							  SessionParams sessionParams,
							  std::shared_ptr<const SoftwareSettings> currentSettingsProfile);

	bool readAppSignals(const QByteArray& fileData);
	bool readGatewayDescription(const QByteArray& fileData);

	void applyNewConfiguration();
	void clearConfiguration();

	void runTimer();
	void stopTimer();

	void onTimer();

	const AppSignal* getSignalFunc(const QString& signalID);

private:
	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	GatewayServiceSettings m_curSettingsProfile;

	std::set<Hash> m_acquiredSignals;		// set of Hash(appSignalID) of acquired signals
	AppSignals m_appSignals;

	QString m_logGatewayIDs;

	Gateway::Gateways m_gateways;
	Gateway::Handlers m_handlers;

	//DynamicAppSignalStates m_appSignalStates;

	QTimer m_timer;
};

