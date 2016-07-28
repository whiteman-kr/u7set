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

	QSqlQuery query, tempQuery;
	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString() == "u7_" + m_databaseName)
		{
			ok = tempQuery.exec(QString("DROP DATABASE %1").arg(query.value(0).toString()));
			QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
			qDebug() << "Project " << query.value(0).toString() << "dropped!";
		}
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

void DbControllerHardwareConfigurationTests::getDeviceTreeLatestVersionTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	QSqlQuery query;

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	Hardware::DeviceObject* parentDeviceForTest =  new Hardware::DeviceRoot();
	Hardware::DeviceObject* firstChildDeviceForTest = new Hardware::DeviceModule();
	Hardware::DeviceObject* secondChildDeviceForTest = new Hardware::DeviceModule();

	assert(parentDeviceForTest);
	assert(firstChildDeviceForTest);

	// ParentDeviceForTest
	//

	parentDeviceForTest->setCaption("parentDeviceObjectTestDevice");
	parentDeviceForTest->setObjectName("parentDeviceObjectTestDevice");
	parentDeviceForTest->setPlace(1);
	parentDeviceForTest->setUuid(QUuid("000fa400-00a0-0045-00b0-000c00000000"));

	bool ok = m_dbController->addDeviceObject(parentDeviceForTest, 1, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	DbFileInfo parentFileInfo;

	ok = query.exec("SELECT MAX(fileid) FROM fileInstance");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int parentDeviceObjectFileId = query.value(0).toInt();

	ok = m_dbController->getFileInfo(parentDeviceObjectFileId, &parentFileInfo, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	// First child device for test
	//

	QUuid firstChildUuid = QUuid("000fa400-0000-0000-0000-000000000000");

	firstChildDeviceForTest->setCaption("firstChildDeviceObjectTestDevice");
	firstChildDeviceForTest->setObjectName("secondChildDeviceObjectTestDevice");
	firstChildDeviceForTest->setPlace(2);
	firstChildDeviceForTest->setUuid(firstChildUuid);

	ok = m_dbController->addDeviceObject(firstChildDeviceForTest, 1, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec("SELECT MAX(fileid) FROM fileInstance");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE file SET parentid = %1 WHERE fileid = %2").arg(parentDeviceObjectFileId).arg(query.value(0).toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Second child device for test
	//

	QUuid secondChildUuid = QUuid("000fa40b-0000-0000-0000-000000000000");

	secondChildDeviceForTest->setCaption("secondChildDeviceObjectTestDevice");
	secondChildDeviceForTest->setObjectName("secondChildDeviceObjectTestDevice");
	secondChildDeviceForTest->setPlace(3);
	secondChildDeviceForTest->setUuid(secondChildUuid);

	ok = m_dbController->addDeviceObject(secondChildDeviceForTest, 1, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec("SELECT MAX(fileid) FROM fileInstance");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE file SET parentid = %1 WHERE fileid = %2").arg(parentDeviceObjectFileId).arg(query.value(0).toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	std::shared_ptr<Hardware::DeviceObject> out;

	qRegisterMetaType<DbFileInfo>("DbFileInfo");

	ok = m_dbController->getDeviceTreeLatestVersion(parentFileInfo, &out, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(out.get()->childrenCount() == 2, qPrintable("Error: wrong amount of children!"));

	QVERIFY2(out.get()->child(0)->uuid() == firstChildUuid, qPrintable("Error: wrong child  (first child)!"));
	QVERIFY2(out.get()->child(1)->uuid() == secondChildUuid, qPrintable("Error: wrong child  (second child)!"));

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
