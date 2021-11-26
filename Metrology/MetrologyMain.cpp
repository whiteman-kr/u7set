#include "MetrologyMainWindow.h"
#include "Options.h"

#include "../Proto/ProtoSerialization.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("Metrology");
    a.setOrganizationName(Manufacturer::RADIY);
    a.setOrganizationDomain(Manufacturer::SITE);

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("2.0.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("2.0.LOCALBUILD"));
#endif

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

	if(lockFile.tryLock(100) == false)
	{
		QMessageBox::information(nullptr, a.applicationName(), a.translate("MetrologyMain", "The application is already running!"));
		return 1;
	}

	// init SoftwareInfo
	//
	SoftwareInfo si;

	QString equipmentID = theOptions.socket().server(OT::ServerType::ConfigurationService).equipmentID(OT::ServerPriority::Primary);
	si.init(E::SoftwareType::Metrology, equipmentID, 1, 0);

	// in order to keep the dumpMemoryLeaks() list clean, the MainWindow is created using "new".
	// MainWindow w(si);
	// w.show();
	//
	MainWindow* pMainWindow = new MainWindow(si);
	pMainWindow->show();

    int result = a.exec();

	delete pMainWindow;

	google::protobuf::ShutdownProtobufLibrary();

    return result;
}

// -------------------------------------------------------------------------------------------------------------------
