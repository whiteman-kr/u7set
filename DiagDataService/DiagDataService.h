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
	DiagDataServiceWorker();

	void initCmdLineParser() override;

	virtual void initialize() override;
	virtual void shutdown() override;

/*	ServiceWorker* createInstance() override
	{
		return new DiagDataServiceWorker(serviceEquipmentID(), cfgServiceIP1(), cfgServiceIP2(), buildPath());
	}*/
};

