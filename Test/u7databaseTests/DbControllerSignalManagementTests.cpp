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

	db.close();
}

void DbControllerSignalTests::getSignalIdsTest()
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
	newSignal.setCaption("firstSignalForGetSignalIdsTest");
	newSignal.setAcquire(true);
	newSignal.setAperture(0.3);
	newSignal.setAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCalculated(true);
	newSignal.setCustomAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setDataFormat(E::DataFormat::Float);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID("firstSignalForGetSignalIdsTest");
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
	newSignal.setObjectName("firstSignalForGetSignalIdsTest");
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

	newSignal.setCaption("firstSignalForGetSignalIdsTest");
	newSignal.setAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setCustomAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setEquipmentID("firstSignalForGetSignalIdsTest");
	newSignal.setObjectName("firstSignalForGetSignalIdsTest");

	signalsToAdd.push_back(newSignal);

	bool ok = query.exec("SELECT * FROM get_signals_ids(1, true)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	QVector <int> result;

	ok = m_dbController->getSignalsIDs(&result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	int amountOfSignalsFromQuery = 0;

	while (query.next())
	{
		QVERIFY2(result.contains(query.value(0).toInt()) == true, qPrintable("Error: function returned wrong data"));
		amountOfSignalsFromQuery++;
	}

	QVERIFY2 (result.size() == amountOfSignalsFromQuery, qPrintable("Function returned wrong amount of signals"));

	db.close();
}

void DbControllerSignalTests::checkInCheckOutSignalsTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString firstCaption, secondCaption;

	firstCaption = "firstCheckInTest";
	secondCaption = "secondCheckInTest";

	Signal newSignal;
	newSignal.setCaption(firstCaption);
	newSignal.setAcquire(true);
	newSignal.setAperture(0.3);
	newSignal.setAppSignalID(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCalculated(true);
	newSignal.setCustomAppSignalID(firstCaption);
	newSignal.setDataFormat(E::DataFormat::Float);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID(firstCaption);
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
	newSignal.setObjectName(firstCaption);
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

	newSignal.setCaption(secondCaption);
	newSignal.setAppSignalID(secondCaption);
	newSignal.setCustomAppSignalID(secondCaption);
	newSignal.setEquipmentID(secondCaption);
	newSignal.setObjectName(secondCaption);

	signalsToAdd.push_back(newSignal);

	bool ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVector<int> signalIds;

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption = '%1' OR caption = '%2'").arg(firstCaption).arg(secondCaption));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		signalIds.push_back(query.value("signalId").toInt());
	}

	QVERIFY2 (signalIds.size() == signalsToAdd.size(), qPrintable("Error: wrong amount of records when detecting signalId"));

	QVector<ObjectState> os;

	ok = m_dbController->checkinSignals(&signalIds, "CheckIn test for signals", &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(os.size() == signalsToAdd.size(), qPrintable("Error: wrong amount of signals object states returned"));

	for (ObjectState buffObjectState : os)
	{
		QVERIFY2(buffObjectState.checkedOut == false, qPrintable("Error: wrong objectState returned"));

		ok = query.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(buffObjectState.id));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first(), qPrintable(query.lastError().databaseText()));

		QVERIFY2(query.value("checkedOutInstanceId").toInt() == 0, qPrintable("Error: Actually, signal was not checked In (Not empty checkedOutInstanceId)"));
		QVERIFY2(query.value("checkedInInstanceId").toInt() != 0, qPrintable("Error: Actually, signal was not checked In (Not empty checkedInInstanceId)"));
	}

	os.clear();

	ok = m_dbController->checkoutSignals(&signalIds, &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(os.size() == signalsToAdd.size(), qPrintable("Error: wrong amount of signals object states returned"));

	for (ObjectState buffObjectState : os)
	{
		QVERIFY2(buffObjectState.checkedOut == true, qPrintable("Error: wrong objectState returned"));

		ok = query.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(buffObjectState.id));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first(), qPrintable(query.lastError().databaseText()));

		QVERIFY2(query.value("checkedOutInstanceId").toInt() != 0, qPrintable("Error: Actually, signal was not checked In (Not empty checkedOutInstanceId)"));
		QVERIFY2(query.value("checkedInInstanceId").toInt() != 0, qPrintable("Error: Actually, signal was not checked In (Not empty checkedInInstanceId)"));
	}

	db.close();
}

void DbControllerSignalTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
}
