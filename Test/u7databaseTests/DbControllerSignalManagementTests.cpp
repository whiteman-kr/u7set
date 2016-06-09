#include "DbControllerSignalManagementTests.h"
#include <QSql>
#include <QSqlError>
#include <QDebug>

DbControllerSignalTests::DbControllerSignalTests()
{
	m_dbController = new DbController();

	m_databaseHost = "127.0.0.1";
	m_databaseName = "dbcontrollersignaltesting";
	m_databaseUser = "u7";
	m_adminPassword = "P2ssw0rd";
}

void DbControllerSignalTests::initTestCase()
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

void DbControllerSignalTests::addSignalTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	Signal newSignal;
	newSignal.setCaption("testCaption");
	newSignal.setAcquire(true);
	newSignal.setAperture(0.3);
	newSignal.setAppSignalID("testAppSignal");
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCalculated(true);
	newSignal.setCustomAppSignalID("testCustomAppSignal");
	newSignal.setDataFormat(E::DataFormat::Float);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID("testEquipmentId");
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
	newSignal.setObjectName("testObjectName");
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

	ok = query.exec("SELECT * FROM get_signals_ids(1, true) ORDER BY get_signals_ids DESC");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(query.value(0).toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int checkedOutInstanceId = query.value("checkedOutInstanceId").toInt();

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId  = %1").arg(checkedOutInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("changeSetId").toInt() == newSignal.changesetID(), qPrintable(QString("Error: changeSetId is wrong")));
	QVERIFY2(query.value("appSignalID").toString() == newSignal.appSignalID(), qPrintable(QString("Error: strId is wrong")));
	QVERIFY2(query.value("customAppSignalID").toString() == newSignal.customAppSignalID(), qPrintable(QString("Error: extStrId is wrong")));
	QVERIFY2(query.value("caption").toString() == newSignal.caption(), qPrintable(QString("Error: caption is wrong")));
	QVERIFY2(query.value("dataFormatId").toInt() == newSignal.dataFormatInt(), qPrintable(QString("Error: dataFormatId is wrong")));
	QVERIFY2(query.value("dataSize").toInt() == newSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == newSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == newSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unitId").toInt() == newSignal.unitID(), qPrintable(QString("Error: unitId is wrong")));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == newSignal.unbalanceLimit(), qPrintable(QString("Error: unbalanceLimit is wrong")));
	QVERIFY2(query.value("inputLowLimit").toDouble() == newSignal.inputLowLimit(), qPrintable(QString("Error: inputLowLimit is wrong")));
	QVERIFY2(query.value("inputHighLimit").toDouble() == newSignal.inputHighLimit(), qPrintable(QString("Error: inputHighLimit is wrong")));
	QVERIFY2(query.value("inputUnitId").toInt() == newSignal.inputUnitID(), qPrintable(QString("Error: inputUnitId is wrong")));
	QVERIFY2(query.value("inputSensorId").toInt() == newSignal.inputSensorID(), qPrintable(QString("Error: inputSensorId is wrong")));
	QVERIFY2(query.value("outputLowLimit").toDouble() == newSignal.outputLowLimit(), qPrintable(QString("Error: outputLowLimit is wrong")));
	QVERIFY2(query.value("outputHighLimit").toDouble() == newSignal.outputHighLimit(), qPrintable(QString("Error: outputHighLimit is wrong")));
	QVERIFY2(query.value("outputUnitId").toInt() == newSignal.outputUnitID(), qPrintable(QString("Error: outputUnitId is wrong")));
	QVERIFY2(query.value("outputSensorId").toInt() == newSignal.outputSensorID(), qPrintable(QString("Error: outputSensorId is wrong")));
	QVERIFY2(query.value("acquire").toBool() == newSignal.acquire(), qPrintable(QString("Error: acquire is wrong")));
	QVERIFY2(query.value("calculated").toBool() == newSignal.calculated(), qPrintable(QString("Error: calculated is wrong")));
	QVERIFY2(query.value("normalState").toInt() == newSignal.normalState(), qPrintable(QString("Error: normalState is wrong")));
	QVERIFY2(query.value("decimalPlaces").toInt() == newSignal.decimalPlaces(), qPrintable(QString("Error: decimalPlaces is wrong")));
	QVERIFY2(query.value("aperture").toDouble() == newSignal.aperture(), qPrintable(QString("Error: aperture is wrong")));
	QVERIFY2(query.value("inOutType").toInt() == int(newSignal.inOutType()), qPrintable(QString("Error: inOutType is wrong")));
	QVERIFY2(query.value("equipmentID").toString() == newSignal.equipmentID(), qPrintable(QString("Error: deviceStrId is wrong")));
	QVERIFY2(query.value("filteringTime").toDouble() == newSignal.filteringTime(), qPrintable(QString("Error: filteringTime is wrong")));
	QVERIFY2(query.value("byteOrder").toInt() == newSignal.byteOrder(), qPrintable(QString("Error: byteOrder is wrong")));
	QVERIFY2(query.value("enableTuning").toBool() == newSignal.enableTuning(), qPrintable(QString("Error: enableTuning is wrong")));
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == newSignal.tuningDefaultValue(), qPrintable(QString("Error: tuningDefaultValue is wrong")));
}

void DbControllerSignalTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
}
