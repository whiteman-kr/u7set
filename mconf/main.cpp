#include "ModuleConfigurator.h"
#include "version.h"

#include <CommonLib/ConstStrings.h>
#include <LicenseLib/AppLicenser.h>
#include <UiLib/OverrideWindows11Style.h>

#include <google/protobuf/stubs/common.h>

#include <QtWidgets/QApplication>

Q_DECLARE_METATYPE(std::vector<quint8>)

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	// Override Windows11 style, the current implementation does not look well.
	//
	UiLib::OverrideWindows11Style(a, argc, argv);

	QCoreApplication::setOrganizationName(Manufacturer::RADIY);
	QCoreApplication::setOrganizationDomain(Manufacturer::SITE);
	QCoreApplication::setApplicationName("ModuleConfigurator");

	a.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	// Load license
	//
	Q_INIT_RESOURCE(LicenseLib); // Init LicenseLib resources

	if (LicenseLib::AppLicenser::guiAppStartValidation(QDateTime::fromSecsSinceEpoch(U7SET_BUILD_DATE_SECONDS).date()) == false)
	{
		return EXIT_FAILURE;
	}

	qRegisterMetaType<std::vector<quint8>>();

	ModuleConfigurator w;
	w.show();
	int result = a.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
