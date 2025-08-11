#include "MainWindow.h"
#include "version.h"
#include <CommonLib/u7_vld.h>
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/CrashExceptionHandler.h"


int main(int argc, char *argv[])
{
	Vld::setVldReportFilterHook();

#if defined (Q_OS_WIN)
	CrashExceptionHandler cdh;
	cdh.EnableDumping(10);
#endif

	QApplication app(argc, argv);

	app.setApplicationName(Manufacturer::SERVICE_CONTROL_MANAGER);
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(QString("%1.%2.%3 (%4)").
									arg(U7SET_MAJOR_VERSION).
									arg(U7SET_MINOR_VERSION).
									arg(U7SET_PATCH_VERSION).
									arg(U7SET_BRANCH_NAME));

	app.setWindowIcon(QIcon(":/images/SearchComputer.png"));

    QSettings settings;

    QString locale = settings.value("locale", QLocale::system().name()).toString().left(2);

	QTranslator* qtTranslator = nullptr;
	QTranslator* qtbaseTranslator = nullptr;
	QTranslator* appTranslator = nullptr;

    if (locale != "en")
    {
		bool loadResult = true;

		qtTranslator = new QTranslator(qApp);
		loadResult &= qtTranslator->load(QString("qt_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(qtTranslator);

		qtbaseTranslator = new QTranslator(qApp);
		loadResult &= qtbaseTranslator->load(QString("qtbase_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(qtbaseTranslator);

		appTranslator = new QTranslator(qApp);
		loadResult &= appTranslator->load(QString("ServiceControlManager_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(appTranslator);

		Q_UNUSED(loadResult);
    }

	SoftwareInfo si(E::SoftwareType::ServiceControlManager, "SCM");

	MainWindow* mainWindow = new MainWindow(si);

	mainWindow->showMaximized();

	int result = app.exec();

	delete mainWindow;

	DELETE_IF_NOT_NULL(qtTranslator);
	DELETE_IF_NOT_NULL(qtbaseTranslator);
	DELETE_IF_NOT_NULL(appTranslator);

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
