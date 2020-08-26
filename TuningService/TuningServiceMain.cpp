#include "TuningService.h"
#include "../lib/MemLeaksDetection.h"

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	QCoreApplication app(argc, argv);

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), Service::getInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	CircularLoggerShared tuningLog = std::make_shared<CircularLogger>();

	LOGGER_INIT(tuningLog, QString("Tuning"), Service::getInstanceID(argc, argv));

	tuningLog->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::TuningService, "", 1, 0);

	Tuning::TuningServiceWorker tuningServiceWorker(si, "RPCT Tuning Service", argc, argv, logger, tuningLog);

	ServiceStarter serviceStarter(app, tuningServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(tuningLog);

	LOGGER_SHUTDOWN(logger);

	dumpMemoryLeaks();

	return result;
}
