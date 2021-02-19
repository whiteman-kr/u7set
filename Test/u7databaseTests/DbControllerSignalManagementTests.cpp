#include "DbControllerSignalManagementTests.h"

DbControllerSignalTests::DbControllerSignalTests()
{
	qRegisterMetaType<E::SignalType>("E::SignalType");
	qRegisterMetaType<DbUser>("DbUser");

	m_databaseHost = "127.0.0.1";
	m_projectName = "signalstests";
	m_databaseName = "u7_" + m_projectName;
	m_databaseUser = "u7";
	m_adminPassword = "P2ssw0rd";

	m_db = new DbController();

	m_db->setHost(m_databaseHost);
	m_db->setServerUsername(m_databaseUser);
	m_db->setServerPassword(m_adminPassword);
}

void DbControllerSignalTests::initTestCase()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery q;

	bool ok = q.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");

	QVERIFY2 (ok == true, qPrintable(q.lastError().databaseText()));

	while (q.next() == true)
	{
		if (q.value(0).toString() == m_databaseName)
		{
			ok = q.exec(QString("DROP DATABASE %1").arg(m_databaseName));

			QVERIFY2 (ok == true, qPrintable(lastError(q)));

			qDebug() << "Project database" << m_databaseName << "dropped!";

			break;
		}
	}

	db.close();

	ok = m_db->createProject(m_projectName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable("Can't create project: " + m_db->lastError()));

	ok = m_db->upgradeProject(m_projectName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable("Can't upgrade project: " + m_db->lastError()));

	TS_VERIFY(applyFutureDatabaseUpgrade());

	ok = m_db->openProject(m_projectName, "Administrator", m_adminPassword, nullptr);
	QVERIFY2 (ok == true, qPrintable("Can't open project: " + m_db->lastError()));

	DbUser dbu;

	dbu.setUserId(USER2_ID);
	dbu.setUsername("user2");
	dbu.setFirstName("User");
	dbu.setLastName("Two");
	dbu.setNewPassword("qqq222");
	dbu.setPassword("qqq222");
	dbu.setAdministrator(false);
	dbu.setReadonly(false);
	dbu.setDisabled(false);

	ok = m_db->createUser(dbu, nullptr);

	QVERIFY2 (ok == true, qPrintable("Can't create user2: " + m_db->lastError()));

	dbu.setUserId(USER3_ID);
	dbu.setUsername("user3");
	dbu.setFirstName("User");
	dbu.setLastName("Three");
	dbu.setNewPassword("qqq333");
	dbu.setPassword("qqq333");
	dbu.setAdministrator(false);
	dbu.setReadonly(false);
	dbu.setDisabled(false);

	ok = m_db->createUser(dbu, nullptr);

	QVERIFY2 (ok == true, qPrintable("Can't create user3: " + m_db->lastError()));

	std::srand(static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch()));
}

void DbControllerSignalTests::test_addSignal()
{
	// Testing of stored procedure:
	//
	//	add_signal(	user_id integer,
	//				signal_type integer,
	//				channel_count integer)
	//				RETURNS SETOF objectstate

	OPEN_DATABASE();

	// add single channel signal
	//
	std::vector<ObjectState> obStates;

	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Bus, 1, &obStates));

	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Added);
	QVERIFY(obStates[0].checkedOut == true);

	TS_VERIFY(check_signalIsExist(USER2_ID, obStates[0].id, E::SignalType::Bus, 0, 0, true));

	// add 2 channel signal
	//
	TS_VERIFY(addSignal(ADMIN_ID, E::SignalType::Analog, 2, &obStates));

	QVERIFY(obStates.size() == 2);

	for(int i = 0; i < 2; i++)
	{
		QVERIFY(obStates[i].action == VcsItemAction::Added);
		QVERIFY(obStates[i].checkedOut == true);
	}

	int sg1 = 0;
	TS_VERIFY(getSignalGroupID(obStates[0].id, &sg1));

	TS_VERIFY(check_signalIsExist(ADMIN_ID, obStates[0].id, E::SignalType::Analog, 0, sg1, true));
	TS_VERIFY(check_signalIsExist(ADMIN_ID, obStates[1].id, E::SignalType::Analog, 1, sg1, true));

	// add 4 (max) channel signal
	//
	TS_VERIFY(addSignal(USER3_ID, E::SignalType::Discrete, 4, &obStates));

	QVERIFY(obStates.size() == 4);

	for(int i = 0; i < 4; i++)
	{
		QVERIFY(obStates[i].action == VcsItemAction::Added);
		QVERIFY(obStates[i].checkedOut == true);
	}

	int sg2 = 0;
	TS_VERIFY(getSignalGroupID(obStates[0].id, &sg2));

	TS_VERIFY(check_signalIsExist(USER3_ID, obStates[0].id, E::SignalType::Discrete, 0, sg2, true));
	TS_VERIFY(check_signalIsExist(USER3_ID, obStates[1].id, E::SignalType::Discrete, 1, sg2, true));
	TS_VERIFY(check_signalIsExist(USER3_ID, obStates[2].id, E::SignalType::Discrete, 2, sg2, true));
	TS_VERIFY(check_signalIsExist(USER3_ID, obStates[3].id, E::SignalType::Discrete, 3, sg2, true));

	// add 9 (wrong) channel signal
	//
	QString errStr = addSignal(ADMIN_ID, E::SignalType::Discrete, 9, &obStates).isEmpty() == false;
	QVERIFY2(errStr.isEmpty() == false, "Error should be rised!");

	db.close();
}

void DbControllerSignalTests::test_checkinSignals()
{
	// Testing of stored procedure:
	//
	//		checkin_signals(user_id integer,
	//						signal_ids integer[],
	//						checkin_comment text)
	//							RETURNS SETOF objectstate

	OPEN_DATABASE();

	std::vector<ObjectState> obStates;

	// try ckeck in unknown signal
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({100500}), "checking", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	// -----------------------------------------------------------------------------------------------
	//
	//	add first signal by USER2_ID
	//
	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Analog, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Added);
	QVERIFY(obStates[0].checkedOut == true);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	int id1 = obStates[0].id;

	// try checkin by user USER2_ID
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id1}), "First checking", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Unknown);
	QVERIFY(obStates[0].checkedOut == false);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	TS_VERIFY(check_signalIsCheckedIn(id1));

	// -----------------------------------------------------------------------------------------------
	//
	// add second signal by USER2_ID
	//
	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Bus, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Added);
	QVERIFY(obStates[0].checkedOut == true);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	int id2 = obStates[0].id;

	// try checkin by another user USER3_ID
	//
	TS_VERIFY(checkinSignals(USER3_ID, std::vector<int>({id2}), "Second checking", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Unknown);
	QVERIFY(obStates[0].checkedOut == true);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try checkin by Adminstrator
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, std::vector<int>({id2}), "Second checking", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Unknown);
	QVERIFY(obStates[0].checkedOut == false);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	TS_VERIFY(check_signalIsCheckedIn(id2));

	// try check in of already checkedin signal
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id2}), "Second checking", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].action == VcsItemAction::Unknown);
	QVERIFY(obStates[0].checkedOut == false);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	// -----------------------------------------------------------------------------------------------
	//
	// add three signals byUSER2_ID
	//
	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Discrete, 3, &obStates));
	QVERIFY(obStates.size() == 3);

	std::vector<int> ids3;

	for(int i = 0; i < 3; i++)
	{
		QVERIFY(obStates[i].errCode == ERR_SIGNAL_OK);
		ids3.push_back(obStates[i].id);
	}

	// try check in three signals
	//
	TS_VERIFY(checkinSignals(USER2_ID, ids3, "Third checking", &obStates));
	QVERIFY(obStates.size() == 3);

	for(int i = 0; i < 3; i++)
	{
		QVERIFY(obStates[i].action == VcsItemAction::Unknown);
		QVERIFY(obStates[i].checkedOut == false);
		QVERIFY(obStates[i].errCode == ERR_SIGNAL_OK);

		TS_VERIFY(check_signalIsCheckedIn(ids3[i]));
	}

	// add tests CheckIn of DELETED signal!

}

