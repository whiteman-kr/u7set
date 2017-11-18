#include <functional>
#include <QDateTime>
#include <QFile>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QHostInfo>

#include "../lib/DbWorker.h"
#include "../lib/DeviceObject.h"
#include "../lib/DbProgress.h"


// Upgrade database
//
const UpgradeItem DbWorker::upgradeItems[] =
{
	{":/DatabaseUpgrade/Upgrade0001.sql", "Create project"},
	{":/DatabaseUpgrade/Upgrade0002.sql", "Upgrade to version 2"},
	{":/DatabaseUpgrade/Upgrade0003.sql", "Upgrade to version 3"},
	{":/DatabaseUpgrade/Upgrade0004.sql", "Upgrade to version 4"},
	{":/DatabaseUpgrade/Upgrade0005.sql", "Upgrade to version 5"},
	{":/DatabaseUpgrade/Upgrade0006.sql", "Upgrade to version 6"},
	{":/DatabaseUpgrade/Upgrade0007.sql", "Upgrade to version 7"},
	{":/DatabaseUpgrade/Upgrade0008.sql", "Upgrade to version 8"},
	{":/DatabaseUpgrade/Upgrade0009.sql", "Upgrade to version 9"},
	{":/DatabaseUpgrade/Upgrade0010.sql", "Upgrade to version 10"},
	{":/DatabaseUpgrade/Upgrade0011.sql", "Upgrade to version 11"},
	{":/DatabaseUpgrade/Upgrade0012.sql", "Upgrade to version 12"},
	{":/DatabaseUpgrade/Upgrade0013.sql", "Upgrade to version 13"},
	{":/DatabaseUpgrade/Upgrade0014.sql", "Upgrade to version 14"},
	{":/DatabaseUpgrade/Upgrade0015.sql", "Upgrade to version 15"},
	{":/DatabaseUpgrade/Upgrade0016.sql", "Upgrade to version 16"},
	{":/DatabaseUpgrade/Upgrade0017.sql", "Upgrade to version 17"},
	{":/DatabaseUpgrade/Upgrade0018.sql", "Upgrade to version 18"},
	{":/DatabaseUpgrade/Upgrade0019.sql", "Upgrade to version 19"},
	{":/DatabaseUpgrade/Upgrade0020.sql", "Upgrade to version 20"},
	{":/DatabaseUpgrade/Upgrade0021.sql", "Upgrade to version 21"},
	{":/DatabaseUpgrade/Upgrade0022.sql", "Upgrade to version 22"},
	{":/DatabaseUpgrade/Upgrade0023.sql", "Upgrade to version 23"},
	{":/DatabaseUpgrade/Upgrade0024.sql", "Upgrade to version 24"},
	{":/DatabaseUpgrade/Upgrade0025.sql", "Upgrade to version 25"},
	{":/DatabaseUpgrade/Upgrade0026.sql", "Upgrade to version 26"},
	{":/DatabaseUpgrade/Upgrade0027.sql", "Upgrade to version 27"},
	{":/DatabaseUpgrade/Upgrade0028.sql", "Upgrade to version 28"},
	{":/DatabaseUpgrade/Upgrade0029.sql", "Upgrade to version 29"},
	{":/DatabaseUpgrade/Upgrade0030.sql", "Upgrade to version 30"},
	{":/DatabaseUpgrade/Upgrade0031.sql", "Upgrade to version 31"},
	{":/DatabaseUpgrade/Upgrade0032.sql", "Upgrade to version 32"},
	{":/DatabaseUpgrade/Upgrade0033.sql", "Upgrade to version 33"},
	{":/DatabaseUpgrade/Upgrade0034.sql", "Upgrade to version 34"},
	{":/DatabaseUpgrade/Upgrade0035.sql", "Upgrade to version 35"},
	{":/DatabaseUpgrade/Upgrade0036.sql", "Upgrade to version 36"},
	{":/DatabaseUpgrade/Upgrade0037.sql", "Upgrade to version 37"},
	{":/DatabaseUpgrade/Upgrade0038.sql", "Upgrade to version 38"},
	{":/DatabaseUpgrade/Upgrade0039.sql", "Upgrade to version 39"},
	{":/DatabaseUpgrade/Upgrade0040.sql", "Upgrade to version 40"},
	{":/DatabaseUpgrade/Upgrade0041.sql", "Upgrade to version 41"},
	{":/DatabaseUpgrade/Upgrade0042.sql", "Upgrade to version 42"},
	{":/DatabaseUpgrade/Upgrade0043.sql", "Upgrade to version 43"},
	{":/DatabaseUpgrade/Upgrade0044.sql", "Upgrade to version 44"},
	{":/DatabaseUpgrade/Upgrade0045.sql", "Upgrade to version 45"},
	{":/DatabaseUpgrade/Upgrade0046.sql", "Upgrade to version 46"},
	{":/DatabaseUpgrade/Upgrade0047.sql", "Upgrade to version 47"},
	{":/DatabaseUpgrade/Upgrade0048.sql", "Upgrade to version 48"},
	{":/DatabaseUpgrade/Upgrade0049.sql", "Upgrade to version 49"},
	{":/DatabaseUpgrade/Upgrade0050.sql", "Upgrade to version 50"},
	{":/DatabaseUpgrade/Upgrade0051.sql", "Upgrade to version 51"},
	{":/DatabaseUpgrade/Upgrade0052.sql", "Upgrade to version 52"},
	{":/DatabaseUpgrade/Upgrade0053.sql", "Upgrade to version 53"},
	{":/DatabaseUpgrade/Upgrade0054.sql", "Upgrade to version 54"},
	{":/DatabaseUpgrade/Upgrade0055.sql", "Upgrade to version 55"},
	{":/DatabaseUpgrade/Upgrade0056.sql", "Upgrade to version 56"},
	{":/DatabaseUpgrade/Upgrade0057.sql", "Upgrade to version 57"},
	{":/DatabaseUpgrade/Upgrade0058.sql", "Upgrade to version 58"},
	{":/DatabaseUpgrade/Upgrade0059.sql", "Upgrade to version 59"},
	{":/DatabaseUpgrade/Upgrade0060.sql", "Upgrade to version 60"},
	{":/DatabaseUpgrade/Upgrade0061.sql", "Upgrade to version 61"},
	{":/DatabaseUpgrade/Upgrade0062.sql", "Upgrade to version 62"},
	{":/DatabaseUpgrade/Upgrade0063.sql", "Upgrade to version 63"},
	{":/DatabaseUpgrade/Upgrade0064.sql", "Upgrade to version 64"},
	{":/DatabaseUpgrade/Upgrade0065.sql", "Upgrade to version 65"},
	{":/DatabaseUpgrade/Upgrade0066.sql", "Upgrade to version 66"},
	{":/DatabaseUpgrade/Upgrade0067.sql", "Upgrade to version 67"},
	{":/DatabaseUpgrade/Upgrade0068.sql", "Upgrade to version 68"},
	{":/DatabaseUpgrade/Upgrade0069.sql", "Upgrade to version 69"},
	{":/DatabaseUpgrade/Upgrade0070.sql", "Upgrade to version 70"},
	{":/DatabaseUpgrade/Upgrade0071.sql", "Upgrade to version 71"},
	{":/DatabaseUpgrade/Upgrade0072.sql", "Upgrade to version 72"},
	{":/DatabaseUpgrade/Upgrade0073.sql", "Upgrade to version 73"},
	{":/DatabaseUpgrade/Upgrade0074.sql", "Upgrade to version 74"},
	{":/DatabaseUpgrade/Upgrade0075.sql", "Upgrade to version 75"},
	{":/DatabaseUpgrade/Upgrade0076.sql", "Upgrade to version 76"},
	{":/DatabaseUpgrade/Upgrade0077.sql", "Upgrade to version 77"},
	{":/DatabaseUpgrade/Upgrade0078.sql", "Upgrade to version 78"},
	{":/DatabaseUpgrade/Upgrade0079.sql", "Upgrade to version 79"},
	{":/DatabaseUpgrade/Upgrade0080.sql", "Upgrade to version 80"},
	{":/DatabaseUpgrade/Upgrade0081.sql", "Upgrade to version 81"},
	{":/DatabaseUpgrade/Upgrade0082.sql", "Upgrade to version 82"},
	{":/DatabaseUpgrade/Upgrade0083.sql", "Upgrade to version 83"},
	{":/DatabaseUpgrade/Upgrade0084.sql", "Upgrade to version 84"},
	{":/DatabaseUpgrade/Upgrade0085.sql", "Upgrade to version 85"},
	{":/DatabaseUpgrade/Upgrade0086.sql", "Upgrade to version 86"},
	{":/DatabaseUpgrade/Upgrade0087.sql", "Upgrade to version 87"},
	{":/DatabaseUpgrade/Upgrade0088.sql", "Upgrade to version 88"},
	{":/DatabaseUpgrade/Upgrade0089.sql", "Upgrade to version 89"},
	{":/DatabaseUpgrade/Upgrade0090.sql", "Upgrade to version 90"},
	{":/DatabaseUpgrade/Upgrade0091.sql", "Upgrade to version 91"},
	{":/DatabaseUpgrade/Upgrade0092.sql", "Upgrade to version 92"},
	{":/DatabaseUpgrade/Upgrade0093.sql", "Upgrade to version 93"},
	{":/DatabaseUpgrade/Upgrade0094.sql", "Upgrade to version 94"},
	{":/DatabaseUpgrade/Upgrade0095.sql", "Upgrade to version 95"},
	{":/DatabaseUpgrade/Upgrade0096.sql", "Upgrade to version 96"},
	{":/DatabaseUpgrade/Upgrade0097.sql", "Upgrade to version 97"},
	{":/DatabaseUpgrade/Upgrade0098.sql", "Upgrade to version 98"},
	{":/DatabaseUpgrade/Upgrade0099.sql", "Upgrade to version 99"},
	{":/DatabaseUpgrade/Upgrade0100.sql", "Upgrade to version 100"},
	{":/DatabaseUpgrade/Upgrade0101.sql", "Upgrade to version 101"},
	{":/DatabaseUpgrade/Upgrade0102.sql", "Upgrade to version 102"},
	{":/DatabaseUpgrade/Upgrade0103.sql", "Upgrade to version 103"},
	{":/DatabaseUpgrade/Upgrade0104.sql", "Upgrade to version 104"},
	{":/DatabaseUpgrade/Upgrade0105.sql", "Upgrade to version 105"},
	{":/DatabaseUpgrade/Upgrade0106.sql", "Upgrade to version 106"},
	{":/DatabaseUpgrade/Upgrade0107.sql", "Upgrade to version 107"},
	{":/DatabaseUpgrade/Upgrade0108.sql", "Upgrade to version 108"},
	{":/DatabaseUpgrade/Upgrade0109.sql", "Upgrade to version 109"},
	{":/DatabaseUpgrade/Upgrade0110.sql", "Upgrade to version 110"},
	{":/DatabaseUpgrade/Upgrade0111.sql", "Upgrade to version 111"},
	{":/DatabaseUpgrade/Upgrade0112.sql", "Upgrade to version 112"},
	{":/DatabaseUpgrade/Upgrade0113.sql", "Upgrade to version 113, add fileinstance_index_fileid, get_file_history, get_file_history_recursive"},
	{":/DatabaseUpgrade/Upgrade0114.sql", "Upgrade to version 114, add get_changeset_details, change retval get_file_history, get_file_history_recursive"},
	{":/DatabaseUpgrade/Upgrade0115.sql", "Upgrade to version 115, add get_history function"},
	{":/DatabaseUpgrade/Upgrade0116.sql", "Upgrade to version 116, remove administrator rights to all except UserID=1"},
	{":/DatabaseUpgrade/Upgrade0117.sql", "Upgrade to version 117, update LM-1 preset"},
	{":/DatabaseUpgrade/Upgrade0118.sql", "Upgrade to version 118, update configuration script to count UniqueID"},
    {":/DatabaseUpgrade/Upgrade0119.sql", "Upgrade to version 119, add RegRawDataDescription to LM-1"},
	{":/DatabaseUpgrade/Upgrade0120.sql", "Upgrade to version 120, added get_signal_history() and get_specific_signal() stored procedures"},
	{":/DatabaseUpgrade/Upgrade0121.sql", "Upgrade to version 121, get_specific_copy() by time, changes in get_specific_copy by changeset"},
    {":/DatabaseUpgrade/Upgrade0122.sql", "Upgrade to version 122, default IP address computing algoruthm has been fixed in configuration"},
	{":/DatabaseUpgrade/Upgrade0123.sql", "Upgrade to version 123, changes in function get_latest_signals_by_appsignalids()"},
	{":/DatabaseUpgrade/Upgrade0124.sql", "Upgrade to version 124, Changing auths functions"},
	{":/DatabaseUpgrade/Upgrade0125.sql", "Upgrade to version 125, CONNECTIONS system folder was added"},
    {":/DatabaseUpgrade/Upgrade0126.sql", "Upgrade to version 126, LM-1 TxDiagDataSize changed to 176"},
	{":/DatabaseUpgrade/Upgrade0127.sql", "Upgrade to version 127, Update AppLogic and UFB templates, macroses are added"},
	{":/DatabaseUpgrade/Upgrade0128.sql", "Upgrade to version 128, fix spelling administrator word in dbuser type"},
    {":/DatabaseUpgrade/Upgrade0129.sql", "Upgrade to version 129, added Units to AfbElementParam in damper, int, der, tctc"},
    {":/DatabaseUpgrade/Upgrade0130.sql", "Upgrade to version 130, integrator and derive afb elements have constant version"},
    {":/DatabaseUpgrade/Upgrade0131.sql", "Upgrade to version 131, afb elements damp and latch were changed"},
	{":/DatabaseUpgrade/Upgrade0132.sql", "Upgrade to version 132, force users to use latest software version"},
    {":/DatabaseUpgrade/Upgrade0133.sql", "Upgrade to version 133, AFB elements update"},
	{":/DatabaseUpgrade/Upgrade0134.sql", "Upgrade to version 134, DOM, LM-1, OCM and OCMN presets update by RPCT-1455"},
	{":/DatabaseUpgrade/Upgrade0135.sql", "Upgrade to version 135, Added metrology preset"},
    {":/DatabaseUpgrade/Upgrade0136.sql", "Upgrade to version 136, Fixed caption in AIM"},
    {":/DatabaseUpgrade/Upgrade0137.sql", "Upgrade to version 137, Removed EnableSerial property from OCM and OCMN script"},
	{":/DatabaseUpgrade/Upgrade0138.sql", "Upgrade to version 138, Updated metrology preset"},
    {":/DatabaseUpgrade/Upgrade0139.sql", "Upgrade to version 139, Logic Module Description file has been added"},
	{":/DatabaseUpgrade/Upgrade0140.sql", "Upgrade to version 140, New Logic Module Configuration script based on TypeScript"},
	{":/DatabaseUpgrade/Upgrade0141.sql", "Upgrade to version 141, Add FSC Chassis preset and Place checking"},
	{":/DatabaseUpgrade/Upgrade0142.sql", "Upgrade to version 142, Changed in TuningClient preset properties"},
	{":/DatabaseUpgrade/Upgrade0143.sql", "Upgrade to version 143, Changes in TuningClient and Monitor presets"},
	{":/DatabaseUpgrade/Upgrade0144.sql", "Upgrade to version 144, Add new properties to LM preset"},
	{":/DatabaseUpgrade/Upgrade0145.sql", "Upgrade to version 145, Changes in the LM description file"},
	{":/DatabaseUpgrade/Upgrade0146.sql", "Upgrade to version 146, Added project database test functions"},
	{":/DatabaseUpgrade/Upgrade0147.sql", "Upgrade to version 147, LM Description file changed"},
	{":/DatabaseUpgrade/Upgrade0148.sql", "Upgrade to version 148, Added version in configuration script fields description"},
	{":/DatabaseUpgrade/Upgrade0149.sql", "Upgrade to version 149, Changed func is_any_checked_out according to current user"},
	{":/DatabaseUpgrade/Upgrade0150.sql", "Upgrade to version 150, LogicModule0000.xml file update"},
	{":/DatabaseUpgrade/Upgrade0151.sql", "Upgrade to version 151, OCM and OCMN presets update"},
	{":/DatabaseUpgrade/Upgrade0152.sql", "Upgrade to version 152, add func public.delete_file_on_update"},
	{":/DatabaseUpgrade/Upgrade0153.sql", "Upgrade to version 153, OCM and OCMN presets update, delete old signals"},
	{":/DatabaseUpgrade/Upgrade0154.sql", "Upgrade to version 154, LogicModule0000.xml file update"},
	{":/DatabaseUpgrade/Upgrade0155.sql", "Upgrade to version 155, append new fields in SignalInstance table"},
	{":/DatabaseUpgrade/Upgrade0156.sql", "Upgrade to version 156, Add BUSTYPES system folder"},
	{":/DatabaseUpgrade/Upgrade0157.sql", "Upgrade to version 157, Modified validity signals in IO modules and LM"},
	{":/DatabaseUpgrade/Upgrade0158.sql", "Upgrade to version 158, In module presets Module temperature has ADC range 0-400"},
	{":/DatabaseUpgrade/Upgrade0159.sql", "Upgrade to version 159, Update Memory settings in module presets, changes in CfgService preset"},
	{":/DatabaseUpgrade/Upgrade0160.sql", "Upgrade to version 160, AFB elements dec, cod and tconv update"},
	{":/DatabaseUpgrade/Upgrade0161.sql", "Upgrade to version 161, AFB elements tconv update"},
	{":/DatabaseUpgrade/Upgrade0162.sql", "Upgrade to version 162, AFB elements"},
	{":/DatabaseUpgrade/Upgrade0163.sql", "Upgrade to version 163, LM Configuration script was updated"},
	{":/DatabaseUpgrade/Upgrade0164.sql", "Upgrade to version 164, Changes in ArchiveService and AppDataService presets"},
	{":/DatabaseUpgrade/Upgrade0165.sql", "Upgrade to version 165, AFBL Library was updated"},
	{":/DatabaseUpgrade/Upgrade0166.sql", "Upgrade to version 166, Implementing safe file functions"},
	{":/DatabaseUpgrade/Upgrade0167.sql", "Upgrade to version 167, Fixing error: Deleted but not checked-in file remains in the result of get_latest_file_tree_version"},
	{":/DatabaseUpgrade/Upgrade0168.sql", "Upgrade to version 168, Changes in signalInstance table, deletion unit table"},
	{":/DatabaseUpgrade/Upgrade0169.sql", "Upgrade to version 169, AFBL library (cmp, dec_num and cod_num) was updated"},
	{":/DatabaseUpgrade/Upgrade0170.sql", "Upgrade to version 170, AFBL library was updated"},
	{":/DatabaseUpgrade/Upgrade0171.sql", "Upgrade to version 171, AFBL library was updated"},
	{":/DatabaseUpgrade/Upgrade0172.sql", "Upgrade to version 172, AFBL library was updated"},
	{":/DatabaseUpgrade/Upgrade0173.sql", "Upgrade to version 173, Fixed lim_fp update error"},
	{":/DatabaseUpgrade/Upgrade0174.sql", "Upgrade to version 174, AFBL library was updated"},
	{":/DatabaseUpgrade/Upgrade0175.sql", "Upgrade to version 175, AFBL library was updated"},
	{":/DatabaseUpgrade/Upgrade0176.sql", "Upgrade to version 176, AFBL: Attr MaxBusSize moved to attr Size in LogicModule description"},
	{":/DatabaseUpgrade/Upgrade0177.sql", "Upgrade to version 177, Add bus_xor, set i_bus_width to 16 for bus_or, bus_and"},
	{":/DatabaseUpgrade/Upgrade0178.sql", "Upgrade to version 178, Add new LM descriptions: LM1-SF00 and LM1-SR01"},
	{":/DatabaseUpgrade/Upgrade0179.sql", "Upgrade to version 179, To LM1-SF00.xml and LM1-SR01.xml added MaxInstCount per AfbComponent"},
	{":/DatabaseUpgrade/Upgrade0180.sql", "Upgrade to version 180, Fixing DataUid generating in OCMN module"},
};


