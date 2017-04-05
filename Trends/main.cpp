#include "TrendsMainWindow.h"
#include "Settings.h"

int main(int argc, char *argv[])
{
	//QCoreApplication::setAttribute(Qt::AA_Use96Dpi);
	//QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	//QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication a(argc, argv);

	a.setApplicationName("Trends 1.0");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	// --
	//
	qDebug() << "GUI Thread ID: " << QThread::currentThreadId();

	// Read settings
	//
	theSettings.load();

	// --
	//
	TrendsMainWindow w;
	w.show();

	return a.exec();
}
