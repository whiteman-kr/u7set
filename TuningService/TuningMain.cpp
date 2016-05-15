#include <QCoreApplication>
#include "TuningService.h"

#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visual Leak Detector
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
	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");
	QString buildPath = ServiceStarter::getCommandLineKeyValue(argc, argv, "b");

	Tuning::TuningServiceWorker* tuningServiceWorker = new Tuning::TuningServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2, buildPath + "/configuration.xml");

	ServiceStarter serviceStarter(argc, argv, "RPCT Tuning Service", tuningServiceWorker);

	return serviceStarter.exec();
}