int DbWorker::counter = 0;

DbWorker::DbWorker()
{
	assert(false);
}

DbWorker::DbWorker(DbProgress* progress) :
	m_host("127.0.0.1"),
	m_port(5432),
	m_serverUsername("u7"),
	m_serverPassword(""),
	m_progress(progress),
	m_instanceNo(counter)
{
	DbWorker::counter ++;		// static variable

	assert(m_progress);
}

QString DbWorker::postgresConnectionName() const
{
	return QString("postgres_%1").arg(m_instanceNo);
}

QString DbWorker::projectConnectionName() const
{
	return QString("project_%1").arg(m_instanceNo);
}

bool DbWorker::checkDatabaseFeatures(QSqlDatabase db)
{
	if (db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	// Check required database features
	//
	bool hasTransaction = db.driver()->hasFeature(QSqlDriver::Transactions);
	bool hasQuerySize = db.driver()->hasFeature(QSqlDriver::QuerySize);
	bool hasBlob = db.driver()->hasFeature(QSqlDriver::BLOB);
	bool hasUnicode = db.driver()->hasFeature(QSqlDriver::Unicode);
	bool hasPreparedQueries = db.driver()->hasFeature(QSqlDriver::PreparedQueries);
	//bool hasPositionalPlaceholders = db.driver()->hasFeature(QSqlDriver::PositionalPlaceholders);
	//bool hasLastInsertId = db.driver()->hasFeature(QSqlDriver::LastInsertId);
	//bool hasLowPrecisionNumbers = db.driver()->hasFeature(QSqlDriver::LowPrecisionNumbers);
	//bool hasEventNotifications = db.driver()->hasFeature(QSqlDriver::EventNotifications);

	// now Postgres or it's driver doen't have the next features
	//

	//bool hasNamedPlaceholders = db.driver()->hasFeature(QSqlDriver::NamedPlaceholders);
	//bool hasBatchOperations = db.driver()->hasFeature(QSqlDriver::BatchOperations);
	//bool hasSimpleLocking = db.driver()->hasFeature(QSqlDriver::SimpleLocking);
	//bool hasFinishQuery = db.driver()->hasFeature(QSqlDriver::FinishQuery);
	//bool hasMultipleResultSets = db.driver()->hasFeature(QSqlDriver::MultipleResultSets);
	//bool hasCancelQuery = db.driver()->hasFeature(QSqlDriver::CancelQuery);

	if (hasTransaction == false)
	{
		qCritical() << "Database Error: Transactions feature is not supported";
	}

	if (hasQuerySize == false)
	{
		qCritical() << "Database Error: QuerySize feature is not supported";
	}

	if (hasBlob == false)
	{
		qCritical() << "Database Error: hasBlob feature is not supported";
	}

	if (hasUnicode == false)
	{
		qCritical() << "Database Error: hasUnicode feature is not supported";
	}

	if (hasPreparedQueries == false)
	{
		qCritical() << "Database Error: hasPreparedQueries feature is not supported";
	}

	return hasTransaction == true &&
		hasQuerySize == true &&
		hasBlob == true &&
		hasUnicode == true &&
		hasPreparedQueries == true;
}

void DbWorker::emitError(QSqlDatabase db, const QSqlError& err, bool addLogRecord)
{
	emitError(db, err.text(), addLogRecord);
}

void DbWorker::emitError(QSqlDatabase db, const QString& err, bool addLogRecord)
{
	QString errorMessage = err;

	if (sessionKey().isEmpty() == false &&
		isProjectOpened() == true)
	{
		errorMessage.replace(sessionKey(), QLatin1String("xxx"));
	}

	if (db.isOpen() == true &&
		addLogRecord == true)
	{
		this->addLogRecord(db, errorMessage);
	}

	qDebug() << errorMessage;
	m_progress->setErrorMessage(errorMessage);
}

int DbWorker::databaseVersion()
{
	return sizeof(upgradeItems) / sizeof(upgradeItems[0]);
}

bool DbWorker::isProjectOpened() const
{
	return !currentProject().databaseName().isEmpty();
}

int DbWorker::rootFileId() const
{
	return 0;
}

int DbWorker::afblFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_afblFileId;
}

int DbWorker::ufblFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_ufblFileId;
}

int DbWorker::alFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_alFileId;
}

int DbWorker::hcFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_hcFileId;
}

int DbWorker::hpFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_hpFileId;
}

int DbWorker::mvsFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_mvsFileId;
}

int DbWorker::dvsFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_dvsFileId;
}

int DbWorker::mcFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_mcFileId;
}

int DbWorker::connectionsFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_connectionsFileId;
}

int DbWorker::busTypesFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_busTypesFileId;
}

std::vector<DbFileInfo> DbWorker::systemFiles() const
{
	QMutexLocker m(&m_mutex);
	std::vector<DbFileInfo> copy(m_systemFiles);
	return copy;
}

QString DbWorker::toSqlStr(QString str)
{
	str = str.replace("'", "''");
	return str;
}

QString DbWorker::toSqlBoolean(bool value)
{
	if (value == true)
	{
		return "TRUE";
	}

	return "FALSE";
}

void DbWorker::slot_getProjectList(std::vector<DbProject>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	// Open database and get project list
	//
	std::shared_ptr<int*> removeDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(postgresConnectionName());		// remove database
		});

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", postgresConnectionName());
		if (db.lastError().isValid() == true)
		{
			emitError(db, db.lastError());
			return;
		}

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName("postgres");
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool ok = db.open();
		if (ok == false)
		{
			emitError(db, db.lastError());
			return;
		}

		ok = checkDatabaseFeatures(db);
		if (ok == false)
		{
			emitError(db, tr("Database driver doesn't have required features."));
			db.close();
			return;
		}

		// Get database list and filter it for projects
		//
		out->clear();

		QSqlQuery query(db);

		bool result = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7\\_%' OR datname LIKE 'U7\\_%' ORDER BY datname;");

		if (result == false)
		{
			emitError(db, query.lastError());
			db.close();
			return;
		}

		while (query.next())
		{
			QString databaseName = query.value(0).toString();
			QString projectName = databaseName;

			projectName.replace("u7_", "", Qt::CaseInsensitive);

			// --
			//
			DbProject p;
			p.setDatabaseName(databaseName);
			p.setProjectName(projectName);
			p.setProjectName(projectName);

			out->push_back(p);
		}

		db.close();
	}

	// Open each project and get it's version
	//

	for (auto pi = out->begin(); pi != out->end(); ++pi)
	{
		QString projectDatabaseConnectionName = QString("%1_%2 connection").arg(m_instanceNo).arg(pi->projectName());

		std::shared_ptr<int*> removeDatabase(nullptr, [projectDatabaseConnectionName](void*)
			{
				QSqlDatabase::removeDatabase(projectDatabaseConnectionName);		// remove database
			});

		{
			QSqlDatabase projectDb = QSqlDatabase::addDatabase("QPSQL", projectDatabaseConnectionName);
			projectDb.setHostName(host());
			projectDb.setPort(port());
			projectDb.setDatabaseName(pi->databaseName());
			projectDb.setUserName(serverUsername());
			projectDb.setPassword(serverPassword());

			bool result = projectDb.open();
			if (result == false)
			{
				emitError(projectDb, projectDb.lastError());
				continue;
			}

			// Get project version, scope is for versionQuery
			//
			{
				QString createVersionTableSql = QString("SELECT max(VersionNo) FROM Version;");

				QSqlQuery versionQuery(projectDb);
				result = versionQuery.exec(createVersionTableSql);

				int projectVersion = -1;

				if (result == false)
				{
					qDebug() << versionQuery.lastError();
				}
				else
				{
					if (versionQuery.next())
					{
						projectVersion = versionQuery.value(0).toInt();
					}
				}

				pi->setVersion(projectVersion);
			}

			// From this version ProjectProperties table is added, so it is possible to request propertyes
			//
			if (pi->version() >= 41)
			{
				QString getProjectDescriptionSql = QString("SELECT * FROM get_project_property('Description');");

				QSqlQuery q(projectDb);
				result = q.exec(getProjectDescriptionSql);

				QString projectDescription;

				if (result == false)
				{
					qDebug() << q.lastError();
				}
				else
				{
					if (q.next())
					{
						projectDescription = q.value(0).toString();
					}
				}

				pi->setDescription(projectDescription);
			}

			// --
			//
			projectDb.close();
		}
	}

	// Database will be removed by the removeDatabase shared_ptr
	//
	return;
}

