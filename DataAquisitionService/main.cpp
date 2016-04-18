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

	ServiceStarter service(argc, argv, "RPCT DataAcquisition Service", new DataServiceWorker());

	return service.exec();
}
