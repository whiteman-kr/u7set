#include "../UtilsLib/SimpleThread.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../VFrame30/VFrame30Library.h"
#include "MonitorAppSettings.h"
#include "MonitorMainWindow.h"
#include "MonitorConfigController.h"
#include "TcpSignalClient.h"

#if __has_include("../gitlabci_version.h")
#include "../gitlabci_version.h"
#endif

AppSignalManager theSignals;
TuningSignalManager theTuningSignals;

MonitorMainWindow* theMonitorMainWindow = nullptr;


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("Monitor3");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("3.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("3.8.LOCALBUILD"));
#endif

	// --
	//
	qDebug() << "GUI Thread ID: " << QThread::currentThreadId();

	qRegisterMetaType<TimeStamp>();
	qRegisterMetaType<TimeSpan>();

	// Parse command line
	//
	QStringList aruments = a.arguments();
	QString settingsFileName;

	if (aruments.size() > 1)
	{
		settingsFileName = aruments[1];
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

	// Init TrendLib resources
	//
	Q_INIT_RESOURCE(TrendView);

	// --
	//
	VFrame30::init();
	//Hardware::Init();

	SoftwareInfo si;
	si.init(E::SoftwareType::Monitor, MonitorAppSettings::instance().equipmentId(), 0, 1);

	// --
	//
	auto settings = MonitorAppSettings::instance().get();

	InstanceResolver instanceResover;

	if (bool ok = instanceResover.init(settings.equipmentId, settings.singleInstance);
		ok == false)
	{
		qDebug() << "Another instance is active";
		return 0;
	}

	// --
	//
	int result = 0;
	{
		MonitorMainWindow w(instanceResover, si);
		theMonitorMainWindow = &w;
		w.show();

		// --
		//
		result = a.exec();
	}

	// Shutting down
	//

	VFrame30::shutdown();
	//Hardware::Shutdwon();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
