#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../include/DeviceObject.h"
#include "DiagDataService.h"


// -------------------------------------------------------------------------------
//
// DiagDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

DiagDataServiceWorker::DiagDataServiceWorker(const QString& serviceStrID,
									 const QString& cfgServiceIP1,
									 const QString& cfgServiceIP2) :
	ServiceWorker(ServiceType::DiagDataService, serviceStrID, cfgServiceIP1, cfgServiceIP2)
{
}


void DiagDataServiceWorker::initialize()
{
}

void DiagDataServiceWorker::shutdown()
{
}

