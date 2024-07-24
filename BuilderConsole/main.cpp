#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QXmlStreamWriter>
#include <iostream>

#include "../Builder/Builder.h"
#include <google/protobuf/message_lite.h>
#include "../version.h"
#include "BuildTask.h"

#include <DbLib/DbController.h>
#include <HardwareLib/HardwareLibrary.h>
#include <VFrame30/VFrame30Library.h>

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
	writer.writeTextElement("DatabasePassword", "Password");

	writer.writeComment("u7 project name");
	writer.writeTextElement("ProjectName", "ProjectName");

	writer.writeComment("u7 project user name");
	writer.writeTextElement("ProjectUserName", "Administrator");

	writer.writeComment("u7 project user password");
	writer.writeTextElement("ProjectUserPassword", "Password");

	writer.writeComment("Build result path, default current directory");
	writer.writeTextElement("BuildOutputPath", "");

	writer.writeEndElement(); // ConsoleBuilderArguments
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
	std::cout << std::endl
			  << "Command line parameters:" << std::endl;
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

bool getArgumentFromXml(QDomElement& docElem, QString name, QString* result)
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return false;
	}

	QDomNodeList softwareNodes = docElem.elementsByTagName(name);
	if (softwareNodes.size() != 1)
	{
		return false;
	}


	QDomElement elem = softwareNodes.item(0).toElement();
	*result = elem.text();

	return true;
}

bool getArgumentFromXml(QDomElement& docElem, QString name, int* result)
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return false;
	}

	QString str;
	if (getArgumentFromXml(docElem, name, &str) == false)
	{
		return false;
	}

	bool ok = false;
	*result = str.toInt(&ok);

	return ok;
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

	if (static_cast<bool>(doc.setContent(&file)) == false)
	{
		QString errorMsg = QObject::tr("Failed to load contents of the file %1.").arg(buildArgsFileName);
		std::cout << errorMsg.toStdString() << std::endl;
		file.close();
		return 1;
	}
	file.close();

	// Read and set task arguments
	//
	QDomElement docElem = doc.documentElement();

	// DatabaseAddress
	//
	QString dbAddress;
	bool ok = getArgumentFromXml(docElem, "DatabaseAddress", &dbAddress);
	if (ok == false)
	{
		std::cout << "Failed to read DatabaseAddress argument from file!" << std::endl;
		return 1;
	}
	if (dbAddress.isEmpty() == true)
	{
		std::cout << "DatabaseAddress argument can't be empty!" << std::endl;
		return 1;
	}

	// DatabasePort
	//
	int port = 0;
	ok = getArgumentFromXml(docElem, "DatabasePort", &port);
	if (ok == false)
	{
		std::cout << "Failed to read DatabasePort argument from file!" << std::endl;
		return 1;
	}

	// DatabaseUserName
	//
	QString dbUserName;
	ok = getArgumentFromXml(docElem, "DatabaseUserName", &dbUserName);
	if (ok == false)
	{
		std::cout << "Failed to read DatabaseUserName argument from file!" << std::endl;
		return 1;
	}
	if (dbUserName.isEmpty() == true)
	{
		std::cout << "DatabaseUserName argument can't be empty!" << std::endl;
		return 1;
	}

	// DatabasePassword
	//
	QString dbPassword;
	ok = getArgumentFromXml(docElem, "DatabasePassword", &dbPassword);
	if (ok == false)
	{
		std::cout << "Failed to read DatabasePassword argument from file!" << std::endl;
		return 1;
	}
	if (dbPassword.isEmpty() == true)
	{
		std::cout << "DatabasePassword argument can't be empty!" << std::endl;
		return 1;
	}

	// ProjectName
	//
	QString projectName;
	ok = getArgumentFromXml(docElem, "ProjectName", &projectName);
	if (ok == false)
	{
		std::cout << "Failed to read ProjectName argument from file!" << std::endl;
		return 1;
	}
	if (projectName.isEmpty() == true)
	{
		std::cout << "ProjectName argument can't be empty!" << std::endl;
		return 1;
	}

	// ProjectUserName
	//
	QString projectUserName;
	ok = getArgumentFromXml(docElem, "ProjectUserName", &projectUserName);
	if (ok == false)
	{
		std::cout << "Failed to read ProjectUserName argument from file!" << std::endl;
		return 1;
	}
	if (projectUserName.isEmpty() == true)
	{
		std::cout << "ProjectUserName argument can't be empty!" << std::endl;
		return 1;
	}

	// ProjectUserPassword
	//
	QString projectUserPassword;
	ok = getArgumentFromXml(docElem, "ProjectUserPassword", &projectUserPassword);
	if (ok == false)
	{
		std::cout << "Failed to read ProjectUserPassword argument from file!" << std::endl;
		return 1;
	}
	if (projectUserPassword.isEmpty() == true)
	{
		std::cout << "ProjectUserPassword argument can't be empty!" << std::endl;
		return 1;
	}

	// BuildOutputPath
	//
	QString buildPath;
	ok = getArgumentFromXml(docElem, "BuildOutputPath", &buildPath);
	if (ok == false)
	{
		std::cout << "Failed to read BuildOutputPath argument from file!" << std::endl;
		return 1;
	}

	// Some inititializations
	//
	VFrame30::init();
	Hardware::init();
	DbController::init();
	Builder::init();

	BuildTask* buildTask = new BuildTask(nullptr /* QCoreApplication::instance() */);

	buildTask->setDatabaseAddress(dbAddress);
	buildTask->setDatabasePort(port);
	buildTask->setDatabaseUserName(dbUserName);
	buildTask->setDatabasePassword(dbPassword);
	buildTask->setProjectName(projectName);
	buildTask->setProjectUserName(projectUserName);
	buildTask->setProjectUserPassword(projectUserPassword);
	if (buildPath.isEmpty() == false)
	{
		buildTask->setBuildOutputPath(buildPath);
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

	delete buildTask;

	// Shutting down
	//
	Builder::shutdown();
	DbController::shutdown();
	VFrame30::shutdown();
	Hardware::shutdown();

	return result;
}

int main(int argc, char* argv[])
{
	originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication a(argc, argv);

	// --
	//
	a.setApplicationName("BuilderConsole");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(QString("%1.%2.%3 (%4)")
								.arg(U7SET_MAJOR_VERSION)
								.arg(U7SET_MINOR_VERSION)
								.arg(U7SET_PATCH_VERSION)
								.arg(U7SET_BRANCH_NAME));

	QStringList args = a.arguments();

	int result = 0;

	switch (args.size())
	{
	case 2:
		result = startBuild(args[1]);
		break;

	case 3:
		// Create a template file?
		//
		if (args[1].trimmed().compare(QLatin1String("/create"), Qt::CaseInsensitive) == 0)
		{
			createTemplateFile(args[2]);
			result = 0;
		}
		else
		{
			showHelp();
			result = 1;
		}
		break;

	default:
		showHelp();
		result = 2;
	}

	google::protobuf::ShutdownProtobufLibrary();
	return result;
}
