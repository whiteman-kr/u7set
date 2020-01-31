#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <Builder.h>
#include <BuildTask.h>
#include "../lib/DeviceObject.h"
#include "../lib/DbController.h"
#include "../VFrame30/VFrame30Library.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

static QtMessageHandler originalMessageHandler = 0;

void messageOutputHandler(QtMsgType /*type*/, const QMessageLogContext& /*context*/, const QString& /*msg*/)
{
	// Do nothing, build process has some debug messages (qDebug),
	// but we want to show only build log items, which comes via std::cout
	//
	return;
}

int main(int argc, char *argv[])
{
	originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication a(argc, argv);

	// --
	//
	a.setApplicationName("BuilderConsole");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.8.LOCALBUILD"));
#endif

	// Example:
	//	BuilderConsole DatabaseAddress DatabaseUserName DatabasePassword ProjectName ProjectUserName ProjectUserPassword
	//
	QStringList args = a.arguments();

	if (args.size() < 8 || args.size() > 10)
	{
		std::cout << "Wrong argument count." << std::endl;
		std::cout << "Arguments:" << std::endl;
		std::cout << "\tBuilderConsole DatabaseAddress DatabasePort DatabaseUserName DatabasePassword ProjectName ProjectUserName ProjectUserPassword [BuildOutputPath] [BuildType]" << std::endl;
		std::cout << "\tDatabaseAddress: Postgresql IP-address\n" <<
					 "\tDatabasePort: Postgresql access port\n" <<
					 "\tDatabaseUserName: Postgresql user name\n" <<
					 "\tDatabasePassword: Postgresql user password\n" <<
					 "\tProjectName: u7 project name\n" <<
					 "\tProjectUserName: u7 project user name\n" <<
					 "\tProjectUserPassword: u7 project user password\n"
					 "\tBuildOutputPath: Build result path, default current directory\n" <<
					 "\tBuildType: Build type, debug or release, default debug" << std::endl;
		std::cout << "Example:" << std::endl;
		std::cout << "\tBuilderConsole.exe 127.0.0.1 5432 u7 P2ssw0rd Simulator Administrator P2ssw0rd" << std::endl;

		return 1;
	}

	// Some inititializations
	//
	VFrame30::VFrame30Library::init();
	Hardware::init();
	DbController::init();
	Builder::init();

	// Task parented to the application so that it
	// will be deleted by the application.
	//
	BuildTask* buildTask = new BuildTask(&a);

	buildTask->setDatabaseAddress(args[1]);
	buildTask->setDatabasePort(args[2].toInt());
	buildTask->setDatabaseUserName(args[3]);
	buildTask->setDatabasePassword(args[4]);
	buildTask->setProjectName(args[5]);
	buildTask->setProjectUserName(args[6]);
	buildTask->setProjectUserPassword(args[7]);

	if (args.size() >= 9)
	{
		// Optional param
		//
		buildTask->seBuildOutputPath(args[8]);
	}
	else
	{
		buildTask->seBuildOutputPath(".");
	}

	if (args.size() >= 10)
	{
		// Optional param
		//
		buildTask->setBuildType(args[9]);
	}
	else
	{
		buildTask->setBuildType("Debug");
	}

	// This will cause the application to exit when
	// the buildTask signals finished.
	//
	QObject::connect(buildTask, &BuildTask::finished, &a, &QCoreApplication::exit);

	// Start build process
	//
	buildTask->start();

	// Run message loop
	//
	int result = a.exec();

	// Shutting down
	//
	Builder::shutdown();
	DbController::shutdown();
	Hardware::shutdown();
	VFrame30::VFrame30Library::shutdown();

	return result;
}
