#include "MetrologyMainWindow.h"
#include "Options.h"
#include <CommonStdLib/u7_vld.h>
#include <UiLib/OverrideWindows11Style.h>
#include "version.h"
#include "../UtilsLib/CrashExceptionHandler.h"
#include "../OnlineLib/CircularLogger.h"

#include <QApplication>
#include <QLockFile>

int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

#if defined (Q_OS_WIN)
	CrashExceptionHandler cdh;
	cdh.EnableDumping(10);
#endif

	QApplication app(argc, argv);

	// Override Windows11 style, the current implementation does not look well.
	//
	UiLib::OverrideWindows11Style(app, argc, argv);

	app.setApplicationName(Manufacturer::METROLOGY);
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));
	theOptions.load();

	// select language
	//

	theOptions.loadTranslators();

	// one instance of the application
	//
	QLockFile lockFile(QDir::temp().absoluteFilePath("Metrology.lock"));

	if (lockFile.tryLock(100) == false)
	{
		QMessageBox::information(nullptr, app.applicationName(), app.translate("MetrologyMain", "The application is already running!"));
		return 1;
	}

	// init SoftwareInfo
	//
	QString equipmentID = theOptions.socket().server(OT::ServerType::ConfigurationService).equipmentID(OT::ServerPriority::Primary);

	SoftwareInfo si(E::SoftwareType::Metrology, equipmentID);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), equipmentID);

	logger->setLogCodeInfo(false);

	// in order to keep the dumpMemoryLeaks() list clean, the MainWindow is created using "new".
	// MainWindow w(si);
	// w.show();
	//
	MainWindow* pMainWindow = new MainWindow(si, logger);
	pMainWindow->show();

	int result = app.exec();

	delete pMainWindow;

	LOGGER_SHUTDOWN(logger);

	google::protobuf::ShutdownProtobufLibrary();

	grpc_shutdown();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
