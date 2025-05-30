#include <ServiceLib/ServiceStarter.h>
#include "AppDataService.h"
#include "version.h"
#include "../UtilsLib/CrashExceptionHandler.h"

int main(int argc, char *argv[])
{
	QString equipmentID = getServiceEquipmentID(argc, argv, Manufacturer::APPLICATION_DATA_SERVICE);

#if defined (Q_OS_WIN)
	CrashExceptionHandler cdh(equipmentID);
	cdh.EnableDumping(10);
#endif

	QCoreApplication app(argc, argv);

	app.setApplicationName(Manufacturer::APPLICATION_DATA_SERVICE);
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(QString("%1.%2.%3 (%4)").
									arg(U7SET_MAJOR_VERSION).
									arg(U7SET_MINOR_VERSION).
									arg(U7SET_PATCH_VERSION).
									arg(U7SET_BRANCH_NAME));

	QThread::currentThread()->setObjectName("MainThread");

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), equipmentID);

	logger->setLogCodeInfo(false);

	SoftwareInfo si(E::SoftwareType::AppDataService, "");

	AppDataServiceWorker appDataServiceWorker(si,
											  Service::getServiceInstanceName(Manufacturer::APPLICATION_DATA_SERVICE, argc, argv),
											  argc, argv, logger);

	ServiceStarter serviceStarter(app, appDataServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
