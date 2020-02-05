#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <Builder.h>
#include <BuildTask.h>
#include "../lib/DeviceObject.h"
#include "../lib/DbController.h"
#include "../VFrame30/VFrame30Library.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>

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

void createTemplateFile(const QString& fileName)
{
	QByteArray data;

	QXmlStreamWriter writer(&data);

	writer.setCodec("UTF-8");
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("BuilderArguments");

	writer.writeComment("Postgresql IP-address");
	writer.writeTextElement("DatabaseAddress", "127.0.0.1");

	writer.writeComment("Postgresql access port");
	writer.writeTextElement("DatabasePort", "5432");

	writer.writeComment("Postgresql user name");
	writer.writeTextElement("DatabaseUserName", "u7");

	writer.writeComment("Postgresql user password");
	writer.writeTextElement("DatabasePassword", "P2ssw0rd");

	writer.writeComment("u7 project name");
	writer.writeTextElement("ProjectName", "Simulator");

	writer.writeComment("u7 project user name");
	writer.writeTextElement("ProjectUserName", "Administrator");

	writer.writeComment("u7 project user password");
	writer.writeTextElement("ProjectUserPassword", "P2ssw0rd");

	writer.writeComment("Build result path, default current directory");
	writer.writeTextElement("BuildOutputPath", "");

	writer.writeComment("Build type, debug or release, default is debug");
	writer.writeTextElement("BuildType", "debug");

	writer.writeEndElement();	// ConsoleBuilderArguments
	writer.writeEndDocument();

	QFile f(fileName);

	if (f.open(QFile::WriteOnly) == false)
	{
		QString errorMsg = QObject::tr("Failed to save file %1.").arg(fileName);
		std::cout << errorMsg.toStdString() << std::endl;
		return;
	}

	f.write(data);

	std::cout << "Arguments template has been written to: " << fileName.toStdString() << std::endl;

	return;
}

void showHelp()
{
	// Show help
	//
	std::cout << "BuilderConsole is a command-line tool that builds RPCT projects." << std::endl;
	std::cout << std::endl << "Command line parameters:" << std::endl;
	std::cout << "\tBuilderConsole <FileName.xml> - run build task with arguments taken from <FileName.xml> file" << std::endl;
	std::cout << "or" << std::endl;
	std::cout << "\tBuilderConsole [/create <FileName.xml>] - create arguments template in <FileName.xml> file" << std::endl;
	std::cout << std::endl;
	std::cout << "Example 1:" << std::endl;
	std::cout << "\tBuilderConsole.exe MyProjectBuildArgs.xml" << std::endl;
	std::cout << "Example 2:" << std::endl;
	std::cout << "\tBuilderConsole.exe /create NewProjectBuildArgs.xml" << std::endl;

	return;
}

int startBuild(QString buildArgsFileName)
{
	// Read arguments from XML document
	//
	QDomDocument doc("Document");

	QFile file(buildArgsFileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QString errorMsg = QObject::tr("Failed to open file %1.").arg(buildArgsFileName);
		std::cout << errorMsg.toStdString() << std::endl;
		return 1;
	}

	if (doc.setContent(&file) == false)
	{
		QString errorMsg = QObject::tr("Failed to load contents of the file %1.").arg(buildArgsFileName);
		std::cout << errorMsg.toStdString() << std::endl;
		file.close();
		return 1;
	}
	file.close();

	// print out the element names of all elements that are direct children
	// of the outermost element.
	//
	std::map<QString, QString> argumentsMap;

	QDomElement docElem = doc.documentElement();

	QDomNode node = docElem.firstChild();
	while(node.isNull() == false)
	{
		QDomElement e = node.toElement(); // try to convert the node to an element.
		if(e.isNull() == false)
		{
			argumentsMap[e.tagName()] = e.text();
		}
		node = node.nextSibling();
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
	BuildTask* buildTask = new BuildTask(QCoreApplication::instance());

	// Set task arguments
	//
	buildTask->setDatabaseAddress(argumentsMap[QLatin1String("DatabaseAddress")]);

	bool ok = false;
	int port = argumentsMap["DatabasePort"].toInt(&ok);
	if (ok == true)
	{
		buildTask->setDatabasePort(port);
	}
	buildTask->setDatabaseUserName(argumentsMap[QLatin1String("DatabaseUserName")]);
	buildTask->setDatabasePassword(argumentsMap[QLatin1String("DatabasePassword")]);
	buildTask->setProjectName(argumentsMap[QLatin1String("ProjectName")]);
	buildTask->setProjectUserName(argumentsMap[QLatin1String("ProjectUserName")]);
	buildTask->setProjectUserPassword(argumentsMap[QLatin1String("ProjectUserPassword")]);

	QString buildOutputPath = argumentsMap[QLatin1String("BuildOutputPath")];
	if (buildOutputPath.isEmpty() == false)
	{
		buildTask->setBuildOutputPath(buildOutputPath);
	}
	else
	{
		buildTask->setBuildType(QLatin1String("."));
	}

	QString buildType = argumentsMap[QLatin1String("BuildType")];
	if (buildType.isEmpty() == false)
	{
		buildTask->setBuildType(buildType);
	}
	else
	{
		buildTask->setBuildType(QLatin1String("debug"));
	}

	// This will cause the application to exit when
	// the buildTask signals finished.
	//
	QObject::connect(buildTask, &BuildTask::finished, QCoreApplication::instance(), &QCoreApplication::exit);

	// Start build process
	//
	buildTask->start();

	// Run message loop
	//
	int result = QCoreApplication::instance()->exec();

	// Shutting down
	//
	Builder::shutdown();
	DbController::shutdown();
	Hardware::shutdown();
	VFrame30::VFrame30Library::shutdown();

	return result;
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

	QStringList args = a.arguments();

	switch (args.size())
	{
		case 2:
			{
				int result = startBuild(args[1]);
				return result;
			}

		case 3:
			// Create a template file?
			//
			if (args[1].trimmed().compare(QLatin1String("/create"), Qt::CaseInsensitive) == 0)
			{
				createTemplateFile(args[2]);
				return 0;
			}
			else
			{
				showHelp();
				return 1;
			}

		default:
			showHelp();
			return 2;
	}
}
