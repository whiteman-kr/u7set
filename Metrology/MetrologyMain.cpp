#include "MetrologyMainWindow.h"
#include "Options.h"
#include <CommonLib/u7_vld.h>
#include <UiLib/OverrideWindows11Style.h>
#include "version.h"
#include "../UtilsLib/CrashExceptionHandler.h"

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
	QTranslator translator;

	if (theOptions.language().languageType() == OT::LanguageType::Russian)
	{
		if (translator.load(QString(":%1/%2").arg(LANGUAGE_OPTIONS_DIR, LANGUAGE_OPTIONS_FILE_RU)) == true)
		{
			qApp->installTranslator(&translator);
		}
		else
		{
			QString languageFilePath = QApplication::applicationDirPath() + LANGUAGE_OPTIONS_DIR + "/" + LANGUAGE_OPTIONS_FILE_RU;
			QMessageBox::critical(nullptr, "Russian language", QString("Didn't load russian language:\n%1").arg(languageFilePath));
			theOptions.language().setLanguageType(OT::LanguageType::English);
		}
	}

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

	// in order to keep the dumpMemoryLeaks() list clean, the MainWindow is created using "new".
	// MainWindow w(si);
	// w.show();
	//
	MainWindow* pMainWindow = new MainWindow(si);
	pMainWindow->show();

	int result = app.exec();

	delete pMainWindow;

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
