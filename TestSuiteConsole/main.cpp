#include "../UtilsLib/LogFile.h"
#include "version.h"
#include <CommonLib/ConstStrings.h>
#include <TestSuiteLib/MatsTestSuite.h>
#include <TestSuiteLib/TestLog.h>
#include <TestSuiteLib/TestSuiteSettings.h>
#include <google/protobuf/message_lite.h>

#include <QGuiApplication>
#include <QStandardPaths>
#include <iostream>

#ifdef Q_OS_WINDOWS
	#include <windows.h>
#endif

const int MajorVersion = U7SET_MAJOR_VERSION;
const int MinorVersion = U7SET_MINOR_VERSION;

void showVersion()
{
	SoftwareInfo si(E::SoftwareType::TestSuite, QString());

	QStringList res;


	res << QString(" %1 v%2.%3.%4 (%5)")
			   .arg("TestSuiteConsole")
			   .arg(si.majorVersion())
			   .arg(si.minorVersion())
			   .arg(si.patchVersion())
			   .arg(si.branchName());
	res << QString();
#ifdef QT_DEBUG
	res << QString(" Build:          %1 Debug").arg(si.releaseType());
#else
	res << QString(" Build:          %1 Release").arg(si.releaseType());
#endif
	res << QString(" Branch name:    %1").arg(si.branchName());
	res << QString(" Commit SHA:     %1").arg(si.commitHash());
	res << QString(" Build date:     %1").arg(si.buildDate());
	res << QString(" Build username: %1").arg(si.buildUserName());
	res << QString(" Build hostname: %1").arg(si.buildHostname());
	res << QString(" Pipeline ID:    %1").arg(si.pipelineID());

	for (const QString& str : res)
	{
		std::cout << str.toStdString() << std::endl;
	}
	std::cout << std::endl;
}


