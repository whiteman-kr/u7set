#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include "../TestSuiteLib/TestSuiteSettings.h"
#include "../TestSuiteLib/TestSuite.h"
#include "../UtilsLib/LogFile.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

const int MajorVersion = 0;
const int MinorVersion = 9;


void showHelp()
{
	// Show help
	//
	std::cout << "TestSuiteConsole is a command-line tool that performs hardware testing of RPCT projects." << std::endl;
	std::cout << std::endl << "Command line parameters:" << std::endl;
	std::cout << "\tTestSuiteConsole -settings=<FileName.xml> [-scripts_path=<ScriptsPath>] - run build task with settings taken from <FileName.xml> file." << std::endl;
	std::cout << "\t\t\t(optional -scripts_path parameter specifies a directory where test scripts are stored)." << std::endl;
	std::cout << "or" << std::endl;
	std::cout << "\tTestSuiteConsole [-create=<FileName.xml>] - create settings template in <FileName.xml> file." << std::endl;
	std::cout << std::endl;

#ifdef Q_OS_WINDOWS
	std::cout << "Example 1 - run tests contained in the project:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml" << std::endl;
	std::cout << "Example 2 - run tests from specified folder:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml -scripts_path=D:\\ProjectTests" << std::endl;
	std::cout << "Example 3 - create settings file template:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -create=TestSuiteSettings.xml" << std::endl;
#else
	std::cout << "Example 1 - run tests contained in the project:" << std::endl;
	std::cout << "\t./TestSuiteConsole -settings=Settings.xml" << std::endl;
	std::cout << "Example 2 - run tests from specified folder:" << std::endl;
	std::cout << "\t./TestSuiteConsole -settings=Settings.xml -scripts_path=~/ProjectTests" << std::endl;
	std::cout << "Example 3 - create settings file template:" << std::endl;
	std::cout << "\t./TestSuiteConsole -create=Settings.xml" << std::endl;
#endif

	return;
}

class ProtobufLibShutdowner
{
public:
	~ProtobufLibShutdowner()
	{
		google::protobuf::ShutdownProtobufLibrary();
	}
};

class ConsoleLogFile : public Log::LogFile
{
public:
	ConsoleLogFile(const QString& logName, const QString& path, int maxFileSize = 1048576, int maxFilesCount = 64, bool addAppInfoOnStart = true) :
		Log::LogFile(logName, path, maxFileSize, maxFilesCount, addAppInfoOnStart)
	{
	}

	bool writeAlert(const QString& text) override	{	qCritical() << text;	return Log::LogFile::writeAlert(text);	}
	bool writeError(const QString& text) override	{	qCritical() << text;	return Log::LogFile::writeError(text);	}
	bool writeWarning(const QString& text) override	{	qWarning() << text;		return Log::LogFile::writeWarning(text);}
	bool writeMessage(const QString& text) override	{	qInfo() << text;		return Log::LogFile::writeMessage(text);}
	bool writeText(const QString& text) override	{	qInfo() << text;		return Log::LogFile::writeText(text);	}
};

class ConsoleTestLog : public TestSuite::ITestLogOutput
{
public:
	virtual void logItemArrived(const TestSuite::TestLogItem& item) override
	{
		switch(item.type())
		{
		case TestSuite::TestLogItemType::Error:	qCritical() << item.toText();	break;
		case TestSuite::TestLogItemType::Warning:	qWarning() << item.toText();	break;
		case TestSuite::TestLogItemType::Message:	qInfo() << item.toText();		break;
		}
	}
};

struct CommandLineArgs
{
	QString settingsFileName;
	QString scriptsPath;
	QString createSettingsTemplateFileName;
};

CommandLineArgs parseCommandLine(const QStringList args)
{
	CommandLineArgs result{};

	for (QString arg : args)
	{
		if (arg.startsWith("-settings=", Qt::CaseInsensitive) == true)
		{
			result.settingsFileName = arg;
			result.settingsFileName.replace("-settings=", "", Qt::CaseInsensitive);
			continue;
		}

		if (arg.startsWith("-scripts_path=", Qt::CaseInsensitive) == true)
		{
			result.scriptsPath = arg;
			result.scriptsPath.replace("-scripts_path=", "", Qt::CaseInsensitive);
			continue;
		}

		if (arg.startsWith("-create=", Qt::CaseInsensitive) == true)
		{
			result.createSettingsTemplateFileName = arg;
			result.createSettingsTemplateFileName.replace("-create=", "", Qt::CaseInsensitive);
			continue;
		}
	}

	return result;
}

int main(int argc, char* argv[])
{
	ProtobufLibShutdowner protobufLibShutdowner;
	Q_UNUSED(protobufLibShutdowner);

	if (argc < 2)
	{
		showHelp();
		return EXIT_FAILURE;
	}

	QCoreApplication app(argc, argv);

	app.setApplicationName("TestSuite");
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

#ifdef GITLAB_CI_BUILD
	const int buildNo = CI_PIPELINE_ID;

	app.setApplicationVersion(QString("%1.%2.%3 (%4)")
							.arg(MajorVersion)
							.arg(MinorVersion)
							.arg(buildNo)
							.arg(CI_BUILD_REF_SLUG));
#else
	const int buildNo = -1;

	app.setApplicationVersion(QString("%1.%2.LOCALBUILD")
							  .arg(MajorVersion)
							  .arg(MinorVersion));
#endif

	// Parse command line arguments
	//
	CommandLineArgs args = parseCommandLine(QCoreApplication::arguments());

	if (args.createSettingsTemplateFileName.isEmpty() == false)
	{
		if (TestSuite::TestSuiteSettings::createTemplateSettingsFile(args.createSettingsTemplateFileName) == false)
		{
			std::cout << "Error creating settings file template: " << args.createSettingsTemplateFileName.toStdString() << std::endl;
			return EXIT_FAILURE;
		}
		else
		{
			std::cout << "Settings template has been written to: " << args.createSettingsTemplateFileName.toStdString() << std::endl;
			return EXIT_SUCCESS;
		}
	}

	if (args.settingsFileName.isEmpty() == true)
	{
		std::cout << "Error: configuration file is not specified.\n";
		showHelp();
		return EXIT_FAILURE;
	}

	// Load settings from XML file.
	//
	TestSuite::TestSuiteSettings settings;

	QString errorMsg;
	bool ok = settings.restoreFromFile(args.settingsFileName, &errorMsg);
	if (ok == false)
	{
		std::cout << errorMsg.toStdString() << std::endl;
		return EXIT_FAILURE;
	}

	// --
	//
	ConsoleLogFile appLog{qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + settings.instanceStrId()};
	ConsoleTestLog testLog;

	SoftwareInfo softwareInfo{E::SoftwareType::TestSuite, settings.instanceStrId(), MajorVersion, MinorVersion, buildNo};

	TestSuite::TestSuite testSuite{softwareInfo, settings, &appLog, &testLog};

	// Run tests.
	//
	ok = testSuite.execute({}, args.scriptsPath);
	if (ok == false)
	{
		return EXIT_FAILURE;
	}

	QObject::connect(&testSuite, &TestSuite::TestSuite::finished, &app, &QCoreApplication::exit);
	return app.exec();
}
