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
	qRegisterMetaType<E::SignalType>("E::SignalType");

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
	//	newSignal.setReadOnly(false);
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
	//	newSignal.setReadOnly(false);
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
	//	newSignal.setReadOnly(false);
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

void DbControllerSignalTests::getUnitsTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	UnitList result;
	QSqlQuery query;

	bool ok = m_dbController->getUnits(&result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec("SELECT * FROM get_units()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int unitIndex = 0;

	while (query.next())
	{
		QVERIFY2(result.at(unitIndex).first == query.value("unitId").toInt(), qPrintable("Error: wrong unitId"));
		QVERIFY2(result.at(unitIndex).second == query.value("unit_en").toString(), qPrintable("Error: wrong unit_en"));
		unitIndex++;
	}

	db.close();
	
}

void DbControllerSignalTests::getLatestSignalTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString firstCaption;

	firstCaption = "getLatestSignalTest";

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
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setType(E::SignalType::Discrete);
	newSignal.setUnbalanceLimit(98769.3);
	newSignal.setUnitID(1);

	signalsToAdd.push_back(newSignal);
	
	bool ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption = '%1'").arg(firstCaption));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	
	int signalId = query.value("signalId").toInt();
	
	QVector<int> signalIds;
	QVector<ObjectState> result;
	
	signalIds.push_back(signalId);
	
	ok = m_dbController->checkinSignals(&signalIds, "First comment of getLatestSignalTest", &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));
	
	result.clear();
	
	ok = m_dbController->checkoutSignals(&signalIds, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	result.clear();

	ok = m_dbController->checkinSignals(&signalIds, "Second comment of getLatestSignalTest", &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	Signal resultSignal;

	ok = m_dbController->getLatestSignal(signalId, &resultSignal, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("signalId").toInt() == resultSignal.ID(), qPrintable(QString("Error: signalId is wrong")));
	QVERIFY2(query.value("changeSetId").toInt() == resultSignal.changesetID(), qPrintable(QString("Error: changeSetId is wrong")));
	QVERIFY2(query.value("appSignalID").toString() == resultSignal.appSignalID(), qPrintable(QString("Error: strId is wrong")));
	QVERIFY2(query.value("customAppSignalID").toString() == resultSignal.customAppSignalID(), qPrintable(QString("Error: extStrId is wrong")));
	QVERIFY2(query.value("caption").toString() == resultSignal.caption(), qPrintable(QString("Error: caption is wrong")));
	QVERIFY2(query.value("dataFormatId").toInt() == resultSignal.dataFormatInt(), qPrintable(QString("Error: dataFormatId is wrong")));
	QVERIFY2(query.value("dataSize").toInt() == resultSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == resultSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == resultSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unitId").toInt() == resultSignal.unitID(), qPrintable(QString("Error: unitId is wrong")));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == resultSignal.unbalanceLimit(), qPrintable(QString("Error: unbalanceLimit is wrong")));
	QVERIFY2(query.value("inputLowLimit").toDouble() == resultSignal.inputLowLimit(), qPrintable(QString("Error: inputLowLimit is wrong")));
	QVERIFY2(query.value("inputHighLimit").toDouble() == resultSignal.inputHighLimit(), qPrintable(QString("Error: inputHighLimit is wrong")));
	QVERIFY2(query.value("inputUnitId").toInt() == resultSignal.inputUnitID(), qPrintable(QString("Error: inputUnitId is wrong")));
	QVERIFY2(query.value("inputSensorId").toInt() == resultSignal.inputSensorID(), qPrintable(QString("Error: inputSensorId is wrong")));
	QVERIFY2(query.value("outputLowLimit").toDouble() == resultSignal.outputLowLimit(), qPrintable(QString("Error: outputLowLimit is wrong")));
	QVERIFY2(query.value("outputHighLimit").toDouble() == resultSignal.outputHighLimit(), qPrintable(QString("Error: outputHighLimit is wrong")));
	QVERIFY2(query.value("outputUnitId").toInt() == resultSignal.outputUnitID(), qPrintable(QString("Error: outputUnitId is wrong")));
	QVERIFY2(query.value("outputSensorId").toInt() == resultSignal.outputSensorID(), qPrintable(QString("Error: outputSensorId is wrong")));
	QVERIFY2(query.value("acquire").toBool() == resultSignal.acquire(), qPrintable(QString("Error: acquire is wrong")));
	QVERIFY2(query.value("calculated").toBool() == resultSignal.calculated(), qPrintable(QString("Error: calculated is wrong")));
	QVERIFY2(query.value("normalState").toInt() == resultSignal.normalState(), qPrintable(QString("Error: normalState is wrong")));
	QVERIFY2(query.value("decimalPlaces").toInt() == resultSignal.decimalPlaces(), qPrintable(QString("Error: decimalPlaces is wrong")));
	QVERIFY2(query.value("aperture").toDouble() == resultSignal.aperture(), qPrintable(QString("Error: aperture is wrong")));
	QVERIFY2(query.value("inOutType").toInt() == int(resultSignal.inOutType()), qPrintable(QString("Error: inOutType is wrong")));
	QVERIFY2(query.value("equipmentID").toString() == resultSignal.equipmentID(), qPrintable(QString("Error: deviceStrId is wrong")));
	QVERIFY2(query.value("filteringTime").toDouble() == resultSignal.filteringTime(), qPrintable(QString("Error: filteringTime is wrong")));
	QVERIFY2(query.value("byteOrder").toInt() == resultSignal.byteOrder(), qPrintable(QString("Error: byteOrder is wrong")));
	QVERIFY2(query.value("enableTuning").toBool() == resultSignal.enableTuning(), qPrintable(QString("Error: enableTuning is wrong")));
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == resultSignal.tuningDefaultValue(), qPrintable(QString("Error: tuningDefaultValue is wrong")));

	db.close();
}

void DbControllerSignalTests::setSignalWorkCopyTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString firstCaption;

	firstCaption = "setSignalWorkcopyTest";

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
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setType(E::SignalType::Discrete);
	newSignal.setUnbalanceLimit(98769.3);
	newSignal.setUnitID(1);

	signalsToAdd.push_back(newSignal);

	bool ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption = '%1'").arg(firstCaption));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("signalId").toInt();


	QVector<int> signalIds;
	QVector<ObjectState> result;

	signalIds.push_back(signalId);

	ok = m_dbController->checkinSignals(&signalIds, "First comment of setSignalWorkCopyTest", &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	result.clear();

	ok = m_dbController->checkoutSignals(&signalIds, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	Signal resultSignal;

	ok = m_dbController->getLatestSignal(signalId, &resultSignal, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	resultSignal.setAcquire(false);
	resultSignal.setAperture(0.4);
	resultSignal.setByteOrder(E::ByteOrder::BigEndian);
	resultSignal.setCalculated(false);
	resultSignal.setDataFormat(E::DataFormat::UnsignedInt);
	resultSignal.setDataSize(40);
	resultSignal.setDecimalPlaces(4);
	resultSignal.setEnableTuning(false);
	resultSignal.setFilteringTime(9.7);
	resultSignal.setHighADC(243);
	resultSignal.setHighEngeneeringUnits(1783.7);
	resultSignal.setHighValidRange(2333.8);
	resultSignal.setInOutType(E::SignalInOutType::Output);
	resultSignal.setInputHighLimit(1928.3);
	resultSignal.setInputLowLimit(12.8);
	resultSignal.setInputSensorID(3452);
	resultSignal.setInputUnitID(1);
	resultSignal.setLowADC(4321);
	resultSignal.setLowEngeneeringUnits(123.4);
	resultSignal.setLowValidRange(125.3);
	resultSignal.setNormalState(7564);
	resultSignal.setOutputHighLimit(5263.1);
	resultSignal.setOutputLowLimit(754.7);
	resultSignal.setOutputMode(E::OutputMode::Minus10_Plus10_V);
	resultSignal.setOutputSensorID(34847);
	resultSignal.setOutputUnitID(1);
	//	resultSignal.setReadOnly(true);
	resultSignal.setSpreadTolerance(2346.8);
	resultSignal.setType(E::SignalType::Discrete);
	resultSignal.setUnbalanceLimit(2345.3);
	resultSignal.setUnitID(2);

	ObjectState os;

	m_dbController->setSignalWorkcopy(&resultSignal, &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId = %1").arg(resultSignal.signalInstanceID()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next(), qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("changeSetId").toInt() == resultSignal.changesetID(), qPrintable(QString("Error: changeSetId is wrong")));
	QVERIFY2(query.value("appSignalID").toString() == resultSignal.appSignalID(), qPrintable(QString("Error: strId is wrong")));
	QVERIFY2(query.value("customAppSignalID").toString() == resultSignal.customAppSignalID(), qPrintable(QString("Error: extStrId is wrong")));
	QVERIFY2(query.value("caption").toString() == resultSignal.caption(), qPrintable(QString("Error: caption is wrong")));
	QVERIFY2(query.value("dataFormatId").toInt() == resultSignal.dataFormatInt(), qPrintable(QString("Error: dataFormatId is wrong")));
	QVERIFY2(query.value("dataSize").toInt() == resultSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == resultSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == resultSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unitId").toInt() == resultSignal.unitID(), qPrintable(QString("Error: unitId is wrong")));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == resultSignal.unbalanceLimit(), qPrintable(QString("Error: unbalanceLimit is wrong")));
	QVERIFY2(query.value("inputLowLimit").toDouble() == resultSignal.inputLowLimit(), qPrintable(QString("Error: inputLowLimit is wrong")));
	QVERIFY2(query.value("inputHighLimit").toDouble() == resultSignal.inputHighLimit(), qPrintable(QString("Error: inputHighLimit is wrong")));
	QVERIFY2(query.value("inputUnitId").toInt() == resultSignal.inputUnitID(), qPrintable(QString("Error: inputUnitId is wrong")));
	QVERIFY2(query.value("inputSensorId").toInt() == resultSignal.inputSensorID(), qPrintable(QString("Error: inputSensorId is wrong")));
	QVERIFY2(query.value("outputLowLimit").toDouble() == resultSignal.outputLowLimit(), qPrintable(QString("Error: outputLowLimit is wrong")));
	QVERIFY2(query.value("outputHighLimit").toDouble() == resultSignal.outputHighLimit(), qPrintable(QString("Error: outputHighLimit is wrong")));
	QVERIFY2(query.value("outputUnitId").toInt() == resultSignal.outputUnitID(), qPrintable(QString("Error: outputUnitId is wrong")));
	QVERIFY2(query.value("outputSensorId").toInt() == resultSignal.outputSensorID(), qPrintable(QString("Error: outputSensorId is wrong")));
	QVERIFY2(query.value("acquire").toBool() == resultSignal.acquire(), qPrintable(QString("Error: acquire is wrong")));
	QVERIFY2(query.value("calculated").toBool() == resultSignal.calculated(), qPrintable(QString("Error: calculated is wrong")));
	QVERIFY2(query.value("normalState").toInt() == resultSignal.normalState(), qPrintable(QString("Error: normalState is wrong")));
	QVERIFY2(query.value("decimalPlaces").toInt() == resultSignal.decimalPlaces(), qPrintable(QString("Error: decimalPlaces is wrong")));
	QVERIFY2(query.value("aperture").toDouble() == resultSignal.aperture(), qPrintable(QString("Error: aperture is wrong")));
	QVERIFY2(query.value("inOutType").toInt() == int(resultSignal.inOutType()), qPrintable(QString("Error: inOutType is wrong")));
	QVERIFY2(query.value("equipmentID").toString() == resultSignal.equipmentID(), qPrintable(QString("Error: deviceStrId is wrong")));
	QVERIFY2(query.value("filteringTime").toDouble() == resultSignal.filteringTime(), qPrintable(QString("Error: filteringTime is wrong")));
	QVERIFY2(query.value("byteOrder").toInt() == resultSignal.byteOrder(), qPrintable(QString("Error: byteOrder is wrong")));
	QVERIFY2(query.value("enableTuning").toBool() == resultSignal.enableTuning(), qPrintable(QString("Error: enableTuning is wrong")));
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == resultSignal.tuningDefaultValue(), qPrintable(QString("Error: tuningDefaultValue is wrong")));

	db.close();
}

void DbControllerSignalTests::undoSignalChangesTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString firstCaption;

	firstCaption = "undoSignalChangesTest";

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
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setType(E::SignalType::Discrete);
	newSignal.setUnbalanceLimit(98769.3);
	newSignal.setUnitID(1);

	signalsToAdd.push_back(newSignal);

	bool ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption = '%1'").arg(firstCaption));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("signalId").toInt();

	QVector<int> signalIds;
	QVector<ObjectState> os;

	signalIds.push_back(signalId);

	ok = m_dbController->checkinSignals(&signalIds, "UndoSignalChangesTest", &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkoutSignals(&signalIds, &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ObjectState currentSignalObjectState = os.first();

	ok = m_dbController->undoSignalChanges(signalId, &currentSignalObjectState, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(currentSignalObjectState.checkedOut == false, qPrintable ("Error: signal is not checkedIn"));

	ok = query.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("checkedOutInstanceId").toInt()  == 0, qPrintable("Error: actually, signal was not checked In"));
	QVERIFY2(query.value("checkedInInstanceId").toInt()  != 0, qPrintable("Error: actually, signal was not checked In"));

	db.close();
}

void DbControllerSignalTests::deleteSignalTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	QVector<Signal> signalsToAdd;

	QString firstCaption;

	firstCaption = "deleteSignalTest";

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
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setType(E::SignalType::Discrete);
	newSignal.setUnbalanceLimit(98769.3);
	newSignal.setUnitID(1);

	signalsToAdd.push_back(newSignal);

	bool ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption = '%1'").arg(firstCaption));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("signalId").toInt();

	QVector<int> signalIds;
	QVector<ObjectState> os;

	signalIds.push_back(signalId);

	ok = m_dbController->checkinSignals(&signalIds, "deleteSignalTest", &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkoutSignals(&signalIds, &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ObjectState currentSignalObjectState;

	ok = m_dbController->deleteSignal(signalId, &currentSignalObjectState, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(currentSignalObjectState.action == 3, qPrintable("Error: wrong action has been returned"));

	ok = m_dbController->checkinSignals(&signalIds, "deleteSignalTest deleted", &os, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT deleted FROM Signal WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: signal has not been deleted"));
}

void DbControllerSignalTests::autoAddSignalsTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	Hardware::DeviceSignal newSignal;

	QString firstCaption = "autoAddSignalTest";

	newSignal.setCaption(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setObjectName(firstCaption);
	newSignal.setType(E::SignalType::Discrete);
	newSignal.setFunction(E::SignalFunction::Input);
	newSignal.setEquipmentIdTemplate(firstCaption);

	std::vector <Hardware::DeviceSignal*> newSignals;

	newSignals.push_back(&newSignal);

	bool ok = m_dbController->autoAddSignals(&newSignals, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption='Signal #%1'").arg(firstCaption));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	db.close();
}

void DbControllerSignalTests::getSignalsTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	SignalSet signalsFromDb;

	bool ok = m_dbController->getSignals(&signalsFromDb, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	assert(signalsFromDb.isEmpty() == false);

	ok = query.exec("SELECT * FROM get_latest_signals_all(1)");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	int signalAmount = 0;

	while (query.next())
	{
		QVERIFY2 (query.value("signalId").toInt() == signalsFromDb[signalAmount].ID(), qPrintable("Error: wrong Id"));

		signalAmount++;
	}

	QVERIFY2 (signalAmount == signalsFromDb.count(), qPrintable(QString("Error: wrong amount of signals returned\nExpected: %1\nGot: %2").arg(signalAmount).arg(signalsFromDb.count())));

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
