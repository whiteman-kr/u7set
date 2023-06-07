#pragma once

#include "../ServiceLib/Service.h"
#include "../lib/DataSource.h"
#include "../AppSignalLib/AppSignal.h"
#include "../OnlineLib/CfgServerLoader.h"
#include "../OnlineLib/SoftwareSettings.h"


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

private:
	std::shared_ptr<const DiagDataServiceSettings> m_serviceSettings;
};

