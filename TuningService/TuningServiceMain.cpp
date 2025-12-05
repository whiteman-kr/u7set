#include <CommonStdLib/u7_vld.h>
#include <ServiceLib/ServiceStarter.h>
#include "TuningService.h"
#include "version.h"
#include "../UtilsLib/CrashExceptionHandler.h"

int main(int argc, char *argv[])
{
	Vld::setVldReportFilterHook();

	QString equipmentID = getServiceEquipmentID(argc, argv, Manufacturer::TUNING_SERVICE);

#if defined (Q_OS_WIN)
	CrashExceptionHandler cdh(equipmentID);
	cdh.EnableDumping(10);
#endif

	QCoreApplication app(argc, argv);

	app.setApplicationName(Manufacturer::TUNING_SERVICE);
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(QString("%1.%2.%3 (%4)").
									arg(U7SET_MAJOR_VERSION).
									arg(U7SET_MINOR_VERSION).
									arg(U7SET_PATCH_VERSION).
									arg(U7SET_BRANCH_NAME));

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), equipmentID);

	logger->setLogCodeInfo(false);

	CircularLoggerShared tuningLog = std::make_shared<CircularLogger>();

	LOGGER_INIT(tuningLog, QString("Tuning"), equipmentID);

	tuningLog->setLogCodeInfo(false);

	SoftwareInfo si(E::SoftwareType::TuningService, "");

	Tuning::TuningServiceWorker tuningServiceWorker(si,
													Service::getServiceInstanceName(Manufacturer::TUNING_SERVICE, argc, argv),
													argc, argv, logger,
													tuningLog);

	ServiceStarter serviceStarter(app, tuningServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(tuningLog);

	LOGGER_SHUTDOWN(logger);

	return result;
}
