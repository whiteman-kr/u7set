#include <QCoreApplication>
#include "ArchivingService.h"

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");
	QString buildPath = ServiceStarter::getCommandLineKeyValue(argc, argv, "b");

	ArchivingServiceWorker* archivingServiceWorker = new ArchivingServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2, buildPath);

	ServiceStarter service(argc, argv, "RPCT Archiving Service", archivingServiceWorker);

	int result = service.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
