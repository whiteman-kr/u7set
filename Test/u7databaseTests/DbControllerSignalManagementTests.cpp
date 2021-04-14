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

	std::vector<std::tuple<int, QString, QString>> users =
	{
		{ ADMIN_ID, "Administrator", m_adminPassword },
		{ USER2_ID, "User2", "qqq222" },
		{ USER3_ID, "User3", "qqq333" },
	};

	Q_ASSERT(users.size() == USERS_COUNT);

	for(int i = 0; i < USERS_COUNT; i++)
	{
		m_dbc[i] = new DbController();

		DbController* dbc = m_dbc[i];

		dbc->setHost(m_databaseHost);
		dbc->setServerUsername(m_databaseUser);
		dbc->setServerPassword(m_adminPassword);

		auto [userID, userLogin, userPassword] = users[i];

		if (i == 0)
		{
			ok = dbc->createProject(m_projectName, m_adminPassword, 0);
			QVERIFY2 (ok == true, qPrintable("Can't create project: " + dbc->lastError()));

			ok = dbc->upgradeProject(m_projectName, m_adminPassword, true, 0);
			QVERIFY2 (ok == true, qPrintable("Can't upgrade project: " + dbc->lastError()));

			TS_VERIFY(applyFutureDatabaseUpgrade());

			ok = dbc->openProject(m_projectName, userLogin, userPassword, nullptr);
			QVERIFY2 (ok == true, qPrintable("Can't open project: " + dbc->lastError()));

			for(int u = 1; u < users.size(); u++)
			{
				auto [id, login, password] = users[u];

				DbUser dbu;

				dbu.setUserId(id);
				dbu.setUsername(login);
				dbu.setFirstName(login);
				dbu.setLastName(login);
				dbu.setNewPassword(password);
				dbu.setPassword(password);
				dbu.setAdministrator(false);
				dbu.setReadonly(false);
				dbu.setDisabled(false);

				ok = dbc->createUser(dbu, nullptr);
				QVERIFY2 (ok == true, C_STR(QString("Can't create %1: %2").
												arg(login).arg(dbc->lastError())));
			}
		}
		else
		{
			ok = dbc->openProject(m_projectName, userLogin, userPassword, nullptr);
			QVERIFY2 (ok == true, qPrintable("Can't open project: " + dbc->lastError()));
		}
	}

	m_dbcAdmin = m_dbc[0];
	m_dbcUser2 = m_dbc[1];
	m_dbcUser3 = m_dbc[2];

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
	QString errStr = addSignal(ADMIN_ID, E::SignalType::Discrete, 9, &obStates);
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

	// try check in unknown signal
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({100500}), "checking", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

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

	int idToDelete = ids3[1];

	ObjectState obState;

	TS_VERIFY(deleteSignal(USER2_ID, idToDelete, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);
	QVERIFY(obState.action == VcsItemAction::Deleted);

	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({idToDelete}), "checkin deleted signal", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	// try checkin of DELETED signal
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({idToDelete}), "checkin deleted signal again", &obStates));
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	db.close();
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

	db.close();
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

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalid=%1").arg(id2));
	QVERIFY2(q.size() == 0, "SignalInstance record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM Signal WHERE signalid=%1").arg(id2));
	QVERIFY2(q.size() == 0, "Signal record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM CheckOut WHERE signalid=%1").arg(id2));
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

	AppSignal s;

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
	TS_VERIFY(setSignalWorkcopy(USER3_ID, s, &obState));

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

	std::vector<int> initialIDs;

	TS_VERIFY(getAllSignalIDs(&initialIDs));

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

