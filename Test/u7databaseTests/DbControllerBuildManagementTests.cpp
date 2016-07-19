#include "DbControllerBuildManagementTests.h"
#include <QSql>
#include <QSqlError>
#include <QFile>
#include <QDebug>

DbControllerBuildTests::DbControllerBuildTests()
{
	m_dbController = new DbController();

	m_databaseHost = "127.0.0.1";
	m_databaseName = "dbcontrollerfiletesting";
	m_databaseUser = "u7";
	m_adminPassword = "P2ssw0rd";
}

void DbControllerBuildTests::initTestCase()
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

void DbControllerBuildTests::buidProcessTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString workstation = "testWorkstation";

	Signal newSignal;

	int lastChangesetId = 0;
	int buildId = 0;

	newSignal.setCaption("BuildTest");
	newSignal.setAcquire(true);
	newSignal.setAperture(0.3);
	newSignal.setAppSignalID("buildTestAppSignal");
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCalculated(true);
	newSignal.setCustomAppSignalID("buildTestCustomAppSignal");
	newSignal.setDataFormat(E::DataFormat::Float);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID("buildTestEquipmentId");
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngeneeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setInputHighLimit(2345.3);
	newSignal.setInputLowLimit(134.4);
	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(1);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName("buildTestObjectName");
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setType(E::SignalType::Discrete);
	newSignal.setUnbalanceLimit(98769.3);
	newSignal.setUnitID(1);

	signalsToAdd.push_back(newSignal);

	bool ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->lastChangesetId(&lastChangesetId);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->buildStart(workstation, false, lastChangesetId, &buildId, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM build WHERE buildId = %1").arg(buildId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("userId").toInt() == 1, qPrintable("Error: wrong user has been recorded when build starts"));
	QVERIFY2 (query.value("workstation").toString() == workstation, qPrintable("Error: wrong workstation has been recorded when build starts"));
	QVERIFY2 (query.value("release").toBool() == false, qPrintable("Error: wrong release value has been recorded when build starts"));
	QVERIFY2 (query.value("changesetId").toInt() == lastChangesetId, qPrintable("Error: wrong changesetId has been recorded when build starts"));

	ok = m_dbController->buildStart(workstation, true, lastChangesetId, &buildId, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM build WHERE buildId = %1").arg(buildId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("release").toBool() == true, qPrintable("Error: wrong release value has been recorded when build starts"));
}

void DbControllerBuildTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	QVERIFY2 (m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0) == true, qPrintable(m_dbController->lastError()));

	delete m_dbController;
}