void showHelp()
{
	// Show help
	//
	std::cout << "TestSuiteConsole is a command-line tool that performs hardware testing of RPCT projects." << std::endl;
	std::cout << std::endl << "Command line parameters:" << std::endl;
#ifdef Q_OS_WINDOWS
	std::cout << "\tTestSuiteConsole -settings=<FileName.xml> [-scripts_path=<ScriptsPath>] [-reports_path=<ReportsPath>] "
				 "[-tests_filter=<TestsFilter>] [-test_log=<filename>|default] [-cp=NNNN] [-nosecurity] - run build task with settings "
				 "taken from <FileName.xml> file."
			  << std::endl;
#else
	std::cout << "\tTestSuiteConsole -settings=<FileName.xml> [-scripts_path=<ScriptsPath>] [-reports_path=<ReportsPath>] "
				 "[-tests_filter=<TestsFilter>] [-test_log=<filename>|default]  [-nosecurity] - run build task with settings taken from "
				 "<FileName.xml> file."
			  << std::endl;
#endif
	std::cout << "\t\t\tOptional -scripts_path parameter specifies a directory where test scripts are stored." << std::endl;
	std::cout << "\t\t\tOptional -tests_filter parameter specifies a filter for running tests. Filter contains test function name"
			  << std::endl;
	std::cout << "\t\t\twith wildcards (\'*\' and \'?\' symbols). If filters starts from '-' symbol, specified tests are excluded."
			  << std::endl;
	std::cout << "\t\t\tSeveral filters can be separated by a semicolon." << std::endl;
	std::cout << "\t\t\tOptional -test_log parameter specifies the file name to store test log(for example, TestLog.tsl or default)."
			  << std::endl;
	std::cout << "\t\t\tIf default specified, log is saved to a file named TestLog_<ddmmyyyy_hhmmss>.tsl." << std::endl;
	std::cout
		<< "\t\t\tOptional -reports_path parameter specifies the path to store generated reports(for example, TestReports or default)."
		<< std::endl;
	std::cout << "\t\t\tIf default specified, log is saved to a folder named TestReport_<ddmmyyyy_hhmmss>." << std::endl;
#ifdef Q_OS_WINDOWS
	std::cout << "\t\t\tOptional -cp parameter specifies the codepage (for example, 1251)." << std::endl;
#endif
	std::cout << "\t\t\tOptional -nosecurity parameter disables requesting username and password if test security control is disabled in "
				 "the project."
			  << std::endl;
	std::cout << "or" << std::endl;
	std::cout << "\tTestSuiteConsole [-create=<FileName.xml>] - create settings template in <FileName.xml> file." << std::endl;
	std::cout << std::endl;

#ifdef Q_OS_WINDOWS
	std::cout << "Example 1 - run tests contained in the project:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml" << std::endl;

	std::cout << "Example 2 - run tests from specified folder:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml -scripts_path=D:\\ProjectTests" << std::endl;

	std::cout << "Example 3 - run tests specified by filter:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml -tests_filter=testReactorTrip*" << std::endl;

	std::cout << "Example 4 - exclude some tests specified by filter:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml -tests_filter=-testPump*;-test*Gcn" << std::endl;

	std::cout << "Example 5 - create settings file template:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -create=TestSuiteSettings.xml" << std::endl;

	std::cout << "Example 6 - run tests contained in the project and save test log to specified file:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml -test_log=d:\\Tests\\ResultLog.tsl" << std::endl;

	std::cout << "Example 7 - run tests contained in the project and save test log to file TestLog_<ddmmyyyy_hhmmss>.tsl:" << std::endl;
	std::cout << "\tTestSuiteConsole.exe -settings=TestSuiteSettings.xml -test_log=default" << std::endl;
#else
	std::cout << "Example 1 - run tests contained in the project:" << std::endl;
	std::cout << "\t./TestSuiteConsole -settings=Settings.xml" << std::endl;

	std::cout << "Example 2 - run tests from specified folder:" << std::endl;
	std::cout << "\t./TestSuiteConsole -settings=Settings.xml -scripts_path=~/ProjectTests" << std::endl;

	std::cout << "Example 3 - run tests specified by filter:" << std::endl;
	std::cout << "\tTestSuiteConsole -settings=TestSuiteSettings.xml -tests_filter=testReactorTrip*" << std::endl;

	std::cout << "Example 4 - exclude some tests specified by filter:" << std::endl;
	std::cout << "\tTestSuiteConsole -settings=TestSuiteSettings.xml -tests_filter=-testPump*;-testOffGcn*" << std::endl;

	std::cout << "Example 5 - create settings file template:" << std::endl;
	std::cout << "\t./TestSuiteConsole -create=Settings.xml" << std::endl;

	std::cout << "Example 6 - run tests contained in the project and save test log to specified file:" << std::endl;
	std::cout << "\t./TestSuiteConsole -settings=TestSuiteSettings.xml -test_log=~/Tests/ResultLog.tsl" << std::endl;

	std::cout << "Example 7 - run tests contained in the project and save test log to file TestLog_<ddmmyyyy_hhmmss>.tsl:" << std::endl;
	std::cout << "\t./TestSuiteConsole -settings=TestSuiteSettings.xml -test_log=default" << std::endl;
#endif

	return;
}

class ProtobufLibShutdowner
{
public:
	~ProtobufLibShutdowner() { google::protobuf::ShutdownProtobufLibrary(); }
};

class ConsoleLogFile : public Log::LogFile
{
public:
	ConsoleLogFile(const QString& logName,
				   const QString& path,
				   int maxFileSize = 1048576,
				   int maxFilesCount = 64,
				   bool addAppInfoOnStart = true) :
		Log::LogFile(logName, path, maxFileSize, maxFilesCount, addAppInfoOnStart)
	{
	}

