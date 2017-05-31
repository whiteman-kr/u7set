#pragma once

#include "../lib/Service.h"
#include "../lib/DataSource.h"
#include "../lib/Signal.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/DataChannel.h"


class DiagDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	DiagDataServiceWorker(const QString& serviceName,
						  int& argc,
						  char** argv,
						  const VersionInfo& versionInfo,
						  std::shared_ptr<CircularLogger> logger);
	virtual ~DiagDataServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

private:
	void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

private:
	QString m_equipmentID;
	QString m_cfgServiceIP1Str;
	QString m_cfgServiceIP2Str;

	HostAddressPort m_cfgServiceIP1;
	HostAddressPort m_cfgServiceIP2;

	std::shared_ptr<CircularLogger> m_logger;
};