void DbWorker::slot_createProject(QString projectName, QString administratorPassword)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (projectName.isEmpty() == true ||
		administratorPassword.isEmpty() == true)
	{
		assert(projectName.isEmpty() == false);
		assert(administratorPassword.isEmpty() == false);
		return;
	}

	// Open database
	//
	QString databaseName;

	std::shared_ptr<int*> removeDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(postgresConnectionName());		// remove database
		});

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", postgresConnectionName());
		if (db.lastError().isValid() == true)
		{
			emitError(db, db.lastError());
			return;
		}

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName("postgres");
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool ok = db.open();
		if (ok == false)
		{
			emitError(db, db.lastError());
			return;
		}

		ok = checkDatabaseFeatures(db);
		if (ok == false)
		{
			emitError(db, tr("Database driver doesn't have required features."));
			db.close();
			return;
		}

		// Create project database
		//
		databaseName = "u7_" + projectName;
		databaseName = databaseName.toLower();

		QSqlQuery query(db);

		QString createDatabaseSql = QString("CREATE DATABASE %1 WITH ENCODING='UTF8' CONNECTION LIMIT=-1;").arg(databaseName);

		bool result = query.exec(createDatabaseSql);

		if (result == false)
		{
			emitError(db, query.lastError());
			db.close();
			return;
		}

		db.close();
	}

	// connect to the new database
	//
	QString connectionDatabaseName = QString("%1_%2 connection").arg(m_instanceNo).arg(databaseName);

	std::shared_ptr<int*> removeNewDatabase(nullptr, [connectionDatabaseName](void*)
	{
		QSqlDatabase::removeDatabase(connectionDatabaseName);		// remove database
	});

	{
		QSqlDatabase newDatabase = QSqlDatabase::addDatabase("QPSQL", connectionDatabaseName);

		newDatabase.setHostName(host());
		newDatabase.setPort(port());
		newDatabase.setDatabaseName(databaseName);
		newDatabase.setUserName(serverUsername());
		newDatabase.setPassword(serverPassword());

		bool result = newDatabase.open();
		if (result == false)
		{
			emitError(newDatabase, newDatabase.lastError());
			return;
		}

		// Create version table
		//
		QSqlQuery newDbQuery(newDatabase);

		QString createVersionTableSql = QString(
					"CREATE TABLE version ("
					"versionid SERIAL PRIMARY KEY,"
					"versionno integer NOT NULL,"
					"date timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,"
					"reasone text NOT NULL"
					");");

		result = newDbQuery.exec(createVersionTableSql);
		if (result == false)
		{
			emitError(newDatabase, newDbQuery.lastError());
			newDatabase.close();
			return;
		}

		// Create get_project_version function
		//
		QString request = "CREATE OR REPLACE FUNCTION get_project_version()"
						  "RETURNS integer AS"
						  "'SELECT max(VersionNo) FROM Version;'"
						  "LANGUAGE sql;";

		result = newDbQuery.exec(request);
		if (result == false)
		{
			emitError(newDatabase, newDbQuery.lastError());
			newDatabase.close();
			return;
		}

		// Add first record to version table
		//
		QString addFirstVersionRecord = QString(
			"INSERT INTO version (VersionNo, Reasone)"
			"VALUES (1, 'Create project');");

		result = newDbQuery.exec(addFirstVersionRecord);
		if (result == false)
		{
			emitError(newDatabase, newDbQuery.lastError());
			newDbQuery.clear();
			newDatabase.close();
			return;
		}

		// Create Users table
		//
		QString createUserTableSql = QString(
			"CREATE TABLE users"
			"("
				"userid serial PRIMARY KEY NOT NULL,"
				"date timestamp with time zone NOT NULL DEFAULT now(),"
				"username text NOT NULL UNIQUE,"
				"firstname text NOT NULL,"
				"lastname text NOT NULL,"
				"password text NOT NULL,"
				"administrator boolean NOT NULL DEFAULT FALSE,"
				"readonly boolean NOT NULL DEFAULT TRUE"
			");"
			);

		result = newDbQuery.exec(createUserTableSql);
		if (result == false)
		{
			emitError(newDatabase, newDbQuery.lastError());
			newDbQuery.clear();
			newDatabase.close();
			return;
		}

		// Add Administrator record to the table
		//
		newDbQuery.clear();

		newDbQuery.prepare(
			"INSERT INTO users(Username, FirstName, LastName, Password, Administrator, ReadOnly)"
			"VALUES (:username, :firstname, :lastname, :password, :administrator, :readonly);");

		newDbQuery.bindValue(":username", "Administrator");
		newDbQuery.bindValue(":firstname", " ");
		newDbQuery.bindValue(":lastname", " ");
		newDbQuery.bindValue(":password", administratorPassword);
		newDbQuery.bindValue(":administrator", true);
		newDbQuery.bindValue(":readonly", false);

		result = newDbQuery.exec();
		if (result == false)
		{
			emitError(newDatabase, newDbQuery.lastError());
			newDatabase.close();
			return;
		}

		// --
		//
		newDbQuery.clear();
		newDatabase.close();
	}

	return;
}

void DbWorker::slot_openProject(QString projectName, QString username, QString password)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// It is possible to get into this function only if the project database is upgraded to actual state
	// So, it is possible to use all the latest functions
	//

	// Check parameters
	//
	projectName = projectName.trimmed();
	username = username.trimmed();

	if (projectName.isEmpty() || username.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(username.isEmpty() == false);
		assert(password.isEmpty() == false);
		return;
	}

	// Open database
	//
	projectName = projectName.trimmed();
	QString databaseName = "u7_" + projectName.toLower();
	username = username.trimmed();

	if (isProjectOpened() == true)
	{
		emitError(QSqlDatabase(), tr("OpenProject error, another project is opened. To open a new project, please close the current project."));
		return;
	}


	// Open database, removeDatabase will be called in slot_closeProject()
	//
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", projectConnectionName());

	db.setHostName(host());
	db.setPort(port());
	db.setDatabaseName(databaseName);
	db.setUserName(serverUsername());
	db.setPassword(serverPassword());

	bool result = db.open();
	if (result == false)
	{
		emitError(db, db.lastError());
		return;
	}

	// log in
	//
	QString errorMessage;

	result = db_logIn(db, username, password, &errorMessage);
	if (result == false)
	{
		emitError(db, errorMessage);
		db.close();
		return;
	}

	// Set user data
	//
	int userId = -1;
	result = db_getCurrentUserId(db, &userId);

	if (result == false ||
		userId == -1)
	{
		emitError(db, tr("Can't get current user id ") + db.lastError().text());
		db.close();
		return;
	}

	DbUser user;
	result = db_getUserData(db, userId, &user);

	if (result == false)
	{
		emitError(db, tr("Can't read user data ") + db.lastError().text());
		db.close();
		return;
	}

	if (user.isDisabled() == true)
	{
		emitError(db, tr("User %1 is not allowed to open the project. User is disabled, contact the project administartor.").arg(username));
		db.close();
		return;
	}

	user.setPassword(password);		// This password will be used to open project for build

	setCurrentUser(user);

	// Set project data
	//
	DbProject project;

	project.setDatabaseName(databaseName);
	project.setProjectName(projectName);

	QString projectDescription;
	getProjectProperty_worker("Description", &projectDescription);

	project.setDescription(projectDescription);

	setCurrentProject(project);

	// Set System Folders File ID
	//
	std::vector<DbFileInfo> systemFiles;

	getFileList_worker(&systemFiles, rootFileId(), "", true);

	m_mutex.lock();
	m_afblFileId = -1;
	m_ufblFileId = -1;
	m_alFileId = -1;
	m_hcFileId = -1;
	m_hpFileId = -1;
	m_mvsFileId = -1;
	m_dvsFileId = -1;
	m_mcFileId = -1;
	m_connectionsFileId = -1;
	m_busTypesFileId = -1;
	m_systemFiles.clear();
	m_mutex.unlock();

	// Root file is filling manually
	//
	{
		QMutexLocker locker(&m_mutex);

		DbFileInfo rfi;
		rfi.setFileId(rootFileId());
		rfi.setFileName(rootFileName);
		m_systemFiles.push_back(rfi);
	}

	for (const DbFileInfo& fi : systemFiles)
	{
		if (fi.fileName() == ::AfblFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_afblFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::UfblFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_ufblFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::AlFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_alFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::HcFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_hcFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::HpFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_hpFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::MvsFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_mvsFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::DvsFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_dvsFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::McFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_mcFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::ConnectionsFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_connectionsFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == ::BusTypesFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_busTypesFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}
	}


	m_mutex.lock();
	result = m_afblFileId != -1;
	result &= m_ufblFileId != -1;
	result &= m_alFileId != -1;
	result &= m_hcFileId != -1;
	result &= m_hpFileId != -1;
	result &= m_mvsFileId != -1;
	result &= m_dvsFileId != -1;
	result &= m_mcFileId != -1;
	result &= m_connectionsFileId != -1;
	result &= m_busTypesFileId != -1;
	m_mutex.unlock();

	if (result == false)
	{
		emitError(db, tr("Can't get system folder.") + db.lastError().text());
		db.close();

		// Lock is nit necessare, we will crash anyway!
		//
		assert(m_afblFileId != -1);
		assert(m_ufblFileId != -1);
		assert(m_alFileId != -1);
		assert(m_hcFileId != -1);
		assert(m_hpFileId != -1);
		assert(m_mvsFileId != -1);
		assert(m_dvsFileId != -1);
		assert(m_mcFileId != -1);

		return;
	}

	// Add log record
	//
	addLogRecord(db, QString("Project is opened. Software version %1, supported DB version %2")
				 .arg(qApp->applicationName() + " " + qApp->applicationVersion())
				 .arg(DbWorker::databaseVersion()));

	return;
}

void DbWorker::slot_closeProject()
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Log close
	//
	{
		QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

		if (db.isOpen() == true)
		{
			addLogRecord(db, "Project is about to close.");
		}
	}

	// Check
	//
	setCurrentUser(DbUser());
	setCurrentProject(DbProject());
	setSessionKey(QString());

	if (QSqlDatabase::contains(projectConnectionName()) == false)
	{
		emitError(QSqlDatabase(), tr("Project is not opened."));
		return;
	}

	// Close database connection and remove it from database list
	//
	{
		QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

		if (db.isOpen() == false)
		{
			emitError(QSqlDatabase(), tr("Project database connection is closed."));
			return;
		}

		db.close();
	}

	QSqlDatabase::removeDatabase(projectConnectionName());

	return;
}

void DbWorker::slot_cloneProject(QString projectName, QString password, QString newProjectName)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	projectName = projectName.trimmed();
	QString username = "Administrator";

	if (projectName.isEmpty() || username.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(password.isEmpty() == false);
		return;
	}

	// Check password for Administrator
	//
	projectName = projectName.trimmed();
	QString databaseName = "u7_" + projectName.toLower();
	username = username.trimmed();

	QString newDatabaseName = "u7_" + newProjectName.trimmed().toLower();

	// Open database, removeDatabase will be called in slot_closeProject()
	//
	{
		std::shared_ptr<int*> removeNewDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(projectConnectionName());		// remove database
		});

		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", projectConnectionName());

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName(databaseName);
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool result = db.open();
		if (result == false)
		{
			emitError(db, db.lastError());
			return;
		}

		addLogRecord(db, QString("About to clone project to.").arg(newDatabaseName));

		result = db_checkUserPassword(db, username, password);
		if (result == false)
		{
			emitError(db, "Wrong password.");
			db.close();
			return;
		}
	}

	// Clone project
	//
	std::shared_ptr<int*> removeDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(postgresConnectionName());		// remove database
		});

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", postgresConnectionName());
		if (db.lastError().isValid() == true)
		{
			emitError(db, db.lastError());
			return;
		}

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName("postgres");
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool ok = db.open();
		if (ok == false)
		{
			emitError(db, db.lastError());
			return;
		}

		QSqlQuery query(db);
		QString sqlRequest = QString("CREATE DATABASE %1 WITH TEMPLATE %2").arg(newDatabaseName).arg(databaseName);

		bool result = query.exec(sqlRequest);

		if (result == false)
		{
			emitError(db, query.lastError(), false);
			db.close();
			return;
		}

		db.close();
	}

	// --
	//
	setCurrentProject(DbProject());
	setCurrentUser(DbUser());

	return;
}

void DbWorker::slot_deleteProject(QString projectName, QString password, bool doNotBackup)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	projectName = projectName.trimmed();
	QString username = "Administrator";

	if (projectName.isEmpty() || username.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(password.isEmpty() == false);
		return;
	}

	// Check password for Administrator
	//
	projectName = projectName.trimmed();
	QString databaseName = "u7_" + projectName.toLower();
	username = username.trimmed();

	// Open database, removeDatabase will be called in slot_closeProject()
	//
	{
		std::shared_ptr<int*> removeNewDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(projectConnectionName());		// remove database
		});

		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", projectConnectionName());

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName(databaseName);
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool result = db.open();
		if (result == false)
		{
			emitError(db, db.lastError());
			return;
		}

		addLogRecord(db, "About to delete project.");

		result = db_checkUserPassword(db, username, password);
		if (result == false)
		{
			emitError(db, "Wrong password.");
			db.close();
			return;
		}
	}

	// Rename project from the template u7_[projectname] to u7deleted_[projectname]_[datetime]
	//
	std::shared_ptr<int*> removeDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(postgresConnectionName());		// remove database
		});

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", postgresConnectionName());
		if (db.lastError().isValid() == true)
		{
			emitError(db, db.lastError());
			return;
		}

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName("postgres");
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool ok = db.open();
		if (ok == false)
		{
			emitError(db, db.lastError());
			return;
		}

		QString strTime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

		QSqlQuery query(db);
		QString createDatabaseSql;

		if (doNotBackup == true)
		{
			createDatabaseSql = QString("DROP DATABASE %1;")
								.arg(databaseName);
		}
		else
		{
			createDatabaseSql = QString("ALTER DATABASE %1 RENAME TO u7deleted_%2_%3;")
								.arg(databaseName)
								.arg(projectName.toLower())
								.arg(strTime);
		}

		bool result = query.exec(createDatabaseSql);

		if (result == false)
		{
			emitError(db, query.lastError(), false);
			db.close();
			return;
		}

		db.close();
	}

	// --
	//
	setCurrentProject(DbProject());
	setCurrentUser(DbUser());

	return;
}

