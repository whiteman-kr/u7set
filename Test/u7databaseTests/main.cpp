#include <QString>
#include <QtTest>
#include <QtSql>
#include <QObject>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
#include "SignalTests.h"
#include "PropertyObjectTests.h"
#include "ProjectPropertyTests.h"
#include "UserPropertyTest.h"
#include "DbControllerProjectTests.h"
#include "DbControllerUserTests.h"
#include "DbControllerFileManagementTests.h"
#include "DbControllerSignalManagementTests.h"
#include "DbControllerHardwareConfigurationTests.h"
#include "DbControllerVersionControlTests.h"
#include "../../lib/DbController.h"

const int DatabaseProjectVersion = 326;

const char* DatabaseHost = "127.0.0.1";
const char* DatabaseUser = "u7";
const char* DatabaseUserPassword = "P2ssw0rd";

const char* ProjectName = "testproject";
const char* ProjectAdministratorName = "Administrator";
const char* ProjectAdministratorPassword = "P2ssw0rd";

const QString ARG_SIGNAL_TESTS("-signals");

void filterCmdLineArgs(int argc, char *argv[], QStringList* cmdLineArgs, QStringList* specificArgs);
int runSpecificTest(const QStringList& stdArgs, QStringList& nativeArgs, bool* exit);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	int returnCode = 0;

	QStringList cmdLineArgs;
	QStringList specificArgs;

	filterCmdLineArgs(argc, argv, &cmdLineArgs, &specificArgs);

	if (specificArgs.isEmpty() == false)
	{
		bool exit = false;

		returnCode = runSpecificTest(cmdLineArgs, specificArgs, &exit);

		if (exit == true)
		{
			return returnCode;
		}
	}

	Hardware::init();

	//
	// Database Signal functions
	//
	////SignalTests signalTests;
	////returnCode |= QTest::qExec(&signalTests, argc, argv);

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

	DbControllerFileTests dbControllerFileTests;
	returnCode |= QTest::qExec(&dbControllerFileTests, cmdLineArgs);

	DbControllerHardwareConfigurationTests dbControllerHardwareConfigurationTests;
	returnCode |= QTest::qExec(&dbControllerHardwareConfigurationTests, cmdLineArgs);

	DbControllerVersionControlTests dbControllerVersionTests;
	returnCode |= QTest::qExec(&dbControllerVersionTests, cmdLineArgs);

	//

	DbControllerSignalTests dbControllerSignalTests;
	returnCode |= QTest::qExec(&dbControllerSignalTests, cmdLineArgs);

	// Shutting down
	//
	Hardware::shutdown();

	return returnCode;
}

void filterCmdLineArgs(int argc, char *argv[], QStringList* cmdLineArgs, QStringList* specificArgs)
{
	TEST_PTR_RETURN(argv);
	TEST_PTR_RETURN(cmdLineArgs);
	TEST_PTR_RETURN(specificArgs);

	cmdLineArgs->clear();
	specificArgs->clear();

	cmdLineArgs->append(argv[0]);

	std::set<QString> specificArgsSet =
	{
		ARG_SIGNAL_TESTS,
	};

	for(int i = 1; i < argc; i++)
	{
		QString arg(argv[i]);

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

	for(const QString specArg : specificArgs)
	{
		if (specArg == ARG_SIGNAL_TESTS)
		{
			testObject = new DbControllerSignalTests();
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

	delete testObject;

	*exit = true;

	return returnCode;
}
