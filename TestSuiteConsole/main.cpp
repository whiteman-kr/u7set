#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include "TestTask.h"
#include "../TestSuiteLib/TestLibrary.h"
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

	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("TestSuiteConsoleArguments");


	writer.writeComment("TestSuite InstanceStrID");
	writer.writeTextElement("InstanceStrID", "SYSTEMID_RACKID_WS00_TESTSUITE");

	writer.writeComment("Configurator IP Address 1");
	writer.writeTextElement("ConfiguratorIPAddress1", "127.0.0.1");

	writer.writeComment("Configurator Port 1");
	writer.writeTextElement("ConfiguratorPort1", "13312");

	writer.writeComment("Configurator IP Address 2");
	writer.writeTextElement("ConfiguratorIPAddress2", "127.0.0.1");

	writer.writeComment("Configurator Port 2");
	writer.writeTextElement("ConfiguratorPort2", "13312");

	writer.writeEndElement();	// TestSuiteConsoleArguments
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
	std::cout << "TestSuiteConsole is a command-line tool that performs hardware testing of RPCT projects." << std::endl;
	std::cout << std::endl << "Command line parameters:" << std::endl;
	std::cout << "\tTestSuiteConsole <FileName.xml> - run build task with arguments taken from <FileName.xml> file" << std::endl;
	std::cout << "or" << std::endl;
	std::cout << "\tTestSuiteConsole [/create <FileName.xml>] - create arguments template in <FileName.xml> file" << std::endl;
	std::cout << std::endl;
	std::cout << "Example 1:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe MyProjectTestArgs.xml" << std::endl;
	std::cout << "Example 2:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe /create NewProjectTestArgs.xml" << std::endl;

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

int startTests(QString testArgsFileName)
{
	// Read arguments from XML document
	//
	QDomDocument doc("Document");

	QFile file(testArgsFileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QString errorMsg = QObject::tr("Failed to open file %1.").arg(testArgsFileName);
		std::cout << errorMsg.toStdString() << std::endl;
		return 1;
	}


	if (doc.setContent(&file) == false)
	{
		QString errorMsg = QObject::tr("Failed to load contents of the file %1.").arg(testArgsFileName);
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
	QString instanceStrId;
	bool ok = getArgumentFromXml(docElem, "InstanceStrID", &instanceStrId);
	if (ok == false)
	{
		std::cout << "Failed to read InstanceStrID argument from file!" << std::endl;
		return 1;
	}
	if (instanceStrId.isEmpty() == true)
	{
		std::cout << "InstanceStrID argument can't be empty!" << std::endl;
		return 1;
	}

	// ConfiguratorIPAddress1
	//
	QString configuratorIPAddress1;
	ok = getArgumentFromXml(docElem, "ConfiguratorIPAddress1", &configuratorIPAddress1);
	if (ok == false)
	{
		std::cout << "Failed to read ConfiguratorIPAddress1 argument from file!" << std::endl;
		return 1;
	}
	if (configuratorIPAddress1.isEmpty() == true)
	{
		std::cout << "ConfiguratorIPAddress1 argument can't be empty!" << std::endl;
		return 1;
	}

	// ConfiguratorPort1
	//
	int configuratorPort1 = 0;
	ok = getArgumentFromXml(docElem, "ConfiguratorPort1", &configuratorPort1);
	if (ok == false)
	{
		std::cout << "Failed to read ConfiguratorPort1 argument from file!" << std::endl;
		return 1;
	}

	// ConfiguratorIPAddress2
	//
	QString configuratorIPAddress2;
	ok = getArgumentFromXml(docElem, "ConfiguratorIPAddress2", &configuratorIPAddress2);
	if (ok == false)
	{
		std::cout << "Failed to read ConfiguratorIPAddress2 argument from file!" << std::endl;
		return 1;
	}
	if (configuratorIPAddress2.isEmpty() == true)
	{
		std::cout << "ConfiguratorIPAddress2 argument can't be empty!" << std::endl;
		return 1;
	}

	// ConfiguratorPort2
	//
	int configuratorPort2 = 0;
	ok = getArgumentFromXml(docElem, "ConfiguratorPort2", &configuratorPort2);
	if (ok == false)
	{
		std::cout << "Failed to read ConfiguratorPort2 argument from file!" << std::endl;
		return 1;
	}

	// Some inititializations
	//
	//VFrame30::init();
	//Hardware::init();
	//DbController::init();
	//Builder::init();
	SoftwareInfo softwareInfo;

	softwareInfo.init(E::SoftwareType::TestSuite, instanceStrId, 0, 1);

	HostAddressPort addr1(configuratorIPAddress1, configuratorPort1);
	HostAddressPort addr2(configuratorIPAddress2, configuratorPort2);

	TestTask* testTask = new TestTask(softwareInfo, addr1, addr2, nullptr /* QCoreApplication::instance() */);

	/*testTask->setDatabaseAddress(dbAddress);
	testTask->setDatabasePort(port);
	testTask->setDatabaseUserName(dbUserName);
	testTask->setDatabasePassword(dbPassword);
	testTask->setProjectName(projectName);
	testTask->setProjectUserName(projectUserName);
	testTask->setProjectUserPassword(projectUserPassword);
	if (buildPath.isEmpty() == false)
	{
		testTask->setBuildOutputPath(buildPath);
	}*/

	// This will cause the application to exit when
	// the testTask signals finished.
	//
	QObject::connect(testTask, &TestTask::finished, QCoreApplication::instance(), &QCoreApplication::exit);

	// Start build process
	//
	//testTask->start();

	// Run message loop
	//
	int result = QCoreApplication::instance()->exec();

	delete testTask;

	// Shutting down
	//
	//Builder::shutdown();
	//DbController::shutdown();
	//VFrame30::shutdown();
	//Hardware::shutdown();

	return result;
}

int main(int argc, char *argv[])
{
	//originalMessageHandler = qInstallMessageHandler(messageOutputHandler);

	QCoreApplication a(argc, argv);

	// --
	//
	a.setApplicationName("TestSuiteConsole");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.9.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.9.LOCALBUILD"));
#endif

	QStringList args = a.arguments();

	int result = 0;

	switch (args.size())
	{
	case 2:
		result = startTests(args[1]);
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
