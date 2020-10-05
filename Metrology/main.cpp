#include "MainWindow.h"
#include "Options.h"
#include "../lib/ProtoSerialization.h"
#include "../lib/MemLeaksDetection.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

    QApplication a(argc, argv);

    a.setApplicationName("Metrology");
    a.setOrganizationName("Radiy");
    a.setOrganizationDomain("radiy.com");

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("1.10.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("1.10.LOCALBUILD"));
#endif

	QTranslator translator;
	if (translator.load("Metrology_ru.qm", QApplication::applicationDirPath() + "/translations") == false)
	{
		qDebug() << "Options::loadLanguage() - didn't load language file";
	}
	qApp->installTranslator(&translator);

    theOptions.load();

	SoftwareInfo si;

	QString equipmentID = theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY);
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

	dumpMemoryLeaks();

    return result;
}

// -------------------------------------------------------------------------------------------------------------------