void DbControllerSignalTests::test_getSignalsActualSignalInstanceID()
{
	// Testing of stored procedure:
	//
	//	get_signals_actual_signalinstanceid(user_id INTEGER, with_deleted BOOLEAN) RETURNS SETOF integer
	//

	OPEN_DATABASE();

	std::vector<int> initialInstances;

	TS_VERIFY(getAllSignalsInstancesIDs(&initialInstances));

	// add User2 signals
	//
	std::vector<int> user2SignalsIDs;

	TS_VERIFY(addTestSignals(USER2_ID, E::SignalType::Discrete, 3, 4 + rand0to(3), &user2SignalsIDs));

	std::vector<int> user2Instances;

	TS_VERIFY(getAllSignalsInstancesIDs(&user2Instances));
	user2Instances = sets_difference<int>(user2Instances, initialInstances);

	// add Admin signals
	//
	std::vector<int> adminSignalsIDs;

	TS_VERIFY(addTestSignals(ADMIN_ID, E::SignalType::Bus, 1, 3 + rand0to(5), &adminSignalsIDs));

	std::vector<int> adminInstances;

	TS_VERIFY(getAllSignalsInstancesIDs(&adminInstances));
	adminInstances = sets_difference<int>(adminInstances,
										  sets_union(initialInstances, user2Instances));

	// get actual instances by User3
	//
	// User3 shouldn't see Admin and User 2 signals instances
	//
	std::vector<int> user3Instances;

	TS_VERIFY(getActualSignalsSignalInstanceID(USER3_ID, false, &user3Instances));
	user3Instances = sets_difference<int>(user3Instances, initialInstances);

	QVERIFY(user3Instances.size() == 0);

	// checkin Admin's signals
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, adminSignalsIDs, "admin signals checkin", nullptr));

	// get actual instances by User3
	//
	// Now User3 should see Admin signals instances
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(USER3_ID, false, &user3Instances));
	user3Instances = sets_difference<int>(user3Instances, initialInstances);

	QVERIFY(sets_equal(user3Instances, adminInstances));

	// get actual instances by User2
	//
	// User2 should see User2 and Admin checkedin instances
	//
	std::vector<int> user2InstancesAfterCheckin;

	TS_VERIFY(getActualSignalsSignalInstanceID(USER2_ID, false, &user2InstancesAfterCheckin));
	user2InstancesAfterCheckin = sets_difference<int>(user2InstancesAfterCheckin, initialInstances);

	QVERIFY(sets_equal(user2InstancesAfterCheckin, sets_union(user2Instances, adminInstances)));

	// checkout Admin signals by User3
	//
	std::vector<int> user3SignalsIDs = adminSignalsIDs;
	TS_VERIFY(checkoutSignals(USER3_ID, user3SignalsIDs, nullptr));

	// get actual instances by User3
	//
	// Now User3 should see only their newly checkedout signals instances
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(USER3_ID, false, &user3Instances));
	user3Instances = sets_difference<int>(user3Instances, initialInstances);

	QVERIFY(sets_intersect(user3Instances, adminInstances) == false);
	QVERIFY(sets_intersect(user3Instances, user2Instances) == false);

	// get actual instances by Admin
	//
	// Admin should see User2 and User3 checked out signals instances
	//
	std::vector<int> newAdminInstances;

	TS_VERIFY(getActualSignalsSignalInstanceID(ADMIN_ID, false, &newAdminInstances));
	newAdminInstances = sets_difference<int>(newAdminInstances, initialInstances);

	QVERIFY(sets_equal(newAdminInstances, sets_union(user2Instances, user3Instances)));

	// get actual instances by User2
	//
	// User2 should see User2 and Admin checkedin signals instances
	//
	std::vector<int> newUser2Instances;

	TS_VERIFY(getActualSignalsSignalInstanceID(USER2_ID, false, &newUser2Instances));
	newUser2Instances = sets_difference<int>(newUser2Instances, initialInstances);

	QVERIFY(sets_equal(newUser2Instances, sets_union(user2Instances, adminInstances)));

	// delete and checkin User3 signals
	//
	TS_VERIFY(deleteSignals(USER3_ID, user3SignalsIDs, nullptr));
	TS_VERIFY(checkinSignals(USER3_ID, user3SignalsIDs, "user3 signals checkin", nullptr));

	// get actual instances by User2, with_deleted == false
	//
	// User2 should see only their signals instances
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(USER2_ID, false, &newUser2Instances));
	newUser2Instances = sets_difference<int>(newUser2Instances, initialInstances);

	QVERIFY(sets_equal(newUser2Instances, user2Instances));

	// get actual instances by Admin, with_deleted == false
	//
	// Admin should see only User2 signals instances
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(ADMIN_ID, false, &newAdminInstances));
	newAdminInstances = sets_difference<int>(newAdminInstances, initialInstances);

	QVERIFY(sets_equal(newAdminInstances, user2Instances));

	// get actual instances by User3, with_deleted == false
	//
	// User3 shouldn't see any signals instances
	//
	std::vector<int> newUser3Instances;

	TS_VERIFY(getActualSignalsSignalInstanceID(USER3_ID, false, &newUser3Instances));
	newUser3Instances = sets_difference<int>(newUser3Instances, initialInstances);

	QVERIFY(newUser3Instances.size() == 0);

	// get actual instances by User2, with_deleted == true
	//
	// User2 should see their signals instances and signal instances deleted by User3
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(USER2_ID, true, &newUser2Instances));
	newUser2Instances = sets_difference<int>(newUser2Instances, initialInstances);

	QVERIFY(sets_equal(newUser2Instances, sets_union(user2Instances, user3Instances)));

	// get actual instances by Admin, with_deleted == true
	//
	// Admin should see User2 signals instances and signal instances deleted by User3
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(ADMIN_ID, true, &newAdminInstances));
	newAdminInstances = sets_difference<int>(newAdminInstances, initialInstances);

	QVERIFY(sets_equal(newAdminInstances, sets_union(user2Instances, user3Instances)));

	// get actual instances by User2, with_deleted == true
	//
	// User3 should see User3 deleted signal instances
	//
	TS_VERIFY(getActualSignalsSignalInstanceID(USER3_ID, true, &newUser3Instances));
	newUser3Instances = sets_difference<int>(newUser3Instances, initialInstances);

	QVERIFY(sets_equal(newUser3Instances, user3Instances));

	db.close();
}

