#include "ConfigurationService.h"
#include "version.h"

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QCoreApplication app(argc, argv);

	INIT_LOGGER(argv[0]);			// init global CircularLogger object

	logger.setLogCodeInfo(false);

	VersionInfo vi = VERSION_INFO(1, 0);

	ConfigurationServiceWorker cfgServiceWorker("RPCT Configuration Service", argc, argv, vi);

	ServiceStarter serviceStarter(app, cfgServiceWorker);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	SHUTDOWN_LOGGER

	return result;
}
