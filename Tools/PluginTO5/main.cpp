#include "PluginTO5MainWindow.h"
#include <CommonLib/ConstStrings.h>
#include <CommonStdLib/u7_vld.h>
#include <QApplication>


int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	QApplication app(argc, argv);

	app.setApplicationName("PluginTO5");
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);
	//app.setApplicationVersion(
	//	QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	PluginTO5MainWindow w;
	w.show();

	return app.exec();
}
