#include <QCoreApplication>

#include "ConfigurationService.h"

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0]);

	LOG_MSG(QString("Run: %1").arg(argv[0]));

	ConfigurationServiceWorker* cfgServiceWorker = new ConfigurationServiceWorker("RPCT Configuration Service", argc, argv, 1, 0);

	ServiceStarter serviceStarter(cfgServiceWorker);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOG_MSG(QString("Exit: %1, result = %2").arg(argv[0]).arg(result));

	return result;
}
