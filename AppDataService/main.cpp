#include <QCoreApplication>
#include "AppDataService.h"


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	//logger.initLog(10, 10, argv[0]);
	//APP_MSG(logger, "Start");

	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");
	QString buildPath = ServiceStarter::getCommandLineKeyValue(argc, argv, "b");

	AppDataServiceWorker* dataServiceWorker = new AppDataServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2, buildPath);

	ServiceStarter service(argc, argv, "RPCT Application Data Service", dataServiceWorker);

	int result = service.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
