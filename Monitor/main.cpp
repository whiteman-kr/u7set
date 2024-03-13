#include "Globals.h"
#include "MonitorAppSettings.h"
#include "MonitorMainWindow.h"
#include "version.h"

#include <HardwareLib/HardwareLibrary.h>
#include "../VFrame30/VFrame30Library.h"


int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("Monitor3");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(QString("%1.%2.%3 (%4)")
								.arg(U7SET_MAJOR_VERSION)
								.arg(U7SET_MINOR_VERSION)
								.arg(U7SET_PATCH_VERSION)
								.arg(U7SET_BRANCH_NAME));

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
		MonitorAppSettings::instance().restore();
	}
	else
	{
		bool loadSettingsOk = MonitorAppSettings::instance().loadFromFile(settingsFileName);
		if (loadSettingsOk == false)
		{
			QMessageBox::critical(nullptr, qAppName(), QObject::tr("Error loading application settings from file %1.").arg(settingsFileName));
			return 1;
		}
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

	if (bool ok = instanceResolver.init(settings.equipmentId, settings.singleInstance);
		ok == false)
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

	return result;
}