void DbControllerSignalTests::test_getSignalsIDAppSignalID()
{
	// Testing of stored procedure:
	//
	//	get_signals_id_appsignalid(	user_id integer,
	//								with_deleted boolean)
	//									RETURNS SETOF signal_id_appsignalid

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
	AppSignal s;

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

void DbControllerSignalTests::test_getLatestSignal()
{
	// Testing of:
	//
	//	bool get_latest_signal(user_id integer,	signal_id integer) RETURNS SETOF signaldata
	//

	OPEN_DATABASE();

	std::vector<int> adminSignals;

	TS_VERIFY(addTestSignals(ADMIN_ID, E::SignalType::Analog, 1, 2, &adminSignals));
	QVERIFY(adminSignals.size() == 2);

	int id1 = adminSignals[0];
	int id2 = adminSignals[1];

	AppSignal s1, s2;
	QString res;

	// Admin should see both his signals
	//
	TS_VERIFY(getLatestSignal(ADMIN_ID, id1, &s1));
	TS_VERIFY(getLatestSignal(ADMIN_ID, id2, &s2));

	// change signal fields of signal2
	//
	s2.setAppSignalID("#NEW12319283_APP_SIGNALID");
	s2.setCustomAppSignalID("NEW12319283_CUSTOM_APP_SIGNALID");
	s2.setCaption("NEW12319283 Caption");
	TS_VERIFY(setSignalWorkcopy(ADMIN_ID, s2, nullptr));

	AppSignal s;

	TS_VERIFY(getLatestSignal(ADMIN_ID, id2, &s));
	QVERIFY(s.appSignalID() == s2.appSignalID());
	QVERIFY(s.customAppSignalID() == s2.customAppSignalID());
	QVERIFY(s.caption() == s2.caption());

	// No latest signal should be return
	//
	res = getLatestSignal(USER3_ID, id2, &s);
	QVERIFY2(res.isEmpty() == false, "Error should be returned");

	// checkin signal2
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, std::vector<int>({id2}), "admin signal2 commit", nullptr));

	// User3 should see checked in signal2
	//
	s.clear();

	TS_VERIFY(getLatestSignal(USER3_ID, id2, &s));
	QVERIFY(s.appSignalID() == s2.appSignalID());
	QVERIFY(s.customAppSignalID() == s2.customAppSignalID());
	QVERIFY(s.caption() == s2.caption());

	// checkin signal1
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, std::vector<int>({id1}), "admin signal1 checkin", nullptr));

	// get latest signals1,2 by User2
	//
	s.clear();

	TS_VERIFY(getLatestSignal(USER2_ID, id1, &s));
	QVERIFY(s.appSignalID() == s1.appSignalID());
	QVERIFY(s.customAppSignalID() == s1.customAppSignalID());
	QVERIFY(s.caption() == s1.caption());

	TS_VERIFY(getLatestSignal(USER2_ID, id2, &s));
	QVERIFY(s.appSignalID() == s2.appSignalID());
	QVERIFY(s.customAppSignalID() == s2.customAppSignalID());
	QVERIFY(s.caption() == s2.caption());

	// delete signal2 by User2
	//
	TS_VERIFY(deleteSignal(USER2_ID, id2, nullptr));

	// Admin and User3 should see signal1, signal2
	//
	TS_VERIFY(getLatestSignal(ADMIN_ID, id1, &s));
	TS_VERIFY(getLatestSignal(USER2_ID, id1, &s));
	TS_VERIFY(getLatestSignal(ADMIN_ID, id2, &s));
	TS_VERIFY(getLatestSignal(USER2_ID, id2, &s));

	// checkin deleted signals
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id2}), "checkin deletedsignals", nullptr));

	// All users should see only signal1
	//
	TS_VERIFY(getLatestSignal(ADMIN_ID, id1, &s));
	TS_VERIFY(getLatestSignal(USER2_ID, id1, &s));
	TS_VERIFY(getLatestSignal(USER3_ID, id1, &s));

	res = getLatestSignal(ADMIN_ID, id2, &s);
	QVERIFY2(res.isEmpty() == false, "Error should be returned");

	res = getLatestSignal(USER3_ID, id2, &s);
	QVERIFY2(res.isEmpty() == false, "Error should be returned");

	res = getLatestSignal(USER3_ID, id2, &s);
	QVERIFY2(res.isEmpty() == false, "Error should be returned");

	db.close();
}

void DbControllerSignalTests::dbcTest_addSignal()
{
	// Testing of:
	//
	//	bool DbController::addSignal(E::SignalType signalType, QVector<Signal>* newSignal, QWidget* parentWidget);
	//
	OPEN_DATABASE();

	QVector<AppSignal> newSignals;

	AppSignal s;

	s.setID(-1);
	s.setAppSignalID("#DBC_ADD_SIGNAL_TEST_1");
	s.setCustomAppSignalID("DBC_ADD_SIGNAL_TEST_1");
	s.setCaption("Caption DBC_ADD_SIGNAL_TEST_1");

	newSignals.append(s);

	QVERIFY(m_dbcAdmin->addSignal(E::SignalType::Discrete, &newSignals, nullptr) == true);
	TS_VERIFY(check_signalIsExist(ADMIN_ID, newSignals[0].ID(), E::SignalType::Discrete, 0, 0, true));

	// try add another one signal with same IDs
	//
	newSignals.clear();
	newSignals.append(s);

	QVERIFY(m_dbcAdmin->addSignal(E::SignalType::Discrete, &newSignals, nullptr) == false);
	QVERIFY(m_dbcAdmin->lastError().contains("already exists"));

	// try add multichannel signal
	//
	newSignals.clear();

	for(int i = 0; i < 4; i++)
	{
		AppSignal s;

		s.setID(-1);
		s.setAppSignalID(QString("#DBC_ADD_SIGNAL_TEST_S%1").arg(i));
		s.setCustomAppSignalID(QString("DBC_ADD_SIGNAL_TEST_S%1").arg(i));
		s.setCaption(QString("Caption DBC_ADD_SIGNAL_TEST_S%1").arg(i));

		newSignals.append(s);
	}

	QVERIFY(m_dbcAdmin->addSignal(E::SignalType::Analog, &newSignals, nullptr) == true);

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
		AppSignal s;

		s.setID(-1);
		s.setAppSignalID(QString("#DBC_ADD_SIGNAL_TEST_SS%1").arg(i));
		s.setCustomAppSignalID(QString("DBC_ADD_SIGNAL_TEST_SS%1").arg(i));
		s.setCaption(QString("Caption DBC_ADD_SIGNAL_TEST_SS%1").arg(i));

		newSignals.append(s);
	}

	QVERIFY(m_dbcAdmin->addSignal(E::SignalType::Analog, &newSignals, nullptr) == false);
	QVERIFY(m_dbcAdmin->lastError().contains("channelCount"));

	// try add multichannel signal by another User2
	//
	newSignals.clear();

	for(int i = 0; i < 3; i++)
	{
		AppSignal s;

		s.setID(-1);
		s.setAppSignalID(QString("#DBC_ADD_SIGNAL_TEST_ANOTHER_S%1").arg(i));
		s.setCustomAppSignalID(QString("DBC_ADD_SIGNAL_TEST_ANOTHER_S%1").arg(i));
		s.setCaption(QString("Caption DBC_ADD_SIGNAL_TEST_ANOTHER_S%1").arg(i));

		newSignals.append(s);
	}

	QVERIFY(m_dbcUser3->addSignal(E::SignalType::Discrete, &newSignals, nullptr) == true);

	sg1 = 0;

	TS_VERIFY(getSignalGroupID(newSignals[0].ID(), &sg1));

	for(int i = 0; i < 3; i++)
	{
		TS_VERIFY(check_signalIsExist(USER3_ID, newSignals[i].ID(), E::SignalType::Discrete, i, sg1, true));
	}

	db.close();
}

