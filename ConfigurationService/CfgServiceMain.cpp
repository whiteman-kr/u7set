#include "ConfigurationService.h"
#include "version.h"

#define CIRCULAR_LOGGER_PTR_ASSERTING

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::ConfigurationService, "", 1, 0);

	ConfigurationServiceWorker cfgServiceWorker(si, "RPCT Configuration Service", argc, argv, logger);

	ServiceStarter serviceStarter(app, cfgServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger)

	return result;
}