void DbWorker::slot_upgradeProject(QString projectName, QString password, bool doNotBackup)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			//qDebug() << "SetCompleted()";
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	projectName = projectName.trimmed();
	QString username = "Administrator";

	if (projectName.isEmpty() || username.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(password.isEmpty() == false);
		return;
	}

	// Check password for Administrator
	//
	projectName = projectName.trimmed();
	QString databaseName = "u7_" + projectName.toLower();
	username = username.trimmed();

	// Open database, removeDatabase will be called in slot_closeProject()
	//

	int projectVersion = 0;

	{
		std::shared_ptr<int*> removeNewDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(projectConnectionName());		// remove database
		});

		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", projectConnectionName());

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName(databaseName);
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool result = db.open();
		if (result == false)
		{
			emitError(QSqlDatabase(), db.lastError());
			return;
		}

		projectVersion = db_getProjectVersion(db);
		if (projectVersion == -1)
		{
			emitError(QSqlDatabase(), "Cannot get project database version.");
			db.close();
			return;
		}

		result = db_checkUserPassword(db, username, password);
		if (result == false)
		{
			emitError(QSqlDatabase(), "Wrong password.");
			db.close();
			return;
		}
	}

	if (doNotBackup == false)
	{
		// Copy project from the template u7_[projectname] to u7upgrade[oldversion]_[projectname]_[datetime]
		//
		std::shared_ptr<int*> removeDatabase(nullptr, [this](void*)
			{
				QSqlDatabase::removeDatabase(postgresConnectionName());		// remove database
			});

		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", postgresConnectionName());
		if (db.lastError().isValid() == true)
		{
			emitError(QSqlDatabase(), db.lastError());
			return;
		}

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName("postgres");
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool ok = db.open();
		if (ok == false)
		{
			emitError(QSqlDatabase(), db.lastError());
			return;
		}

		QString strTime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

		QSqlQuery query(db);
		QString copyDatabaseSql =
			QString("CREATE DATABASE u7upgrade%1_%2_%3 WITH TEMPLATE %4 OWNER %5;")
				.arg(projectVersion)
				.arg(projectName.toLower())
				.arg(strTime)
				.arg(databaseName)
				.arg(serverUsername());

		bool result = query.exec(copyDatabaseSql);

		if (result == false)
		{
			emitError(QSqlDatabase(), query.lastError());
			db.close();
			return;
		}

		db.close();
	}

	// Upgrade
	//
	{
		std::shared_ptr<int*> removeNewDatabase(nullptr, [this](void*)
		{
			QSqlDatabase::removeDatabase(projectConnectionName());		// remove database
		});

		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", projectConnectionName());

		db.setHostName(host());
		db.setPort(port());
		db.setDatabaseName(databaseName);
		db.setUserName(serverUsername());
		db.setPassword(serverPassword());

		bool result = db.open();
		if (result == false)
		{
			emitError(QSqlDatabase(), db.lastError());
			return;
		}

		// Start transaction
		//
		result = db.transaction();
		if (result == false)
		{
			emitError(QSqlDatabase(), db.lastError());
			db.close();
			return;
		}

		{
			std::shared_ptr<int*> closeDb(nullptr, [&db, &result](void*)
				{
					if (result == true)
					{
						//qDebug() << "Upgrade: Commit changes.";
						db.commit();
					}
					else
					{
						qDebug() << "Upgrade: Rollback changes.";
						db.rollback();
					}

					db.close();
				});

			// Get project version, check it
			//

			// Lock Version table
			// LOCK TABLE "Version" IN ACCESS EXCLUSIVE MODE NOWAIT;
			//
			QSqlQuery versionQuery(db);
			result = versionQuery.exec("LOCK TABLE Version IN ACCESS EXCLUSIVE MODE NOWAIT;");

			if (result == false)
			{
				emitError(QSqlDatabase(), versionQuery.lastError());
				return;
			}
			versionQuery.clear();

			projectVersion = db_getProjectVersion(db);
			if (projectVersion == -1)
			{
				emitError(QSqlDatabase(), versionQuery.lastError());
				return;
			}

			// Some checks
			//
			if (projectVersion == databaseVersion())
			{
				emitError(QSqlDatabase(), tr("Database %1 is up to date.").arg(databaseName));
				return;
			}

			if (projectVersion > databaseVersion())
			{
				emitError(QSqlDatabase(), tr("Database %1 is newer than the software version.").arg(databaseName));
				return;
			}

			// Log action
			//
			if (projectVersion >= 111)
			{
				QString logMessage = QString("Upgrading project DB from version %1 to %2")
									 .arg(projectVersion + 1)
									 .arg(databaseVersion() + 1);

				addLogRecord(db, logMessage);
			}

			if (projectVersion >= 124)
			{
				// Log in to obtaine session key
				//
				QString errorMessage;
				result = db_logIn(db, username, password, &errorMessage);

				if (result == false)
				{
					emitError(db, errorMessage, true);
					return;
				}

				assert(m_sessionKey.isEmpty() == false);
			}

			// Upgrade database
			//
			for (int i = projectVersion; i < databaseVersion(); i++)
			{
				m_progress->setValue(static_cast<int>(100.0 / (databaseVersion() - projectVersion) * (i - projectVersion)));

				// Get  Update
				//
				const UpgradeItem& ui = upgradeItems[i];

				// Perform upgade action
				//
				QFile upgradeFile(ui.upgradeFileName);

				//qDebug() << "Begin upgrade: item " << i << " completed, file: " << ui.upgradeFileName;

				result = upgradeFile.open(QIODevice::ReadOnly | QIODevice::Text);

				if (result == false)
				{
					emitError(QSqlDatabase(), tr("Can't open file %1. \n%2").arg(ui.upgradeFileName).arg(upgradeFile.errorString()), false);
					break;
				}

				QString upgradeScript = upgradeFile.readAll();

				// Set Session key
				//
				if (i + 1 > 124)		// 'i' is index of update file, +1 to get project version
				{
					assert(m_sessionKey.isEmpty() == false);

					upgradeScript.replace("$(SessionKey)", sessionKey());
				}

				// Run upgrade script
				//
				{
					QSqlQuery upgradeQuery(db);

					result = upgradeQuery.exec(upgradeScript);

					if (result == false)
					{
						emitError(QSqlDatabase(), upgradeQuery.lastError(), false);
						break;
					}
				}

				// Add record to Version table
				//
				{
					QString addVersionRecord;

					if (i < 107)	// i < {"Upgrade to version 108", ":/DatabaseUpgrade/Upgrade0108.sql"},
					{
						addVersionRecord = QString("INSERT INTO Version (VersionNo, Reasone) VALUES (%1, '%2');").arg(i + 1).arg(ui.text);
					}
					else
					{
						// Column host is already added to table Version
						//
						addVersionRecord = QString("INSERT INTO Version (VersionNo, Reasone, Host) VALUES (%1, '%2', '%3');")
										   .arg(i + 1)
										   .arg(ui.text)
										   .arg(QHostInfo::localHostName());
					}


					QSqlQuery versionQuery(db);

					result = versionQuery.exec(addVersionRecord);

					if (result == false)
					{
						emitError(QSqlDatabase(), versionQuery.lastError(), false);
						break;
					}
				}

				// Function log_in is created in update 124, after it we need to log in
				//
				if (i + 1 == 124)	// 'i' is index of update file, +1 to get project version
				{
					// Log in to obtaine session key
					//
					QString errorMessage;
					result = db_logIn(db, username, password, &errorMessage);

					if (result == false)
					{
						emitError(db, errorMessage, true);
						return;
					}

					assert(m_sessionKey.isEmpty() == false);
				}

				//qDebug() << "End upgrade item";
			}

			// The table FileInstance has details field which is JSONB, details for some files
			// DeviceObjects has some description in details() method
			// Some files can be added during update and most likely
			// these instances will not contain details,
			// Here, read Equipment Configuration Files, parse them, update details column
			//
			{
				QString reqEquipmentList =
R"(
SELECT
	FI.FileInstanceID AS FileInstanceID, F.FileID AS FileID, F.Name AS Name
FROM
	FileInstance AS FI, File AS F
WHERE
	FI.FileID = F.FileID AND
	(FI.FileInstanceID = F.CheckedInInstanceID OR FI.FileInstanceID = F.CheckedOutInstanceID) AND
	(Name ILIKE '%.hsm' OR Name ILIKE '%.hrk' OR Name ILIKE '%.hcs' OR Name ILIKE '%.hmd' OR Name ILIKE '%.hcr' OR Name ILIKE '%.hws' OR Name ILIKE '%.hsw' OR Name ILIKE '%.hds') AND
	FI.Details = '{}';
)";

				//qDebug() << "Update file details";

				QSqlQuery euipmentListQuery(db);
				result = euipmentListQuery.exec(reqEquipmentList);

				if (result == false)
				{
					emitError(QSqlDatabase(), euipmentListQuery.lastError());
					return;
				}

				while (euipmentListQuery.next())
				{
					QUuid fileInstanceId = euipmentListQuery.value(0).toUuid();
					/*int fileId = */euipmentListQuery.value(1).toInt();
					QString fileName = euipmentListQuery.value(2).toString();

					//qDebug() << "FileName: " << fileName << ", FileID: " << fileId << ", FileInstanceID: " << fileInstanceId.toString();

					// Get file instance, read it to DeviceObject
					//
					{
						QSqlQuery getFileQuery(db);

						result = getFileQuery.exec(QString("SELECT Data FROM FileInstance WHERE FileInstanceID = '%1';").arg(fileInstanceId.toString()));

						if (result == false || getFileQuery.next() == false)
						{
							emitError(QSqlDatabase(), tr("Cannot get file data, FileInstanceID: %1").arg(fileInstanceId.toString()));
							return;
						}

						QByteArray data = getFileQuery.value(0).toByteArray();
						std::shared_ptr<Hardware::DeviceObject> device = Hardware::DeviceObject::Create(data);

						if (device == nullptr)
						{
							result = false;
							emitError(QSqlDatabase(), tr("Cannot read file data, FileName %1, FileInstanceID %2.").arg(fileName).arg(fileInstanceId.toString()));
							return;
						}

						getFileQuery.clear();

						QString details = device->details();

						// Update details field in DB
						//
						QSqlQuery updateDetailsQuery(db);

						updateDetailsQuery.prepare("UPDATE FileInstance SET Details = :details WHERE FileInstanceID = :fileInstanceId;");
						updateDetailsQuery.bindValue(":details", details);
						updateDetailsQuery.bindValue(":fileInstanceId", fileInstanceId.toString());

						result = updateDetailsQuery.exec();

						if (result == false)
						{
							emitError(QSqlDatabase(), tr("Cannot update file details, FileInstanceID: %1").arg(fileInstanceId.toString()));
							return;
						}
					}
				}
			}
		}
	}

	return;
}

void DbWorker::slot_setProjectProperty(QString propertyName, QString propertyValue)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (propertyName.isEmpty() == true)
	{
		assert(propertyName.isEmpty() == false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not openned."));
		return;
	}

	// Log action
	//
	addLogRecord(db, tr("slot_setProjectProperty: propertyName '%1', propertyValue '%2'").arg(propertyName).arg(propertyValue));

	// --
	//
	QSqlQuery query(db);

	query.prepare("SELECT * FROM set_project_property(:propertyName, :propertyValue);");
	query.bindValue(":propertyName", propertyName);
	query.bindValue(":propertyValue", propertyValue);

	bool result = query.exec();

	if (result == false)
	{
		emitError(db, tr("Cannot set property %1, error: %2").arg(propertyName).arg(db.lastError().text()));
		return;
	}

	if (query.size() > 0)
	{
		result = query.next();
		assert(result);
	}

	return;
}

void DbWorker::slot_getProjectProperty(QString propertyName, QString* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	return getProjectProperty_worker(propertyName, out);
}

void DbWorker::getProjectProperty_worker(QString propertyName, QString* out)
{
	// Check parameters
	//
	if (propertyName.isEmpty() == true || out == nullptr)
	{
		assert(propertyName.isEmpty() == false);
		assert(out);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not openned."));
		return;
	}

	// Check if such user already exists
	// SELECT * FROM creat_user();
	//
	QSqlQuery query(db);

	query.prepare("SELECT * FROM get_project_property(:propertyName);");
	query.bindValue(":propertyName", propertyName);

	bool result = query.exec();

	if (result == false)
	{
		emitError(db, tr("Cannot get project property value %1, error: %2").arg(propertyName).arg(db.lastError().text()));
		return;
	}

	if (query.size() > 0)
	{
		result = query.next();
		assert(result);

		*out = query.value(0).toString();
	}

	return;
}

void DbWorker::slot_createUser(DbUser user)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (user.username().isEmpty() == true)
	{
		assert(user.username().isEmpty() == false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get user list. Database connection is not openned."));
		return;
	}

	if (currentUser().isAdminstrator() == false)
	{
		emitError(db, tr("Current user does not have administrator privileges."));
		return;
	}

	// Log action
	//
	addLogRecord(db, tr("slot_createUser: Username '%1'").arg(user.username()));

	// Check if such user already exists
	// SELECT * FROM creat_user();
	//
	QSqlQuery query(db);

	query.prepare("SELECT * FROM user_api.create_user(:sessionkey, :username, :firstname, :lastname, :newpassword, :isreadonly, :isdisabled);");
	query.bindValue(":sessionkey", sessionKey());
	query.bindValue(":username", user.username());
	query.bindValue(":firstname", user.firstName());
	query.bindValue(":lastname", user.lastName());
	query.bindValue(":newpassword", user.newPassword());
	query.bindValue(":isreadonly", user.isReadonly());
	query.bindValue(":isdisabled", user.isDisabled());

	bool result = query.exec();

	if (result == false)
	{
		emitError(db, tr("Can't create user %1, error: %2").arg(user.username()).arg(query.lastError().text()));
		return;
	}

	if (query.size() > 0)
	{
		result = query.next();
		assert(result);
		//int userID = query.value("UserID").toInt();
	}

	return;
}

void DbWorker::slot_updateUser(DbUser user)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (user.username().isEmpty() == true)
	{
		assert(user.username().isEmpty() == false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not openned."));
		return;
	}

	if (currentUser().username() != user.username() && currentUser().isAdminstrator() == false)
	{
		assert(false);
		emitError(db, tr("Only administrators can change other users' details."));
		return;
	}

	// Log action
	//
	addLogRecord(db, tr("slot_updateUser: Username '%1'").arg(user.username()));

	// update user
	//
	QSqlQuery query(db);

	QString requestStr = QString("SELECT * FROM user_api.update_user('%1', '%2', '%3', '%4', '%5', '%6', %7, %8);")
						 .arg(sessionKey())
						 .arg(DbWorker::toSqlStr(user.username()))
						 .arg(DbWorker::toSqlStr(user.firstName()))
						 .arg(DbWorker::toSqlStr(user.lastName()))
						 .arg(DbWorker::toSqlStr(user.password()))
						 .arg(user.newPassword().isEmpty() ? QString::null : DbWorker::toSqlStr(user.newPassword()))
						 .arg(user.isReadonly() ? "TRUE" : "FALSE")
						 .arg(user.isDisabled() ? "TRUE" : "FALSE");

	bool result = query.exec(requestStr);

	if (result == false)
	{
		emitError(db, tr("Can't update user %1, error: %2").arg(user.username()).arg(query.lastError().text()));
		return;
	}

	if (query.size() > 0)
	{
		result = query.next();
		assert(result);

		//int userID = query.value("UserID").toInt();
	}

	// If update is ok AND user updates iself AND new password was set
	// update password in m_currentUser, it will be used for opening project on build
	//

	if (user.newPassword().isEmpty() == false &&
		currentUser().username() == user.username())
	{
		DbUser cu = currentUser();
		cu.setPassword(user.newPassword());
		setCurrentUser(cu);
	}

	return;
}

void DbWorker::slot_getUserList(std::vector<DbUser>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	// Operation
	//

	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get user list. Database connection is not openned."));
		return;
	}

	// SELECT UserID FROM Users ORDER BY Username;
	//
	QSqlQuery q(db);

	bool result = q.exec("SELECT UserID FROM Users ORDER BY Username;");
	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << q.lastError();
		assert(result);
		return;
	}

	out->clear();

	while (q.next())
	{
		DbUser user;

		int userId = q.value(0).toInt();

		bool ok = db_getUserData(db, userId, &user);

		if (ok == true)
		{
			out->push_back(user);
		}
	}

	return;
}

void DbWorker::slot_isFileExists(QString fileName, int parentId, int* fileId)
{
	// Check parameters
	//
	if (fileId == nullptr)
	{
		assert(fileId != nullptr);
		return;
	}

	*fileId = -1;

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not openned."));
		return;
	}

	QString request = QString("SELECT * FROM api.is_file_exists('%1', %2, '%3');")
			.arg(sessionKey())
			.arg(parentId)
			.arg(fileName);

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Can't run query. Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() == false)
	{
		*fileId = -1;			// Return NULL, file does not exist
	}
	else
	{
		*fileId = q.value(0).toInt();
	}

	return;
}

void DbWorker::slot_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	return getFileList_worker(files, parentId, filter, removeDeleted);
}

void DbWorker::getFileList_worker(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted)
{
	// Check parameters
	//
	if (files == nullptr)
	{
		assert(files != nullptr);
		return;
	}

	files->clear();

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	QSqlQuery q(db);
	q.setForwardOnly(true);

	if (filter.isEmpty() == true)
	{
		q.prepare("SELECT * FROM api.get_file_list(:session_key, :parentid);");
		q.bindValue(":session_key", sessionKey());
		q.bindValue(":parentid", parentId);
	}
	else
	{
		q.prepare("SELECT * FROM api.get_file_list(:session_key, :parentid, :filter);");
		q.bindValue(":session_key", sessionKey());
		q.bindValue(":parentid", parentId);
		q.bindValue(":filter", "%" + filter);
	}

	bool result = q.exec();

	if (result == false)
	{
		emitError(db, tr("Can't get file list. Error: ") +  q.lastError().text());
		return;
	}

	files->reserve(q.size());

	while (q.next())
	{
		DbFileInfo fileInfo;

		db_dbFileInfo(q, &fileInfo);

		if (removeDeleted == false ||
			(removeDeleted == true && fileInfo.deleted() == false))
		{
			files->push_back(fileInfo);
		}
	}

	return;
}

void DbWorker::slot_getFileInfo(int parentId, QString fileName, DbFileInfo* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	QString request = QString("SELECT * FROM api.get_file_info('%1', %2, '%3');")
			.arg(sessionKey())
			.arg(parentId)
			.arg(fileName);

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't get file info. Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() == false)
	{
		emitError(db, tr("File %1 not found, reply is empty.").arg(fileName));
	}
	else
	{
		db_dbFileInfo(q, out);
	}

	return;
}