	bool writeAlert(const QString& text, const QString& /*tag*/ = {}) override
	{
		std::string msg = std::string("\x1B[91m") + text.toStdString() + std::string("\x1B[0m");
		qCritical() << msg.data();
		return Log::LogFile::writeAlert(text);
	}
	bool writeError(const QString& text, const QString& /*tag*/ = {}) override
	{
		std::string msg = std::string("\x1B[91m") + text.toStdString() + std::string("\x1B[0m");
		qCritical() << msg.data();
		return Log::LogFile::writeError(text);
	}
	bool writeWarning(const QString& text, const QString& /*tag*/ = {}) override
	{
		std::string msg = std::string("\x1B[33m") + text.toStdString() + std::string("\x1B[0m");
		qWarning() << msg.data();
		return Log::LogFile::writeWarning(text);
	}
	bool writeMessage(const QString& text, const QString& /*tag*/ = {}) override
	{
		qInfo() << text.toStdString().data();
		return Log::LogFile::writeMessage(text);
	}
	bool writeText(const QString& text, const QString& /*tag*/ = {}) override
	{
		qInfo() << text.toStdString().data();
		return Log::LogFile::writeText(text);
	}
};

class ConsoleTestLog : public TestSuite::ITestLogOutput
{
public:
	virtual void logItemArrived(const TestSuite::TestLogItem& item) override
	{
		switch (item.type())
		{
		case TestSuite::TestLogItemType::Error:
			{
				std::string msg = std::string("\x1B[91m") + item.toText().toStdString() + std::string("\x1B[0m");
				qCritical() << msg.data();
				break;
			}
		case TestSuite::TestLogItemType::Warning:
			{
				std::string msg = std::string("\x1B[33m") + item.toText().toStdString() + std::string("\x1B[0m");
				qWarning() << msg.data();
				break;
			}
		case TestSuite::TestLogItemType::Message:
		case TestSuite::TestLogItemType::Text:
			{
				qInfo() << item.toText().toStdString().data();
				break;
			}
		}
	}
};

struct CommandLineArgs
{
	QString createSettingsTemplateFileName;
	QString settingsFileName;
	QString scriptsPath;
	QString testsFilter;
	QString codepage;
	QString testLogFileName;
	QString reportsPath;
	bool nosecurity = false;
};

CommandLineArgs parseCommandLine(const QStringList args)
{
	CommandLineArgs result{};

	for (const QString& arg : args)
	{
		if (arg.startsWith("-create=", Qt::CaseInsensitive) == true)
		{
			result.createSettingsTemplateFileName = arg;
			result.createSettingsTemplateFileName.replace("-create=", "", Qt::CaseInsensitive);
			continue;
		}

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

		if (arg.startsWith("-tests_filter=", Qt::CaseInsensitive) == true)
		{
			result.testsFilter = arg;
			result.testsFilter.replace("-tests_filter=", "", Qt::CaseInsensitive);
			continue;
		}

		if (arg.startsWith("-cp=", Qt::CaseInsensitive) == true)
		{
			result.codepage = arg;
			result.codepage.replace("-cp=", "", Qt::CaseInsensitive);
			continue;
		}

		if (arg.startsWith("-test_log=", Qt::CaseInsensitive) == true)
		{
			result.testLogFileName = arg;
			result.testLogFileName.replace("-test_log=", "", Qt::CaseInsensitive);
			continue;
		}

		if (arg.startsWith("-reports_path=", Qt::CaseInsensitive) == true)
		{
			result.reportsPath = arg;
			result.reportsPath.replace("-reports_path=", "", Qt::CaseInsensitive);
			continue;
		}

		if (arg.startsWith("-nosecurity", Qt::CaseInsensitive) == true)
		{
			result.nosecurity = true;
			continue;
		}
	}

	return result;
}

