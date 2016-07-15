#include "DbControllerHardwareConfigurationTests.h"
#include <QSql>
#include <QSqlError>
#include <QDebug>

using namespace Hardware;

DbControllerHardwareConfigurationTests::DbControllerHardwareConfigurationTests()
{
	m_dbController = new DbController();

	m_databaseHost = "127.0.0.1";
	m_databaseName = "dbcontrollerhardwareconfigurationtesting";
	m_databaseUser = "u7";
	m_adminPassword = "P2ssw0rd";
}

void DbControllerHardwareConfigurationTests::initTestCase()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query;
	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString() == "u7_" + m_databaseName)
			m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
	}

	db.close();

	ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not create project: " + m_dbController->lastError()));

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not upgrade project: " + m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not open project: " + m_dbController->lastError()));
}

void DbControllerHardwareConfigurationTests::addAndRemoveDeviceObjectTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	QSqlQuery query;

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QString dataForTest = "DataForFileFromAddDeviceObjectTest";

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	bool ok = query.exec(QString("SELECT * FROM add_file(1, 'addDeviceObjectTest', 1, '%1', '{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}')").arg(dataForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	DbFileInfo fileInfo;

	ok = m_dbController->getFileInfo(query.value("id").toInt(), &fileInfo, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	Hardware::DeviceObject* deviceForTest =  new Hardware::DeviceRoot();
	assert(deviceForTest);

	//deviceForTest

	deviceForTest->setFileInfo(fileInfo);
	deviceForTest->setCaption("addDeviceObjectTestDevice");
	deviceForTest->setObjectName("addDeviceObjectTestDevice");
	deviceForTest->setPlace(1);
	deviceForTest->setChildRestriction("ChildRestriction");

	ok = m_dbController->addDeviceObject(deviceForTest, 1, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec("SELECT MAX(fileid) FROM fileInstance");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int deviceObjectFileId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1").arg(deviceObjectFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("details").toString().contains("addDeviceObjectTestDevice") == true, qPrintable("Error: wrong record in fileinstance"));

	std::vector<Hardware::DeviceObject*> devices;
	devices.push_back(deviceForTest);

	ok = m_dbController->deleteDeviceObjects(devices,0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM fileInstance WHERE fileId = %1").arg(deviceObjectFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == 0, qPrintable("Error: deleteDeviceObject function not delete the object from fileInstance"));

	ok = query.exec(QString("SELECT COUNT(*) FROM file WHERE fileId = %1").arg(deviceObjectFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == 0, qPrintable("Error: deleteDeviceObject function not delete the object from file table"));

	db.close();
}

void DbControllerHardwareConfigurationTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
}
