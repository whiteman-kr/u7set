#pragma once

#include "../ServiceLib/Service.h"
#include "../lib/DataSource.h"
#include "../lib/AppSignal.h"
#include "../OnlineLib/CfgServerLoader.h"
#include "../lib/SoftwareSettings.h"


class DiagDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	DiagDataServiceWorker(const SoftwareInfo& softwareInfo,
						  const QString& serviceInstanceName,
						  int& argc,
						  char** argv,
						  std::shared_ptr<CircularLogger> logger);
	virtual ~DiagDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

private:
	void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

private:
	std::shared_ptr<CircularLogger> m_logger;

	std::shared_ptr<const DiagDataServiceSettings> m_serviceSettings;
};

