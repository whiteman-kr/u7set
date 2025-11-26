#include "RvModelSimBridge.h"
#include "version.h"
#include <CommonStdLib/u7_vld.h>
#include <ServiceLib/ServiceStarter.h>

int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	// Add extra options to command-line params
	//
	char argID[] = "-id=ID";

	int argcExt = argc + 1;
	char** argvExt = new char*[argc + 1];
	for (int i = 0; i < argc; i++)
	{
		argvExt[i] = argv[i];
	}
	argvExt[argcExt - 1] = argID;

	QCoreApplication app(argcExt, argvExt);

	app.setApplicationName("ModelSimBridge");
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), getServiceInstanceID(argcExt, argvExt));

	logger->setLogCodeInfo(false);

	CircularLoggerShared simLogger = std::make_shared<CircularLogger>();

	LOGGER_INIT(simLogger, "RvModelSimBridgeSignals", getServiceInstanceID(argcExt, argvExt));

	SoftwareInfo si(E::SoftwareType::BaseService, "");

	ModelSimBridgeWorker worker(si,
								Service::getServiceInstanceName("ModelSimBridge", argcExt, argvExt),
								argcExt,
								argvExt,
								logger,
								simLogger);

	ServiceStarter serviceStarter(app, worker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(simLogger);

	LOGGER_SHUTDOWN(logger);

	delete argvExt;

	return result;
}
