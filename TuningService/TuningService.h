#pragma once

#include "../include/Service.h"

class TuningServiceWorker : public ServiceWorker
{
public:
	TuningServiceWorker(const QString& serviceStrID,
						const QString& cfgServiceIP1,
						const QString& cfgServiceIP2);

	TuningServiceWorker* createInstance() override;
};

