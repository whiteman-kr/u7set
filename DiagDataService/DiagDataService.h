#pragma once

#include "../include/Service.h"
#include "../include/DataSource.h"
#include "../include/Signal.h"
#include "../include/CfgServerLoader.h"
#include "../include/ServiceSettings.h"

#include "DataChannel.h"

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

