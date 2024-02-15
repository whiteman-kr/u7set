#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
//#include "SignalTests.h"
#include "PropertyObjectTests.h"
#include "ProjectPropertyTests.h"
#include "UserPropertyTest.h"
#include "DbControllerProjectTests.h"
#include "DbControllerUserTests.h"
#include "DbControllerFileManagementTests.h"
#include "DbControllerSignalManagementTests.h"
#include "DbControllerHardwareConfigurationTests.h"
#include "DbControllerVersionControlTests.h"
#include "DeviceObjectTests.h"

#include "Settings.h"
#include "../UtilsLib/WUtils.h"

const int DatabaseProjectVersion = 326;

const QString ARG_SIGNAL_TESTS("-signals");
const QString CONFIG_FILE_PARAM("-config=");

void filterCmdLineArgs(int argc, char *argv[], QStringList* cmdLineArgs, QStringList* specificArgs, QString* configFileName);
int runSpecificTest(const QStringList& stdArgs, QStringList& nativeArgs, bool* exit);

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	int returnCode = 0;

	QStringList cmdLineArgs;
	QStringList specificArgs;
	QString configFileName;

	filterCmdLineArgs(argc, argv, &cmdLineArgs, &specificArgs, &configFileName);

	if (configFileName.isEmpty() == false)
	{
		returnCode = theSettings.loadConfigurationFile(configFileName);

		if (returnCode != 0)
		{
			google::protobuf::ShutdownProtobufLibrary();

			return returnCode;
		}
	}

	if (specificArgs.isEmpty() == false)
	{
		bool exit = false;

		returnCode = runSpecificTest(cmdLineArgs, specificArgs, &exit);

		if (exit == true)
		{
			google::protobuf::ShutdownProtobufLibrary();

			return returnCode;
		}
	}

	Hardware::init();

	// DeviceObject tests
	//
	DeviceObjectTests deviceObjectTests;
	returnCode |= QTest::qExec(&deviceObjectTests, cmdLineArgs);

	//
	// Database Signal functions
	//
	// SignalTests signalTests;
	// returnCode |= QTest::qExec(&signalTests, argc, argv);

	// Database User Management functions
	//
	UserTests userTests;
	returnCode |= QTest::qExec(&userTests, cmdLineArgs);

	// Database File functions
	//
	FileTests fileTests;
	returnCode |= QTest::qExec(&fileTests, cmdLineArgs);

	// Database Other functions
	//
	OtherTests otherTests;
	returnCode |= QTest::qExec(&otherTests, cmdLineArgs);

	// Database Project Property functions
	//
	ProjectPropertyTests projectPropertyTests;
	returnCode |= QTest::qExec(&projectPropertyTests, cmdLineArgs);

	// Database User Property functions
	//
	UserPropertyTests userPropertyTests;
	returnCode |= QTest::qExec(&userPropertyTests, cmdLineArgs);

	// Property Obejct functions
	//
	PropertyObjectTests propertyObjectTests;
	returnCode |= QTest::qExec(&propertyObjectTests, cmdLineArgs);

	// --
	//
	DbControllerProjectTests dbControllerProjectTests;
	dbControllerProjectTests.setProjectVersion(DatabaseProjectVersion);
	returnCode |= QTest::qExec(&dbControllerProjectTests, cmdLineArgs);

	DbControllerUserTests dbControllerUserTests;
	returnCode |= QTest::qExec(&dbControllerUserTests, cmdLineArgs);

	DbControllerFileTests dbControllerFileTests(theSettings.fileManagementTestsProjectName());
	returnCode |= QTest::qExec(&dbControllerFileTests, cmdLineArgs);

	DbControllerHardwareConfigurationTests dbControllerHardwareConfigurationTests(theSettings.hardwareConfigurationTestsProjectName());
	returnCode |= QTest::qExec(&dbControllerHardwareConfigurationTests, cmdLineArgs);

	DbControllerVersionControlTests dbControllerVersionTests(theSettings.versionControlTestsProjectName());
	returnCode |= QTest::qExec(&dbControllerVersionTests, cmdLineArgs);

	//

	DbControllerSignalTests dbControllerSignalTests(theSettings.signalTestsProjectName());
	returnCode |= QTest::qExec(&dbControllerSignalTests, cmdLineArgs);

	// Shutting down
	//
	Hardware::shutdown();

	google::protobuf::ShutdownProtobufLibrary();

	return returnCode;
}

void filterCmdLineArgs(int argc, char *argv[], QStringList* cmdLineArgs, QStringList* specificArgs, QString* configFileName)
{
	TEST_PTR_RETURN(argv);
	TEST_PTR_RETURN(cmdLineArgs);
	TEST_PTR_RETURN(specificArgs);
	TEST_PTR_RETURN(configFileName);

	cmdLineArgs->clear();
	specificArgs->clear();
	configFileName->clear();

	cmdLineArgs->append(argv[0]);

	std::set<QString> specificArgsSet =
	{
		ARG_SIGNAL_TESTS,
	};

	for(int i = 1; i < argc; i++)
	{
		QString arg(argv[i]);

		if (arg.startsWith(CONFIG_FILE_PARAM, Qt::CaseInsensitive) == true)
		{
			*configFileName = arg;
			*configFileName = configFileName->remove(CONFIG_FILE_PARAM).trimmed();
			configFileName->remove('\"');
			configFileName->remove('\'');
			continue;
		}

		auto it = specificArgsSet.find(arg.trimmed().toLower());

		if (it == specificArgsSet.end())
		{
			cmdLineArgs->append(arg);
		}
		else
		{
			specificArgs->append(arg.trimmed().toLower());
		}
	}
}

int runSpecificTest(const QStringList& cmdLineArgs, QStringList& specificArgs, bool* exit)
{
	if (exit == nullptr)
	{
		Q_ASSERT(false);
		return -1;
	}

	QObject* testObject = nullptr;

	for(const QString& specArg : specificArgs)
	{
		if (specArg == ARG_SIGNAL_TESTS)
		{
			testObject = new DbControllerSignalTests(theSettings.signalTestsProjectName());
			break;
		}
	};

	if (testObject == nullptr)
	{
		*exit = false;
		return 0;
	}

	Hardware::init();

	bool returnCode = QTest::qExec(testObject, cmdLineArgs);

	Hardware::shutdown();
	google::protobuf::ShutdownProtobufLibrary();

	delete testObject;

	*exit = true;

	return returnCode;
}