void DbWorker::slot_getFilesInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (fileIds == nullptr ||
		fileIds->empty() == true ||
		out == nullptr)
	{
		assert(fileIds != nullptr);
		assert(out != nullptr);
		return;
	}

	out->clear();

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	QString request = QString("SELECT * FROM api.get_file_info('%1', ARRAY[")
			.arg(sessionKey());

	for (auto it = fileIds->begin(); it != fileIds->end(); ++it)
	{
		if (it == fileIds->begin())
		{
			request += QString("%1").arg(*it);
		}
		else
		{
			request += QString(", %1").arg(*it);
		}
	}

	request += "]);";

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't get file info. Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbFileInfo fileInfo;

		db_dbFileInfo(q, &fileInfo);

		out->push_back(fileInfo);
	}

	return;
}

void DbWorker::slot_addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr || files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_addFiles: FileCount %1, ParentID %2, FileNames: ")
						 .arg(files->size())
						 .arg(parentId);

	for (auto& f : *files)
	{
		logMessage += f->fileName() + QLatin1String(" ");
	}

	addLogRecord(db, logMessage);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		std::shared_ptr<DbFile> file = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QSqlQuery q(db);
		q.setForwardOnly(true);

		q.prepare("SELECT * FROM add_file(:userid, :filename, :parentid, :filedata, :details);");

		q.bindValue(":userid", currentUser().userId());
		q.bindValue(":filename", file->fileName());
		q.bindValue(":parentid", parentId);
		q.bindValue(":filedata", file->data());
		q.bindValue(":details", file->details());

		bool result = q.exec();

		if (result == false)
		{
			emitError(db, tr("Can't add file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't get request result."));
			return;
		}

		file->setFileId(q.value(0).toInt());		// File just created, init it's fileId and parentid
		file->setParentId(parentId);

		db_updateFileState(q, file.get(), true);
	}

	return;
}

void DbWorker::slot_deleteFiles(std::vector<DbFileInfo>* files)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr || files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot delete files. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_deleteFiles: FileCount %1, FileNames: ").arg(files->size());
	for (auto& f : *files)
	{
		logMessage += f.fileName() + QLatin1String(" ");
	}

	addLogRecord(db, logMessage);

	// files for deletion shoud be sorted in DESCENDING FileID order, to delete dependant files first
	//
	std::vector<DbFileInfo> filesToDetele;
	filesToDetele.reserve(files->size());

	filesToDetele.assign(files->begin(), files->end());

	std::sort(filesToDetele.begin(), filesToDetele.end(),
		[](const DbFileInfo& f1, const DbFileInfo& f2)
		{
			return f1.fileId() >= f2.fileId();
		});

	// Iterate through files
	//
	for (unsigned int i = 0; i < filesToDetele.size(); i++)
	{
		DbFileInfo& file = filesToDetele[i];

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM delete_file(%1, %2);")
				.arg(currentUser().userId())
				.arg(file.fileId());

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(db, tr("Can't delete file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't get result"));
			return;
		}

		db_updateFileState(q, &file, true);
	}

	// set back DbFilInfo states
	//
	files->swap(filesToDetele);

	return;
}

void DbWorker::slot_getLatestVersion(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true ||
		out == nullptr)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		const DbFileInfo& fi = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM api.get_latest_file_version('%1', %2);")
				.arg(sessionKey())
				.arg(fi.fileId());

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(db, tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't find file: %1").arg(fi.fileName()));
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		db_updateFile(q, file.get());

		assert(fi.fileId() == file->fileId());

		out->push_back(file);
	}

	return;
}

void DbWorker::slot_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::vector<std::shared_ptr<DbFile>>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (parentFileInfo.fileId() == -1 ||
		out == nullptr)
	{
		assert(parentFileInfo.fileId() != -1);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// request, result is a list of DbFile
	//
	QTime timerObject;
	timerObject.start();

	QString request = QString("SELECT * FROM api.get_latest_file_tree_version('%1', %2);")
			.arg(sessionKey())
			.arg(parentFileInfo.fileId());

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Can't get file. Error: ") +  q.lastError().text());
		return;
	}

	out->clear();
	out->reserve(q.size());

//	qint64 memoryAllocationEllpased = 0;
//	qint64 updateFileEllpased = 0;
//	qint64 pushBackEllpased = 0;

	while (q.next())
	{
		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		//QElapsedTimer updateFileTimer;
		//updateFileTimer.start();

		db_updateFile(q, file.get());

		//updateFileEllpased += updateFileTimer.nsecsElapsed();

		out->push_back(file);
	}

	qDebug() << "Request time is " << timerObject.elapsed() << " ms, request: " << request;
//	qDebug() << "\tmemoryAllocationEllpased " << memoryAllocationEllpased / 1000000;
//	qDebug() << "\tupdateFileEllpased " << updateFileEllpased / 1000000;
//	qDebug() << "\tpushBackEllpased " << pushBackEllpased / 1000000;

	return;
}

void DbWorker::slot_getCheckedOutFiles(const std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (parentFiles == nullptr ||
		parentFiles->empty() == true ||
		out == nullptr)
	{
		assert(parentFiles != nullptr);
		assert(parentFiles->empty() == false);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// ARRAY[1, 2, 3];
	//
	QString filesArray;
	for (size_t pi = 0; pi < parentFiles->size(); pi++)
	{
		if (pi == 0)
		{
			filesArray = QString("ARRAY[%1").arg(parentFiles->at(pi).fileId());
		}
		else
		{
			filesArray += QString(", %1").arg(parentFiles->at(pi).fileId());
		}
	}
	filesArray += "]";

	// request, result is a list of DbFile
	//
	QString request = QString("SELECT * FROM api.get_checked_out_files('%1', %2);")
			.arg(sessionKey())
			.arg(filesArray);

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Can't get file. Error: ") +  q.lastError().text());
		return;
	}

	out->reserve(q.size());

	while (q.next())
	{
		DbFileInfo fileInfo;

		db_dbFileInfo(q, &fileInfo);

		out->push_back(fileInfo);
	}

	return;
}


void DbWorker::slot_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true ||
		out == nullptr)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		const DbFileInfo& fi = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM api.get_workcopy('%1', %2);")
				.arg(sessionKey())
				.arg(fi.fileId());

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(db, tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't find workcopy for file: %1. Is file CheckedOut?").arg(fi.fileName()));
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		db_updateFile(q, file.get());

		out->push_back(file);

		assert(fi.fileId() == file->fileId());
	}

	return;
}

void DbWorker::slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_setWorkcopy: FileCount %1, FileNames: ").arg(files->size());
	for (auto& f : *files)
	{
		logMessage += QString("%1 %2, ").arg(f->fileName()).arg(f->fileId());
	}

	addLogRecord(db, logMessage);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		std::shared_ptr<DbFile> file = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM api.set_workcopy('%1', %2, ")
				.arg(sessionKey())
				.arg(file->fileId());

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);
		data.clear();

		request += QString(", '%1');").arg(file->details());

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(db, tr("Can't save file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't get FileID"));
			return;
		}

		int fileId = q.value(0).toInt();

		if (fileId != file->fileId())
		{
			assert(fileId == file->fileId());
			emitError(db, tr("Write file error. filename: %1").arg(file->fileName()));
			continue;
		}
	}

	return;
}

void DbWorker::slot_getSpecificCopy(const std::vector<DbFileInfo>* files, int changesetId, std::vector<std::shared_ptr<DbFile>>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true ||
		out == nullptr)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		const DbFileInfo& fi = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM get_specific_copy(%1, %2, %3);")
				.arg(currentUser().userId())
				.arg(fi.fileId())
				.arg(changesetId);

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(db, tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't find workcopy for file: %1. Is file CheckedOut?").arg(fi.fileName()));
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		db_updateFile(q, file.get());

		out->push_back(file);

		assert(fi.fileId() == file->fileId());
	}

	return;
}

void DbWorker::slot_getSpecificCopy(const std::vector<DbFileInfo>* files, QDateTime date, std::vector<std::shared_ptr<DbFile>>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true ||
		out == nullptr)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		const DbFileInfo& fi = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM get_specific_copy(%1, %2, '%3'::timestamp with time zone);")
				.arg(currentUser().userId())
				.arg(fi.fileId())
				.arg(date.toString("yyyy-MM-dd HH:mm:ss"));

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(db, tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't find workcopy for file: %1. Is file CheckedOut?").arg(fi.fileName()));
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		db_updateFile(q, file.get());

		out->push_back(file);

		assert(fi.fileId() == file->fileId());
	}

	return;
}

void DbWorker::slot_checkIn(std::vector<DbFileInfo>* files, QString comment)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_checkIn: Comment '%1', FileCount %2, FileNames: ").arg(comment).arg(files->size());
	for (auto& f : *files)
	{
		logMessage += QString("%1 %2, ").arg(f.fileName()).arg(f.fileId());
	}

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM check_in(%1, ARRAY[")
		.arg(currentUser().userId());

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		auto file = files->at(i);

		if (i == 0)
		{
			request += QString("%1").arg(file.fileId());
		}
		else
		{
			request += QString(", %1").arg(file.fileId());
		}
	}

	request += QString("], '%1');")
			.arg(DbWorker::toSqlStr(comment));

	// request
	//
	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't check in. Error: ") +  q.lastError().text());
		return;
	}

	// Result is table of (ObjectState);
	//
	while (q.next())
	{
		int fileId = q.value(0).toInt();

		// Set file state to CheckedIn
		//
		bool updated = false;
		for (auto& fi : *files)
		{
			if (fi.fileId() == fileId)
			{
				db_updateFileState(q, &fi, true);
				updated = true;
				break;
			}
		}
		assert(updated == true);
	}

	return;
}

void DbWorker::slot_checkInTree(std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* outCheckedIn, QString comment)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (parentFiles == nullptr ||
		parentFiles->empty() == true ||
		outCheckedIn == nullptr)
	{
		assert(parentFiles != nullptr);
		assert(parentFiles->empty() != true);
		assert(outCheckedIn != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_checkInTree: Comment '%1', FileCount %2, ParentFileNames: ")
						 .arg(comment)
						 .arg(parentFiles->size());

	for (auto& f : *parentFiles)
	{
		logMessage += f.fileName() + QLatin1String(" ");
	}

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM check_in_tree(%1, ARRAY[")
		.arg(currentUser().userId());

	// Iterate through files
	//
	for (unsigned int i = 0; i < parentFiles->size(); i++)
	{
		auto file = parentFiles->at(i);

		if (i == 0)
		{
			request += QString("%1").arg(file.fileId());
		}
		else
		{
			request += QString(", %1").arg(file.fileId());
		}
	}

	request += QString("], '%1');")
			.arg(DbWorker::toSqlStr(comment));

	// request
	//
	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't check in. Error: ") +  q.lastError().text());
		return;
	}

	// Result is table of (ObjectState);
	//
	outCheckedIn->clear();

	int resultSize = q.size();
	if (resultSize != -1)
	{
		outCheckedIn->reserve(resultSize);
	}

	while (q.next())
	{
		// Update file state
		//
		DbFileInfo fi;

		db_updateFileState(q, &fi, false);
		outCheckedIn->push_back(fi);
	}

	return;
}

void DbWorker::slot_checkOut(std::vector<DbFileInfo>* files)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (files == nullptr ||
		files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_checkOut: FileCount %1, FileNames: ").arg(files->size());
	for (auto& f : *files)
	{
		logMessage += QString("%1 %2, ").arg(f.fileName()).arg(f.fileId());
	}

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM check_out(%1, ARRAY[")
		.arg(currentUser().userId());

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		auto file = files->at(i);

		if (i == 0)
		{
			request += QString("%1").arg(file.fileId());
		}
		else
		{
			request += QString(", %1").arg(file.fileId());
		}
	}

	request += QString("]);");

	// request
	//
	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't check out file. Error: ") +  q.lastError().text());
		return;
	}

	// Result is table of (ObjectState);
	//
	while (q.next())
	{
		int fileId = q.value(0).toInt();

		// Set file state to CheckedIn
		//
		bool updated = false;
		for (auto& fi : *files)
		{
			if (fi.fileId() == fileId)
			{
				db_updateFileState(q, &fi, true);
				updated = true;
				break;
			}
		}
		assert(updated == true);
	}

	return;
}

void DbWorker::slot_undoChanges(std::vector<DbFileInfo>* files)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (files == nullptr ||
			files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_undoChanges: FileCount %1, FileNames: ").arg(files->size());
	for (auto& f : *files)
	{
		logMessage += QString("%1 %2, ").arg(f.fileName()).arg(f.fileId());
	}

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM undo_changes(%1, ARRAY[")
			.arg(currentUser().userId());

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		auto file = files->at(i);

		if (i == 0)
		{
			request += QString("%1").arg(file.fileId());
		}
		else
		{
			request += QString(", %1").arg(file.fileId());
		}
	}

	request += "]);";

	// request
	//
	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't check out. Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		int fileId = q.value(0).toInt();

		bool updated = false;
		for (auto& fi : *files)
		{
			if (fi.fileId() == fileId)
			{
				db_updateFileState(q, &fi, true);
				updated = true;
				break;
			}
		}
		assert(updated == true);
	}

	return;
}

void DbWorker::slot_fileHasChildren(bool* hasChildren, DbFileInfo* fileInfo)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (hasChildren == nullptr || fileInfo == nullptr)
	{
		assert(hasChildren != nullptr);
		assert(fileInfo != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM file_has_children(%1, %2)")
			.arg(currentUser().userId())
			.arg(fileInfo->fileId());

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() == false)
	{
		emitError(db, tr("Can't get result."));
		return;
	}

	int childCount = q.value(0).toInt();

	*hasChildren = childCount > 0;

	return;
}

void DbWorker::slot_getHistory(std::vector<DbChangeset>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (out == nullptr)
	{
		assert(false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// Get user list at first
	//

	// Request for history
	//
	QString request = QString("SELECT * FROM get_history(%1)")
			.arg(currentUser().userId());

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbChangeset ci;
		db_dbChangeset(q, &ci);

		out->push_back(ci);
	}

	return;
}

void DbWorker::slot_getFileHistory(DbFileInfo file, std::vector<DbChangeset>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (file.fileId() == -1 || out == nullptr)
	{
		assert(false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// Get user list at first
	//

	// Request for history
	//
	QString request = QString("SELECT * FROM get_file_history(%1, %2)")
			.arg(currentUser().userId())
			.arg(file.fileId());

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbChangeset ci;
		db_dbChangeset(q, &ci);

		out->push_back(ci);
	}

	return;
}

void DbWorker::slot_getFileHistoryRecursive(DbFileInfo parentFile, std::vector<DbChangeset>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (parentFile.fileId() == -1 || out == nullptr)
	{
		assert(false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// Get user list at first
	//

	// Request for history
	//
	QString request = QString("SELECT * FROM get_file_history_recursive(%1, %2)")
			.arg(currentUser().userId())
			.arg(parentFile.fileId());

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbChangeset ci;
		db_dbChangeset(q, &ci);

		out->push_back(ci);
	}

	return;
}

void DbWorker::slot_getChangesetDetails(int changeset, DbChangesetDetails* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// Request for data
	//
	QString request = QString("SELECT * FROM get_changeset_details(%1, %2)")
			.arg(currentUser().userId())
			.arg(changeset);

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Error: ") +  q.lastError().text());
		return;
	}

	// --
	//

//	QMapIterator<QString, QVariant> i(q.boundValues());
//	while (i.hasNext())
//	{
//		i.next();
//		qDebug() << i.key().toUtf8().data() << ": " << i.value().toString().toUtf8().data() << endl;
//	}

	while (q.next())
	{
		db_dbChangesetObject(q, out);
	}

	return;
}


void DbWorker::slot_addDeviceObject(Hardware::DeviceObject* device, int parentId)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);   // set complete flag on return
	});

	// Check parameters
	//
	if (device == nullptr)
	{
		assert(device != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_addDeviceObject: Device %1, FileID %2, ParentID %3")
						 .arg(device->equipmentId())
						 .arg(device->fileId())
						 .arg(parentId);

	addLogRecord(db, logMessage);

	// Recursive function
	//
	int nesting = 0;

	std::function<bool(Hardware::DeviceObject*, int)> addDevice =
			[&addDevice, &db, device, this, &nesting]
			(Hardware::DeviceObject* current, int parentId)
	{
		if (nesting >= static_cast<int>(Hardware::DeviceType::DeviceTypeCount) ||
				current == nullptr ||
				parentId == -1)
		{
			assert(nesting < static_cast<int>(Hardware::DeviceType::DeviceTypeCount));
			assert(current != nullptr);
			assert(parentId == -1);
			return false;
		}

		nesting ++;

		// request
		// FUNCTION add_device(user_id integer, file_data bytea, parent_id integer, file_extension text, details text)
		//
		QByteArray data;
		bool result = current->Save(data);
		if (result == false)
		{
			assert(result);
			nesting --;
			emitError(db, tr("Argument errors."));
			return false;
		}

		QSqlQuery q(db);

		q.prepare("SELECT * FROM add_device(:userid, :data, :parentid, :fileext, :details);");

		q.bindValue(":userid", currentUser().userId());
		q.bindValue(":data", data);
		q.bindValue(":parentid", parentId);
		q.bindValue(":fileext", current->fileExtension());
		q.bindValue(":details", current->details());

		result = q.exec();

		if (result == false)
		{
			nesting --;
			emitError(db, tr("Can't add device. Error: ") +  q.lastError().text());
			return false;
		}

		if (q.next() == false)
		{
			nesting --;
			emitError(db, tr("Can't get result."));
			return false;
		}

		DbFileInfo fi;
		fi.setParentId(parentId);

		db_updateFileState(q, &fi, false);
		current->setFileInfo(fi);

		// Call it for all children
		//
		for (int i = 0; i < current->childrenCount(); i++)
		{
			result = addDevice(current->child(i), current->fileInfo().fileId());
			if (result == false)
			{
				nesting --;
				return false;
			}
		}

		nesting --;
		return true;
	};

	// Start
	//
	bool ok = addDevice(device, parentId);
	assert(nesting == 0);

	if (ok == false)
	{
		return;
	}

	return;
}

void DbWorker::slot_getSignalsIDs(QVector<int> *signalsIDs)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (signalsIDs == nullptr)
	{
		assert(signalsIDs != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get signals' IDs. Database connection is not opened."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM get_signals_ids(%1, %2)")
		.arg(currentUser().userId()).arg("false");
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't get signals' IDs! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		int signalID = q.value(0).toInt();

		signalsIDs->append(signalID);
	}

	return;
}


void DbWorker::slot_getSignals(SignalSet* signalSet, bool excludeDeleted)
{
	getSignals(signalSet, excludeDeleted, false);
}


void DbWorker::slot_getTuningableSignals(SignalSet* signalSet)
{
	getSignals(signalSet, true, true);
}


void DbWorker::getSignals(SignalSet* signalSet, bool excludeDeleted, bool tuningableOnly)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (signalSet == nullptr)
	{
		assert(signalSet != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get signals. Database connection is not opened."));
		return;
	}

	int signalCount = 0;

	QString request = QString("SELECT * FROM get_signal_count(%1)")
		.arg(currentUser().userId());

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result != false && q.next() != false)
	{
		signalCount = q.value(0).toInt();
	}

	int dProgress = 0;

	if (signalCount != 0)
	{
		dProgress = signalCount / 20;
	}

	request = QString("SELECT * FROM get_latest_signals_all(%1)")
		.arg(currentUser().userId());

	result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't get signal workcopy! Error: ") +  q.lastError().text());
		return;
	}

	int n = 0;

	while(q.next() != false)
	{
		n++;

		if (dProgress != 0 && (n % dProgress) == 0)
		{
			m_progress->setValue((n / dProgress) * 5);
		}

		Signal* s = new Signal;

		getSignalData(q, *s);

		if (excludeDeleted == true && s->instanceAction() == VcsItemAction::Deleted)
		{
			delete s;
			continue;
		}

		if (tuningableOnly == true && s->enableTuning() == false)
		{
			delete s;
			continue;
		}

		signalSet->append(s->ID(), s);
	}

	signalSet->buildID2IndexMap();

	m_progress->setValue(100);

	return;
}



