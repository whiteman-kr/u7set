#include <QCoreApplication>
#include "DiagDataService.h"


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0]);

	LOG_MSG("Start");

	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");
	QString buildPath = ServiceStarter::getCommandLineKeyValue(argc, argv, "b");

	DiagDataServiceWorker* dataServiceWorker = new DiagDataServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2, buildPath);

	ServiceStarter service(argc, argv, "RPCT Diagnostics Data Service", dataServiceWorker);

	return service.exec();
}
