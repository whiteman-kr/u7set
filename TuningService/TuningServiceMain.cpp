#include "TuningService.h"
#include "version.h"


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QCoreApplication app(argc, argv);

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	CircularLoggerShared tuningLog = std::make_shared<CircularLogger>();

	LOGGER_INIT2(tuningLog, QString("Tuning"));

	tuningLog->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::TuningService, "", 1, 0);

	Tuning::TuningServiceWorker tuningServiceWorker(si, "RPCT Tuning Service", argc, argv, logger, tuningLog);

	ServiceStarter serviceStarter(app, tuningServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(tuningLog);

	LOGGER_SHUTDOWN(logger);

	return result;
}
