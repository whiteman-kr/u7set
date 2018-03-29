#include <QApplication>

#include "MainWindow.h"
#include "Options.h"
//#include "version.h"
//#include <vld.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("Packet Source");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	//a.setApplicationVersion(QString("1.0.%1 (%2)").arg(USED_SERVER_COMMIT_NUMBER).arg(BUILD_BRANCH));

	theOptions.load();

	MainWindow w;
	w.show();

	int result = a.exec();

	theOptions.unload();

	return result;
}
