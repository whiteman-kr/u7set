#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../lib/DeviceObject.h"
#include "DiagDataService.h"


// -------------------------------------------------------------------------------
//
// DiagDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

DiagDataServiceWorker::DiagDataServiceWorker() :
	ServiceWorker(ServiceType::DiagDataService)
{
}


void DiagDataServiceWorker::initCmdLineParser()
{
	CommandLineParser* clp = cmdLineParser();

	if (clp == nullptr)
	{
		assert(false);
		return;
	}

	clp->addSingleValueOption("cfgip1", "IP-addres of first Configuration Service.");
	clp->addSingleValueOption("cfgip2", "IP-addres of second Configuration Service.");
}



void DiagDataServiceWorker::initialize()
{
}

void DiagDataServiceWorker::shutdown()
{
}

