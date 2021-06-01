#include "TuningService.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), Service::getInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	CircularLoggerShared tuningLog = std::make_shared<CircularLogger>();

	LOGGER_INIT(tuningLog, QString("Tuning"), Service::getInstanceID(argc, argv));

	tuningLog->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::TuningService, "", 1, 0);

	Tuning::TuningServiceWorker tuningServiceWorker(si,
													Service::getServiceInstanceName("Tuning Service", argc, argv),
													argc, argv, logger,
													E::ServiceRunMode::ConsoleApp,	// run mode will be refined after cmd line processing
													tuningLog);

	ServiceStarter serviceStarter(app, tuningServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(tuningLog);

	LOGGER_SHUTDOWN(logger);

	return result;
}
