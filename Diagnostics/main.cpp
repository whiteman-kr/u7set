#include "DiagnosticsAppSettings.h"
#include "DiagnosticsMainWindow.h"
#include "Globals.h"
#include "version.h"

#include <CommonLib/ConstStrings.h>
#include <CommonStdLib/u7_vld.h>
#include <HardwareLib/HardwareLibrary.h>
#include <UiLib/OverrideWindows11Style.h>
#include <VFrame30/VFrame30Library.h>


int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	QApplication a(argc, argv);

	UiLib::OverrideWindows11Style(a, argc, argv);

	// --
	//
	a.setApplicationName("Diagnostics3");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	// --
	//
	qDebug() << "GUI Thread ID: " << QThread::currentThreadId();

	qRegisterMetaType<TimeStamp>();
	qRegisterMetaType<TimeSpan>();

	// Parse command line
	//
	QStringList arguments = a.arguments();
	QString settingsFileName;

	if (arguments.size() > 1)
	{
		settingsFileName = arguments[1];
	}

	if (settingsFileName.isEmpty() == false && QFile::exists(settingsFileName) == false)
	{
		QMessageBox::critical(nullptr, qAppName(), QObject::tr("Application settings file %1 is not exist.").arg(settingsFileName));
		return 1;
	}

	// Read settings
	//
	if (settingsFileName.isEmpty() == true)
	{
		DiagnosticsAppSettings::instance().restore();
	}
	else
	{
		bool loadSettingsOk = DiagnosticsAppSettings::instance().loadFromFile(settingsFileName);
		if (loadSettingsOk == false)
		{
			QMessageBox::critical(nullptr,
								  qAppName(),
								  QObject::tr("Error loading application settings from file %1.").arg(settingsFileName));
			return 1;
		}
	}

	// Set application name again, as new app caption could be assign via settings.
	// Set application name so all message boxes will have correct caption.
	//
	a.setApplicationName(DiagnosticsAppSettings::instance().windowCaption());

	// Init TrendLib resources
	//
	Q_INIT_RESOURCE(TrendView);

	// --
	//
	VFrame30::init();
	Hardware::init();

	SoftwareInfo softwareInfo(E::SoftwareType::Diagnostics, DiagnosticsAppSettings::instance().equipmentId());

	// --
	//
	auto settings = DiagnosticsAppSettings::instance().get();

	InstanceResolver instanceResolver;

	if (bool ok = instanceResolver.init(settings.equipmentId, settings.singleInstance); ok == false)
	{
		qDebug() << "Another instance is active";
		return 0;
	}

	// --
	//
	int result = 0;
	{
		DiagnosticsMainWindow mainWindow(instanceResolver, softwareInfo);

		theApp.setMainWindow(&mainWindow);

		mainWindow.show();

		// --
		//
		result = a.exec();

		theApp.setMainWindow(nullptr);
	}

	// Shutting down
	//

	VFrame30::shutdown();
	Hardware::shutdown();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
