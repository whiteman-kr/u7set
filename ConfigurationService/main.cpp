#include <QCoreApplication>

#include "ConfigurationService.h"

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	ConfigurationServiceWorker* cfgServiceWorker = new ConfigurationServiceWorker(serviceStrID, buildFolder, ipStr);

	ServiceStarter serviceStarter(argc, argv, "RPCT Configuration Service", cfgServiceWorker);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
