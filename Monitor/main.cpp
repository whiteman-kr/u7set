#include "Globals.h"
#include "MonitorAppSettings.h"
#include "MonitorMainWindow.h"
#include "version.h"

#include <CommonStdLib/u7_vld.h>
#include <HardwareLib/HardwareLibrary.h>
#include <UiLib/OverrideWindows11Style.h>
#include <VFrame30/VFrame30Library.h>

#include <grpcpp/grpcpp.h>


int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	// Initialize gRPC early so its singleton allocations happen before main logic.
#ifdef VLD_IS_INCLUDED
	VLDDisable();
#endif

	grpc_init();
	[[maybe_unused]] const auto clientLocalMs = currentMSecsLocal(); // A call to this function that is the first reference to the time zone
																	 // database will cause it to be initialized. This suppress VLD false-positive memory leak detection.

#ifdef VLD_IS_INCLUDED
	VLDEnable();
#endif

	QApplication a(argc, argv);

	// Override Windows11 style, the current implementation does not look well.
	//
	UiLib::OverrideWindows11Style(a, argc, argv);

	// --
	//
	a.setApplicationName("Monitor3");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	// --
	//
	qDebug() << "main(), GUI ThreadID: " << QThread::currentThreadId();

	qRegisterMetaType<TimeStamp>();
	qRegisterMetaType<TimeSpan>();

	// Parse command line
	//
	QStringList arguments = a.arguments();
	QString settingsFileName;

	if (arguments.size() > 1)
	{
		for (int i = 1; i < arguments.size(); i++)
		{
			if (arguments[i].startsWith('-') == true) // Skip '-' options
			{
				continue;
			}

			settingsFileName = arguments[i];
			break;
		}
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
		MonitorAppSettings::instance().restore();
	}
	else
	{
		bool loadSettingsOk = MonitorAppSettings::instance().loadFromFile(settingsFileName);
		if (loadSettingsOk == false)
		{
			QMessageBox::critical(nullptr,
								  qAppName(),
								  QObject::tr("Error loading application settings from file %1.").arg(settingsFileName));
			return 1;
		}
	}

	// No-Disk-Log option
	//
	if (arguments.contains("--no-disk-log", Qt::CaseInsensitive) == true)
	{
		auto data = MonitorAppSettings::instance().get();
		data.noDiskLog = true;
		MonitorAppSettings::instance().set(data);
	}


	// Set application name again, as new app caption could be assign via settings.
	// Set application name so all message boxes will have correct caption.
	//
	a.setApplicationName(MonitorAppSettings::instance().windowCaption());

	// Init TrendLib resources
	//
	Q_INIT_RESOURCE(TrendView);

	// --
	//
	VFrame30::init();
	Hardware::init();

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, MonitorAppSettings::instance().equipmentId());

	// --
	//
	auto settings = MonitorAppSettings::instance().get();

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
		MonitorMainWindow mainWindow(instanceResolver, softwareInfo);

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

	// Shutdown gRPC AFTER all channels/stubs have been destroyed (main window scope ended)
	grpc_shutdown();

	return result;
}
