#include <QCoreApplication>
#include "DataAcquisitionService.h"

#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visula Leak Detector
	// vld.h includes windows.h wich redefine min/max stl functions
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif


int main(int argc, char *argv[])
{
	logger.initLog("da.log", 10, 10, "");

	APP_MSG(logger, "Start");

	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");

	DataServiceWorker* dataServiceWorker = new DataServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2);

	ServiceStarter service(argc, argv, "RPCT DataAcquisition Service", dataServiceWorker);

	return service.exec();
}
