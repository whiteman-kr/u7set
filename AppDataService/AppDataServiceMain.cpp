#include "AppDataService.h"
#include "../lib/MemLeaksDetection.h"

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), Service::getInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::AppDataService, "", 1, 0);

	AppDataServiceWorker appDataServiceWorker(si,
											  Service::getServiceInstanceName("Application Data Service", argc, argv),
											  argc, argv, logger);

	ServiceStarter serviceStarter(app, appDataServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	dumpMemoryLeaks();

	return result;
}