void DbControllerSignalTests::test_checkoutSignals()
{
	OPEN_DATABASE();

	std::vector<ObjectState> obStates;

	// add signal and checkin it
	//
	TS_VERIFY(addSignal(USER3_ID, E::SignalType::Discrete, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	int id1 = obStates[0].id;

	TS_VERIFY(checkinSignals(USER3_ID, std::vector({ id1 }), "Checkin comment", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	// try checkout NOT exist signal
	//
	TS_VERIFY(checkoutSignals(USER3_ID, std::vector({ id1 + 100500 }), &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_NOT_FOUND);

	// try checkout signal by USER2_ID
	//
	TS_VERIFY(checkoutSignals(USER2_ID, std::vector({ id1 }), &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	VcsItemAction action;
	TS_VERIFY(check_signalIsCheckedOut(id1, &action));
	QVERIFY(action == VcsItemAction::Modified);

	// try checkout by another USER3_ID
	//
	TS_VERIFY(checkoutSignals(USER3_ID, std::vector({ id1 }), &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try checkout by Admin
	//
	TS_VERIFY(checkoutSignals(ADMIN_ID, std::vector({ id1 }), &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try to check out prevously deleted signal
	//
	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Bus, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	int id2 = obStates[0].id;

	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id2}), "Checkin signal", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	ObjectState obState;

	TS_VERIFY(deleteSignal(USER2_ID, id2, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id2}), "Checkin signal", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	TS_VERIFY(checkoutSignals(USER2_ID, std::vector<int>({id2}), &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_DELETED);
}

void DbControllerSignalTests::test_deleteSignal()
{
	// Testing of stored procedure:
	//
	//	delete_signal(
	//		user_id integer,
	//		signal_id integer)
	//		RETURNS objectstate
	//

	OPEN_DATABASE();

	std::vector<ObjectState> obStates;
	ObjectState obState;

	// add TWO channel signal
	// both signals are checked out and have not checked in instances
	//
	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Discrete, 2, &obStates));
	QVERIFY(obStates.size() == 2);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);
	QVERIFY(obStates[1].errCode == ERR_SIGNAL_OK);

	int id1 = obStates[0].id;
	int id2 = obStates[1].id;

	int signalGroupID1 = 0;

	TS_VERIFY(getSignalGroupID(id1, &signalGroupID1));

	// try delete first channel of signal by another user
	//
	TS_VERIFY(deleteSignal(USER3_ID, id1, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try delete first channel of signal
	//
	TS_VERIFY(deleteSignal(USER2_ID, id1, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	QSqlQuery q;

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalid=%1").arg(id1));
	QVERIFY2(q.size() == 0, "SignalInstance record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM Signal WHERE signalid=%1").arg(id1));
	QVERIFY2(q.size() == 0, "Signal record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM CheckOut WHERE signalid=%1").arg(id1));
	QVERIFY2(q.size() == 0, "CheckOut record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalGroup WHERE signalgroupid=%1").arg(signalGroupID1));
	QVERIFY2(q.size() == 1, "SignalGroup record is deleted");

	// try delete second channel of signal
	//
	TS_VERIFY(deleteSignal(USER2_ID, id2, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalid=%1").arg(id1));
	QVERIFY2(q.size() == 0, "SignalInstance record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM Signal WHERE signalid=%1").arg(id1));
	QVERIFY2(q.size() == 0, "Signal record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM CheckOut WHERE signalid=%1").arg(id1));
	QVERIFY2(q.size() == 0, "CheckOut record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalGroup WHERE signalgroupid=%1").arg(signalGroupID1));
	QVERIFY2(q.size() == 0, "SignalGroup record is not deleted");

	// add THREE channel signal
	//
	TS_VERIFY(addSignal(USER3_ID, E::SignalType::Analog, 3, &obStates));
	QVERIFY(obStates.size() == 3);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);
	QVERIFY(obStates[1].errCode == ERR_SIGNAL_OK);
	QVERIFY(obStates[2].errCode == ERR_SIGNAL_OK);

	int sid1 = obStates[0].id;
	int sid2 = obStates[1].id;
	int sid3 = obStates[2].id;

	int sg2 = 0;

	TS_VERIFY(getSignalGroupID(sid1, &sg2));

	//	Check In second and third channel
	//
	TS_VERIFY(checkinSignals(USER3_ID, std::vector<int>({ sid2, sid3 }), "Signal check in", &obStates));
	QVERIFY(obStates.size() == 2);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);
	QVERIFY(obStates[1].errCode == ERR_SIGNAL_OK);

	// check out second channel by USER3_ID and try delete by another user
	//
	TS_VERIFY(checkoutSignals(USER3_ID, std::vector({ sid2 }), &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	TS_VERIFY(deleteSignal(USER2_ID, sid2, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try delete by current USER3_ID
	//
	TS_VERIFY(deleteSignal(USER3_ID, sid2, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_EXEC_QUERY(q, QString("SELECT * FROM Signal WHERE signalid=%1").arg(sid2));
	QVERIFY(q.first() == true);

	int chInInstanceID = q.value("checkedininstanceid").toInt();
	int chOutInstanceID = q.value("checkedoutinstanceid").toInt();

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalinstanceid=%1").arg(chInInstanceID));
	QVERIFY(q.first() == true);

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalinstanceid=%1").arg(chOutInstanceID));
	QVERIFY(q.first() == true);
	QVERIFY(q.value("action").toInt() == static_cast<int>(VcsItemAction::Deleted));

	// delete checked in 3 channel
	//
	TS_VERIFY(deleteSignal(USER2_ID, sid3, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	// signal should be automatically checked out
	//
	VcsItemAction action;
	TS_VERIFY(check_signalIsCheckedOut(sid3, &action));
	QVERIFY(action == VcsItemAction::Deleted);
}

void DbControllerSignalTests::test_setSignalWorkcopy()
{
	// Testing of stored procedure:
	//
	//	set_signal_workcopy(user_id integer,
	//						sd signaldata)
	//						RETURNS objectstate

	OPEN_DATABASE();

	std::vector<ObjectState> obStates;

	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Discrete, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	int id1 = obStates[0].id;

	const QString appSignalID1("#SET_SIGNAL_WORKCOPY_SIGNAL_1");
	const QString appSignalID2("#SET_SIGNAL_WORKCOPY_SIGNAL_2");
	const QString customAppSignalID1("SET_SIGNAL_WORKCOPY_SIGNAL1");
	const QString caption1("Set signal workcopy signal 1");
	const QString equipmentID1("EQUIPMENT_SET_SIGNAL_WORKCOPY_SIGNAL1");
	const QString specPropStruct1("Dummy SpecProStruct string");

	const QByteArray specPropValues1("SpecPropValues_dummy_array", 26 + 1);

	Signal s;

	s.setID(id1);

	s.initCreatedDates();

	s.setAppSignalID(appSignalID1);
	s.setCustomAppSignalID(customAppSignalID1);
	s.setEquipmentID(equipmentID1);
	s.setInOutType(E::SignalInOutType::Output);

	s.setSpecPropStruct(specPropStruct1);
	s.setProtoSpecPropValues(specPropValues1);

	// initialization of some fields wich will be save in SignalInstance.protoData
	//
	s.setCaption(caption1);
	s.setAcquire(true);
	s.setArchive(false);

	ObjectState obState;

	TS_VERIFY(setSignalWorkcopy(USER2_ID, s, &obState));

	QVERIFY2(obState.errCode == ERR_SIGNAL_OK, "Error: setSignalWorkcopy returns not ERR_SIGNAL_OK");

	QSqlQuery q;

	TS_VERIFY(TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalid=%1").arg(id1)));
	QVERIFY(q.size() == 1);
	QVERIFY(q.first());

	QVERIFY2(q.value("appsignalid").toString() == appSignalID1, "Error: SignalInstance.AppSignalID");
	QVERIFY2(q.value("customappsignalid").toString() == customAppSignalID1, "Error: SignalInstance.CustomAppSignalID");
	QVERIFY2(q.value("equipmentid").toString() == equipmentID1, "Error: SignalInstance.EquipmentID");
	QVERIFY2(q.value("inouttype").toInt() == static_cast<int>(E::SignalInOutType::Output), "Error: SignalInstance.InOutType");

	QVERIFY2(q.value("specpropstruct").toString() == specPropStruct1, "Error: SignalInstance.SpecPropStruct");
	QVERIFY2(q.value("specpropvalues").toByteArray().endsWith(specPropValues1), "Error: SignalInstance.SpecPropValues");

	Proto::ProtoAppSignalData protoData;

	QByteArray protoBinData = q.value("protodata").toByteArray();

	QVERIFY2(protoData.ParseFromArray(protoBinData, protoBinData.size()) == true, "Error parsing Signal.protodata");

	QVERIFY2(QString::fromStdString(protoData.caption()) == caption1, "Error: Signal.Caption");
	QVERIFY2(protoData.acquire() == true, "Error: Signal.Acquire");
	QVERIFY2(protoData.archive() == false, "Error: Signal.Archive");

	// try serWorkCopy by another user
	//

	TS_VERIFY(setSignalWorkcopy(USER3_ID, s, &obState));

	QVERIFY2(obState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER, "Error: setSignalWorkcopy returns not ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER");

	// check in signal
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id1}), "Checkin comment1", &obStates));

	// try setSignalWorkcopy for checked in signal
	//
	TS_VERIFY(setSignalWorkcopy(USER2_ID, s, &obState));
	QVERIFY2(obState.errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT, "Error: setSignalWorkcopy returns not ERR_SIGNAL_IS_NOT_CHECKED_OUT");

	// create another one signal
	//
	TS_VERIFY(addSignal(USER3_ID, E::SignalType::Discrete, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	s.setID(obStates[0].id);

	// try set Signal.appSignalID equal to appSignalID1
	//
	QString res = setSignalWorkcopy(USER3_ID, s, &obState);

	QVERIFY2(res.contains("55011"), "Error: setSignalWorkcopy not raise 550011 error");

	s.setAppSignalID(appSignalID1 + "_COPY");

	// try set Signal.customAppSignalID equal to customAppSignalID1
	//
	res = setSignalWorkcopy(USER3_ID, s, &obState);

	QVERIFY2(res.contains("55022"), "Error: setSignalWorkcopy not raise 550022 error");

	s.setCustomAppSignalID(customAppSignalID1 + "_COPY");

	// now should be OK
	//
	TS_VERIFY(setSignalWorkcopy(USER3_ID, s, &obState);)

	//
	//
	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Analog, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	s.setID(obStates[0].id);

	s.setAppSignalID(appSignalID2);
	s.setCustomAppSignalID("$(Macro)_CUSTOM_ID");

	TS_VERIFY(setSignalWorkcopy(USER2_ID, s, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_VERIFY(addSignal(USER2_ID, E::SignalType::Analog, 1, &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	// change appSignalID, customAppSignalID is not change
	//
	s.setID(obStates[0].id);
	s.setAppSignalID(s.appSignalID() + "_COPY");

	// $(Macro) in customAppSignalID should disable uniquiness checking of cunstomAppSignalID
	//
	TS_VERIFY(setSignalWorkcopy(USER2_ID, s, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	db.close();
}

void DbControllerSignalTests::test_getSignalsIDs()
{
	// Testing of stored procedure:
	//
	//	get_signals_ids (user_id INTEGER, with_deleted BOOLEAN) RETURNS SETOF INTEGER
	//

	OPEN_DATABASE();

	// Request ALL exists signals IDs for now
	//
	QSqlQuery q;

//	TS_EXEC_QUERY(q, "SELECT signalid FROM Signal");

	std::vector<int> initialIDs;

	TS_VERIFY(getAllSignalIDs(&initialIDs));
/*	while(q.next() == true)
	{
		initialIDs.push_back(q.value(0).toInt());
	}*/

	std::vector<ObjectState> obStates;

	// add signals by user ADMIN_ID
	//
	std::vector<int> adminSignalsIDs;

	TS_VERIFY(addTestSignals(ADMIN_ID, E::SignalType::Bus, 1, 5 + rand0to(4), &adminSignalsIDs));

	// add signals by user USER2_ID
	//
	std::vector<int> user2SignalsIDs;

	TS_VERIFY(addTestSignals(USER2_ID, E::SignalType::Discrete, 2, 4 + rand0to(5), &user2SignalsIDs));

	// add signals by user USER3_ID
	//
	std::vector<int> user3SignalsIDs;

	TS_VERIFY(addTestSignals(USER3_ID, E::SignalType::Analog, 3, 2 + rand0to(2), &user3SignalsIDs));

	// checking that sets is NOT intersect
	//
	QVERIFY(sets_intersect<int>(adminSignalsIDs, user2SignalsIDs) == false);
	QVERIFY(sets_intersect<int>(adminSignalsIDs, user3SignalsIDs) == false);
	QVERIFY(sets_intersect<int>(user2SignalsIDs, user3SignalsIDs) == false);

	std::vector<int> ids;

	// request IDs under Admin
	// signals added by Admin, User2 and User3 should  be visible
	//
	TS_VERIFY(getSignalsIDs(ADMIN_ID, true, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, sets_union(adminSignalsIDs, sets_union(user2SignalsIDs, user3SignalsIDs))) == true);

	// request IDs under User2
	// only signals added by User2 should  be visible
	//
	TS_VERIFY(getSignalsIDs(USER2_ID, true, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, user2SignalsIDs) == true);

	// checkin signals added by User2
	//
	TS_VERIFY(checkinSignals(USER2_ID, user2SignalsIDs, "user2 checkin", &obStates));

	// request IDs under User3
	// signals added by User3 and checked in by User2 should  be visible
	//
	TS_VERIFY(getSignalsIDs(USER3_ID, true, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, sets_union(user2SignalsIDs, user3SignalsIDs)));

	// request IDs under Admin
	// signals added by Admin, User3 and checked in by User2 should  be visible
	//
	TS_VERIFY(getSignalsIDs(ADMIN_ID, true, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, sets_union(adminSignalsIDs, sets_union(user2SignalsIDs, user3SignalsIDs))) == true);

	// checkin signals added by Admin and User3
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, adminSignalsIDs, "admin checkin", &obStates));
	TS_VERIFY(checkinSignals(USER3_ID, user3SignalsIDs, "user3 checkin", &obStates));

	// request IDs under User2
	// signals added by Admin, User2 and User3 should  be visible
	//
	TS_VERIFY(getSignalsIDs(USER2_ID, true, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, sets_union(adminSignalsIDs, sets_union(user2SignalsIDs, user3SignalsIDs))) == true);

	// Delete and checkin signals added by User2
	//
	TS_VERIFY(deleteSignals(USER2_ID, user2SignalsIDs, &obStates));
	TS_VERIFY(checkinSignals(USER2_ID, user2SignalsIDs, "checkin deleted", &obStates));

	// request withDeleted == true
	// Admin, User2 (deleted), User3 signals should be visible
	//
	TS_VERIFY(getSignalsIDs(USER3_ID, true, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, sets_union(adminSignalsIDs, sets_union(user2SignalsIDs, user3SignalsIDs))) == true);

	// request withDeleted == false
	// Admin, User3 signals should be visible
	//
	TS_VERIFY(getSignalsIDs(USER3_ID, false, &ids));

	ids = sets_difference<int>(ids, initialIDs);

	QVERIFY(sets_equal(ids, sets_union(adminSignalsIDs, user3SignalsIDs)) == true);

	db.close();
}

void DbControllerSignalTests::test_getSignalsIDAppSignalID()
{
	OPEN_DATABASE();

	std::vector<int> initialIDs;

	TS_VERIFY(getAllSignalIDs(&initialIDs));

	std::vector<int> adminSignalsIDs;

	TS_VERIFY(addTestSignals(ADMIN_ID, E::SignalType::Discrete, 2, 2 + rand0to(3), &adminSignalsIDs));

	std::vector<int> user2SignalsIDs;

	TS_VERIFY(addTestSignals(USER2_ID, E::SignalType::Analog, 1, 5 + rand0to(2), &user2SignalsIDs));

	std::vector<std::pair<int,QString>> result;

	// Admin should see all signals
	//
	TS_VERIFY(getSignalsIDAppSignalID(ADMIN_ID, false, &result));
	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == adminSignalsIDs.size() + user2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(sets_union(adminSignalsIDs, user2SignalsIDs), result));

	// User2 should see only their signals
	//
	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, false, &result));
	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == user2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(user2SignalsIDs, result));

	// Checkin Admin's signals
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, adminSignalsIDs, "checkin admin signals", nullptr));

	// User2 should see their signals and Admin's signals
	//
	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, false, &result));
	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == adminSignalsIDs.size() + user2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(sets_union(adminSignalsIDs, user2SignalsIDs), result));

	// Delete Admin's signals
	//
	TS_VERIFY(deleteSignals(ADMIN_ID, adminSignalsIDs, nullptr));
	TS_VERIFY(checkinSignals(ADMIN_ID, adminSignalsIDs, "checkin admin deleted signals", nullptr));

	// withDeleted == false, User2 should see only their signals
	//
	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, false, &result));
	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == user2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(user2SignalsIDs, result));

	// withDeleted == true, User2 should see their signals and Admin's signals
	//
	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, true, &result));
	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == adminSignalsIDs.size() + user2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(sets_union(adminSignalsIDs, user2SignalsIDs), result));

	int id = user2SignalsIDs[0];

	std::pair<int, QString> p;

	QVERIFY(findPairWithID(id, result, &p));

	const QString OLD_APP_SIGNAL_ID(p.second);

	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id}), "checkin user2 signal", nullptr));

	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, true, &result));

	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == OLD_APP_SIGNAL_ID);

	// try change AppSignalID under User3
	//
	Signal s;

	const QString NEW_APP_SIGNAL_ID("#USER3_TEST_SIGNAL_APP_SIGNAL_ID_4567");

	s.initCreatedDates();

	s.setID(id);
	s.setAppSignalID(NEW_APP_SIGNAL_ID);

	TS_VERIFY(checkoutSignals(USER3_ID, std::vector<int>({id}), nullptr));
	TS_VERIFY(setSignalWorkcopy(USER3_ID, s,nullptr));

	// under User2, should see OLD_APP_SIGNAL_ID
	//
	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, true, &result));
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == OLD_APP_SIGNAL_ID);

	// under Admin and User3, should see NEW_APP_SIGNAL_ID
	//
	TS_VERIFY(getSignalsIDAppSignalID(ADMIN_ID, true, &result));
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	TS_VERIFY(getSignalsIDAppSignalID(USER3_ID, true, &result));
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	TS_VERIFY(checkinSignals(USER3_ID, std::vector<int>({id}), "checkin user3 signal", nullptr));

	// All should see NEW_APP_SIGNAL_ID
	//
	TS_VERIFY(getSignalsIDAppSignalID(ADMIN_ID, true, &result));
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	TS_VERIFY(getSignalsIDAppSignalID(USER2_ID, true, &result));
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	TS_VERIFY(getSignalsIDAppSignalID(USER3_ID, true, &result));
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	db.close();
}

