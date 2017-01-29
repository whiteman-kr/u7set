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
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName("testObjectName");
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
//	newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
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
	//QVERIFY2(query.value("dataFormatId").toInt() == newSignal.dataFormatInt(), qPrintable(QString("Error: dataFormatId is wrong")));
	QVERIFY2(query.value("dataSize").toInt() == newSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == newSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == newSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unitId").toInt() == newSignal.unitID(), qPrintable(QString("Error: unitId is wrong")));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == newSignal.unbalanceLimit(), qPrintable(QString("Error: unbalanceLimit is wrong")));
	QVERIFY2(query.value("inputLowLimit").toDouble() == newSignal.inputLowLimit(), qPrintable(QString("Error: inputLowLimit is wrong")));
	QVERIFY2(query.value("inputHighLimit").toDouble() == newSignal.inputHighLimit(), qPrintable(QString("Error: inputHighLimit is wrong")));
	QVERIFY2(query.value("inputUnitId").toInt() == newSignal.inputUnitID(), qPrintable(QString("Error: inputUnitId is wrong")));
//	QVERIFY2(query.value("inputSensorId").toInt() == newSignal.inputSensorID(), qPrintable(QString("Error: inputSensorId is wrong")));
	QVERIFY2(query.value("outputLowLimit").toDouble() == newSignal.outputLowLimit(), qPrintable(QString("Error: outputLowLimit is wrong")));
	QVERIFY2(query.value("outputHighLimit").toDouble() == newSignal.outputHighLimit(), qPrintable(QString("Error: outputHighLimit is wrong")));
	QVERIFY2(query.value("outputUnitId").toInt() == newSignal.outputUnitID(), qPrintable(QString("Error: outputUnitId is wrong")));
//	QVERIFY2(query.value("outputSensorId").toInt() == newSignal.outputSensorID(), qPrintable(QString("Error: outputSensorId is wrong")));
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

	// Test signal creation with special symbols
	//

/*	signalsToAdd.clear();

	newSignal.setCaption("\\'\"testCaption\\'\"");
	newSignal.setAcquire(true);
	newSignal.setAperture(0.3);
	newSignal.setAppSignalID("\\'\"testAppSignal\\'\"");
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCalculated(true);
	newSignal.setCustomAppSignalID("\\'\"testCustomAppSignal\\'\"");
	newSignal.setDataFormat(E::DataFormat::Float);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID("\\'\"testEquipmentId\\'\"");
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
	newSignal.setObjectName("\\'\"testObjectName\\'\"");
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

	ok = m_dbController->addSignal(E::SignalType::Discrete, &signalsToAdd, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec("SELECT * FROM get_signals_ids(1, true) ORDER BY get_signals_ids DESC");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(query.value(0).toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	checkedOutInstanceId = query.value("checkedOutInstanceId").toInt();

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId  = %1").arg(checkedOutInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	qDebug() << query.value("appSignalID").toString() << ":::" << newSignal.appSignalID();
	qDebug() << query.value("caption").toString() << ":::" << newSignal.caption();
	qDebug() << query.value("customAppSignalID").toString() << ":::" << newSignal.customAppSignalID();
	qDebug() << query.value("equipmentID").toString() << ":::" << newSignal.equipmentID();

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
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == newSignal.tuningDefaultValue(), qPrintable(QString("Error: tuningDefaultValue is wrong")));*/

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
 newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName("firstSignalForGetSignalIdsTest");
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
//	newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
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
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName(firstCaption);
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	//newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
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
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName(firstCaption);
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
//	newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
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
	//QVERIFY2(query.value("dataFormatId").toInt() == resultSignal.dataFormatInt(), qPrintable(QString("Error: dataFormatId is wrong")));
	QVERIFY2(query.value("dataSize").toInt() == resultSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == resultSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == resultSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unitId").toInt() == resultSignal.unitID(), qPrintable(QString("Error: unitId is wrong")));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == resultSignal.unbalanceLimit(), qPrintable(QString("Error: unbalanceLimit is wrong")));
	QVERIFY2(query.value("inputLowLimit").toDouble() == resultSignal.inputLowLimit(), qPrintable(QString("Error: inputLowLimit is wrong")));
	QVERIFY2(query.value("inputHighLimit").toDouble() == resultSignal.inputHighLimit(), qPrintable(QString("Error: inputHighLimit is wrong")));
	QVERIFY2(query.value("inputUnitId").toInt() == resultSignal.inputUnitID(), qPrintable(QString("Error: inputUnitId is wrong")));
//	QVERIFY2(query.value("inputSensorId").toInt() == resultSignal.inputSensorID(), qPrintable(QString("Error: inputSensorId is wrong")));
	QVERIFY2(query.value("outputLowLimit").toDouble() == resultSignal.outputLowLimit(), qPrintable(QString("Error: outputLowLimit is wrong")));
	QVERIFY2(query.value("outputHighLimit").toDouble() == resultSignal.outputHighLimit(), qPrintable(QString("Error: outputHighLimit is wrong")));
	QVERIFY2(query.value("outputUnitId").toInt() == resultSignal.outputUnitID(), qPrintable(QString("Error: outputUnitId is wrong")));
//	QVERIFY2(query.value("outputSensorId").toInt() == resultSignal.outputSensorID(), qPrintable(QString("Error: outputSensorId is wrong")));
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
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName(firstCaption);
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	//newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
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
	resultSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	resultSignal.setInputSensorID(3452);
	resultSignal.setInputUnitID(E::InputUnit::V);
	resultSignal.setLowADC(4321);
	resultSignal.setLowEngeneeringUnits(123.4);
	resultSignal.setLowValidRange(125.3);
	resultSignal.setNormalState(7564);
	resultSignal.setOutputHighLimit(5263.1);
	resultSignal.setOutputLowLimit(754.7);
	resultSignal.setOutputMode(E::OutputMode::Minus10_Plus10_V);
	//resultSignal.setOutputSensorID(34847);
	resultSignal.setOutputUnitID(1);
	resultSignal.setSpreadTolerance(2346.8);
	resultSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
	QVERIFY2(query.value("dataSize").toInt() == resultSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == resultSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == resultSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unitId").toInt() == resultSignal.unitID(), qPrintable(QString("Error: unitId is wrong")));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == resultSignal.unbalanceLimit(), qPrintable(QString("Error: unbalanceLimit is wrong")));
	QVERIFY2(query.value("inputLowLimit").toDouble() == resultSignal.inputLowLimit(), qPrintable(QString("Error: inputLowLimit is wrong")));
	QVERIFY2(query.value("inputHighLimit").toDouble() == resultSignal.inputHighLimit(), qPrintable(QString("Error: inputHighLimit is wrong")));
	QVERIFY2(query.value("inputUnitId").toInt() == resultSignal.inputUnitID(), qPrintable(QString("Error: inputUnitId is wrong")));
//	QVERIFY2(query.value("inputSensorId").toInt() == resultSignal.inputSensorID(), qPrintable(QString("Error: inputSensorId is wrong")));
	QVERIFY2(query.value("outputLowLimit").toDouble() == resultSignal.outputLowLimit(), qPrintable(QString("Error: outputLowLimit is wrong")));
	QVERIFY2(query.value("outputHighLimit").toDouble() == resultSignal.outputHighLimit(), qPrintable(QString("Error: outputHighLimit is wrong")));
	QVERIFY2(query.value("outputUnitId").toInt() == resultSignal.outputUnitID(), qPrintable(QString("Error: outputUnitId is wrong")));
//	QVERIFY2(query.value("outputSensorId").toInt() == resultSignal.outputSensorID(), qPrintable(QString("Error: outputSensorId is wrong")));
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
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName(firstCaption);
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
//	newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);// setType(E::SignalType::Discrete);
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
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
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
//	newSignal.setInputSensorID(5345);
	newSignal.setInputUnitID(E::InputUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngeneeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setNormalState(1234);
	newSignal.setObjectName(firstCaption);
	newSignal.setOutputHighLimit(85678.5);
	newSignal.setOutputLowLimit(12536.5);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
//	newSignal.setOutputSensorID(13443);
	newSignal.setOutputUnitID(1);
	//	newSignal.setReadOnly(false);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
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

	QStringList signalCaptions;

	Hardware::DeviceSignal firstSignalToAdd;

	signalCaptions << "autoAddSignalTestFirst";

	firstSignalToAdd.setCaption(signalCaptions.at(0));
	firstSignalToAdd.setByteOrder(E::ByteOrder::LittleEndian);
	firstSignalToAdd.setObjectName(signalCaptions.at(0));
	firstSignalToAdd.setType(E::SignalType::Discrete);
	firstSignalToAdd.setFunction(E::SignalFunction::Input);
	firstSignalToAdd.setEquipmentIdTemplate(signalCaptions.at(0));

	Hardware::DeviceSignal secondSignalToAdd;

	signalCaptions << "autoAddSignalTestSecond";

	secondSignalToAdd.setCaption(signalCaptions.at(1));
	secondSignalToAdd.setByteOrder(E::ByteOrder::LittleEndian);
	secondSignalToAdd.setObjectName(signalCaptions.at(1));
	secondSignalToAdd.setType(E::SignalType::Discrete);
	secondSignalToAdd.setFunction(E::SignalFunction::Input);
	secondSignalToAdd.setEquipmentIdTemplate(signalCaptions.at(1));

	std::vector <Hardware::DeviceSignal*> newSignals;
	std::vector <Signal> addedSignals;

	newSignals.push_back(&firstSignalToAdd);
	newSignals.push_back(&secondSignalToAdd);

	bool ok = m_dbController->autoAddSignals(&newSignals, &addedSignals, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(addedSignals.size() == 2, qPrintable("Error: wrong amount of added signals"));

	for (uint currentSignal = 0; currentSignal < addedSignals.size(); currentSignal++)
	{
		QString addedSignalCaption = Signal(addedSignals.at(currentSignal)).caption();
		QString newSignalCaption = "Signal #" + Hardware::DeviceSignal(*newSignals.at(currentSignal)).caption();

		QVERIFY2 (addedSignalCaption == newSignalCaption, qPrintable("Error: signal mistmatch in result check"));
		//qDebug() << addedSignalCaption << ":::" << newSignalCaption;

		ok = query.exec(QString("SELECT * FROM signalInstance WHERE caption='Signal #%1'").arg(signalCaptions.at(currentSignal)));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	}

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

void DbControllerSignalTests::getSignalHistoryTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query, signalInstanceQuery, changesetQuery, usersQuery;

	std::vector<DbChangeset> result;

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'First checkIn')").arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(1).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'Second checkIn')").arg(1).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(1).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'Third checkIn')").arg(1).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = m_dbController->getSignalHistory(signalId, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(result.size() == 3, qPrintable("Error: wrong changeset amount returned."));

	ok = signalInstanceQuery.exec(QString("SELECT * FROM signalInstance WHERE SignalId = %1 ORDER BY changesetId DESC").arg(signalId));

	QVERIFY2(ok == true, qPrintable(signalInstanceQuery.lastError().databaseText()));

	for (DbChangeset buff : result)
	{
		QVERIFY2(signalInstanceQuery.next() == true, qPrintable(signalInstanceQuery.lastError().databaseText()));

		int changesetFromQuery = buff.changeset();
		int changesetFromCheck = signalInstanceQuery.value("changesetId").toInt();

		QVERIFY2(changesetFromQuery == changesetFromCheck, qPrintable("Error: get_signal_history returned wrong changesetId"));
		QVERIFY2(buff.action().toInt() == signalInstanceQuery.value("action").toInt(), qPrintable("Error: wrong action has been returned"));

		ok = changesetQuery.exec(QString("SELECT * FROM changeset WHERE changesetId = %1").arg(changesetFromQuery));

		QVERIFY2(ok == true, qPrintable(changesetQuery.lastError().databaseText()));
		QVERIFY2(changesetQuery.next() == true, qPrintable(changesetQuery.lastError().databaseText()));

		QVERIFY2(buff.comment() == changesetQuery.value("comment").toString(), qPrintable("Error: wrong comment has been set"));
		QVERIFY2(buff.userId() == changesetQuery.value("userId").toInt(), qPrintable("Error: wrong userId has been set"));
		QVERIFY2(buff.date() == changesetQuery.value("time").toDateTime(), qPrintable("Error: wrong checkInTime has been set"));

		ok = usersQuery.exec(QString("SELECT username FROM users WHERE userId = %1").arg(buff.userId()));

		QVERIFY2(ok == true, qPrintable(usersQuery.lastError().databaseText()));
		QVERIFY2(usersQuery.next() == true, qPrintable(usersQuery.lastError().databaseText()));

		QVERIFY2(usersQuery.value(0).toString() == buff.username(), qPrintable("Error: wrong username"));
	}
}

void DbControllerSignalTests::getSpecificSignalsTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	std::vector<int> signalIds;
	std::vector<Signal> result;

	QSqlQuery query;
	QString keyComment = "keyComment";

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int firstSignalId = query.value("Id").toInt();

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int secondSignalId = query.value("Id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1, %2}', 'First checkIn')").arg(firstSignalId).arg(secondSignalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1, %2}')").arg(firstSignalId).arg(secondSignalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1, %2}', '%3')").arg(firstSignalId).arg(secondSignalId).arg(keyComment));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT changesetId FROM changeset WHERE comment = '%1'").arg(keyComment));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int changesetId = query.value(0).toInt();

	signalIds.push_back(firstSignalId);
	signalIds.push_back(secondSignalId);

	ok = m_dbController->getSpecificSignals(&signalIds, changesetId, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	int signalNumber = 0;

	for (Signal currentSignal : result)
	{
		QVERIFY2(currentSignal.ID() == signalIds.at(signalNumber), qPrintable("Error: wrong signalId returned"));

		ok = query.exec(QString("SELECT * FROM get_specific_signal(1, %1, %2)").arg(currentSignal.ID()).arg(changesetId));

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2(query.value("signalId").toInt() == currentSignal.ID(), qPrintable(QString("Error: value signalId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("signalId").toInt()).arg(currentSignal.ID())));
		QVERIFY2(query.value("signalGroupId").toInt() == currentSignal.signalGroupID(), qPrintable(QString("Error: value signalGroupId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("signalGroupId").toInt()).arg(currentSignal.signalGroupID())));
		QVERIFY2(query.value("signalInstanceId").toInt() == currentSignal.signalInstanceID(), qPrintable(QString("Error: value signalInstanceId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("signalInstanceId").toInt()).arg(currentSignal.signalInstanceID())));
		QVERIFY2(query.value("changesetId").toInt() == currentSignal.changesetID(), qPrintable(QString("Error: value changesetId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("changesetId").toInt()).arg(currentSignal.changesetID())));
		QVERIFY2(query.value("checkedOut").toBool() == currentSignal.checkedOut(), qPrintable(QString("Error: value checkedOut is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("checkedOut").toInt()).arg(currentSignal.checkedOut())));
		QVERIFY2(query.value("userId").toInt() == currentSignal.userID(), qPrintable(QString("Error: value userId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("userId").toInt()).arg(currentSignal.userID())));
		QVERIFY2(query.value("channel").toInt() == currentSignal.channelInt(), qPrintable(QString("Error: value channel is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("channel").toInt()).arg(currentSignal.channelInt())));
		QVERIFY2(query.value("type").toInt() == currentSignal.signalTypeInt(), qPrintable(QString("Error: value type is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("type").toInt()).arg(currentSignal.signalTypeInt())));
		QVERIFY2(query.value("created").toDateTime() == currentSignal.created(), qPrintable(QString("Error: value created is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("created").toString()).arg(currentSignal.created().toString())));
		QVERIFY2(query.value("deleted").toBool() == currentSignal.deleted(), qPrintable(QString("Error: value deleted is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("deleted").toBool()).arg(currentSignal.deleted())));
		QVERIFY2(query.value("instanceCreated").toDateTime() == currentSignal.instanceCreated(), qPrintable(QString("Error: value instanceCreated is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("instanceCreated").toString()).arg(currentSignal.instanceCreated().toString())));
		QVERIFY2(query.value("action").toInt() == currentSignal.instanceAction().toInt(), qPrintable(QString("Error: value instanceAction is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("action").toInt()).arg(currentSignal.instanceAction().toInt())));
		QVERIFY2(query.value("appSignalID").toString() == currentSignal.appSignalID(), qPrintable(QString("Error: value appSignalID is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("appSignalID").toString()).arg(currentSignal.appSignalID())));
		QVERIFY2(query.value("customAppSignalID").toString() == currentSignal.customAppSignalID(), qPrintable(QString("Error: value customAppSignalID is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("customAppSignalID").toString()).arg(currentSignal.customAppSignalID())));
		QVERIFY2(query.value("caption").toString() == currentSignal.caption(), qPrintable(QString("Error: value name is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("caption").toString()).arg(currentSignal.caption())));
		QVERIFY2(query.value("dataFormatId").toInt() == currentSignal.analogSignalFormatInt(), qPrintable(QString("Error: value dataFormatId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("dataFormatId").toInt()).arg(currentSignal.analogSignalFormatInt())));
		QVERIFY2(query.value("dataSize").toInt() == currentSignal.dataSize(), qPrintable(QString("Error: value dataSize is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("dataSize").toInt()).arg(currentSignal.dataSize())));
		QVERIFY2(query.value("lowAdc").toInt() == currentSignal.lowADC(), qPrintable(QString("Error: value lowAdc is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("lowAdc").toInt()).arg(currentSignal.lowADC())));
		QVERIFY2(query.value("highAdc").toInt() == currentSignal.highADC(), qPrintable(QString("Error: value highAdc is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("highAdc").toInt()).arg(currentSignal.highADC())));
		QVERIFY2(query.value("lowengeneeringunits").toInt() == currentSignal.lowEngeneeringUnits(), qPrintable(QString("Error: value lowengeneeringunits is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("lowengeneeringunits").toInt()).arg(currentSignal.lowEngeneeringUnits())));
		QVERIFY2(query.value("highengeneeringunits").toInt() == currentSignal.highEngeneeringUnits(), qPrintable(QString("Error: value highengeneeringunits is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("highengeneeringunits").toInt()).arg(currentSignal.highEngeneeringUnits())));
		QVERIFY2(query.value("unitId").toInt() == currentSignal.unitID(), qPrintable(QString("Error: value unitId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("unitId").toInt()).arg(currentSignal.unitID())));
		QVERIFY2(query.value("lowvalidrange").toInt() == currentSignal.lowValidRange(), qPrintable(QString("Error: value lowvalidrange is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("lowvalidrange").toInt()).arg(currentSignal.lowValidRange())));
		QVERIFY2(query.value("highvalidrange").toInt() == currentSignal.highValidRange(), qPrintable(QString("Error: value highvalidrange is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("highvalidrange").toInt()).arg(currentSignal.highValidRange())));
		QVERIFY2(query.value("unbalanceLimit").toInt() == currentSignal.unbalanceLimit(), qPrintable(QString("Error: value unbalanceLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("unbalanceLimit").toInt()).arg(currentSignal.unbalanceLimit())));
		QVERIFY2(query.value("inputLowLimit").toInt() == currentSignal.inputLowLimit(), qPrintable(QString("Error: value inputLowLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("inputLowLimit").toInt()).arg(currentSignal.inputLowLimit())));
		QVERIFY2(query.value("inputHighLimit").toInt() == currentSignal.inputHighLimit(), qPrintable(QString("Error: value inputHighLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("inputHighLimit").toInt()).arg(currentSignal.inputHighLimit())));
		QVERIFY2(query.value("inputUnitId").toInt() == currentSignal.inputUnitIDInt(), qPrintable(QString("Error: value inputUnitId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("inputUnitId").toInt()).arg(currentSignal.inputUnitIDInt())));
		QVERIFY2(query.value("inputSensorId").toInt() == currentSignal.inputSensorTypeInt(), qPrintable(QString("Error: value inputSensorId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("inputSensorId").toInt()).arg(currentSignal.inputSensorTypeInt())));
		QVERIFY2(query.value("outputLowLimit").toInt() == currentSignal.outputLowLimit(), qPrintable(QString("Error: value outputLowLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("outputLowLimit").toInt()).arg(currentSignal.outputLowLimit())));
		QVERIFY2(query.value("outputHighLimit").toInt() == currentSignal.outputHighLimit(), qPrintable(QString("Error: value outputHighLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("outputHighLimit").toInt()).arg(currentSignal.outputHighLimit())));
		QVERIFY2(query.value("outputUnitId").toInt() == currentSignal.outputUnitID(), qPrintable(QString("Error: value outputUnitId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("outputUnitId").toInt()).arg(currentSignal.outputUnitID())));
		QVERIFY2(query.value("outputSensorId").toInt() == currentSignal.outputSensorTypeInt(), qPrintable(QString("Error: value outputSensorId is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("outputSensorId").toInt()).arg(currentSignal.outputSensorTypeInt())));
		QVERIFY2(query.value("acquire").toBool() == currentSignal.acquire(), qPrintable(QString("Error: value acquire is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("acquire").toBool()).arg(currentSignal.acquire())));
		QVERIFY2(query.value("calculated").toBool() == currentSignal.calculated(), qPrintable(QString("Error: value calculated is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("calculated").toBool()).arg(currentSignal.calculated())));
		QVERIFY2(query.value("normalState").toInt() == currentSignal.normalState(), qPrintable(QString("Error: value normalState is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("normalState").toInt()).arg(currentSignal.normalState())));
		QVERIFY2(query.value("decimalPlaces").toInt() == currentSignal.decimalPlaces(), qPrintable(QString("Error: value decimalPlaces is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("decimalPlaces").toInt()).arg(currentSignal.decimalPlaces())));
		QVERIFY2(query.value("aperture").toInt() == currentSignal.aperture(), qPrintable(QString("Error: value aperture is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("aperture").toInt()).arg(currentSignal.aperture())));
		QVERIFY2(query.value("inOutType").toInt() == currentSignal.inOutTypeInt(), qPrintable(QString("Error: value inOutType is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("inOutType").toInt()).arg(currentSignal.inOutTypeInt())));
		QVERIFY2(query.value("equipmentID").toString() == currentSignal.equipmentID(), qPrintable(QString("Error: value equipmentID is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("equipmentID").toString()).arg(currentSignal.equipmentID())));
		QVERIFY2(query.value("outputRangeMode").toInt() == currentSignal.outputModeInt(), qPrintable(QString("Error: value outputRangeMode is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("outputRangeMode").toInt()).arg(currentSignal.outputModeInt())));
		QVERIFY2(query.value("filteringTime").toDouble() == currentSignal.filteringTime(), qPrintable(QString("Error: value filteringTime is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("filteringTime").toInt()).arg(currentSignal.filteringTime())));
		QVERIFY2(query.value("spreadtolerance").toInt() == currentSignal.spreadTolerance(), qPrintable(QString("Error: value spreadtolerance is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("spreadtolerance").toInt()).arg(currentSignal.spreadTolerance())));
		QVERIFY2(query.value("byteOrder").toInt() == currentSignal.byteOrder(), qPrintable(QString("Error: value byteOrder is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("byteOrder").toInt()).arg(currentSignal.byteOrder())));
		QVERIFY2(query.value("enableTuning").toBool() == currentSignal.enableTuning(), qPrintable(QString("Error: value enableTuning is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("enableTuning").toBool()).arg(currentSignal.enableTuning())));
		QVERIFY2(query.value("tuningDefaultValue").toDouble() == currentSignal.tuningDefaultValue(), qPrintable(QString("Error: value tuningDefaultValue is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("tuningDefaultValue").toDouble()).arg(currentSignal.tuningDefaultValue())));

		signalNumber++;
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
