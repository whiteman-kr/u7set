#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include "../TestSuiteLib/TestLibrarySettings.h"
#include "../TestSuiteLib/TestLibrary.h"
#include "../UtilsLib/LogFile.h"

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


void showHelp()
{
	// Show help
	//
	std::cout << "TestSuiteConsole is a command-line tool that performs hardware testing of RPCT projects." << std::endl;
	std::cout << std::endl << "Command line parameters:" << std::endl;
	std::cout << "\tTestSuiteConsole --config <FileName.xml> [--scriptspath <ScriptsPath>] - run build task with arguments taken from <FileName.xml> file." << std::endl;
	std::cout << "\t\t\t(optional --scriptspath parameter specifies a directory where test scripts are stored)." << std::endl;
	std::cout << "or" << std::endl;
	std::cout << "\tTestSuiteConsole [--create <FileName.xml>] - create arguments template in <FileName.xml> file." << std::endl;
	std::cout << std::endl;
	std::cout << "Example 1 - run tests contained in the project:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe --config MyProjectTestArgs.xml" << std::endl;
	std::cout << "Example 2 - run tests from specified folder:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe --config MyProjectTestArgs.xml --scriptspath D:\\ProjectTests" << std::endl;
	std::cout << "Example 3 - create configuration file template:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe --create NewProjectTestArgs.xml" << std::endl;

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
	ConsoleLogFile(const QString& fileName, const QString& path, int maxFileSize = 1048576, int maxFilesCount = 64, bool addAppInfoOnStart = true)
		:Log::LogFile(fileName, path, maxFileSize, maxFilesCount, addAppInfoOnStart){}

	bool writeAlert(const QString& text) override	{	std::cout << text.toStdString() << std::endl; return Log::LogFile::writeAlert(text);	}
	bool writeError(const QString& text) override	{	std::cout << text.toStdString() << std::endl; return Log::LogFile::writeError(text);	}
	bool writeWarning(const QString& text) override	{	std::cout << text.toStdString() << std::endl; return Log::LogFile::writeWarning(text);	}
	bool writeMessage(const QString& text) override	{	std::cout << text.toStdString() << std::endl; return Log::LogFile::writeMessage(text);	}
	bool writeText(const QString& text) override	{	std::cout << text.toStdString() << std::endl; return Log::LogFile::writeText(text);	}
};

class ConsoleOutputLog : public IOutputLog
{
	void writeMessage(const QString& text)	{	std::cout << text.toStdString() << std::endl; }
	void writeWarning(const QString& text)	{	std::cout << text.toStdString() << std::endl; }
	void writeError(const QString& text)	{	std::cout << text.toStdString() << std::endl; }
};

int main(int argc, char *argv[])
{
	ProtobufLibShutdowner protobufLibShutdowner;
	Q_UNUSED(protobufLibShutdowner);

	if (argc < 2)
	{
		showHelp();
		return EXIT_FAILURE;
	}

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

	QCommandLineParser parser;

	QCommandLineOption createOption("create", "Create configuration file template in <FileName.xml>", "<FileName.xml>");
	parser.addOption(createOption);

	QCommandLineOption configFileOption("config", "Configuration file name", "<FileName.xml>");
	parser.addOption(configFileOption);

	parser.process(*qApp);

	QString templateFile = parser.value(createOption);
	if (templateFile.isEmpty() == false)
	{
		if (TestLibrarySettings::createTemplateConfigurationFile(templateFile) == false)
		{
			std::cout << "Error creating configuration file template: " << templateFile.toStdString() << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Arguments template has been written to: " << templateFile.toStdString() << std::endl;
		return EXIT_SUCCESS;
	}

	QString configFileName = parser.value(configFileOption);
	if (configFileName.isEmpty() == true)
	{
		std::cout << "Error: configuration file is not specified.\n";
		showHelp();
		return EXIT_FAILURE;
	}

	TestLibrarySettings settings;

	QString errorMsg;
	bool ok = settings.restoreFromFile(configFileName, &errorMsg);
	if (ok == false)
	{
		std::cout << errorMsg.toStdString() << std::endl;
		return EXIT_FAILURE;
	}

	ConsoleLogFile logFile(qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + settings.instanceStrId());

	ConsoleOutputLog consoleLog;

	SoftwareInfo softwareInfo;

	softwareInfo.init(E::SoftwareType::TestSuite, settings.instanceStrId(), 0, 1);

	TestLibrary testLibrary(softwareInfo, settings, &logFile, &consoleLog);

	testLibrary.execute();

	QObject::connect(&testLibrary, &TestLibrary::testingFinished, &a, &QCoreApplication::quit);

	int result = a.exec();

	return result;
}