void DbControllerSignalTests::dbcTest_addSignal()
{
	// Testing of:
	//
	//	bool DbController::addSignal(E::SignalType signalType, QVector<Signal>* newSignal, QWidget* parentWidget);
	//

	OPEN_DATABASE();

	QVector<Signal> newSignals;

	Signal s;

	s.setID(-1);
	s.setAppSignalID("#DBC_ADD_SIGNAL_TEST_1");
	s.setCustomAppSignalID("DBC_ADD_SIGNAL_TEST_1");
	s.setCaption("Caption DBC_ADD_SIGNAL_TEST_1");

	newSignals.append(s);

	QVERIFY(m_db->addSignal(E::SignalType::Discrete, &newSignals, nullptr) == true);
	TS_VERIFY(check_signalIsExist(ADMIN_ID, newSignals[0].ID(), E::SignalType::Discrete, 0, 0, true));

	// try add another one signal with same IDs
	//
	newSignals.clear();
	newSignals.append(s);

	QVERIFY(m_db->addSignal(E::SignalType::Discrete, &newSignals, nullptr) == false);
	QVERIFY(m_db->lastError().contains("already exists"));

	// try add multichannel signal
	//
	newSignals.clear();

	for(int i = 0; i < 4; i++)
	{
		Signal s;

		s.setID(-1);
		s.setAppSignalID(QString("#DBC_ADD_SIGNAL_TEST_S%1").arg(i));
		s.setCustomAppSignalID(QString("DBC_ADD_SIGNAL_TEST_S%1").arg(i));
		s.setCaption(QString("Caption DBC_ADD_SIGNAL_TEST_S%1").arg(i));

		newSignals.append(s);
	}

	QVERIFY(m_db->addSignal(E::SignalType::Analog, &newSignals, nullptr) == true);

	int sg1 = 0;

	TS_VERIFY(getSignalGroupID(newSignals[0].ID(), &sg1));

	for(int i = 0; i < 4; i++)
	{
		TS_VERIFY(check_signalIsExist(ADMIN_ID, newSignals[i].ID(), E::SignalType::Analog, i, sg1, true));
	}

	// try add 5-channel signal
	//
	newSignals.clear();

	for(int i = 0; i < 5; i++)
	{
		Signal s;

		s.setID(-1);
		s.setAppSignalID(QString("#DBC_ADD_SIGNAL_TEST_SS%1").arg(i));
		s.setCustomAppSignalID(QString("DBC_ADD_SIGNAL_TEST_SS%1").arg(i));
		s.setCaption(QString("Caption DBC_ADD_SIGNAL_TEST_SS%1").arg(i));

		newSignals.append(s);
	}

	QVERIFY(m_db->addSignal(E::SignalType::Analog, &newSignals, nullptr) == false);
	QVERIFY(m_db->lastError().contains("channelCount"));

	db.close();
}

