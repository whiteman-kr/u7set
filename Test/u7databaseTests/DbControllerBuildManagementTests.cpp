#include "DbControllerBuildManagementTests.h"
#include <QSql>
#include <QSqlError>
#include <QDebug>

DbControllerBuildTests::DbControllerBuildTests() :
	m_db(new DbController())
{
	qRegisterMetaType<E::SignalType>("E::SignalType");
}

void DbControllerBuildTests::initTestCase()
{
	m_db->setServerUsername(m_databaseUser);
	m_db->setServerPassword(m_databasePassword);

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

	ok = m_db->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not create project: " + m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not upgrade project: " + m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not open project: " + m_db->lastError()));
}

void DbControllerBuildTests::buidProcessTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2(db.open() == true, qPrintable(db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString workstation = "testWorkstation";
	QString buildLog;

	Signal newSignal;

	int lastChangesetId = 0;
	int buildId = 0;
	int errors = 0;
	int warnings = 0;

	newSignal.setCaption("BuildTest");
	newSignal.setAcquire(true);
	newSignal.setCoarseAperture(1.3);
	newSignal.setFineAperture(1.2);
	newSignal.setAppSignalID("buildTestAppSignal");
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID("buildTestCustomAppSignal");
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID("buildTestEquipmentId");
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngeneeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("lp");

	signalsToAdd.push_back(newSignal);

	bool ok = m_db->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	ok = m_db->lastChangesetId(&lastChangesetId);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	ok = m_db->buildStart(workstation, false, lastChangesetId, &buildId, 0);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	ok = query.exec(QString("SELECT * FROM build WHERE buildId = %1").arg(buildId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("userId").toInt() == 1, qPrintable("Error: wrong user has been recorded when build starts"));
	QVERIFY2 (query.value("workstation").toString() == workstation, qPrintable("Error: wrong workstation has been recorded when build starts"));
	QVERIFY2 (query.value("release").toBool() == false, qPrintable("Error: wrong release value has been recorded when build starts"));
	QVERIFY2 (query.value("changesetId").toInt() == lastChangesetId, qPrintable("Error: wrong changesetId has been recorded when build starts"));

	ok = m_db->buildStart(workstation, true, lastChangesetId, &buildId, 0);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	ok = query.exec(QString("SELECT * FROM build WHERE buildId = %1").arg(buildId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("release").toBool() == true, qPrintable("Error: wrong release value has been recorded when build starts"));

	ok = m_db->buildFinish(buildId, errors, warnings, buildLog, 0);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	ok = query.exec(QString("SELECT * FROM build WHERE buildId = %1").arg(buildId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("errors").toInt() == errors, qPrintable("Error: wrong errors value has been returned"));
	QVERIFY2 (query.value("warnings").toInt() == warnings, qPrintable("Error: wrong warnings value has been returned"));
	QVERIFY2 (query.value("buildLog").toString() == buildLog, qPrintable("Error: wrong buildLog value has been returned"));

	db.close();
}

void DbControllerBuildTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	QVERIFY2 (m_db->deleteProject(m_databaseName, m_adminPassword, true, 0) == true, qPrintable(m_db->lastError()));

	return;
}
