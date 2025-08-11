#include <CommonLib/ConstStrings.h>
#include <CommonLib/u7_vld.h>

#include "LicenserMainWindow.h"
#include "version.h"

#include <QApplication>

int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	QApplication app(argc, argv);

	QCoreApplication::setOrganizationName(Manufacturer::RADIY);
	QCoreApplication::setOrganizationDomain(Manufacturer::SITE);
	QCoreApplication::setApplicationName("Licenser");

	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	// Init LicenseLib resources
	//
	Q_INIT_RESOURCE(LicenseLib);

	LicenserMainWindow window;
	window.show();

	return app.exec();
}
