#include <QCoreApplication>

#include "ConfigurationService.h"


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
	QString buildFolder = ServiceStarter::getCommandLineKeyValue(argc, argv, "b");
	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString ipStr = ServiceStarter::getCommandLineKeyValue(argc, argv, "ip");

	ConfigurationServiceWorker* cfgServiceWorker = new ConfigurationServiceWorker(serviceStrID, buildFolder, ipStr);

	ServiceStarter serviceStarter(argc, argv, "RPCT Configuration Service", cfgServiceWorker);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
