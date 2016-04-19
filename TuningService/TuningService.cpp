#include "TuningService.h"


TuningServiceWorker::TuningServiceWorker(const QString& serviceStrID,
										 const QString& cfgServiceIP1,
										 const QString& cfgServiceIP2) :
	ServiceWorker(ServiceType::TuningService, serviceStrID, cfgServiceIP1, cfgServiceIP2)
{
}


TuningServiceWorker* TuningServiceWorker::createInstance()
{
	return new TuningServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2());
}