void DbWorker::slot_getLatestSignal(int signalID, Signal* signal)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (signal == nullptr)
	{
		assert(signal != nullptr);
		return;
	}

	signal->setID(0);		// bad signal flag

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get latest signal. Database connection is not opened."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM get_latest_signal(%1, %2)")
		.arg(currentUser().userId()).arg(signalID);
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't get signal workcopy! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		getSignalData(q, *signal);
	}

	return;
}


void DbWorker::slot_getLatestSignalsByAppSignalIDs(QStringList appSignalIds, QVector<Signal>* signalArray)
{
	AUTO_COMPLETE

	if (signalArray == nullptr)
	{
		assert(false);
		return;
	}

	signalArray->clear();

	if (appSignalIds.empty())
	{
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get latest signals by AppSignalIDs. Database connection is not opened."));
		return;
	}

	QString appSignalIdsStr;

	bool first = true;

	for(const QString& id : appSignalIds)
	{
		if (first == false)
		{
			appSignalIdsStr.append(",");
		}

		appSignalIdsStr.append(QString("'%1'").arg(toSqlStr(id)));

		first = false;
	}

	// request
	//
	QString request = QString("SELECT * FROM get_latest_signals_by_appsignalids(%1, ARRAY[%2])")
		.arg(currentUser().userId()).arg(appSignalIdsStr);
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't get_latest_signals_by_appsignalids! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		Signal signal;

		getSignalData(q, signal);

		signalArray->append(signal);
	}

	return;

}


void DbWorker::getSignalData(QSqlQuery& q, Signal& s)
{
	// indexes of SignalData's fields
	//
	const int SD_APP_SIGNAL_ID = 0;
	const int SD_CUSTOM_APP_SIGNAL_ID = 1;
	const int SD_CAPTION = 2;
	const int SD_EQUIPMENT_ID = 3;
	const int SD_BUS_TYPE_ID = 4;
	const int SD_CHANNEL = 5;

	const int SD_SIGNAL_TYPE = 6;
	const int SD_IN_OUT_TYPE = 7;

	const int SD_DATA_SIZE = 8;
	const int SD_BYTE_ORDER = 9;

	const int SD_ANALOG_SIGNAL_FORMAT = 10;
	const int SD_UNIT = 11;

	const int SD_LOW_ADC = 12;
	const int SD_HIGH_ADC = 13;
	const int SD_LOW_ENGENEERING_UNITS = 14;
	const int SD_HIGH_ENGENEERING_UNITS = 15;
	const int SD_LOW_VALID_RANGE = 16;
	const int SD_HIGH_VALID_RANGE = 17;
	const int SD_FILTERING_TIME = 18;
	const int SD_SPREADTOLERANCE = 19;

	const int SD_ELECTRIC_LOW_LIMIT = 20;
	const int SD_ELECTRIC_HIGH_LIMIT = 21;
	const int SD_ELECTRIC_UNIT = 22;
	const int SD_SENSOR_TYPE = 23;
	const int SD_OUTPUT_MODE = 24;

	const int SD_ENABLE_TUNING = 25;
	const int SD_TUNING_DEFAULT_VALUE = 26;
	const int SD_TUNING_LOW_BOUND = 27;
	const int SD_TUNING_HIGH_BOUND = 28;

	const int SD_ACQUIRE = 29;
	const int SD_DECIMAL_PLACES = 30;
	const int SD_COARSE_APERTURE = 31;
	const int SD_FINE_APERTURE = 32;
	const int SD_ADAPTIVE_APERTURE = 33;

	const int SD_SIGNAL_ID = 34;
	const int SD_SIGNAL_GROUP_ID = 35;
	const int SD_SIGNAL_INSTANCE_ID = 36;
	const int SD_CHANGESET_ID = 37;
	const int SD_CHECKEDOUT = 38;
	const int SD_USER_ID = 39;
	const int SD_CREATED = 40;
	const int SD_DELETED = 41;
	const int SD_INSTANCE_CREATED = 42;
	const int SD_INSTANCE_ACTION = 43;

	// read fields
	//
	s.setAppSignalID(q.value(SD_APP_SIGNAL_ID).toString());
	s.setCustomAppSignalID(q.value(SD_CUSTOM_APP_SIGNAL_ID).toString());
	s.setCaption(q.value(SD_CAPTION).toString());
	s.setEquipmentID(q.value(SD_EQUIPMENT_ID).toString());
	s.setBusTypeID(q.value(SD_BUS_TYPE_ID).toString());
	s.setChannel(static_cast<E::Channel>(q.value(SD_CHANNEL).toInt()));

	s.setSignalType(static_cast<E::SignalType>(q.value(SD_SIGNAL_TYPE).toInt()));
	s.setInOutType(static_cast<E::SignalInOutType>(q.value(SD_IN_OUT_TYPE).toInt()));

	s.setDataSize(q.value(SD_DATA_SIZE).toInt());
	s.setByteOrder(static_cast<E::ByteOrder>(q.value(SD_BYTE_ORDER).toInt()));

	int f = q.value(SD_ANALOG_SIGNAL_FORMAT).toInt();

	if (f == TO_INT(E::DataFormat::UnsignedInt))
	{
		// Convert data format from E::DataFormat::UnsignedInt to E::AnalogAppSignalFormat::SignedInt32
		//
		f = TO_INT(E::AnalogAppSignalFormat::SignedInt32);
	}
	s.setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(f));
	s.setUnit(q.value(SD_UNIT).toString());

	s.setLowADC(q.value(SD_LOW_ADC).toInt());
	s.setHighADC(q.value(SD_HIGH_ADC).toInt());
	s.setLowEngeneeringUnits(q.value(SD_LOW_ENGENEERING_UNITS).toDouble());
	s.setHighEngeneeringUnits(q.value(SD_HIGH_ENGENEERING_UNITS).toDouble());
	s.setLowValidRange(q.value(SD_LOW_VALID_RANGE).toDouble());
	s.setHighValidRange(q.value(SD_HIGH_VALID_RANGE).toDouble());
	s.setFilteringTime(q.value(SD_FILTERING_TIME).toDouble());
	s.setSpreadTolerance(q.value(SD_SPREADTOLERANCE).toDouble());

	s.setElectricLowLimit(q.value(SD_ELECTRIC_LOW_LIMIT).toDouble());
	s.setElectricHighLimit(q.value(SD_ELECTRIC_HIGH_LIMIT).toDouble());
	s.setElectricUnit(static_cast<E::ElectricUnit>(q.value(SD_ELECTRIC_UNIT).toInt()));
	s.setSensorType(static_cast<E::SensorType>(q.value(SD_SENSOR_TYPE).toInt()));
	s.setOutputMode(static_cast<E::OutputMode>(q.value(SD_OUTPUT_MODE).toInt()));

	s.setEnableTuning(q.value(SD_ENABLE_TUNING).toBool());
	s.setTuningDefaultValue(q.value(SD_TUNING_DEFAULT_VALUE).toFloat());
	s.setTuningLowBound(q.value(SD_TUNING_LOW_BOUND).toFloat());
	s.setTuningHighBound(q.value(SD_TUNING_HIGH_BOUND).toFloat());

	s.setAcquire(q.value(SD_ACQUIRE).toBool());
	s.setDecimalPlaces(q.value(SD_DECIMAL_PLACES).toInt());
	s.setCoarseAperture(q.value(SD_COARSE_APERTURE).toDouble());
	s.setFineAperture(q.value(SD_FINE_APERTURE).toDouble());
	s.setAdaptiveAperture(q.value(SD_ADAPTIVE_APERTURE).toBool());

	s.setID(q.value(SD_SIGNAL_ID).toInt());
	s.setSignalGroupID(q.value(SD_SIGNAL_GROUP_ID).toInt());
	s.setSignalInstanceID(q.value(SD_SIGNAL_INSTANCE_ID).toInt());
	s.setChangesetID(q.value(SD_CHANGESET_ID).toInt());
	s.setCheckedOut(q.value(SD_CHECKEDOUT).toBool());
	s.setUserID(q.value(SD_USER_ID).toInt());
	s.setCreated(q.value(SD_CREATED).toDateTime());
	s.setDeleted(q.value(SD_DELETED).toBool());
	s.setInstanceCreated(q.value(SD_INSTANCE_CREATED).toDateTime());
	s.setInstanceAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(SD_INSTANCE_ACTION).toInt()));
}


QString DbWorker::getSignalDataStr(const Signal& s)
{
	QString str = QString(
			"('%1','%2','%3','%4','%5',%6,"
			"%7,%8,%9,%10,%11,'%12',"
			"%13,%14,%15,%16,%17,%18,%19,%20,"
			"%21,%22,%23,%24,%25,"
			"%26,%27,%28,%29,"
			"%30,%31,%32,%33,%34,"
			"%35,%36,%37,%38,%39,%40,'%41',%42,'%43',%44)").

	arg(toSqlStr(s.appSignalID())).
	arg(toSqlStr(s.customAppSignalID())).
	arg(toSqlStr(s.caption())).
	arg(toSqlStr(s.equipmentID())).
	arg(s.busTypeID()).
	arg(TO_INT(s.channel())).

	arg(TO_INT(s.signalType())).
	arg(TO_INT(s.inOutType())).
	arg(s.dataSize()).
	arg(TO_INT(s.byteOrder())).
	arg(TO_INT(s.analogSignalFormat())).
	arg(toSqlStr(s.unit())).

	arg(s.lowADC()).
	arg(s.highADC()).
	arg(s.lowEngeneeringUnits()).
	arg(s.highEngeneeringUnits()).
	arg(s.lowValidRange()).
	arg(s.highValidRange()).
	arg(s.filteringTime()).
	arg(s.spreadTolerance()).

	arg(s.electricLowLimit()).
	arg(s.electricHighLimit()).
	arg(TO_INT(s.electricUnit())).
	arg(TO_INT(s.sensorType())).
	arg(TO_INT(s.outputMode())).

	arg(toSqlBoolean(s.enableTuning())).
	arg(s.tuningDefaultValue()).
	arg(s.tuningLowBound()).
	arg(s.tuningHighBound()).

	arg(toSqlBoolean(s.acquire())).
	arg(s.decimalPlaces()).
	arg(s.coarseAperture()).
	arg(s.fineAperture()).
	arg(toSqlBoolean(s.adaptiveAperture())).

	arg(s.ID()).
	arg(s.signalGroupID()).
	arg(s.signalInstanceID()).
	arg(s.changesetID()).
	arg(toSqlBoolean(s.checkedOut())).
	arg(s.userID()).
	arg(s.created().toString(DATE_TIME_FORMAT_STR)).
	arg(toSqlBoolean(s.deleted())).
	arg(s.instanceCreated().toString(DATE_TIME_FORMAT_STR)).
	arg(s.instanceAction().toInt());

	return str;
}


