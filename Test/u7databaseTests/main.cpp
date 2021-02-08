//#include <QString>
#include <QtTest>
//#include <QtSql>
//#include <QObject>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
//#include "SignalTests.h"
#include "PropertyObjectTests.h"
#include "ProjectPropertyTests.h"
#include "UserPropertyTest.h"
#include "DbControllerProjectTests.h"
#include "DbControllerUserTests.h"
//#include "DbControllerFileManagementTests.h"
//#include "DbControllerSignalManagementTests.h"
//#include "DbControllerHardwareConfigurationTests.h"
//#include "DbControllerVersionControlTests.h"
//#include "../../lib/DbController.h"

const int DatabaseProjectVersion = 326;

const char* DatabaseHost = "127.0.0.1";
const char* DatabaseUser = "u7";
const char* DatabaseUserPassword = "P2ssw0rd";

const char* ProjectName = "testproject";
const char* ProjectAdministratorName = "Administrator";
const char* ProjectAdministratorPassword = "P2ssw0rd";


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

//	Hardware::init();

	// --
	//
	int returnCode = 0;

//	//
//	// Database Signal functions
//	//
//	////SignalTests signalTests;
//	////returnCode |= QTest::qExec(&signalTests, argc, argv);

	// Database User Management functions
	//
	UserTests userTests;
	returnCode |= QTest::qExec(&userTests, argc, argv);

	// Database File functions
	//
	FileTests fileTests;
	returnCode |= QTest::qExec(&fileTests, argc, argv);

	// Database Other functions
	//
	OtherTests otherTests;
	returnCode |= QTest::qExec(&otherTests, argc, argv);

	// Database Project Property functions
	//
	ProjectPropertyTests projectPropertyTests;
	returnCode |= QTest::qExec(&projectPropertyTests, argc, argv);

	// Database User Property functions
	//
	UserPropertyTests userPropertyTests;
	returnCode |= QTest::qExec(&userPropertyTests, argc, argv);

	// Property Obejct functions
	//
	PropertyObjectTests propertyObjectTests;
	returnCode |= QTest::qExec(&propertyObjectTests, argc, argv);

	// --
	//
	DbControllerProjectTests dbControllerProjectTests;
	dbControllerProjectTests.setProjectVersion(DatabaseProjectVersion);
	returnCode |= QTest::qExec(&dbControllerProjectTests, argc, argv);

	DbControllerUserTests dbControllerUserTests;
	returnCode |= QTest::qExec(&dbControllerUserTests, argc, argv);

//	DbControllerFileTests dbControllerFileTests;
//	returnCode |= QTest::qExec(&dbControllerFileTests, argc, argv);

//////	DbControllerSignalTests dbControllerSignalTests;
//////	returnCode |= QTest::qExec(&dbControllerSignalTests, argc, argv);

//	DbControllerHardwareConfigurationTests dbControllerHardwareConfigurationTests;
//	returnCode |= QTest::qExec(&dbControllerHardwareConfigurationTests, argc, argv);

//	DbControllerVersionControlTests dbControllerVersionTests;
//	returnCode |= QTest::qExec(&dbControllerVersionTests, argc, argv);

//	// Shutting down
//	//
//	Hardware::shutdown();

	return returnCode;
}
