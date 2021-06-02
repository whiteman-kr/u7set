#include "ConfigurationService.h"

#define CIRCULAR_LOGGER_PTR_ASSERTING

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), Service::getInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::ConfigurationService, "", 1, 0);

	ConfigurationServiceWorker cfgServiceWorker(si,
												Service::getServiceInstanceName("Configuration Service", argc, argv),
												argc, argv, logger,
												E::ServiceRunMode::ConsoleApp);	// run mode will be refined after cmd line processing

	ServiceStarter serviceStarter(app, cfgServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
