#include "DiagDataService.h"
#include "../lib/MemLeaksDetection.h"

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::DiagDataService, "", 0, 0);

	DiagDataServiceWorker diagDataServiceWorker(si, "RPCT Diag Data Service", argc, argv, logger);

	ServiceStarter serviceStarter(app, diagDataServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	dumpMemoryLeaks();

	return result;
}