void DbControllerSignalTests::dbcTest_checkinSignals()
{
	QVector<ObjectState> obStates;
	std::vector<ObjectState> stdObStates;
	ObjectState obState;

	// try check in unknown signal
	//
	QVector<int> ids({100500});

	QVERIFY(m_dbcAdmin->checkinSignals(&ids, "comment", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	QVERIFY(m_dbcUser2->checkinSignals(&ids, "comment", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	ids.clear();

	//	add signal by Admin
	//
	std::vector<int> stdAdminSignals;

	TS_VERIFY(dbc_addSignal(m_dbcAdmin, E::SignalType::Analog, 1, &stdAdminSignals));

	QVector<int> adminSignals = toQVector(stdAdminSignals);

	// try checkin by User3
	//
	QVERIFY(m_dbcUser3->checkinSignals(&adminSignals, "comment user3", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try checkin by Admin
	//
	QVERIFY(m_dbcAdmin->checkinSignals(&adminSignals, "comment admin", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);
	TS_VERIFY(check_signalIsCheckedIn(adminSignals[0]));

	// add signal by User3
	//
	std::vector<int> stdUser3Signals;

	TS_VERIFY(dbc_addSignal(m_dbcUser3, E::SignalType::Bus, 2, &stdUser3Signals));

	QVector<int> user3Signals = toQVector(stdUser3Signals);

	// try checkin by User2
	//
	QVERIFY(m_dbcUser2->checkinSignals(&user3Signals, "comment user2", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 2);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);
	QVERIFY(obStates[1].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);
	QVERIFY(check_signalIsCheckedIn(user3Signals[0]).isEmpty() == false);
	QVERIFY(check_signalIsCheckedIn(user3Signals[1]).isEmpty() == false);

	int ch1ID = obStates[0].id;
	int ch2ID = obStates[1].id;

	// try checkin first channel by Admin
	//
	ids.clear();
	ids.append(ch1ID);

	QVERIFY(m_dbcAdmin->checkinSignals(&ids, "comment admin", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);
	TS_VERIFY(check_signalIsCheckedIn(ids[0]));

	// try checkin already checkedin signal
	//
	QVERIFY(m_dbcUser3->checkinSignals(&ids, "comment user3", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	// delete signal ch1ID
	//
	TS_VERIFY(deleteSignal(USER3_ID, ch1ID, nullptr));		// signal is auto checkedout here
	QVERIFY(m_dbcUser3->checkinSignals(&ids, "comment user3", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	// try checkin deleted signal that has checkedin instance
	//
	QVERIFY(m_dbcUser3->checkinSignals(&ids, "comment user3", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);

	// delete signal ch2ID
	//
	TS_VERIFY(deleteSignal(USER3_ID, ch2ID, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	// try checkin deleted signal that has no checkedin instance
	//
	ids.clear();
	ids.append(ch2ID);

	QVERIFY(m_dbcUser3->checkinSignals(&ids, "comment user3", &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT);
}

void DbControllerSignalTests::dbcTest_checkoutSignals()
{
	QVector<ObjectState> obStates;
	std::vector<ObjectState> stdObStates;
	ObjectState obState;

	// add signal and checkin it
	//
	std::vector<int> stdAdminSignals;

	TS_VERIFY(dbc_addSignal(m_dbcAdmin, E::SignalType::Discrete, 2, &stdAdminSignals));
	QVERIFY(stdAdminSignals.size() == 2);

	int id1 = stdAdminSignals[0];
	int id2 = stdAdminSignals[1];

	QVector adminSignals = toQVector<int>(stdAdminSignals);

	QVERIFY(m_dbcAdmin->checkinSignals(&adminSignals, "checkout", &obStates, nullptr));
	QVERIFY(obStates.size() == 2);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);
	QVERIFY(obStates[1].errCode == ERR_SIGNAL_OK);

	// try checkout NOT exist signal
	//
	QVector<int> ids;
	ids.append(id1 + 100500);

	QVERIFY(m_dbcAdmin->checkoutSignals(&ids, &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_NOT_FOUND);

	ids.clear();
	ids.append(id1);

	// try checkout signal by Admin
	//
	QVERIFY(m_dbcAdmin->checkoutSignals(&ids, &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	VcsItemAction action;
	TS_VERIFY(check_signalIsCheckedOut(id1, &action));
	QVERIFY(action == VcsItemAction::Modified);

	// try checkout by User2
	//
	ids.clear();
	ids.append(id2);

	QVERIFY(m_dbcUser2->checkoutSignals(&ids, &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_OK);

	TS_VERIFY(check_signalIsCheckedOut(id2, &action));
	QVERIFY(action == VcsItemAction::Modified);

	// try checkout by Admin
	//
	QVERIFY(m_dbcAdmin->checkoutSignals(&ids, &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try to check out prevously deleted signal
	//
	TS_VERIFY(deleteSignal(USER2_ID, id2, &obState));
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id2}), "Checkin signal", &stdObStates));
	QVERIFY(stdObStates.size() == 1);
	QVERIFY(stdObStates[0].errCode == ERR_SIGNAL_OK);

	QVERIFY(m_dbcAdmin->checkoutSignals(&ids, &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_DELETED);

	QVERIFY(m_dbcUser3->checkoutSignals(&ids, &obStates, nullptr) == true);
	QVERIFY(obStates.size() == 1);
	QVERIFY(obStates[0].errCode == ERR_SIGNAL_DELETED);
}

void DbControllerSignalTests::dbcTest_deleteSignal()
{
	OPEN_DATABASE();

	std::vector<ObjectState> obStates;
	ObjectState obState;

	// add TWO channel signal
	// both signals are checked out and have not checked in instances
	//
	std::vector<int> adminSignals;
	TS_VERIFY(dbc_addSignal(m_dbcAdmin, E::SignalType::Discrete, 2, &adminSignals));
	QVERIFY(adminSignals.size() == 2);

	int id1 = adminSignals[0];
	int id2 = adminSignals[1];

	int signalGroupID1 = 0;

	TS_VERIFY(getSignalGroupID(id1, &signalGroupID1));

	// try delete first channel of signal by another user
	//
	QVERIFY(m_dbcUser2->deleteSignal(id1, &obState, nullptr) == true);
	QVERIFY(obState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try delete first channel of signal
	//
	QVERIFY(m_dbcAdmin->deleteSignal(id1, &obState, nullptr) == true);
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
	QVERIFY(m_dbcAdmin->deleteSignal(id2, &obState, nullptr) == true);
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalInstance WHERE signalid=%1").arg(id2));
	QVERIFY2(q.size() == 0, "SignalInstance record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM Signal WHERE signalid=%1").arg(id2));
	QVERIFY2(q.size() == 0, "Signal record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM CheckOut WHERE signalid=%1").arg(id2));
	QVERIFY2(q.size() == 0, "CheckOut record is not deleted");

	TS_EXEC_QUERY(q, QString("SELECT * FROM SignalGroup WHERE signalgroupid=%1").arg(signalGroupID1));
	QVERIFY2(q.size() == 0, "SignalGroup record is not deleted");

	// add THREE channel signal
	//
	std::vector<int> user3Signals;
	TS_VERIFY(dbc_addSignal(m_dbcUser3, E::SignalType::Discrete, 3, &user3Signals));
	QVERIFY(user3Signals.size() == 3);

	int sid1 = user3Signals[0];
	int sid2 = user3Signals[1];
	int sid3 = user3Signals[2];

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

	QVERIFY(m_dbcUser2->deleteSignal(sid2, &obState, nullptr) == true);
	QVERIFY(obState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER);

	// try delete by current USER3_ID
	//
	QVERIFY(m_dbcUser3->deleteSignal(sid2, &obState, nullptr) == true);
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
	QVERIFY(m_dbcUser2->deleteSignal(sid3, &obState, nullptr) == true);
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	// signal should be automatically checked out
	//
	VcsItemAction action;
	TS_VERIFY(check_signalIsCheckedOut(sid3, &action));
	QVERIFY(action == VcsItemAction::Deleted);
}

void DbControllerSignalTests::dbcTest_setSignalWorkcopy()
{
	OPEN_DATABASE();

	std::vector<ObjectState> obStates;

	std::vector<int> user2Signals;

	TS_VERIFY(dbc_addSignal(m_dbcUser2, E::SignalType::Discrete, 1, &user2Signals));
	QVERIFY(user2Signals.size() == 1);

	int id1 = user2Signals[0];

	const QString appSignalID1("#DBC_SET_SIGNAL_WORKCOPY_SIGNAL_1");
	const QString appSignalID2("#DBC_SET_SIGNAL_WORKCOPY_SIGNAL_2");
	const QString customAppSignalID1("DBC_SET_SIGNAL_WORKCOPY_SIGNAL1");
	const QString caption1("DBC set signal workcopy signal 1");
	const QString equipmentID1("EQUIPMENT_DBC_SET_SIGNAL_WORKCOPY_SIGNAL1");
	const QString specPropStruct1("Dummy SpecProStruct string");

	const QByteArray specPropValues1("SpecPropValues_dummy_array", 26 + 1);

	AppSignal s;

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

	QVERIFY(m_dbcUser2->setSignalWorkcopy(&s, &obState, nullptr) == true);
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
	QVERIFY(m_dbcUser3->setSignalWorkcopy(&s, &obState, nullptr) == false);
	QVERIFY2(obState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER, "Error: setSignalWorkcopy returns not ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER");

	// check in signal
	//
	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id1}), "Checkin comment1", &obStates));

	// try setSignalWorkcopy for checked in signal
	//
	QVERIFY(m_dbcUser2->setSignalWorkcopy(&s, &obState, nullptr) == true);
	QVERIFY2(obState.errCode == ERR_SIGNAL_IS_NOT_CHECKED_OUT, "Error: setSignalWorkcopy returns not ERR_SIGNAL_IS_NOT_CHECKED_OUT");

	// create another one signal
	//
	std::vector<int> user3Signals;

	TS_VERIFY(dbc_addSignal(m_dbcUser3, E::SignalType::Discrete, 1, &user3Signals));
	QVERIFY(user3Signals.size() == 1);

	s.setID(user3Signals[0]);

	// try set Signal.appSignalID equal to appSignalID1
	//
	QVERIFY(m_dbcUser3->setSignalWorkcopy(&s, &obState, nullptr) == false);
	QVERIFY(m_dbcUser3->lastError().contains("AppSignalID") == true);
	QVERIFY(m_dbcUser3->lastError().contains("already exists") == true);

	s.setAppSignalID(appSignalID1 + "_COPY");

	// try set Signal.customAppSignalID equal to customAppSignalID1
	//
	QVERIFY(m_dbcUser3->setSignalWorkcopy(&s, &obState, nullptr) == false);
	QVERIFY(m_dbcUser3->lastError().contains("CustomAppSignalID") == true);
	QVERIFY(m_dbcUser3->lastError().contains("already exists") == true);

	s.setCustomAppSignalID(customAppSignalID1 + "_COPY");

	// now should be OK
	//
	QVERIFY(m_dbcUser3->setSignalWorkcopy(&s, &obState, nullptr) == true);

	//
	//
	TS_VERIFY(dbc_addSignal(m_dbcUser2, E::SignalType::Analog, 1, &user2Signals));
	QVERIFY(user2Signals.size() == 1);

	s.setID(user2Signals[0]);

	s.setAppSignalID(appSignalID2);
	s.setCustomAppSignalID("$(Macro)_DBC_CUSTOM_ID");

	QVERIFY(m_dbcUser2->setSignalWorkcopy(&s, &obState, nullptr) == true);
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	TS_VERIFY(dbc_addSignal(m_dbcUser2, E::SignalType::Analog, 1, &user2Signals));
	QVERIFY(user2Signals.size() == 1);

	// change appSignalID, customAppSignalID is not change
	//
	s.setID(user2Signals[0]);
	s.setAppSignalID(s.appSignalID() + "_COPY");

	// $(Macro) in customAppSignalID should disable uniquiness checking of cunstomAppSignalID
	//
	QVERIFY(m_dbcUser2->setSignalWorkcopy(&s, &obState, nullptr) == true);
	QVERIFY(obState.errCode == ERR_SIGNAL_OK);

	db.close();
}

void DbControllerSignalTests::dbcTest_getSignalsIDs()
{
	OPEN_DATABASE();

	// Request ALL exists signals IDs for now
	//
	QSqlQuery q;

	std::vector<int> initialIDs;

	TS_VERIFY(getAllSignalIDs(&initialIDs));

	std::vector<ObjectState> obStates;

	// add signals by user ADMIN_ID
	//
	std::vector<int> stdAdminSignalsIDs;

	TS_VERIFY(addTestSignals(ADMIN_ID, E::SignalType::Bus, 1, 5 + rand0to(4), &stdAdminSignalsIDs));

	// add signals by user USER2_ID
	//
	std::vector<int> stdUser2SignalsIDs;

	TS_VERIFY(addTestSignals(USER2_ID, E::SignalType::Discrete, 2, 4 + rand0to(5), &stdUser2SignalsIDs));

	// add signals by user USER3_ID
	//
	std::vector<int> stdUser3SignalsIDs;

	TS_VERIFY(addTestSignals(USER3_ID, E::SignalType::Analog, 3, 2 + rand0to(2), &stdUser3SignalsIDs));

	// checking that sets is NOT intersect
	//
	QVERIFY(sets_intersect<int>(stdAdminSignalsIDs, stdUser2SignalsIDs) == false);
	QVERIFY(sets_intersect<int>(stdAdminSignalsIDs, stdUser3SignalsIDs) == false);
	QVERIFY(sets_intersect<int>(stdUser2SignalsIDs, stdUser3SignalsIDs) == false);

	QVector<int> ids;
	std::vector<int> stdIds;

	// request IDs under Admin
	// signals added by Admin, User2 and User3 should  be visible
	//
	QVERIFY(m_dbcAdmin->getSignalsIDs(&ids, nullptr) == true);

	stdIds = sets_difference<int>(toStdVector<int>(ids), initialIDs);

	QVERIFY(sets_equal(stdIds, sets_union(stdAdminSignalsIDs, sets_union(stdUser2SignalsIDs, stdUser3SignalsIDs))) == true);

	// request IDs under User2
	// only signals added by User2 should  be visible
	//
	QVERIFY(m_dbcUser2->getSignalsIDs(&ids, nullptr) == true);

	stdIds = sets_difference<int>(toStdVector(ids), initialIDs);

	QVERIFY(sets_equal(stdIds, stdUser2SignalsIDs) == true);

	// checkin signals added by User2
	//
	TS_VERIFY(checkinSignals(USER2_ID, stdUser2SignalsIDs, "user2 checkin", &obStates));

	// request IDs under User3
	// signals added by User3 and checked in by User2 should  be visible
	//
	QVERIFY(m_dbcUser3->getSignalsIDs(&ids, nullptr) == true);

	stdIds = sets_difference<int>(toStdVector<int>(ids), initialIDs);

	QVERIFY(sets_equal(stdIds, sets_union(stdUser2SignalsIDs, stdUser3SignalsIDs)));

	// request IDs under Admin
	// signals added by Admin, User3 and checked in by User2 should  be visible
	//
	QVERIFY(m_dbcAdmin->getSignalsIDs(&ids, nullptr) == true);

	stdIds = sets_difference<int>(toStdVector(ids), initialIDs);

	QVERIFY(sets_equal(stdIds, sets_union(stdAdminSignalsIDs, sets_union(stdUser2SignalsIDs, stdUser3SignalsIDs))) == true);

	// checkin signals added by Admin and User3
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, stdAdminSignalsIDs, "admin checkin", &obStates));
	TS_VERIFY(checkinSignals(USER3_ID, stdUser3SignalsIDs, "user3 checkin", &obStates));

	// request IDs under User2
	// signals added by Admin, User2 and User3 should  be visible
	//
	QVERIFY(m_dbcUser2->getSignalsIDs(&ids, nullptr) == true);

	stdIds = sets_difference<int>(toStdVector(ids), initialIDs);

	QVERIFY(sets_equal(stdIds, sets_union(stdAdminSignalsIDs, sets_union(stdUser2SignalsIDs, stdUser3SignalsIDs))) == true);

	// Delete and checkin signals added by User2
	//
	TS_VERIFY(deleteSignals(USER2_ID, stdUser2SignalsIDs, &obStates));
	TS_VERIFY(checkinSignals(USER2_ID, stdUser2SignalsIDs, "checkin deleted", &obStates));

	// DbControlle::getSignalsIDs() always return IDs withDeleted == false
	// Admin, User3 signals should be visible, User2 (deleted) - not
	//
	QVERIFY(m_dbcUser3->getSignalsIDs(&ids, nullptr) == true);

	stdIds = sets_difference<int>(toStdVector(ids), initialIDs);

	QVERIFY(sets_equal(stdIds, sets_union(stdAdminSignalsIDs, stdUser3SignalsIDs)) == true);

	db.close();
}

void DbControllerSignalTests::dbcTest_getSignalsIDAppSignalID()
{
	OPEN_DATABASE();

	std::vector<int> initialIDs;

	TS_VERIFY(getAllSignalIDs(&initialIDs));

	std::vector<int> stdAdminSignalsIDs;

	TS_VERIFY(addTestSignals(ADMIN_ID, E::SignalType::Discrete, 2, 2 + rand0to(3), &stdAdminSignalsIDs));

	std::vector<int> stdUser2SignalsIDs;

	TS_VERIFY(addTestSignals(USER2_ID, E::SignalType::Analog, 1, 5 + rand0to(2), &stdUser2SignalsIDs));

	QVector<ID_AppSignalID> qvResult;
	std::vector<std::pair<int, QString>> result;

	// Admin should see all signals
	//
	QVERIFY(m_dbcAdmin->getSignalsIDAppSignalID(&qvResult, nullptr));
	result = toPairsVector(qvResult);

	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == stdAdminSignalsIDs.size() + stdUser2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(sets_union(stdAdminSignalsIDs, stdUser2SignalsIDs), result));

	// User2 should see only their signals
	//
	QVERIFY(m_dbcUser2->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);

	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == stdUser2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(stdUser2SignalsIDs, result));

	// Checkin Admin's signals
	//
	TS_VERIFY(checkinSignals(ADMIN_ID, stdAdminSignalsIDs, "checkin admin signals", nullptr));

	// User2 should see their signals and Admin's signals
	//
	QVERIFY(m_dbcUser2->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);

	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == stdAdminSignalsIDs.size() + stdUser2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(sets_union(stdAdminSignalsIDs, stdUser2SignalsIDs), result));

	// Delete Admin's signals
	//
	TS_VERIFY(deleteSignals(ADMIN_ID, stdAdminSignalsIDs, nullptr));
	TS_VERIFY(checkinSignals(ADMIN_ID, stdAdminSignalsIDs, "checkin admin deleted signals", nullptr));

	// DbController::getSignalsIDAppSignalID() always return withDeleted == false,
	// User2 should see only their signals
	//
	QVERIFY(m_dbcUser2->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);

	TS_VERIFY(removePairsWithID(&result, initialIDs));

	QVERIFY(result.size() == stdUser2SignalsIDs.size());
	TS_VERIFY(checkSignalIDsAppSignalID(stdUser2SignalsIDs, result));

	int id = stdUser2SignalsIDs[0];

	std::pair<int, QString> p;

	QVERIFY(findPairWithID(id, result, &p));

	const QString OLD_APP_SIGNAL_ID(p.second);

	TS_VERIFY(checkinSignals(USER2_ID, std::vector<int>({id}), "checkin user2 signal", nullptr));

	QVERIFY(m_dbcUser2->getSignalsIDAppSignalID(&qvResult, nullptr) == true);

	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == OLD_APP_SIGNAL_ID);

	// try change AppSignalID under User3
	//
	AppSignal s;

	const QString NEW_APP_SIGNAL_ID("#DBC_USER3_TEST_SIGNAL_APP_SIGNAL_ID_4567");

	s.initCreatedDates();

	s.setID(id);
	s.setAppSignalID(NEW_APP_SIGNAL_ID);
	s.setCustomAppSignalID(QString(NEW_APP_SIGNAL_ID).replace("#", ""));

	TS_VERIFY(checkoutSignals(USER3_ID, std::vector<int>({id}), nullptr));
	TS_VERIFY(setSignalWorkcopy(USER3_ID, s, nullptr));

	// under User2, should see OLD_APP_SIGNAL_ID
	//
	QVERIFY(m_dbcUser2->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);

	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == OLD_APP_SIGNAL_ID);

	// under Admin and User3, should see NEW_APP_SIGNAL_ID
	//
	QVERIFY(m_dbcAdmin->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);

	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	QVERIFY(m_dbcUser3->getSignalsIDAppSignalID(&qvResult, nullptr) == true);

	result = toPairsVector(qvResult);

	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	TS_VERIFY(checkinSignals(USER3_ID, std::vector<int>({id}), "checkin user3 signal", nullptr));

	// All should see NEW_APP_SIGNAL_ID
	//
	QVERIFY(m_dbcAdmin->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	QVERIFY(m_dbcUser2->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	QVERIFY(m_dbcUser3->getSignalsIDAppSignalID(&qvResult, nullptr) == true);
	result = toPairsVector(qvResult);
	QVERIFY(findPairWithID(id, result, &p) == true);
	QVERIFY(p.second == NEW_APP_SIGNAL_ID);

	db.close();
}

void DbControllerSignalTests::cleanupTestCase()
{
	QVERIFY(m_dbcUser3->closeProject(nullptr) == true);
	QVERIFY(m_dbcUser2->closeProject(nullptr) == true);
	QVERIFY(m_dbcAdmin->closeProject(nullptr) == true);

	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	QVERIFY(m_dbcAdmin->deleteProject(m_projectName, m_adminPassword, true, 0) == true);

	for(DbController* dbc : m_dbc)
	{
		delete dbc;
	}
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

QString DbControllerSignalTests::dbc_addSignal(DbController* dbc,
											E::SignalType type,
											QStringList& appSignalIDs,
											std::vector<int>* addedIDs)
{
	TS_TEST_PTR_RETURN(dbc);
	TS_VERIFY_RETURN_ERR(appSignalIDs.isEmpty() == false, "appSignalIDs shouldn't be empty");

	if (addedIDs != nullptr)
	{
		addedIDs->clear();
	}

	QVector<AppSignal> newSignals;

	for(QString appSignalID : appSignalIDs)
	{
		AppSignal s;

		s.setID(-1);
		s.initCreatedDates();
		s.setAppSignalID(appSignalID);
		s.setCustomAppSignalID(appSignalID.replace("#", ""));
		s.setCustomAppSignalID(QString("Caption ") + s.customAppSignalID());

		newSignals.append(s);
	}

	TS_VERIFY_RETURN_ERR(dbc->addSignal(type, &newSignals, nullptr) == true, dbc->lastError());

	if (addedIDs != nullptr)
	{
		for(const AppSignal& s : newSignals)
		{
			addedIDs->push_back(s.ID());
		}

		std::sort(addedIDs->begin(), addedIDs->end());
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::dbc_addSignal(DbController* dbc,
											   E::SignalType type,
											   int channelCount,
											   std::vector<int>* addedIDs)
{
	Q_ASSERT(channelCount > 0);

	QStringList appSignalIDs;

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	for(int i = 0; i < channelCount; i++)
	{
		appSignalIDs.append(QString("#SIGNAL_%1_%2").
								arg(E::valueToString<E::SignalType>(type).toUpper()).arg(now + i));
	}

	return dbc_addSignal(dbc, type, appSignalIDs, addedIDs);
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

QString DbControllerSignalTests::setSignalWorkcopy(int userID, const AppSignal& s, ObjectState* obState)
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
	QSqlDatabase db;

	TS_VERIFY_RETURN_ERR(openDatabase(db) == true, "Database isn't open");

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

	if (obState != nullptr)
	{
		DbWorker::db_objectState(q, obState);
	}

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

QString DbControllerSignalTests::getAllSignalsInstancesIDs(std::vector<int>* allSignalsInstancesIDsSorted)
{
	TS_TEST_PTR_RETURN(allSignalsInstancesIDsSorted);

	allSignalsInstancesIDsSorted->clear();

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, "SELECT signalinstanceid FROM SignalInstance ORDER BY signalinstanceid ASC");

	while(q.next() == true)
	{
		allSignalsInstancesIDsSorted->push_back(q.value(0).toInt());
	}

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getActualSignalsSignalInstanceID(int userID, bool with_deleted, std::vector<int>* ids)
{
	TS_TEST_PTR_RETURN(ids);

	ids->clear();

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM get_signals_actual_signalinstanceid(%1, %2)").
										arg(userID).arg(with_deleted == true ? "TRUE" : "FALSE"));

	while(q.next() == true)
	{
		ids->push_back(q.value(0).toInt());
	}

	std::sort(ids->begin(), ids->end());

	TS_RETURN_SUCCESS();
}

QString DbControllerSignalTests::getLatestSignal(int userID, int signalID, AppSignal* s)
{
	TS_TEST_PTR_RETURN(s);

	QSqlQuery q;

	TS_EXEC_QUERY_RETURN_ERR(q, QString("SELECT * FROM get_latest_signal(%1, %2)").
										arg(userID).arg(signalID));

	TS_VERIFY_RETURN_ERR(q.first() == true, "Can't get Signal record");

	DbWorker::getSignalData(q, *s);

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

std::vector<std::pair<int, QString>> DbControllerSignalTests::toPairsVector(const QVector<ID_AppSignalID>& qv)
{
	std::vector<std::pair<int, QString>> result;

	for(const ID_AppSignalID& v : qv)
	{
		result.push_back({v.ID, v.appSignalID});
	}

	return result;
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


