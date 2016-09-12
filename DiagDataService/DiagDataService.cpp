#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../lib/DeviceObject.h"
#include "DiagDataService.h"


// -------------------------------------------------------------------------------
//
// DiagDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

DiagDataServiceWorker::DiagDataServiceWorker(const QString& serviceStrID,
									 const QString& cfgServiceIP1,
									 const QString& cfgServiceIP2,
									 const QString& buildPath) :
	ServiceWorker(ServiceType::DiagDataService, serviceStrID, cfgServiceIP1, cfgServiceIP2, buildPath)
{
}


void DiagDataServiceWorker::initialize()
{
}

void DiagDataServiceWorker::shutdown()
{
}

