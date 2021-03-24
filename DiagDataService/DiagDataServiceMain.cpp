#include "DiagDataService.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), Service::getInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::DiagDataService, "", 0, 0);

	DiagDataServiceWorker diagDataServiceWorker(si,
												Service::getServiceInstanceName("RPCT Diag Data Service", argc, argv),
												argc, argv, logger);

	ServiceStarter serviceStarter(app, diagDataServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
