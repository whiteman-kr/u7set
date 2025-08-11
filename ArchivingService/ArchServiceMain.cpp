#include <ServiceLib/ServiceStarter.h>
#include <CommonLib/u7_vld.h>
#include "ArchivingService.h"
#include "version.h"
#include "../UtilsLib/CrashExceptionHandler.h"

// To increase time that system waiting to the service shutting down, change value:
//
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\WaitToKillServiceTimeout.
//

int main(int argc, char *argv[])
{
	Vld::setVldReportFilterHook();

	QString equipmentID = getServiceEquipmentID(argc, argv, Manufacturer::ARCHIVING_SERVICE);

#if defined (Q_OS_WIN)
	CrashExceptionHandler cdh(equipmentID);
	cdh.EnableDumping(10);
#endif

	QCoreApplication app(argc, argv);

	app.setApplicationName(Manufacturer::ARCHIVING_SERVICE);
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(QString("%1.%2.%3 (%4)").
									arg(U7SET_MAJOR_VERSION).
									arg(U7SET_MINOR_VERSION).
									arg(U7SET_PATCH_VERSION).
									arg(U7SET_BRANCH_NAME));

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), equipmentID);

	logger->setLogCodeInfo(false);

	SoftwareInfo si(E::SoftwareType::ArchiveService, "");

	ArchivingService archServiceWorker(si,
									   Service::getServiceInstanceName(Manufacturer::ARCHIVING_SERVICE, argc, argv),
									   argc, argv, logger);

	ServiceStarter serviceStarter(app, archServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