/*
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
	newSignal.setCoarseAperture(1.3);
	newSignal.setFineAperture(0.8);
	newSignal.setAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID("firstSignalForGetSignalIdsTest");
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngineeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngineeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("mV");

	signalsToAdd.push_back(newSignal);

	newSignal.setCaption("firstSignalForGetSignalIdsTest");
	newSignal.setAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setCustomAppSignalID("firstSignalForGetSignalIdsTest");
	newSignal.setEquipmentID("firstSignalForGetSignalIdsTest");

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
	newSignal.setCoarseAperture(1.3);
	newSignal.setAppSignalID(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID(firstCaption);
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID(firstCaption);
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngineeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngineeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("kg");

	signalsToAdd.push_back(newSignal);

	newSignal.setCaption(secondCaption);
	newSignal.setAppSignalID(secondCaption);
	newSignal.setCustomAppSignalID(secondCaption);
	newSignal.setEquipmentID(secondCaption);

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
	newSignal.setCoarseAperture(1.3);
	newSignal.setFineAperture(0.34);
	newSignal.setAppSignalID(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID(firstCaption);
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID(firstCaption);
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngineeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngineeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("kg");

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
	//QVERIFY2(query.value("analogSignalFormat").toInt() == resultSignal.dataFormatInt(), qPrintable(QString("Error: analogSignalFormat is wrong")));
	QVERIFY2(query.value("dataSize").toInt() == resultSignal.dataSize(), qPrintable(QString("Error: dataSize is wrong")));
	QVERIFY2(query.value("lowAdc").toInt() == resultSignal.lowADC(), qPrintable(QString("Error: lowAdc is wrong")));
	QVERIFY2(query.value("highAdc").toInt() == resultSignal.highADC(), qPrintable(QString("Error: highAdc is wrong")));
	QVERIFY2(query.value("unit").toString() == resultSignal.unit(), qPrintable(QString("Error: unit is wrong")));
	QVERIFY2(query.value("electricLowLimit").toDouble() == resultSignal.electricLowLimit(), qPrintable(QString("Error: electricLowLimit is wrong")));
	QVERIFY2(query.value("electricHighLimit").toDouble() == resultSignal.electricHighLimit(), qPrintable(QString("Error: electricHighLimit is wrong")));
	QVERIFY2(query.value("electricUnit").toInt() == resultSignal.electricUnit(), qPrintable(QString("Error: electricUnit is wrong")));
	QVERIFY2(query.value("acquire").toBool() == resultSignal.acquire(), qPrintable(QString("Error: acquire is wrong")));
	QVERIFY2(query.value("decimalPlaces").toInt() == resultSignal.decimalPlaces(), qPrintable(QString("Error: decimalPlaces is wrong")));
	QVERIFY2(query.value("coarseAperture").toDouble() == resultSignal.coarseAperture(), qPrintable(QString("Error: coarse aperture is wrong")));
	QVERIFY2(query.value("fineAperture").toDouble() == resultSignal.fineAperture(), qPrintable(QString("Error: fine aperture is wrong")));
	QVERIFY2(query.value("inOutType").toInt() == int(resultSignal.inOutType()), qPrintable(QString("Error: inOutType is wrong")));
	QVERIFY2(query.value("equipmentID").toString() == resultSignal.equipmentID(), qPrintable(QString("Error: deviceStrId is wrong")));
	QVERIFY2(query.value("filteringTime").toDouble() == resultSignal.filteringTime(), qPrintable(QString("Error: filteringTime is wrong")));
	QVERIFY2(query.value("byteOrder").toInt() == resultSignal.byteOrder(), qPrintable(QString("Error: byteOrder is wrong")));
	QVERIFY2(query.value("enableTuning").toBool() == resultSignal.enableTuning(), qPrintable(QString("Error: enableTuning is wrong")));
//	QVERIFY2(query.value("tuningDefaultValue").toDouble() == resultSignal.tuningDefaultValue(), qPrintable(QString("Error: tuningDefaultValue is wrong")));

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
	newSignal.setCoarseAperture(1.32);
	newSignal.setFineAperture(1.0);
	newSignal.setAppSignalID(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID(firstCaption);
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID(firstCaption);
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngineeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngineeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("kg");

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
	resultSignal.setCoarseAperture(1.4);
	resultSignal.setFineAperture(1.1);
	resultSignal.setByteOrder(E::ByteOrder::BigEndian);
	resultSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	resultSignal.setDataSize(40);
	resultSignal.setDecimalPlaces(4);
	resultSignal.setEnableTuning(false);
	resultSignal.setFilteringTime(9.7);
	resultSignal.setHighADC(243);
	resultSignal.setHighEngineeringUnits(1783.7);
	resultSignal.setHighValidRange(2333.8);
	resultSignal.setInOutType(E::SignalInOutType::Output);
	resultSignal.setElectricHighLimit(1928.3);
	resultSignal.setElectricLowLimit(12.8);
	resultSignal.setElectricUnit(E::ElectricUnit::V);
	resultSignal.setLowADC(4321);
	resultSignal.setLowEngineeringUnits(123.4);
	resultSignal.setLowValidRange(125.3);
	resultSignal.setOutputMode(E::OutputMode::Minus10_Plus10_V);
	resultSignal.setSpreadTolerance(2346.8);
	resultSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	resultSignal.setUnit("kq");

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
	QVERIFY2(query.value("unit").toString() == resultSignal.unit(), qPrintable(QString("Error: unit is wrong")));
	QVERIFY2(query.value("electricLowLimit").toDouble() == resultSignal.electricLowLimit(), qPrintable(QString("Error: electricLowLimit is wrong")));
	QVERIFY2(query.value("electricHighLimit").toDouble() == resultSignal.electricHighLimit(), qPrintable(QString("Error: electricHighLimit is wrong")));
	QVERIFY2(query.value("electricUnit").toInt() == resultSignal.electricUnit(), qPrintable(QString("Error: electricUnit is wrong")));
	QVERIFY2(query.value("acquire").toBool() == resultSignal.acquire(), qPrintable(QString("Error: acquire is wrong")));
	QVERIFY2(query.value("decimalPlaces").toInt() == resultSignal.decimalPlaces(), qPrintable(QString("Error: decimalPlaces is wrong")));
	QVERIFY2(query.value("coarseAperture").toDouble() == resultSignal.coarseAperture(), qPrintable(QString("Error: coarse aperture is wrong")));
	QVERIFY2(query.value("fineAperture").toDouble() == resultSignal.fineAperture(), qPrintable(QString("Error: fine aperture is wrong")));
	QVERIFY2(query.value("inOutType").toInt() == int(resultSignal.inOutType()), qPrintable(QString("Error: inOutType is wrong")));
	QVERIFY2(query.value("equipmentID").toString() == resultSignal.equipmentID(), qPrintable(QString("Error: deviceStrId is wrong")));
	QVERIFY2(query.value("filteringTime").toDouble() == resultSignal.filteringTime(), qPrintable(QString("Error: filteringTime is wrong")));
	QVERIFY2(query.value("byteOrder").toInt() == resultSignal.byteOrder(), qPrintable(QString("Error: byteOrder is wrong")));
	QVERIFY2(query.value("enableTuning").toBool() == resultSignal.enableTuning(), qPrintable(QString("Error: enableTuning is wrong")));
//	QVERIFY2(query.value("tuningDefaultValue").toDouble() == resultSignal.tuningDefaultValue(), qPrintable(QString("Error: tuningDefaultValue is wrong")));

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
	newSignal.setCoarseAperture(1.3);
	newSignal.setFineAperture(1.0);
	newSignal.setAppSignalID(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID(firstCaption);
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID(firstCaption);
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngineeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngineeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setOutputMode(E::OutputMode::Plus0_Plus5_mA);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("kl");

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
	newSignal.setCoarseAperture(1.3);
	newSignal.setFineAperture(1.1);
	newSignal.setAppSignalID(firstCaption);
	newSignal.setByteOrder(E::ByteOrder::LittleEndian);
	newSignal.setCustomAppSignalID(firstCaption);
	newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
	newSignal.setDataSize(30);
	newSignal.setDecimalPlaces(3);
	newSignal.setEnableTuning(true);
	newSignal.setEquipmentID(firstCaption);
	newSignal.setFilteringTime(7.3);
	newSignal.setHighADC(500);
	newSignal.setHighEngineeringUnits(3245.6);
	newSignal.setHighValidRange(3546.4);
	newSignal.setInOutType(E::SignalInOutType::Input);
	newSignal.setElectricHighLimit(2345.3);
	newSignal.setElectricLowLimit(134.4);
	newSignal.setElectricUnit(E::ElectricUnit::V);
	newSignal.setLowADC(1234);
	newSignal.setLowEngineeringUnits(345.1);
	newSignal.setLowValidRange(134.9);
	newSignal.setSpreadTolerance(35634.6);
	newSignal.setSignalType(E::SignalType::Discrete);
	newSignal.setUnit("qq");

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

	bool ok = m_dbController->getSignals(&signalsFromDb, false, 0);
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
		QVERIFY2(query.value("analogSignalFormat").toInt() == currentSignal.analogSignalFormatInt(), qPrintable(QString("Error: value analogSignalFormat is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("analogSignalFormat").toInt()).arg(currentSignal.analogSignalFormatInt())));
		QVERIFY2(query.value("dataSize").toInt() == currentSignal.dataSize(), qPrintable(QString("Error: value dataSize is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("dataSize").toInt()).arg(currentSignal.dataSize())));
		QVERIFY2(query.value("lowAdc").toInt() == currentSignal.lowADC(), qPrintable(QString("Error: value lowAdc is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("lowAdc").toInt()).arg(currentSignal.lowADC())));
		QVERIFY2(query.value("highAdc").toInt() == currentSignal.highADC(), qPrintable(QString("Error: value highAdc is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("highAdc").toInt()).arg(currentSignal.highADC())));
		QVERIFY2(query.value("unit").toString() == currentSignal.unit(), qPrintable(QString("Error: value unit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("unit").toString()).arg(currentSignal.unit())));
		QVERIFY2(query.value("lowvalidrange").toInt() == currentSignal.lowValidRange(), qPrintable(QString("Error: value lowvalidrange is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("lowvalidrange").toInt()).arg(currentSignal.lowValidRange())));
		QVERIFY2(query.value("highvalidrange").toInt() == currentSignal.highValidRange(), qPrintable(QString("Error: value highvalidrange is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("highvalidrange").toInt()).arg(currentSignal.highValidRange())));
		QVERIFY2(query.value("electricLowLimit").toInt() == currentSignal.electricLowLimit(), qPrintable(QString("Error: value electricLowLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("electricLowLimit").toInt()).arg(currentSignal.electricLowLimit())));
		QVERIFY2(query.value("electricHighLimit").toInt() == currentSignal.electricHighLimit(), qPrintable(QString("Error: value electricHighLimit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("electricHighLimit").toInt()).arg(currentSignal.electricHighLimit())));
		QVERIFY2(query.value("electricUnit").toInt() == currentSignal.electricUnit(), qPrintable(QString("Error: value electricUnit is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("electricUnit").toInt()).arg(currentSignal.electricUnit())));
		QVERIFY2(query.value("sensorType").toInt() == currentSignal.sensorType(), qPrintable(QString("Error: value sensorType is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("sensorType").toInt()).arg(currentSignal.sensorType())));
		QVERIFY2(query.value("acquire").toBool() == currentSignal.acquire(), qPrintable(QString("Error: value acquire is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("acquire").toBool()).arg(currentSignal.acquire())));
		QVERIFY2(query.value("decimalPlaces").toInt() == currentSignal.decimalPlaces(), qPrintable(QString("Error: value decimalPlaces is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("decimalPlaces").toInt()).arg(currentSignal.decimalPlaces())));
		QVERIFY2(query.value("coarseAperture").toInt() == currentSignal.coarseAperture(), qPrintable(QString("Error: value coarse aperture is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("coarseAperture").toDouble()).arg(currentSignal.coarseAperture())));
		QVERIFY2(query.value("fineAperture").toInt() == currentSignal.fineAperture(), qPrintable(QString("Error: value fine aperture is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("fineAperture").toDouble()).arg(currentSignal.fineAperture())));
		QVERIFY2(query.value("inOutType").toInt() == currentSignal.inOutTypeInt(), qPrintable(QString("Error: value inOutType is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("inOutType").toInt()).arg(currentSignal.inOutTypeInt())));
		QVERIFY2(query.value("equipmentID").toString() == currentSignal.equipmentID(), qPrintable(QString("Error: value equipmentID is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("equipmentID").toString()).arg(currentSignal.equipmentID())));
		//QVERIFY2(query.value("outputRangeMode").toInt() == currentSignal.outputModeInt(), qPrintable(QString("Error: value outputRangeMode is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("outputRangeMode").toInt()).arg(currentSignal.outputModeInt())));
		QVERIFY2(query.value("filteringTime").toDouble() == currentSignal.filteringTime(), qPrintable(QString("Error: value filteringTime is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("filteringTime").toInt()).arg(currentSignal.filteringTime())));
		QVERIFY2(query.value("spreadtolerance").toInt() == currentSignal.spreadTolerance(), qPrintable(QString("Error: value spreadtolerance is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("spreadtolerance").toInt()).arg(currentSignal.spreadTolerance())));
		QVERIFY2(query.value("byteOrder").toInt() == currentSignal.byteOrder(), qPrintable(QString("Error: value byteOrder is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("byteOrder").toInt()).arg(currentSignal.byteOrder())));
		QVERIFY2(query.value("enableTuning").toBool() == currentSignal.enableTuning(), qPrintable(QString("Error: value enableTuning is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("enableTuning").toBool()).arg(currentSignal.enableTuning())));
		QVERIFY2(query.value("tuningDefaultValue").toDouble() == currentSignal.tuningDefaultValue().toDouble(), qPrintable(QString("Error: value tuningDefaultValue is not match (Database: %1, DbControllerFunction: %2)").arg(query.value("tuningDefaultValue").toDouble()).arg(currentSignal.tuningDefaultValue().toDouble())));

		signalNumber++;
	}

	db.close();
} */

void DbControllerSignalTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	m_db->deleteProject(m_projectName, m_adminPassword, true, 0);
}

bool DbControllerSignalTests::openDatabase(QSqlDatabase& db)
{
	db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName(m_databaseName);

	return db.open();
}

QString DbControllerSignalTests::applyFutureDatabaseUpgrade()
{
	QSqlDatabase db;

	TS_VERIFY_RETURN_ERR(openDatabase(db) == true, QString("Can't connect to database %1, error: %2" ).
														arg(m_databaseName).arg(db.lastError().text()));

	QFile upgradeFile(":/FutureDatabaseUpgrade/FutureUpgrade.sql");

	bool result = upgradeFile.open(QIODevice::ReadOnly | QIODevice::Text);

	if (result == false)
	{
		return "Error opening FutureUpgrade.sql";
	}

	QString upgradeScript = upgradeFile.readAll();

	if (upgradeScript.trimmed().isEmpty() == true)
	{
		TS_RETURN_SUCCESS();
	}

	QString res = TS_EXEC_QUERY_STR(upgradeScript);

	TS_VERIFY_RETURN_ERR(res.isEmpty() == true, "Error executing FutureUpgrade.sql");

	db.close();

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::addSignal(	int userID,
											E::SignalType type,
											int channelCount,
											std::vector<ObjectState>* obStates)
{
	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM add_signal(%1, %2, %3)").
								arg(userID).arg(static_cast<int>(type)).arg(channelCount));
	obStates->clear();

	while(q.next() == true)
	{
		ObjectState obState;

		DbWorker::db_objectState(q, &obState);

		obStates->push_back(obState);
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::check_signalIsExist(int userID,
													 int signalID,
													 E::SignalType type,
													 int channel,
													 int signalGroupID,
													 bool isCheckedOut)
{
	// check record in Signal table
	//
	QSqlQuery qSignal;

	TS_EXEC_QUERY_RETURN_ERR(qSignal, QString("SELECT * FROM Signal WHERE signalid=%1").arg(signalID));

	TS_VERIFY_RETURN_ERR(qSignal.next() == true, "Can't get next Signal record");

	if (isCheckedOut == true)
	{
		TS_VERIFY_RETURN_ERR(qSignal.value("userid").toInt() == userID, "Signal.userID error");
	}
	else
	{
		TS_VERIFY_RETURN_ERR(qSignal.value("userid").isNull() == true, "Signal.userID is not Null");
	}

	TS_VERIFY_RETURN_ERR(qSignal.value("signalid").toInt() == signalID, "Signal.signalID error");

	TS_VERIFY_RETURN_ERR(qSignal.value("type").toInt() == static_cast<int>(type), "Signal.type error");

	TS_VERIFY_RETURN_ERR(qSignal.value("channel").toInt() == channel, "Signal.channel error");

	TS_VERIFY_RETURN_ERR(qSignal.value("signalgroupid").toInt() == signalGroupID, "Signal.signalGroupID error");

	QSqlQuery qSignalInst;

	// check record in SignalInstance table
	//
	TS_EXEC_QUERY_RETURN_ERR(qSignalInst, QString("SELECT * FROM SignalInstance WHERE signalid=%1").
							   arg(signalID));

	TS_VERIFY_RETURN_ERR(qSignalInst.first() == true, "Can't get SignalInstance record");

	int signalInstanceID = qSignalInst.value("signalinstanceid").toInt();

	if (isCheckedOut == true)
	{
		TS_VERIFY_RETURN_ERR(qSignal.value("checkedoutinstanceid").toInt() == signalInstanceID, "Signal.checkOutInstanceID error");
		TS_VERIFY_RETURN_ERR(qSignal.value("checkedininstanceid").isNull() == true, "Signal.checkInInstanceID error");
	}
	else
	{
		TS_VERIFY_RETURN_ERR(qSignal.value("checkedoutinstanceid").isNull() == true,  "Signal.checkOutInstanceID error");
		TS_VERIFY_RETURN_ERR(qSignal.value("checkedininstanceid").isNull() == false,  "Signal.checkInInstanceID error");
	}

	// check record in CheckOutTable
	//
	QSqlQuery qCheckout;

	TS_EXEC_QUERY_RETURN_ERR(qCheckout, QString("SELECT * FROM Checkout WHERE signalid=%1").arg(signalID));

	if (isCheckedOut == true)
	{
		TS_VERIFY_RETURN_ERR(qCheckout.size() == 1, "Checkout records count must be 1");
	}
	else
	{
		TS_VERIFY_RETURN_ERR(qCheckout.size() == 0, "Checkout records count must be 0");
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getFieldValue(QString valueField, int* value, QString table, QString whereField, int whereValue)
{
	TS_TEST_PTR_RETURN(value);

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT %1 FROM %2 WHERE %3=%4").
									arg(valueField).arg(table).arg(whereField).arg(whereValue));

	TS_VERIFY_RETURN_ERR(q.size() == 1, "More then one record found");

	TS_VERIFY_RETURN_ERR(q.first() == true, QString("Can't get field '%1' value from table '%2'").
												arg(valueField).arg(table));

	*value = q.value(0).toInt();

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getSignalGroupID(int signalID, int* signalGroupID)
{
	TS_VERIFY_RETURN(getFieldValue("signalgroupid", signalGroupID, "signal", "signalid", signalID));
	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getSignalInstanceID(int signalID, int* signalInstanceID)
{
	TS_VERIFY_RETURN(getFieldValue("signalginstanced", signalInstanceID, "signalinstance", "signalid", signalID));
	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::setSignalWorkcopy(int userID, const Signal& s, ObjectState* obState)
{
	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM set_signal_workcopy(%1, %2)").
								arg(userID).arg(DbWorker::getSignalDataStr(s)));

	q.first();

	if (obState != nullptr)
	{
		DbWorker::db_objectState(q, obState);
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::checkinSignals(int userID,
												const std::vector<int>& ids,
												const QString& comment,
												std::vector<ObjectState>* obStates)
{
	QStringList idsList;

	for(auto id : ids)
	{
		idsList.append(QString::number(id));
	}

	if (obStates != nullptr)
	{
		obStates->clear();
	}

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM checkin_signals(%1, ARRAY[%2], '%3')").
								arg(userID).arg(idsList.join(",")).arg(comment));

	int count = 0;

	while(q.next() == true)
	{
		ObjectState obState;

		DbWorker::db_objectState(q, &obState);

		count++;

		if (obStates != nullptr)
		{
			obStates->push_back(obState);
		}
	}

	TS_VERIFY_RETURN_ERR(count == static_cast<int>(ids.size()), "ObjectStates.size() error");

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::checkoutSignals(int userID,
												const std::vector<int>& ids,
												std::vector<ObjectState>* obStates)
{
	QStringList idsList;

	for(auto id : ids)
	{
		idsList.append(QString::number(id));
	}

	if (obStates != nullptr)
	{
		obStates->clear();
	}

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM checkout_signals(%1, ARRAY[%2])").
								arg(userID).arg(idsList.join(",")));

	while(q.next() == true)
	{
		ObjectState obState;

		DbWorker::db_objectState(q, &obState);

		if (obStates != nullptr)
		{
			obStates->push_back(obState);
		}
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::check_signalIsCheckedIn(int signalID)
{
	QSqlQuery q;

	// check Signal table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM signal WHERE signalid=%1").arg(signalID));
	TS_VERIFY_RETURN_ERR(q.size() == 1, "Signal record not found");

	q.first();

	TS_VERIFY_RETURN_ERR(q.value("checkedoutinstanceid").isNull() == true, "Error Signal.CheckoutInstanceID");
	TS_VERIFY_RETURN_ERR(q.value("checkedininstanceid").isNull() == false, "Error Signal.CheckinInstanceID");

	int checkinInstanceID = q.value("checkedininstanceid").toInt();

	// check SignalInstance table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM signalinstance WHERE signalinstanceid=%1").arg(checkinInstanceID));
	TS_VERIFY_RETURN_ERR(q.size() == 1, "SignalInstance record not found");

	q.first();

	TS_VERIFY_RETURN_ERR(q.value("changesetid").isNull() == false, "Error SignalInstance.ChangesetID");

	int checngesetID = q.value("changesetid").toInt();

	// check Changeset table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM changeset WHERE changesetid=%1").arg(checngesetID));
	TS_VERIFY_RETURN_ERR(q.size() == 1, "Changeset record not found");

	// check Checkout table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM checkout WHERE signalid=%1").arg(signalID));
	TS_VERIFY_RETURN_ERR(q.size() == 0, "Checkout record is exists");

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::check_signalIsCheckedOut(int signalID, VcsItemAction* action)
{
	QSqlQuery q;

	// check Signal table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM signal WHERE signalid=%1").arg(signalID));
	TS_VERIFY_RETURN_ERR(q.size() == 1, "Signal record not found");

	q.first();

	TS_VERIFY_RETURN_ERR(q.value("checkedoutinstanceid").isNull() == false, "Error Signal.CheckoutInstanceID");

	int checkoutInstanceID = q.value("checkedoutinstanceid").toInt();

	// check SignalInstance table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM signalinstance WHERE signalinstanceid=%1").arg(checkoutInstanceID));
	TS_VERIFY_RETURN_ERR(q.size() == 1, "SignalInstance record not found");

	q.first();

	TS_VERIFY_RETURN_ERR(q.value("changesetid").isNull() == true, "Error SignalInstance.ChangesetID");

	if (action != nullptr)
	{
		action->setValue(q.value("action").toInt());
	}

	// check Checkout table
	//
	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM checkout WHERE signalid=%1").arg(signalID));
	TS_VERIFY_RETURN_ERR(q.size() == 1, "Checkout record is exists");

	q.first();

	TS_VERIFY_RETURN_ERR(q.value("signalid") == signalID, "Checkout record is not exists");

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::deleteSignal(int userID, int signalID, ObjectState* obState)
{
	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM delete_signal(%1, %2)").
								arg(userID).arg(signalID));
	TS_VERIFY_RETURN_ERR(q.first() == true, "Can't get ObjectState");

	DbWorker::db_objectState(q, obState);

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::deleteSignals(int userID, const std::vector<int>& ids, std::vector<ObjectState>* obStates)
{
	if (obStates != nullptr)
	{
		obStates->clear();
	}

	for(int id : ids)
	{
		ObjectState obState;

		TS_VERIFY_RETURN(deleteSignal(userID, id, &obState));

		if (obStates != nullptr)
		{
			obStates->push_back(obState);
		}
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getSignalsIDs(int userID, bool withDeleted, std::vector<int>* ids)
{
	TS_TEST_PTR_RETURN(ids);

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM get_signals_ids(%1, %2)").
								arg(userID).arg(withDeleted == true ? "true" : "false"));

	ids->clear();

	while(q.next() == true)
	{
		ids->push_back(q.value(0).toInt());
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getSignalsIDAppSignalID(int userID,
														 bool withDeleted,
														 std::vector<std::pair<int, QString>>* ids)
{
	TS_TEST_PTR_RETURN(ids);

	ids->clear();

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM get_signals_id_appsignalid(%1, %2)").
												arg(userID).arg(withDeleted == true ? "true" : "false"));
	while(q.next() == true)
	{
		std::pair<int, QString> p;

		p.first = q.value(0).toInt();
		p.second = q.value(1).toString();

		ids->push_back(p);
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::addTestSignals(int userID,
												E::SignalType signalType,
											   int channelCount,
											   int signalCount,
												std::vector<int>* addedSignalsIDsSorted)
{
	TS_TEST_PTR_RETURN(addedSignalsIDsSorted);

	addedSignalsIDsSorted->clear();

	std::vector<ObjectState> obStates;

	for(int i = 0; i < signalCount; i++)
	{
		TS_VERIFY_RETURN(addSignal(userID, signalType, channelCount, &obStates));
		TS_VERIFY_RETURN_ERR(obStates.size() == channelCount, "ChannelCount error");

		for(int ch = 0; ch < channelCount; ch++)
		{
			addedSignalsIDsSorted->push_back(obStates[ch].id);
		}
	}

	std::sort(addedSignalsIDsSorted->begin(), addedSignalsIDsSorted->end());

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getAllSignalIDs(std::vector<int>* allSignalIDsSorted)
{
	TS_TEST_PTR_RETURN(allSignalIDsSorted);

	allSignalIDsSorted->clear();

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, "SELECT signalid FROM Signal ORDER BY signalid ASC");

	while(q.next() == true)
	{
		allSignalIDsSorted->push_back(q.value(0).toInt());
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::removePairsWithID(std::vector<std::pair<int, QString>>* pairs,
												   const std::vector<int>& idsToRemove)
{
	TS_TEST_PTR_RETURN(pairs);

	for(int idToRemove : idsToRemove)
	{
		for(auto it = pairs->begin(); it != pairs->end(); it++)
		{
			if (it->first == idToRemove)
			{
				pairs->erase(it, it + 1);
				break;
			}
		}
	}

	TS_RETURN_SUCCESS();
}

bool DbControllerSignalTests::findPairWithID(	int id,
												const std::vector<std::pair<int, QString>>& pairs,
												std::pair<int, QString>* pair)
{
	TEST_PTR_RETURN_FALSE(pair);

	for(auto p : pairs)
	{
		if (p.first == id)
		{
			*pair = p;
			return true;
		}
	}

	return false;
}

QString DbControllerSignalTests::checkSignalIDsAppSignalID(std::vector<int> ids,
								const std::vector<std::pair<int, QString>>& pairs)
{
	for(int id : ids)
	{
		std::pair<int, QString> p;

		bool find = findPairWithID(id, pairs, &p);

		TS_VERIFY_RETURN_ERR(find == true, "Pair SignalID + AppSignalID is not found");

		TS_VERIFY_RETURN_ERR(p.second.startsWith(QString("#SIGNAL%1").arg(id)) == true, "AppSignalID is wrong");
	}

	TS_RETURN_SUCCESS();
}

int DbControllerSignalTests::rand0to(int upRange) const
{
	Q_ASSERT(upRange > 0 && upRange <= RAND_MAX);

	return std::rand() % (upRange + 1);
}