void DbWorker::getObjectState(QSqlQuery& q, ObjectState& os)
{
	os.id = q.value("id").toInt();
	os.deleted = q.value("deleted").toBool();
	os.checkedOut = q.value("checkedout").toBool();
	os.action = q.value("action").toInt();
	os.userId = q.value("userid").toInt();
	os.errCode = q.value("errCode").toInt();
}



void DbWorker::slot_addSignal(E::SignalType signalType, QVector<Signal>* newSignal)
{
	AUTO_COMPLETE

	addSignal(signalType, newSignal);
}


bool DbWorker::addSignal(E::SignalType signalType, QVector<Signal>* newSignal)
{
	if (newSignal == nullptr)
	{
		assert(newSignal != nullptr);
		return false;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot add signals. Database connection is not opened."));
		return false;
	}

	// Log action
	//
	QString logMessage = QString("addSignal: SiganlCount %1, SignalIDs ")
						 .arg(newSignal->size());

	for (const Signal& s : *newSignal)
	{
		logMessage += s.appSignalID() + QLatin1String(" ");
	}
	addLogRecord(db, logMessage);

	// request
	//
	QString request = QString("SELECT * FROM add_signal(%1, %2, %3)")
		.arg(currentUser().userId()).arg(TO_INT(signalType)).arg(newSignal->count());
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't add new signal(s)! Error: ") +  q.lastError().text());
		return false;
	}

	int i = 0;

	int readed = 0;

	while(q.next() != false)
	{
		ObjectState os;

		getObjectState(q, os);

		int signalID =  os.id;

		Signal& signal = (*newSignal)[i];

		signal.setID(signalID);
		signal.setCreated(QDateTime::currentDateTime());
		signal.setInstanceCreated(QDateTime::currentDateTime());

		QString errMsg;
		ObjectState objectState;

		bool result = setSignalWorkcopy(db, signal, objectState, errMsg);

		if (result == false)
		{
			emitError(db, errMsg);
			return false;
		}

		QString request2 = QString("SELECT * FROM get_latest_signal(%1, %2)")
			.arg(currentUser().userId()).arg(signalID);

		QSqlQuery q3(db);

		result = q3.exec(request2);

		if (result == false)
		{
			emitError(db, tr("Can't get latest signal! Error: ") +  q3.lastError().text());
			return false;
		}

		while(q3.next() != false)
		{
			getSignalData(q3, (*newSignal)[i]);
			readed++;
		}

		i++;
	}

	assert(i == readed);

	return true;
}


bool DbWorker::setSignalWorkcopy(QSqlDatabase& db, const Signal& s, ObjectState& objectState, QString& errMsg)
{
	errMsg.clear();

	QString sds = getSignalDataStr(s);

	QString request = QString("SELECT * FROM set_signal_workcopy(%1, %2)")
		.arg(currentUser().userId()).arg(sds);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		QString err = q.lastError().text();

		if (err.contains("55011"))
		{
			errMsg = QString(tr("Application signal with AppSignalID '%1' already exists!")).arg(s.appSignalID());
			objectState.errCode = ERR_SIGNAL_EXISTS;
		}
		else
		{
			if (err.contains("55022"))
			{
				errMsg = QString(tr("Application signal with CustomAppSignalID '%1' already exists!")).arg(s.customAppSignalID());
				objectState.errCode = ERR_SIGNAL_EXISTS;
			}
			else
			{
				errMsg = err;
			}
		}
		return false;
	}

	if (q.next() == false)
	{
		return false;
	}

	getObjectState(q, objectState);

	return true;
}

void DbWorker::slot_checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return;
	}

	if (objectStates == nullptr)
	{
		assert(objectStates != nullptr);
		return;
	}

	objectStates->clear();

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot checkout signals. Database connection is not opened."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_checkoutSignals: SignalCount %1, SignalIDs ")
						 .arg(signalIDs->size());

	for (int id : *signalIDs)
	{
		logMessage += QString("%1 ").arg(id);
	}
	addLogRecord(db, logMessage);

	// request
	//
	QString request = QString("SELECT * FROM checkout_signals(%1,ARRAY[").arg(currentUser().userId());

	int count = signalIDs->count();

	for(int i = 0; i < count; i++)
	{
		if (i < count - 1)
		{
			request += QString("%1,").arg((*signalIDs)[i]);
		}
		else
		{
			request += QString("%1])").arg((*signalIDs)[i]);
		}
	}

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't checkout signals! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		ObjectState os;

		getObjectState(q, os);

		objectStates->append(os);
	}
}


void DbWorker::slot_setSignalWorkcopy(Signal* signal, ObjectState *objectState)
{
	AUTO_COMPLETE

	if (signal == nullptr)
	{
		assert(signal != nullptr);
		return;
	}

	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot set signal workcopy. Database connection is not opened."));
		return;
	}

	// request
	//
	signal->setCreated(QDateTime::currentDateTime());
	signal->setInstanceCreated(QDateTime::currentDateTime());

	QString errMsg;

	bool result = setSignalWorkcopy(db, *signal, *objectState, errMsg);

	if (result == false)
	{
		emitError(db, errMsg);
		return;
	}

	QString request2 = QString("SELECT * FROM get_latest_signal(%1, %2)")
		.arg(currentUser().userId()).arg(signal->ID());

	QSqlQuery q2(db);

	result = q2.exec(request2);

	if (result == false)
	{
		emitError(db, tr("Can't get latest signal! Error: ") +  q2.lastError().text());
		return;
	}

	if (q2.next() != false)
	{
		getSignalData(q2, *signal);
	}
	else
	{
		emitError(db, tr("Can't get latest signal! No data returned!"));
	}
}


void DbWorker::slot_deleteSignal(int signalID, ObjectState* objectState)
{
	AUTO_COMPLETE

	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot delete signal. Database connection is not opened."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_deleteSignal: SignalID %1").arg(signalID);

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM delete_signal(%1, %2)")
		.arg(currentUser().userId()).arg(signalID);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't delete signal! Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() != false)
	{
		getObjectState(q, *objectState);
	}
	else
	{
		emitError(db, tr("Can't delete signal! No data returned!"));
	}
}


void DbWorker::slot_undoSignalChanges(int signalID, ObjectState* objectState)
{
	AUTO_COMPLETE

	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot undo signal changes. Database connection is not opened."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_undoSignalChanges: SignalID %1").arg(signalID);

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM undo_signal_changes(%1, %2)")
		.arg(currentUser().userId()).arg(signalID);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't undo signal changes! Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() != false)
	{
		getObjectState(q, *objectState);
	}
	else
	{
		emitError(db, tr("Can't undo signal changes! No data returned!"));
	}
}


void DbWorker::slot_checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState> *objectState)
{
	AUTO_COMPLETE

	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return;
	}

	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return;
	}

	objectState->clear();

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot checkin signals. Database connection is not opened."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_checkinSignals: Comment '%1', SiganlCount %2, SignalIDs ")
						 .arg(comment)
						 .arg(signalIDs->size());

	for (int id : *signalIDs)
	{
		logMessage += QString("%1 ").arg(id);
	}
	addLogRecord(db, logMessage);

	// --
	//
	int count = signalIDs->count();

	QString request = QString("SELECT * FROM checkin_signals(%1, ARRAY[")
		.arg(currentUser().userId());


	for(int i=0; i < count; i++)
	{
		if (i < count-1)
		{
			request += QString("%1,").arg(signalIDs->at(i));
		}
		else
		{
			request += QString("%1],").arg(signalIDs->at(i));
		}
	}

	request += QString("'%1')").arg(comment);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, tr("Can't checkin signals! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next())
	{
		ObjectState os;

		getObjectState(q, os);

		objectState->append(os);
	}
}


void DbWorker::slot_autoAddSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals,
								   std::vector<Signal>* addedSignals)
{
	AUTO_COMPLETE

	if (deviceSignals == nullptr)
	{
		assert(deviceSignals != nullptr);
		return;
	}

	if (addedSignals != nullptr)
	{
		addedSignals->clear();
	}

	int signalCount = int(deviceSignals->size());

	for(int i = 0; i < signalCount; i++)
	{
		if ((i % 5) == 0)
		{
			m_progress->setValue((i * 100) / signalCount);
		}

		const Hardware::DeviceSignal* deviceSignal = deviceSignals->at(i);

		if (deviceSignal == nullptr)
		{
			assert(false);
			continue;
		}

		if (deviceSignal->isInputSignal() || deviceSignal->isOutputSignal() || deviceSignal->isValiditySignal())
		{
			if (isSignalWithEquipmentIDExists(deviceSignal->equipmentIdTemplate()) == false)
			{
				Signal signal(*deviceSignal);

				QVector<Signal> newSignals;

				newSignals.append(signal);

				bool result = addSignal(signal.signalType(), &newSignals);

				if (result == true && addedSignals != nullptr)
				{
					addedSignals->push_back(newSignals[0]);
				}
			}
		}
	}

	m_progress->setValue(100);
}


void DbWorker::slot_autoDeleteSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals)
{
	AUTO_COMPLETE

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot delete signal. Database connection is not opened."));
		return;
	}

	//--
	//
	ObjectState os;

	for(Hardware::DeviceSignal* deviceSignal : *deviceSignals)
	{
		QString request = QString("SELECT * FROM delete_signal_by_equipmentid(%1, '%2')")
			.arg(currentUser().userId()).arg(toSqlStr(deviceSignal->equipmentIdTemplate()));

		QSqlQuery q(db);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(db, tr("Can't delete signal! Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() != false)
		{
			getObjectState(q, os);
		}
		else
		{
			emitError(db, tr("Can't delete signal! No data returned!"));
		}
	}
}


bool DbWorker::isSignalWithEquipmentIDExists(const QString& equipmentID)
{
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not opened."));
		return false;
	}

	QString request = QString("SELECT * FROM is_signal_with_equipmentid_exists(%1, '%2')")
					  .arg(currentUser().userId())
					  .arg(toSqlStr(equipmentID));

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return false;
	}

	result = q.next();

	if (result == false)
	{
		return false;
	}

	result = q.value(0).toBool();

	return result;
}


void DbWorker::slot_getSignalsIDsWithAppSignalID(QString appSignalID, QVector<int>* signalIDs)
{
	AUTO_COMPLETE

	if (signalIDs == nullptr)
	{
		assert(false);
		return;
	}

	signalIDs->clear();

	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get ignal IDs with AppSignalID. Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM get_signal_ids_with_appsignalid(%1, '%2')")
					  .arg(currentUser().userId())
					  .arg(toSqlStr(appSignalID));

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	while(q.next() == true)
	{
		signalIDs->append(q.value(0).toInt());
	}
}


void DbWorker::slot_getSignalsIDsWithCustomAppSignalID(QString customAppSignalID, QVector<int>* signalIDs)
{
	AUTO_COMPLETE

	if (signalIDs == nullptr)
	{
		assert(false);
		return;
	}

	signalIDs->clear();

	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get ignal IDs with CustomAppSignalID. Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM get_signal_ids_with_customappsignalid(%1, '%2')")
					  .arg(currentUser().userId())
					  .arg(toSqlStr(customAppSignalID));

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	while(q.next() == true)
	{
		signalIDs->append(q.value(0).toInt());
	}
}


void DbWorker::slot_getSignalsIDsWithEquipmentID(QString equipmentID, QVector<int>* signalIDs)
{
	AUTO_COMPLETE

	if (signalIDs == nullptr)
	{
		assert(false);
		return;
	}

	signalIDs->clear();

	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get ignal IDs with EquipmentID. Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM get_signal_ids_with_equipmentid(%1, '%2')")
					  .arg(currentUser().userId())
					  .arg(toSqlStr(equipmentID));

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	while(q.next() == true)
	{
		signalIDs->append(q.value(0).toInt());
	}
}


