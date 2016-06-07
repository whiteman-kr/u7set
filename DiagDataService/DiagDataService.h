#pragma once

#include "../lib/Service.h"
#include "../lib/DataSource.h"
#include "../lib/Signal.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/DataChannel.h"

namespace Hardware
{
	class DeviceRoot;
}

class DiagDataServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:

public:
	DiagDataServiceWorker(const QString& serviceStrID,
					  const QString& cfgServiceIP1,
					  const QString& cfgServiceIP2);

	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override
	{
		return new DiagDataServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2());
	}
};