void saveTestLog(QString fileName, TestSuite::TestLog& testLog, ConsoleLogFile& appLog)
{
	if (fileName.isEmpty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	if (fileName.contains("default") == true)
	{
		fileName.replace("default", QString("TestLog_%1.tsl").arg(QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss")));
	}

	QString errorMsg;
	bool ok = testLog.saveToCSV(fileName, &errorMsg);
	if (ok == false)
	{
		appLog.writeError(errorMsg);
	}
	else
	{
		appLog.writeMessage(QObject::tr("Test log is saved to the file: '%1'.").arg(fileName));
	}
	return;
}

int main(int argc, char* argv[])
{
	ProtobufLibShutdowner protobufLibShutdowner;
	Q_UNUSED(protobufLibShutdowner);

	showVersion();

	if (argc < 2)
	{
		showHelp();
		return EXIT_FAILURE;
	}

	QGuiApplication app(argc, argv);
	// QCoreApplication app(argc, argv);

	app.setApplicationName("TestSuite");
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);


	app.setApplicationVersion(
		QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

	// Parse command line arguments
	//
	CommandLineArgs args = parseCommandLine(QCoreApplication::arguments());

#ifdef Q_OS_WINDOWS
	{
		// Enable colors support
		//
		HANDLE hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode;
		GetConsoleMode(hStdHandle, &mode);
		SetConsoleMode(hStdHandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}

	// Set codepage
	//
	if (args.codepage.isEmpty() == false)
	{
		bool ok = false;
		uint cp = args.codepage.toUInt(&ok);
		if (ok == true)
		{
			SetConsoleOutputCP(cp);
			SetConsoleCP(cp);
		}
	}
#endif

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

	// Ask for password
	//
	std::string userName;
	std::string password;

	if (args.nosecurity == true)
	{
		std::cout << "Warning: No username and password supplied." << std::endl;
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			std::cout << "Enter test user name: " << std::flush;
			std::getline(std::cin, userName);
			if (userName.empty() == false)
			{
				break;
			}
			if (i < 2)
			{
				std::cout << "Error - test user name can't be empty! Please try again." << std::endl;
			}
		}
		if (userName.empty() == true)
		{
			std::cout << "Error - test user name was not supplied." << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Enter password: ";
		std::getline(std::cin, password);
	}

	// --
	//
	SoftwareInfo softwareInfo(E::SoftwareType::TestSuite, settings.instanceStrId());
	ConsoleLogFile appLog{qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + settings.instanceStrId()};
	ConsoleTestLog testLogOutput;

	TestSuite::TestSuiteConfigController configController(softwareInfo,
														  settings.configuratorAddress1(),
														  settings.configuratorAddress2(),
														  &appLog);

	TestSuite::MatsTestSuite testSuite{configController, &appLog, &testLogOutput};

	configController.start();

	// Run tests.
	//
	TestSuite::TestScriptSelection filter(args.testsFilter);
	std::unique_ptr<TestSuite::TestScriptsStorage> scriptProvider;

	TestSuite::ControlParams controlParams{{},
										   args.reportsPath,
										   filter,
										   QString::fromStdString(userName),
										   QString::fromStdString(password)};

	if (args.scriptsPath.isEmpty() == true)
	{
		// Load scripts from CfgService.
		//
		ok = testSuite.execute(controlParams);
	}
	else
	{
		// Load scripts from disk.
		//
		QString loadError;

		scriptProvider = std::make_unique<TestSuite::TestScriptsStorage>();
		bool loadResult = scriptProvider->loadFromPath(args.scriptsPath, &loadError);

		if (loadResult == false)
		{
			appLog.writeError(QObject::tr("Error loading scripts from the path: %1, error: ").arg(args.scriptsPath).arg(loadError));
			return EXIT_FAILURE;
		}

		ok = testSuite.execute(*scriptProvider, controlParams);
	}

	if (ok == false)
	{
		return EXIT_FAILURE;
	}

	QObject::connect(&testSuite,
					 &TestSuite::MatsTestSuite::finished,
					 [&args, &testSuite, &appLog](int result)
					 {
						 // Save test log to the file
						 //
						 if (args.testLogFileName.isEmpty() == false)
						 {
							 saveTestLog(args.testLogFileName, testSuite.testLog(), appLog);
						 }

						 // Exit the application
						 //
						 QCoreApplication::exit(result);
					 });

	return app.exec();
}