void DbWorker::slot_getSignalHistory(int signalID, std::vector<DbChangeset>* out)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (out == nullptr)
	{
		assert(false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// Request for history
	//
	QString request = QString("SELECT * FROM get_signal_history(%1, %2)")
			.arg(currentUser().userId())
			.arg(signalID);

	QSqlQuery q(db);
	q.setForwardOnly(true);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(db, tr("Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbChangeset ci;
		db_dbChangeset(q, &ci);

		out->push_back(ci);
	}

	return;
}


void DbWorker::slot_getSpecificSignals(const std::vector<int>* signalIDs, int changesetId, std::vector<Signal>* out)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (signalIDs == nullptr ||
		signalIDs->empty() == true ||
		out == nullptr)
	{
		assert(signalIDs != nullptr);
		assert(signalIDs->empty() != true);
		assert(out != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(db, tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Iterate through signalIDs
	//
	for (unsigned int i = 0; i < signalIDs->size(); i++)
	{
		int signalID = signalIDs->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM get_specific_signal(%1, %2, %3);")
				.arg(currentUser().userId())
				.arg(signalID)
				.arg(changesetId);

		QSqlQuery q(db);
		q.setForwardOnly(true);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(db, tr("Can't get signal. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(db, tr("Can't find workcopy for signalID: %1. Is signal CheckedOut?").arg(signalID));
			return;
		}

		Signal s;

		getSignalData(q, s);

		out->push_back(s);
	}

	return;
}


// Build management
//

void DbWorker::slot_buildStart(QString workstation, bool release, int changeset, int* buildID)
{
	AUTO_COMPLETE

	if (buildID == nullptr)
	{
		assert(buildID != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not opened."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_buildStart: Release %1, Changeset %2")
						 .arg(release)
						 .arg(changeset);

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM build_start(%1, '%2', cast(%3 as boolean), %4)")
					  .arg(currentUser().userId())
					  .arg(DbWorker::toSqlStr(workstation))
					  .arg(release)
					  .arg(changeset);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	q.next();
	*buildID = q.value(0).toInt();

	return;
}


void DbWorker::slot_buildFinish(int buildID, int errors, int warnings, QString buildLog)
{
	AUTO_COMPLETE

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not opened."));
		return;
	}

	// Log action
	//
	QString logMessage = QString("slot_buildFinish: BuildID %1, Errors %2, Warnings %3")
						 .arg(buildID)
						 .arg(errors)
						 .arg(warnings);

	addLogRecord(db, logMessage);

	// --
	//
	QString request = QString("SELECT * FROM build_finish(%1, %2, %3, '%4')")
					  .arg(buildID)
					  .arg(errors)
					  .arg(warnings)
					  .arg(toSqlStr(buildLog));

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	return;
}


void DbWorker::slot_isAnyCheckedOut(int* checkedOutCount)
{
	AUTO_COMPLETE

	if (checkedOutCount == nullptr)
	{
		assert(checkedOutCount != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM api.is_any_checked_out('%1');").arg(sessionKey());

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	q.next();
	*checkedOutCount = q.value(0).toInt();

	return;
}

void DbWorker::slot_lastChangesetId(int* lastChangesetId)
{
	AUTO_COMPLETE

	if (lastChangesetId == nullptr)
	{
		assert(lastChangesetId != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not opened."));
		return;
	}

	QString request = "SELECT * FROM get_last_changeset();";

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	q.next();
	*lastChangesetId = q.value(0).toInt();

	return;
}

void DbWorker::slot_nextCounterValue(int* counter)
{
	AUTO_COMPLETE

	if (counter == nullptr)
	{
		assert(counter != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(db, tr("Database connection is not opened."));
		return;
	}

	QString request = "SELECT * FROM nextval('global_counter');";

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(db, q.lastError().text());
		return;
	}

	q.next();
	*counter = q.value(0).toInt();

	return;
}

bool DbWorker::addLogRecord(QSqlDatabase db, QString text)
{
	if (db.isOpen() == false)
	{
		return false;
	}

	int userId = currentUser().userId();
	QString host = QHostInfo::localHostName();
	quint64 processId = QCoreApplication::applicationPid();
	QString sqlText = toSqlStr(text);

	QString request = QString("SELECT * FROM add_log_record(%1, '%2', %3, '%4');")
					  .arg(userId < 1 ? "NULL" : QString::number(userId))		// UserID starts from 1 (is Administartor)
					  .arg(host)
					  .arg(processId)
					  .arg(sqlText);

	QSqlQuery query(db);
	bool result = query.exec(request);

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << query.lastError();
		emitError(db, query.lastError(), false);
		return false;
	}

	return true;
}

bool DbWorker::db_logIn(QSqlDatabase db, QString username, QString password, QString* errorMessage)
{
	if (db.isOpen() == false)
	{
		return false;
	}

	int projectVersion = db_getProjectVersion(db);
	if (projectVersion < 124)
	{
		// This function does not exists yet
		//
		assert(projectVersion >= 124);
		return false;
	}

	QSqlQuery query(db);

	query.prepare("SELECT * FROM user_api.log_in(:username, :password);");
	query.bindValue(":username", username);
	query.bindValue(":password", password);

	bool result = query.exec();
	if (result == false)
	{
		if (errorMessage != nullptr)
		{
			*errorMessage = query.lastError().text();
		}
		addLogRecord(db, QString("log_in error: ").arg(query.lastError().text()));
		return false;
	}

	if (query.next() == false || query.isNull(0) == true)
	{
		return false;
	}

	QString sessionKey = query.value(0).toString();
	if (sessionKey.isEmpty() == true)
	{
		if (errorMessage != nullptr)
		{
			*errorMessage = "LogIn Error, wrong session key";
		}
		addLogRecord(db, QString("log_in error: LogIn Error, wrong session key"));
		return false;
	}

	setSessionKey(sessionKey);

	addLogRecord(db, QString("Username %1 is logged in").arg(username));

	return true;
}

bool DbWorker::db_logOut(QSqlDatabase db)
{
	if (db.isOpen() == false)
	{
		return false;
	}

	int projectVersion = db_getProjectVersion(db);
	if (projectVersion < 124)
	{
		// This function does not exists yet
		//
		assert(projectVersion >= 124);
		return false;
	}

	QSqlQuery query(db);

	bool result = query.exec("SELECT * FROM user_api.log_out();");
	if (result == false)
	{
		return false;
	}

	addLogRecord(db, QString("Username %1 is logged out").arg(currentUser().username()));

	setSessionKey(QString());

	return true;
}

bool DbWorker::db_getCurrentUserId(QSqlDatabase db, int* userId)
{
	if (userId == nullptr)
	{
		assert(userId != nullptr);
		return false;
	}

	QSqlQuery query(db);

	bool result = query.exec(QString("SELECT * FROM user_api.current_user_id('%1')")
							 .arg(sessionKey()));

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << query.lastError();
		return false;
	}

	if (query.size() != 1)
	{
		qDebug() << Q_FUNC_INFO << " " << query.size();
		return false;
	}

	if (query.next() == false)
	{
		qDebug() << Q_FUNC_INFO << " " << "query.next() == false";
		return false;
	}

	*userId = query.value(0).toInt();

	return true;
}

bool DbWorker::db_getUserData(QSqlDatabase db, int userId, DbUser* user)
{
	if (user == nullptr)
	{
		assert(user != nullptr);
		return false;
	}

	QSqlQuery query(db);

	bool result = query.exec(QString("SELECT * FROM user_api.get_user_data('%1', %2)")
							 .arg(sessionKey())
							 .arg(userId));

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << query.lastError();
		return false;
	}

	if (query.size() != 1)
	{
		qDebug() << Q_FUNC_INFO << " " << query.size();
		return false;
	}

	if (query.next() == false)
	{
		qDebug() << Q_FUNC_INFO << " " << "query.next() == false";
		return false;
	}

	int returnedUserId = query.value(0).toInt();

	if (userId != returnedUserId)
	{
		emitError(QSqlDatabase(), QString("user_api.get_user_data error, the data was asked for UserID %1 but returned for UserID %2")
				  .arg(userId)
				  .arg(returnedUserId));
		return false;
	}

	user->setUserId(returnedUserId);
	user->setUsername(query.value(1).toString());
	user->setFirstName(query.value(2).toString());
	user->setLastName(query.value(3).toString());
	user->setPassword(QString());
	user->setAdministrator(query.value(4).toBool());
	user->setReadonly(query.value(5).toBool());
	user->setDisabled(query.value(6).toBool());

	return true;
}

bool DbWorker::db_checkUserPassword(QSqlDatabase db, QString username, QString password)
{
	if (db.isOpen() == false)
	{
		return false;
	}

	int projectVersion = db_getProjectVersion(db);

	if (projectVersion == -1)
	{
		return false;
	}

	if (projectVersion < 4)
	{
		// Check by query
		//
		QSqlQuery query(db);

		query.prepare("SELECT UserID FROM Users WHERE Username ILIKE :username AND Password = :password");
		query.bindValue(":username", username);
		query.bindValue(":password", password);

		bool result = query.exec();

		if (result == false)
		{
			return false;
		}

		if (query.next() == false)
		{
			return false;
		}

		return true;
	}
	else

	if (projectVersion < 124)
	{
		// Check by store function
		//
		QSqlQuery query(db);

		query.prepare("SELECT * FROM public.check_user_password(:username, :password);");
		query.bindValue(":username", username);
		query.bindValue(":password", password);

		bool result = query.exec();

		if (result == false)
		{
			return false;
		}

		if (query.next() == false || query.isNull(0) == true)
		{
			return false;
		}

		bool passwordCorrect = query.value(0).toBool();
		return passwordCorrect;
	}
	else
	{
		// Check by store function
		//
		QSqlQuery query(db);

		query.prepare("SELECT * FROM user_api.check_user_password(:username, :password);");
		query.bindValue(":username", username);
		query.bindValue(":password", password);

		bool result = query.exec();
		if (result == false)
		{
			return false;
		}

		if (query.next() == false ||
			query.isNull(0) == true)
		{
			return false;
		}

		bool passwordCorrect = query.value(0).toBool();
		return passwordCorrect;
	}

	return true;
}

int DbWorker::db_getProjectVersion(QSqlDatabase db)
{
	if (db.isOpen() == false)
	{
		return -1;
	}

	QString createVersionTableSql = QString("SELECT max(VersionNo) FROM Version;");

	QSqlQuery versionQuery(db);
	bool result = versionQuery.exec(createVersionTableSql);

	if (result == false)
	{
		emitError(QSqlDatabase(), versionQuery.lastError());
		versionQuery.clear();
		return -1;
	}

	if (versionQuery.next())
	{
		int projectVersion = versionQuery.value(0).toInt();
		return projectVersion;
	}
	else
	{
		return -1;
	}
}

bool DbWorker::db_updateFileState(const QSqlQuery& q, DbFileInfo* fileInfo, bool checkFileId) const
{
	//qDebug() << Q_FUNC_INFO << " FileId = " << q.value(0).toInt();
	//qDebug() << Q_FUNC_INFO << " Deleted = " << q.value(1).toBool();
	//qDebug() << Q_FUNC_INFO << " CheckedOut = " << q.value(2).toBool();
	//qDebug() << Q_FUNC_INFO << " Action = " << static_cast<int>(q.value(3).toInt());

	assert(fileInfo);

	int fileId = q.value(0).toInt();
	bool deleted  = q.value(1).toBool();
	VcsState::VcsStateType state = q.value(2).toBool() ? VcsState::CheckedOut : VcsState::CheckedIn;
	VcsItemAction::VcsItemActionType action = static_cast<VcsItemAction::VcsItemActionType>(q.value(3).toInt());
	int userId = q.value(4).toInt();
	//int errcode = q.value(5).toInt();

	if (checkFileId == true && fileInfo->fileId() != fileId)
	{
		assert(fileInfo->fileId() == fileId);
		return false;
	}

	fileInfo->setFileId(fileId);
	fileInfo->setDeleted(deleted);
	fileInfo->setState(state);
	fileInfo->setAction(action);
	fileInfo->setUserId(userId);

	return true;
}

bool DbWorker::db_updateFile(const QSqlQuery& q, DbFile* file) const
{
static thread_local bool columnIndexesInitialized = false;
static thread_local int fileIdNo = -1;
static thread_local int deletedNo = -1;
static thread_local int nameNo = -1;
static thread_local int parentIdNo = -1;
static thread_local int changesetIdNo = -1;
static thread_local int createdNo = -1;
static thread_local int checkOutTimeNo = -1;
static thread_local int checkedOutNo = -1;
static thread_local int actionNo = -1;
static thread_local int userIdNo = -1;
static thread_local int detailsNo = -1;
static thread_local int dataNo = -1;

	QSqlRecord record = q.record();

    if (columnIndexesInitialized == false)
	{
		columnIndexesInitialized = true;

		fileIdNo = record.indexOf("FileID");
		deletedNo = record.indexOf("Deleted");
		nameNo = record.indexOf("Name");
		parentIdNo = record.indexOf("ParentID");
		changesetIdNo = record.indexOf("ChangesetID");
		createdNo = record.indexOf("Created");
		checkOutTimeNo = record.indexOf("CheckOutTime");
		checkedOutNo = record.indexOf("CheckedOut");
		actionNo = record.indexOf("Action");
		userIdNo = record.indexOf("UserID");
		detailsNo = record.indexOf("Details");
		dataNo = record.indexOf("Data");

		if (fileIdNo == -1 ||
		    deletedNo == -1 ||
		    nameNo == -1 ||
		    parentIdNo == -1 ||
		    changesetIdNo == -1 ||
		    createdNo == -1 ||
		    checkOutTimeNo == -1 ||
		    checkedOutNo == -1 ||
		    actionNo == -1 ||
		    userIdNo == -1 ||
		    detailsNo == -1 ||
		    dataNo == -1)
		{
			assert(false);
			return false;
		}
	}

	file->setFileId(record.value(fileIdNo).toInt());
	file->setDeleted(record.value(deletedNo).toBool());
	file->setFileName(record.value(nameNo).toString());
	file->setParentId(record.value(parentIdNo).toInt());
	file->setChangeset(record.value(changesetIdNo).toInt());
	file->setCreated(record.value(createdNo).toDateTime());
	file->setLastCheckIn(record.value(checkOutTimeNo).toDateTime());		// setLastCheckIn BUT TIME IS CheckOutTime

	bool checkedOut = record.value(checkedOutNo).toBool();
	file->setState(checkedOut ? VcsState::CheckedOut : VcsState::CheckedIn);

	int action = record.value(actionNo).toInt();
	file->setAction(static_cast<VcsItemAction::VcsItemActionType>(action));

	file->setUserId(record.value(userIdNo).toInt());
	file->setDetails(record.value(detailsNo).toString());
	QByteArray data = record.value(dataNo).toByteArray();
	file->swapData(data);

	return true;
}

bool DbWorker::db_dbFileInfo(const QSqlQuery& q, DbFileInfo* fileInfo)
{
	// Database custom type DbFileInfo
	//
	//	CREATE TYPE dbfileinfo AS
	//	   (fileid integer,
	//	    deleted boolean,
	//	    name text,
	//	    parentid integer,
	//	    changesetid integer,
	//	    created timestamp with time zone,
	//	    size integer,
	//	    checkedout boolean,
	//	    checkouttime timestamp with time zone,
	//	    userid integer,
	//	    action integer,
	//	    details text);
	//	ALTER TYPE dbfileinfo
	//	  OWNER TO postgres;

	assert(fileInfo);

	fileInfo->setFileId(q.value(0).toInt());
	fileInfo->setDeleted(q.value(1).toBool());
	fileInfo->setFileName(q.value(2).toString());
	fileInfo->setParentId(q.value(3).toInt());
	fileInfo->setChangeset(q.value(4).toInt());
	fileInfo->setCreated(q.value(5).toDateTime());
	fileInfo->setSize(q.value(6).toInt());
	fileInfo->setState(q.value(7).toBool() ? VcsState::CheckedOut : VcsState::CheckedIn);
	//fileInfo->setCheckoutTime(q.value(8).toString());
	fileInfo->setUserId(q.value(9).toInt());
	fileInfo->setAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(10).toInt()));
	fileInfo->setDetails(q.value(11).toString());

	return true;
}

bool DbWorker::db_dbChangeset(const QSqlQuery& q, DbChangeset* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	out->setChangeset(q.value(0).toInt());
	out->setUserId(q.value(1).toInt());
	out->setUsername(q.value(2).toString());
	out->setDate(q.value(3).toDateTime());
	out->setComment(q.value(4).toString());
	out->setAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(5).toInt()));

	return true;
}

bool DbWorker::db_dbChangesetObject(const QSqlQuery& q, DbChangesetDetails* destination)
{
	if (destination == nullptr)
	{
		assert(destination);
		return false;
	}

	destination->setChangeset(q.value(0).toInt());
	destination->setUserId(q.value(1).toInt());
	destination->setUsername(q.value(2).toString());
	destination->setDate(q.value(3).toDateTime());
	destination->setComment(q.value(4).toString());
	destination->setAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(5).toInt()));

	DbChangesetObject csObject;

	csObject.setType(static_cast<DbChangesetObject::Type>(q.value(6 + 0).toInt()));
	csObject.setId(q.value(6 + 1).toInt());
	csObject.setName(q.value(6 + 2).toString());
	csObject.setCaption(q.value(6 + 3).toString());
	csObject.setAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(6 + 4).toInt()));
	csObject.setParent(q.value(6 + 5).toString());

	destination->addObject(csObject);

	return true;
}


const QString& DbWorker::host() const
{
	QMutexLocker locker(&m_mutex);
	return m_host;
}

void DbWorker::setHost(const QString& host)
{
	QMutexLocker locker(&m_mutex);
	m_host = host;
}

int DbWorker::port() const
{
	QMutexLocker locker(&m_mutex);
	return m_port;
}

void DbWorker::setPort(int port)
{
	QMutexLocker locker(&m_mutex);
	m_port = port;
}

const QString& DbWorker::serverUsername() const
{
	QMutexLocker locker(&m_mutex);
	return m_serverUsername;
}

void DbWorker::setServerUsername(const QString& username)
{
	QMutexLocker locker(&m_mutex);
	m_serverUsername = username;
}

const QString& DbWorker::serverPassword() const
{
	QMutexLocker locker(&m_mutex);
	return m_serverPassword;
}

void DbWorker::setServerPassword(const QString& password)
{
	QMutexLocker locker(&m_mutex);
	m_serverPassword = password;
}

DbUser DbWorker::currentUser() const
{
	QMutexLocker locker(&m_mutex);
	return m_currentUser;
}

void DbWorker::setCurrentUser(const DbUser& user)
{
	QMutexLocker locker(&m_mutex);
	m_currentUser = user;
}

DbProject DbWorker::currentProject() const
{
	QMutexLocker locker(&m_mutex);
	return m_currentProject;
}

void DbWorker::setCurrentProject(const DbProject& project)
{
	QMutexLocker locker(&m_mutex);
	m_currentProject = project;
}

const QString& DbWorker::sessionKey() const
{
	return m_sessionKey;
}

void DbWorker::setSessionKey(QString value)
{
	m_sessionKey = value;
}
