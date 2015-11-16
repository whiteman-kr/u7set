#include <functional>
#include <QDateTime>
#include <QFile>
#include <QSqlDriver>
#include <QSqlError>
#include <QDebug>

#include "../include/DbWorker.h"
#include "../include/DeviceObject.h"
#include "../include/DbProgress.h"


// Upgrade database
//
const UpgradeItem DbWorker::upgradeItems[] =
{
	{"Create project", ":/DatabaseUpgrade/Upgrade0001.sql"},
	{"Upgrade to version 2", ":/DatabaseUpgrade/Upgrade0002.sql"},
	{"Upgrade to version 3", ":/DatabaseUpgrade/Upgrade0003.sql"},
	{"Upgrade to version 4", ":/DatabaseUpgrade/Upgrade0004.sql"},
	{"Upgrade to version 5", ":/DatabaseUpgrade/Upgrade0005.sql"},
	{"Upgrade to version 6", ":/DatabaseUpgrade/Upgrade0006.sql"},
	{"Upgrade to version 7", ":/DatabaseUpgrade/Upgrade0007.sql"},
	{"Upgrade to version 8", ":/DatabaseUpgrade/Upgrade0008.sql"},
	{"Upgrade to version 9", ":/DatabaseUpgrade/Upgrade0009.sql"},
	{"Upgrade to version 10", ":/DatabaseUpgrade/Upgrade0010.sql"},
	{"Upgrade to version 11", ":/DatabaseUpgrade/Upgrade0011.sql"},
	{"Upgrade to version 12", ":/DatabaseUpgrade/Upgrade0012.sql"},
	{"Upgrade to version 13", ":/DatabaseUpgrade/Upgrade0013.sql"},
	{"Upgrade to version 14", ":/DatabaseUpgrade/Upgrade0014.sql"},
	{"Upgrade to version 15", ":/DatabaseUpgrade/Upgrade0015.sql"},
	{"Upgrade to version 16", ":/DatabaseUpgrade/Upgrade0016.sql"},
	{"Upgrade to version 17", ":/DatabaseUpgrade/Upgrade0017.sql"},
	{"Upgrade to version 18", ":/DatabaseUpgrade/Upgrade0018.sql"},
	{"Upgrade to version 19", ":/DatabaseUpgrade/Upgrade0019.sql"},
	{"Upgrade to version 20", ":/DatabaseUpgrade/Upgrade0020.sql"},
	{"Upgrade to version 21", ":/DatabaseUpgrade/Upgrade0021.sql"},
	{"Upgrade to version 22", ":/DatabaseUpgrade/Upgrade0022.sql"},
	{"Upgrade to version 23", ":/DatabaseUpgrade/Upgrade0023.sql"},
	{"Upgrade to version 24", ":/DatabaseUpgrade/Upgrade0024.sql"},
	{"Upgrade to version 25", ":/DatabaseUpgrade/Upgrade0025.sql"},
	{"Upgrade to version 26", ":/DatabaseUpgrade/Upgrade0026.sql"},
	{"Upgrade to version 27", ":/DatabaseUpgrade/Upgrade0027.sql"},
	{"Upgrade to version 28", ":/DatabaseUpgrade/Upgrade0028.sql"},
	{"Upgrade to version 29", ":/DatabaseUpgrade/Upgrade0029.sql"},
	{"Upgrade to version 30", ":/DatabaseUpgrade/Upgrade0030.sql"},
	{"Upgrade to version 31", ":/DatabaseUpgrade/Upgrade0031.sql"},
	{"Upgrade to version 32", ":/DatabaseUpgrade/Upgrade0032.sql"},
	{"Upgrade to version 33", ":/DatabaseUpgrade/Upgrade0033.sql"},
	{"Upgrade to version 34", ":/DatabaseUpgrade/Upgrade0034.sql"},
	{"Upgrade to version 35", ":/DatabaseUpgrade/Upgrade0035.sql"},
	{"Upgrade to version 36", ":/DatabaseUpgrade/Upgrade0036.sql"},
	{"Upgrade to version 37", ":/DatabaseUpgrade/Upgrade0037.sql"},
	{"Upgrade to version 38", ":/DatabaseUpgrade/Upgrade0038.sql"},
	{"Upgrade to version 39", ":/DatabaseUpgrade/Upgrade0039.sql"},
	{"Upgrade to version 40", ":/DatabaseUpgrade/Upgrade0040.sql"},
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

void DbWorker::emitError(const QSqlError& err)
{
	emitError(err.text());
}

void DbWorker::emitError(const QString& err)
{
	qDebug() << err;
	m_progress->setErrorMessage(err);
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

int DbWorker::wvsFileId() const
{
	QMutexLocker m(&m_mutex);
	return m_wvsFileId;
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

std::vector<DbFileInfo> DbWorker::systemFiles() const
{
	QMutexLocker m(&m_mutex);
	std::vector<DbFileInfo> copy(m_systemFiles);
	return copy;
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
			emitError(db.lastError());
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
			emitError(db.lastError());
			return;
		}

		ok = checkDatabaseFeatures(db);
		if (ok == false)
		{
			emitError(tr("Database driver doesn't have required features."));
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
			emitError(query.lastError());
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
				emitError(projectDb.lastError());
				continue;
			}

			// Get project version
			//

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

			versionQuery.clear();
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
			emitError(db.lastError());
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
			emitError(db.lastError());
			return;
		}

		ok = checkDatabaseFeatures(db);
		if (ok == false)
		{
			emitError(tr("Database driver doesn't have required features."));
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
			emitError(query.lastError());
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
			emitError(newDatabase.lastError());
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
			emitError(newDbQuery.lastError());
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
			emitError(newDbQuery.lastError());
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
			emitError(newDbQuery.lastError());
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
			emitError(newDbQuery.lastError());
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
			emitError(newDbQuery.lastError());
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
		emitError(tr("OpenProject error, another project is opened. To open a new project, please close the current project."));
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
		emitError(db.lastError());
		return;
	}

	// Check username and password
	//
	QSqlQuery query(db);
	result = query.exec(QString("SELECT get_user_id('%1', '%2');").arg(username).arg(password));

	if (result == false)
	{
		emitError(query.lastError());

		query.clear();
		db.close();
		return;
	}

	if (query.next() == false)
	{
		emitError(tr("Internal error. Can't check username and password."));

		query.clear();
		db.close();
		return;
	}

	int userId = query.value(0).toInt();

	if (userId <= 0)
	{
		emitError(tr("Can't open project. Wrong username or password."));

		query.clear();
		db.close();
		return;
	}

	// Set user data
	//
	DbUser user;
	result = db_getUserData(db, userId, &user);

	if (result == false)
	{
		emitError(tr("Can't read user data ") + db.lastError().text());

		query.clear();
		db.close();
		return;
	}

	if (user.isDisabled() == true)
	{
		emitError(tr("User %1 is not allowed to open the project. User was disabled by Administrator.").arg(username));

		query.clear();
		db.close();
		return;
	}

	setCurrentUser(user);

	// Set project data
	//
	DbProject project;

	project.setDatabaseName(databaseName);
	project.setProjectName(projectName);

	setCurrentProject(project);

	// Set System Folders File ID
	//
	std::vector<DbFileInfo> systemFiles;

	getFileList_worker(&systemFiles, rootFileId(), "%", true);

	m_mutex.lock();
	m_afblFileId = -1;
	m_alFileId = -1;
	m_hcFileId = -1;
	m_hpFileId = -1;
	m_wvsFileId = -1;
	m_dvsFileId = -1;
	m_mcFileId = -1;
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
		if (fi.fileName() == AfblFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_afblFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == AlFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_alFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == HcFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_hcFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == HpFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_hpFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == WvsFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_wvsFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == DvsFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_dvsFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}

		if (fi.fileName() == McFileName)
		{
			QMutexLocker locker(&m_mutex);
			m_mcFileId = fi.fileId();
			m_systemFiles.push_back(fi);
			continue;
		}
	}


	m_mutex.lock();
	result = m_afblFileId != -1;
	result &= m_alFileId != -1;
	result &= m_hcFileId != -1;
	result &= m_hpFileId != -1;
	result &= m_wvsFileId != -1;
	result &= m_dvsFileId != -1;
	result &= m_mcFileId != -1;
	m_mutex.unlock();

	if (result == false)
	{
		emitError(tr("Can't get system folder.") + db.lastError().text());
		query.clear();
		db.close();

		// Lock is nit necessare, we will crash anyway!
		//
		assert(m_afblFileId != -1);
		assert(m_alFileId != -1);
		assert(m_hcFileId != -1);
		assert(m_hpFileId != -1);
		assert(m_wvsFileId != -1);
		assert(m_dvsFileId != -1);
		assert(m_mcFileId != -1);

		return;
	}


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

	// Check
	//
	setCurrentUser(DbUser());
	setCurrentProject(DbProject());

	if (QSqlDatabase::contains(projectConnectionName()) == false)
	{
		emitError(tr("Project is not opened."));
		return;
	}

	// Close database connection and remove it from database list
	//
	{
		QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

		if (db.isOpen() == false)
		{
			emitError(tr("Project database connection is closed."));
			return;
		}

		db.close();
	}

	QSqlDatabase::removeDatabase(projectConnectionName());

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
			emitError(db.lastError());
			return;
		}

		result = db_checkUserPassword(db, username, password);

		if (result == false)
		{
			emitError("Wrong password.");
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
			emitError(db.lastError());
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
			emitError(db.lastError());
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
			emitError(query.lastError());
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
			qDebug() << "SetCompleted()";
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
			emitError(db.lastError());
			return;
		}

		projectVersion = db_getProjectVersion(db);
		if (projectVersion == -1)
		{
			emitError("Cannot get project database version.");
			db.close();
			return;
		}

		result = db_checkUserPassword(db, username, password);
		if (result == false)
		{
			emitError("Wrong password.");
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
			emitError(db.lastError());
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
			emitError(db.lastError());
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
			emitError(query.lastError());
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
			emitError(db.lastError());
			return;
		}

		// Start transaction
		//
		result = db.transaction();
		if (result == false)
		{
			emitError(db.lastError());
			db.close();
			return;
		}

		{
			std::shared_ptr<int*> closeDb(nullptr, [&db, &result](void*)
				{
					if (result == true)
					{
						qDebug() << "Upgrade: Commit changes.";
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
				emitError(versionQuery.lastError());
				return;
			}
			versionQuery.clear();

			projectVersion = db_getProjectVersion(db);
			if (projectVersion == -1)
			{
				emitError(versionQuery.lastError());
				return;
			}

			// Some checks
			//
			if (projectVersion == databaseVersion())
			{
				emitError(tr("Database %1 is up to date.").arg(databaseName));
				return;
			}

			if (projectVersion > databaseVersion())
			{
				emitError(tr("Database %1 is newer than the software version.").arg(databaseName));
				return;
			}

			// Upgrade database
			//
			for (int i = projectVersion; i < databaseVersion(); i++)
			{
				m_progress->setValue(static_cast<int>(100.0 / (databaseVersion() - projectVersion) * (i - projectVersion)));

				const UpgradeItem& ui = upgradeItems[i];

				// Perform upgade action
				//
				QFile upgradeFile(ui.upgradeFileName);

				qDebug() << "Begin upgrade: item " << i << " completed, file: " << ui.upgradeFileName;

				result = upgradeFile.open(QIODevice::ReadOnly | QIODevice::Text);

				if (result == false)
				{
					emitError(tr("Can't open file %1. \n%2").arg(ui.upgradeFileName).arg(upgradeFile.errorString()));
					break;
				}

				QString upgradeScript = upgradeFile.readAll();

				// Run upgrade script
				//
				{
					QSqlQuery upgradeQuery(db);

					result = upgradeQuery.exec(upgradeScript);

					if (result == false)
					{
						emitError(upgradeQuery.lastError());
						break;
					}
				}

				// Add record to Version table
				//
				{
					QString addVersionRecord = QString(
						"INSERT INTO Version (VersionNo, Reasone)"
						"VALUES (%1, '%2');").arg(i + 1).arg(ui.text);

					QSqlQuery versionQuery(db);

					result = versionQuery.exec(addVersionRecord);

					if (result == false)
					{
						emitError(versionQuery.lastError());
						break;
					}
				}

				qDebug() << "End upgrade item";
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

				qDebug() << "Update file details";

				QSqlQuery euipmentListQuery(db);
				result = euipmentListQuery.exec(reqEquipmentList);

				if (result == false)
				{
					emitError(euipmentListQuery.lastError());
					return;
				}

				while (euipmentListQuery.next())
				{
					QUuid fileInstanceId = euipmentListQuery.value(0).toUuid();
					int fileId = euipmentListQuery.value(1).toInt();
					QString fileName = euipmentListQuery.value(2).toString();

					qDebug() << "FileName: " << fileName << ", FileID: " << fileId << ", FileInstanceID: " << fileInstanceId.toString();

					// Get file instance, read it to DeviceObject
					//
					{
						QSqlQuery getFileQuery(db);

						result = getFileQuery.exec(QString("SELECT Data FROM FileInstance WHERE FileInstanceID = '%1';").arg(fileInstanceId.toString()));

						if (result == false || getFileQuery.next() == false)
						{
							emitError(tr("Cannot get file data, FileInstanceID: %1").arg(fileInstanceId.toString()));
							return;
						}

						QByteArray data = getFileQuery.value(0).toByteArray();
						Hardware::DeviceObject* device = Hardware::DeviceObject::Create(data);

						if (device == nullptr)
						{
							result = false;
							emitError(tr("Cannot read file data, FileName %1, FileInstanceID %2.").arg(fileName).arg(fileInstanceId.toString()));
							return;
						}

						getFileQuery.clear();

						QString details = device->details();

						// Update details field in DB
						//
						QSqlQuery updateDetailsQuery(db);

						result = updateDetailsQuery.exec(
							QString("UPDATE FileInstance SET Details = '%1' WHERE FileInstanceID = '%2';")
								.arg(details)
								.arg(fileInstanceId.toString()));

						if (result == false)
						{
							emitError(tr("Cannot update file details, FileInstanceID: %1").arg(fileInstanceId.toString()));
							return;
						}
					}
				}
			}
		}
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
		emitError(tr("Cannot get user list. Database connection is not openned."));
		return;
	}

	if (currentUser().isAdminstrator() == false)
	{
		emitError(tr("Current user does not have administrator privileges."));
		return;
	}

	// Check if such user already exists
	// SELECT * FROM creat_user();
	//
	QString request = QString("SELECT * FROM create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
					  .arg(currentUser().userId())
					  .arg(user.username())
					  .arg(user.firstName())
					  .arg(user.lastName())
					  .arg(user.newPassword())
					  .arg(user.isAdminstrator() ? "true" : "false")
					  .arg(user.isReadonly() ? "true" : "false")
					  .arg(user.isDisabled() ? "true" : "false");

	QSqlQuery query(db);
	bool result = query.exec(request);

	if (result == false)
	{
		emitError(tr("Can't create user %1, error: %2").arg(user.username()).arg(db.lastError().text()));
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
		emitError(tr("Database connection is not openned."));
		return;
	}

	if (currentUser().username() != user.username() && currentUser().isAdminstrator() == false)
	{
		assert(false);
		emitError(tr("Only administrators can change other users' details."));
		return;
	}


	// update user
	//
	QString request;

	if (user.newPassword().isEmpty() == false)
	{
		request = QString("SELECT * FROM update_user(%1, '%2', '%3', '%4', '%5', '%6', %7, %8, %9);")
				  .arg(currentUser().userId())
				  .arg(user.username())
				  .arg(user.firstName())
				  .arg(user.lastName())
				  .arg(user.password())
				  .arg(user.newPassword())
				  .arg(user.isAdminstrator() ? "true" : "false")
				  .arg(user.isReadonly() ? "true" : "false")
				  .arg(user.isDisabled() ? "true" : "false");
	}
	else
	{
		request = QString("SELECT * FROM update_user(%1, '%2', '%3', '%4', '%5', NULL, %6, %7, %8);")
				  .arg(currentUser().userId())
				  .arg(user.username())
				  .arg(user.firstName())
				  .arg(user.lastName())
				  .arg(user.password())
				  .arg(user.isAdminstrator() ? "true" : "false")
				  .arg(user.isReadonly() ? "true" : "false")
				  .arg(user.isDisabled() ? "true" : "false");
	}

	QSqlQuery query(db);
	bool result = query.exec(request);

	if (result == false)
	{
		emitError(tr("Can't update user %1, error: %2").arg(user.username()).arg(query.lastError().text()));
		return;
	}

	if (query.size() > 0)
	{
		result = query.next();
		assert(result);

		//int userID = query.value("UserID").toInt();
	}

	return;


//	// Start transaction
//	//
//	bool result = db.transaction();

//	if (result == false)
//	{
//		emitError(db.lastError());
//		return;
//	}

	// Operation
	//
//	{
//		std::shared_ptr<int*> finishTransaction(nullptr, [this, &db, &result](void*)
//		{
//			if (result == true)
//			{
//				qDebug() << "UpdateUser: Commit changes.";
//				result = db.commit();
//				if (result == false)
//				{
//					emitError(db.lastError());
//				}
//			}
//			else
//			{
//				qDebug() << "UpdateUser: Rollback changes.";
//				db.rollback();
//			}
//			});

//		// Check if such user already exists
//		// SELECT UserID, Password FROM Users WHERE Username=user.username();
//		//
//		QSqlQuery query(db);
//		result = query.exec(QString("SELECT UserID, Password FROM Users WHERE Username = '%1';").arg(user.username()));

//		if (result == false || query.size() != 1)
//		{
//			emitError(tr("Can't update user data, error: %1").arg(db.lastError().text()));
//			result = false;
//			return;
//		}

//		result = query.next();
//		assert(result);

//		int userID = query.value("UserID").toInt();
//		QString password = query.value("Password").toString();

//		if (userID == 0)
//		{
//			emitError(tr("User %1 is not exists").arg(user.username()));
//			result = false;
//			return;
//		}

//		bool updatePassword = false;

//		if (user.newPassword().isEmpty() == false)
//		{
//			if (user.password() != password)
//			{
//				emitError(tr("Wrong old password."));
//				result = false;
//				return;
//			}
//			else
//			{
//				updatePassword = true;
//			}
//		}

//		// Update Users request
//		// UPDATE Users
//		//		SET UserID=?, Date=?, Username=?, FirstName=?, LastName=?,
//		//		Password=?, Administrator=?, ReadOnly=?, Disabled=?
//		//		WHERE <condition>;

//		QString updateQurery;

//		if (updatePassword == true)
//		{
//			updateQurery = QString(
//				"UPDATE Users "
//					"SET FirstName='%1', LastName='%2', Password='%3', "
//					"Administrator=%4, ReadOnly=%5, Disabled=%6 "
//					"WHERE UserID = %7;")
//				.arg(user.firstName())
//				.arg(user.lastName())
//				.arg(user.newPassword())
//				.arg(user.isAdminstrator() ? "TRUE" : "FALSE")
//				.arg(user.isReadonly() ? "TRUE" : "FALSE")
//				.arg(user.isDisabled() ? "TRUE" : "FALSE")
//				.arg(userID);

//		}
//		else
//		{
//			updateQurery = QString(
//				"UPDATE Users "
//					"SET FirstName='%1', LastName='%2', "
//					"Administrator=%3, ReadOnly=%4, Disabled=%5 "
//					"WHERE UserID = %6;")
//				.arg(user.firstName())
//				.arg(user.lastName())
//				.arg(user.isAdminstrator() ? "TRUE" : "FALSE")
//				.arg(user.isReadonly() ? "TRUE" : "FALSE")
//				.arg(user.isDisabled() ? "TRUE" : "FALSE")
//				.arg(userID);
//		}

//		result = query.exec(updateQurery);

//		if (result == false)
//		{
//			emitError(tr("Update user data error: %1").arg(query.lastError().text()));
//			return;
//		}
//	}
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
		emitError(tr("Cannot get user list. Database connection is not openned."));
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
		emitError(tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	QString request = QString("SELECT * FROM get_file_list(%1, %2, '%%%3');")
			.arg(currentUser().userId())
			.arg(parentId)
			.arg(filter);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't get file list. Error: ") +  q.lastError().text());
		return;
	}

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

void DbWorker::slot_getFileInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out)
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
		emitError(tr("Cannot get file list. Database connection is not openned."));
		return;
	}

	QString request = QString("SELECT * FROM get_file_info(%1, ARRAY[")
			.arg(currentUser().userId());

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

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't get file info. Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbFileInfo fileInfo;

		db_dbFileInfo(q, &fileInfo);
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
		emitError(tr("Cannot get file list. Database connection is not openned."));
		return;
	}

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
		QString request = QString("SELECT * FROM add_file(%1,'%2', %3, ")
				.arg(currentUser().userId())
				.arg(file->fileName())
				.arg(parentId);

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);
		data.clear();

		request += QString(", '%1');").arg(file->details());

		QSqlQuery q(db);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(tr("Can't add file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't get request result."));
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
		emitError(tr("Cannot delete files. Database connection is not openned."));
		return;
	}

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

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(tr("Can't delete file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't get result"));
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
		emitError(tr("Cannot get file. Database connection is not openned."));
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
		QString request = QString("SELECT * FROM get_latest_file_version(%1, %2);")
				.arg(currentUser().userId())
				.arg(fi.fileId());

		QSqlQuery q(db);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't find file: %1").arg(fi.fileName()));
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		db_updateFile(q, file.get());

		assert(fi.fileId() == file->fileId());

		out->push_back(file);
	}

	return;
}

void DbWorker::slot_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::list<std::shared_ptr<DbFile>>* out)
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
		emitError(tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// request, result is a list of DbFile
	//
	QString request = QString("SELECT * FROM get_latest_file_tree_version(%1, %2);")
			.arg(currentUser().userId())
			.arg(parentFileInfo.fileId());

	QSqlQuery q(db);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(tr("Can't get file. Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		db_updateFile(q, file.get());

		out->push_back(file);
	}

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
		emitError(tr("Cannot get file. Database connection is not openned."));
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
	QString request = QString("SELECT * FROM get_checked_out_files(%1, %2);")
			.arg(currentUser().userId())
			.arg(filesArray);

	QSqlQuery q(db);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(tr("Can't get file. Error: ") +  q.lastError().text());
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
		emitError(tr("Cannot get file. Database connection is not openned."));
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
		QString request = QString("SELECT * FROM get_workcopy(%1, %2);")
				.arg(currentUser().userId())
				.arg(fi.fileId());

		QSqlQuery q(db);

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't find workcopy for file: %1. Is file CheckedOut?").arg(fi.fileName()));
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
		emitError(tr("Cannot get file. Database connection is not openned."));
		return;
	}

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		auto file = files->at(i);

		// Set progress value here
		// ...
		// -- end ofSet progress value here

		if (m_progress->wasCanceled() == true)
		{
			break;
		}

		// request
		//
		QString request = QString("SELECT * FROM set_workcopy(%1, %2, ")
				.arg(currentUser().userId())
				.arg(file->fileId());

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);
		data.clear();

		request += QString(", '%1');").arg(file->details());

		QSqlQuery q(db);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(tr("Can't save file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't get FileID"));
			return;
		}

		int fileId = q.value(0).toInt();

		if (fileId != file->fileId())
		{
			assert(fileId == file->fileId());
			emitError(tr("Write file error. filename: %1").arg(file->fileName()));
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
		emitError(tr("Cannot get file. Database connection is not openned."));
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

		bool result = q.exec(request);
		if (result == false)
		{
			emitError(tr("Can't get file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't find workcopy for file: %1. Is file CheckedOut?").arg(fi.fileName()));
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
		emitError(tr("Cannot get file. Database connection is not openned."));
		return;
	}

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
			.arg(comment);

	// request
	//
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't check in. Error: ") +  q.lastError().text());
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
		emitError(tr("Cannot get file. Database connection is not openned."));
		return;
	}

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
			.arg(comment);

	// request
	//
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't check in. Error: ") +  q.lastError().text());
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
		emitError(tr("Database connection is not openned."));
		return;
	}

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

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't undo changes. Error: ") +  q.lastError().text());
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
		emitError(tr("Cannot get file. Database connection is not openned."));
		return;
	}

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

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't check out. Error: ") +  q.lastError().text());
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
		emitError(tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM file_has_children(%1, %2)")
			.arg(currentUser().userId())
			.arg(fileInfo->fileId());

	QSqlQuery q(db);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(tr("Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() == false)
	{
		emitError(tr("Can't get result."));
		return;
	}

	int childCount = q.value(0).toInt();

	*hasChildren = childCount > 0;

	return;
}

void DbWorker::slot_getFileHistory(DbFileInfo* file, std::vector<DbChangesetInfo>* out)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
	{
		this->m_progress->setCompleted(true);			// set complete flag on return
	});

	// Check parameters
	//
	if (file == nullptr || file->fileId() == -1 || out == nullptr)
	{
		assert(false);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emitError(tr("Cannot execute function. Database connection is not openned."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM get_file_history(%1)")
			.arg(file->fileId());

	QSqlQuery q(db);

	bool result = q.exec(request);
	if (result == false)
	{
		emitError(tr("Error: ") +  q.lastError().text());
		return;
	}

	while (q.next())
	{
		DbChangesetInfo ci;

		ci.setChangeset(q.value("ChangesetID").toInt());
		ci.setDate(q.value("CheckInTime").toString());
		ci.setComment(q.value("Comment").toString());
		ci.setUserId(q.value("UserID").toInt());
		ci.setAction(static_cast<VcsItemAction::VcsItemActionType>(q.value("Action").toInt()));

		out->push_back(ci);
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
		emitError(tr("Cannot get file list. Database connection is not openned."));
		return;
	}

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
			emitError(tr("Argument errors."));
			return false;
		}

		QString strData;
		DbFile::convertToDatabaseString(data, &strData);

		QString request = QString("SELECT * FROM add_device(%1, %2, %3, '%4', '%5');")
				.arg(currentUser().userId())
				.arg(strData)
				.arg(parentId)
				.arg(current->fileExtension())
				.arg(current->details());

		strData.clear();

		QSqlQuery q(db);
		result = q.exec(request);

		if (result == false)
		{
			nesting --;
			emitError(tr("Can't add device. Error: ") +  q.lastError().text());
			return false;
		}

		if (q.next() == false)
		{
			nesting --;
			emitError(tr("Can't get result."));
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
		emitError(tr("Cannot get signals' IDs. Database connection is not opened."));
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
		emitError(tr("Can't get signals' IDs! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		int signalID = q.value(0).toInt();

		//qDebug() << signalID;

		signalsIDs->append(signalID);
	}

	return;
}


void DbWorker::slot_getSignals(SignalSet* signalSet)
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
		emitError(tr("Cannot get signals. Database connection is not opened."));
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


	quint64 start = QDateTime::currentMSecsSinceEpoch();

	result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't get signal workcopy! Error: ") +  q.lastError().text());
		return;
	}

	int n = 0;

//	OrderedHash<int, Signal*> set;

	while(q.next() != false)
	{
		n++;

		if (dProgress != 0 && (n % dProgress) == 0)
		{
			m_progress->setValue((n / dProgress) * 5);
		}

		Signal* s = new Signal;

		getSignalData(q, *s);

		signalSet->append(s->ID(), s);

		//set.append(s->ID(), s);
	}

	quint64 finish = QDateTime::currentMSecsSinceEpoch();

	qDebug() << (finish - start);

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
		emitError(tr("Cannot get latest signal. Database connection is not opened."));
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
		emitError(tr("Can't get signal workcopy! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		getSignalData(q, *signal);
	}

	return;
}



void DbWorker::getSignalData(QSqlQuery& q, Signal& s)
{
	s.setID(q.value(0).toInt());
	s.setSignalGroupID(q.value(1).toInt());
	s.setSignalInstanceID(q.value(2).toInt());
	s.setChangesetID(q.value(3).toInt());
	s.setCheckedOut(q.value(4).toBool());
	s.setUserID(q.value(5).toInt());
	s.setChannel(q.value(6).toInt());
	s.setType(static_cast<E::SignalType>(q.value(7).toInt()));
	s.setCreated(q.value(8).toString());
	s.setDeleted(q.value(9).toBool());
	s.setInstanceCreated(q.value(10).toString());
	s.setInstanceAction(static_cast<E::InstanceAction>(q.value(11).toInt()));
	s.setStrID(q.value(12).toString());
	s.setExtStrID(q.value(13).toString());
	s.setName(q.value(14).toString());
	s.setDataFormat(static_cast<E::DataFormat>(q.value(15).toInt()));
	s.setDataSize(q.value(16).toInt());
	s.setLowADC(q.value(17).toInt());
	s.setHighADC(q.value(18).toInt());
	s.setLowLimit(q.value(19).toDouble());
	s.setHighLimit(q.value(20).toDouble());
	s.setUnitID(q.value(21).toInt());
	s.setAdjustment(q.value(22).toDouble());
	s.setDropLimit(q.value(23).toDouble());
	s.setExcessLimit(q.value(24).toDouble());
	s.setUnbalanceLimit(q.value(25).toDouble());
	s.setInputLowLimit(q.value(26).toDouble());
	s.setInputHighLimit(q.value(27).toDouble());
	s.setInputUnitID(q.value(28).toInt());
	s.setInputSensorID(q.value(29).toInt());
	s.setOutputLowLimit(q.value(30).toDouble());
	s.setOutputHighLimit(q.value(31).toDouble());
	s.setOutputUnitID(q.value(32).toInt());
	s.setOutputSensorID(q.value(33).toInt());
	s.setAcquire(q.value(34).toBool());
	s.setCalculated(q.value(35).toBool());
	s.setNormalState(q.value(36).toInt());
	s.setDecimalPlaces(q.value(37).toInt());
	s.setAperture(q.value(38).toDouble());
	s.setInOutType(static_cast<E::SignalInOutType>(q.value(39).toInt()));
	s.setDeviceStrID(q.value(40).toString());
	s.setOutputRangeMode(static_cast<E::OutputRangeMode>(q.value(41).toInt()));		// since version 35 of database
	s.setFilteringTime(q.value(42).toDouble());										//
	s.setMaxDifference(q.value(43).toDouble());										//
	s.setByteOrder(static_cast<E::ByteOrder>(q.value(44).toInt()));					//

/*	s.setID(q.value("signalid").toInt());
	s.setSignalGroupID(q.value("signalgroupid").toInt());
	s.setSignalInstanceID(q.value("signalinstanceid").toInt());
	s.setChangesetID(q.value("changesetid").toInt());
	s.setCheckedOut(q.value("checkedout").toBool());
	s.setUserID(q.value("userid").toInt());
	s.setChannel(q.value("channel").toInt());
	s.setType(static_cast<SignalType>(q.value("type").toInt()));
	s.setCreated(q.value("created").toString());
	s.setDeleted(q.value("deleted").toBool());
	s.setInstanceCreated(q.value("instancecreated").toString());
	s.setInstanceAction(static_cast<InstanceAction>(q.value("action").toInt()));
	s.setStrID(q.value("strid").toString());
	s.setExtStrID(q.value("extstrid").toString());
	s.setName(q.value("name").toString());
	s.setDataFormat(static_cast<DataFormat>(q.value("dataformatid").toInt()));
	s.setDataSize(q.value("datasize").toInt());
	s.setLowADC(q.value("lowadc").toInt());
	s.setHighADC(q.value("highadc").toInt());
	s.setLowLimit(q.value("lowlimit").toDouble());
	s.setHighLimit(q.value("highlimit").toDouble());
	s.setUnitID(q.value("unitid").toInt());
	s.setAdjustment(q.value("adjustment").toDouble());
	s.setDropLimit(q.value("droplimit").toDouble());
	s.setExcessLimit(q.value("excesslimit").toDouble());
	s.setUnbalanceLimit(q.value("unbalancelimit").toDouble());
	s.setInputLowLimit(q.value("inputlowlimit").toDouble());
	s.setInputHighLimit(q.value("inputhighlimit").toDouble());
	s.setInputUnitID(q.value("inputunitid").toInt());
	s.setInputSensorID(q.value("inputsensorid").toInt());
	s.setOutputLowLimit(q.value("outputlowlimit").toDouble());
	s.setOutputHighLimit(q.value("outputhighlimit").toDouble());
	s.setOutputUnitID(q.value("outputunitid").toInt());
	s.setOutputSensorID(q.value("outputsensorid").toInt());
	s.setAcquire(q.value("acquire").toBool());
	s.setCalculated(q.value("calculated").toBool());
	s.setNormalState(q.value("normalstate").toInt());
	s.setDecimalPlaces(q.value("decimalplaces").toInt());
	s.setAperture(q.value("aperture").toDouble());
	s.setInOutType(static_cast<SignalInOutType>(q.value("inouttype").toInt()));
	s.setDeviceStrID(q.value("devicestrid").toString());
	s.setOutputRangeMode(static_cast<OutputRangeMode>(q.value("outputrangemode").toInt()));		// since version 35 of database
	s.setFilteringTime(q.value("filteringtime").toDouble());									//
	s.setMaxDifference(q.value("maxdifference").toDouble());									//
	s.setByteOrder(static_cast<ByteOrder>(q.value("byteorder").toInt()));		*/				//
}


QString DbWorker::getSignalDataStr(const Signal& s)
{
	QString str = QString(
			"'(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,"
			"%11,%12,\"%13\",\"%14\",\"%15\",%16,%17,%18,%19,%20,"
			"%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,"
			"%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,"
			"\"%41\",%42,%43,%44,%45)'")
	.arg(s.ID())
	.arg(s.signalGroupID())
	.arg(s.signalInstanceID())
	.arg(s.changesetID())
	.arg(s.checkedOut())
	.arg(s.userID())
	.arg(s.channel())
	.arg(TO_INT(s.type()))
	.arg(s.created().toString(DATE_TIME_FORMAT_STR))
	.arg(s.deleted())
	.arg(s.instanceCreated().toString(DATE_TIME_FORMAT_STR))
	.arg(s.instanceAction())
	.arg(s.strID())
	.arg(s.extStrID())
	.arg(s.name())
	.arg(TO_INT(s.dataFormat()))
	.arg(s.dataSize())
	.arg(s.lowADC())
	.arg(s.highADC())
	.arg(s.lowLimit())
	.arg(s.highLimit())
	.arg(s.unitID())
	.arg(s.adjustment())
	.arg(s.dropLimit())
	.arg(s.excessLimit())
	.arg(s.unbalanceLimit())
	.arg(s.inputLowLimit())
	.arg(s.inputHighLimit())
	.arg(s.inputUnitID())
	.arg(s.inputSensorID())
	.arg(s.outputLowLimit())
	.arg(s.outputHighLimit())
	.arg(s.outputUnitID())
	.arg(s.outputSensorID())
	.arg(s.acquire() ? "TRUE" : "FALSE")
	.arg(s.calculated() ? "TRUE" : "FALSE")
	.arg(s.normalState())
	.arg(s.decimalPlaces())
	.arg(s.aperture())
	.arg(s.inOutType())
	.arg(s.deviceStrID())
	.arg(s.outputRangeMode())			// since version 35 of database
	.arg(s.filteringTime())				//
	.arg(s.maxDifference())				//
	.arg(TO_INT(s.byteOrder()));				//

	qDebug() << str;

	return str;
}


void DbWorker::getObjectState(QSqlQuery& q, ObjectState &os)
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


void DbWorker::addSignal(E::SignalType signalType, QVector<Signal>* newSignal)
{
	if (newSignal == nullptr)
	{
		assert(newSignal != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(tr("Cannot add signals. Database connection is not opened."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM add_signal(%1, %2, %3)")
		.arg(currentUser().userId()).arg(static_cast<int>(signalType)).arg(newSignal->count());
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't add new signal(s)! Error: ") +  q.lastError().text());
		return;
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

		QString sds = getSignalDataStr(signal);

		QString request2 = QString("SELECT * FROM set_signal_workcopy(%1, %2)")
			.arg(currentUser().userId()).arg(sds);

		QSqlQuery q2(db);

		result = q2.exec(request2);

		if (result == false)
		{
			emitError(tr("Can't set signal workcopy! Error: ") +  q2.lastError().text());
			return;
		}

		assert(i<newSignal->count());

		request2 = QString("SELECT * FROM get_latest_signal(%1, %2)")
			.arg(currentUser().userId()).arg(signalID);

		QSqlQuery q3(db);

		result = q3.exec(request2);

		if (result == false)
		{
			emitError(tr("Can't get latest signal! Error: ") +  q2.lastError().text());
			return;
		}

		while(q3.next() != false)
		{
			getSignalData(q3, (*newSignal)[i]);
			readed++;
		}

		i++;
	}

	assert(i == readed);
}


void DbWorker::slot_getUnits(UnitList *units)
{
	AUTO_COMPLETE

	// Check parameters
	//
	if (units == nullptr)
	{
		assert(units != nullptr);
		return;
	}

	units->clear();

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(tr("Cannot get units. Database connection is not opened."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM get_units()");
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't get units! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next() != false)
	{
		int unitID = q.value("unitid").toInt();
		QString unitNameEn = q.value("unit_en").toString();

		units->append(unitID, unitNameEn);
	}
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
		emitError(tr("Cannot checkout signals. Database connection is not opened."));
		return;
	}

	// request
	//
	QString request = QString("SELECT * FROM checkout_signals(%1,ARRAY[").arg(currentUser().userId());

	int count = signalIDs->count();

	for(int i = 0; i < count; i++)
	{
		if (i < count -1)
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
		emitError(tr("Can't checkout signals! Error: ") +  q.lastError().text());
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
		emitError(tr("Cannot set signal workcopy. Database connection is not opened."));
		return;
	}

	// request
	//
	signal->setCreated(QDateTime::currentDateTime());
	signal->setInstanceCreated(QDateTime::currentDateTime());

	QString sds = getSignalDataStr(*signal);

	QString request = QString("SELECT * FROM set_signal_workcopy(%1, %2)")
		.arg(currentUser().userId()).arg(sds);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't set signal workcopy! Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() != false)
	{
		getObjectState(q, *objectState);
	}

	QString request2 = QString("SELECT * FROM get_latest_signal(%1, %2)")
		.arg(currentUser().userId()).arg(signal->ID());

	QSqlQuery q2(db);

	result = q2.exec(request2);

	if (result == false)
	{
		emitError(tr("Can't get latest signal! Error: ") +  q2.lastError().text());
		return;
	}

	if (q2.next() != false)
	{
		getSignalData(q2, *signal);
	}
	else
	{
		emitError(tr("Can't get latest signal! No data returned!"));
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
		emitError(tr("Cannot delete signal. Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM delete_signal(%1, %2)")
		.arg(currentUser().userId()).arg(signalID);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't delete signal! Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() != false)
	{
		getObjectState(q, *objectState);
	}
	else
	{
		emitError(tr("Can't delete signal! No data returned!"));
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
		emitError(tr("Cannot undo signal changes. Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM undo_signal_changes(%1, %2)")
		.arg(currentUser().userId()).arg(signalID);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't undo signal changes! Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() != false)
	{
		getObjectState(q, *objectState);
	}
	else
	{
		emitError(tr("Can't undo signal changes! No data returned!"));
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
		emitError(tr("Cannot checkin signals. Database connection is not opened."));
		return;
	}

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
		emitError(tr("Can't checkin signals! Error: ") +  q.lastError().text());
		return;
	}

	while(q.next())
	{
		ObjectState os;

		getObjectState(q, os);

		objectState->append(os);
	}
}


void DbWorker::slot_autoAddSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals)
{
	AUTO_COMPLETE

	if (deviceSignals == nullptr)
	{
		assert(deviceSignals != nullptr);
		return;
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
			Signal signal(*deviceSignal);

			QVector<Signal> newSignals;

			newSignals.append(signal);

			addSignal(signal.type(), &newSignals);
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
		emitError(tr("Cannot delete signal. Database connection is not opened."));
		return;
	}

	ObjectState os;

	for(Hardware::DeviceSignal* deviceSignal : *deviceSignals)
	{
		QString request = QString("SELECT * FROM delete_signal_by_device_str_id(%1, '%2'')")
			.arg(currentUser().userId()).arg(deviceSignal->strId());

		QSqlQuery q(db);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(tr("Can't delete signal! Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() != false)
		{
			getObjectState(q, os);
		}
		else
		{
			emitError(tr("Can't delete signal! No data returned!"));
		}
	}
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
		emitError(tr("Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM build_start(%1, '%2', cast(%3 as boolean), %4)")
		.arg(currentUser().userId()).arg(workstation).arg(release).arg(changeset);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(q.lastError().text());
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
		emitError(tr("Database connection is not opened."));
		return;
	}

	QString request = QString("SELECT * FROM build_finish(%1, %2, %3, '%4')")
		.arg(buildID).arg(errors).arg(warnings).arg(buildLog);

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(q.lastError().text());
		return;
	}

	return;
}


void DbWorker::slot_isAnyCheckedOut(bool* checkedOut)
{
	AUTO_COMPLETE

	if (checkedOut == nullptr)
	{
		assert(checkedOut != nullptr);
		return;
	}

	// Operation
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

	if (db.isOpen() == false)
	{
		emitError(tr("Database connection is not opened."));
		return;
	}

	QString request = "SELECT * FROM is_any_checked_out();";

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(q.lastError().text());
		return;
	}

	q.next();
	*checkedOut = q.value(0).toBool();

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
		emitError(tr("Database connection is not opened."));
		return;
	}

	QString request = "SELECT * FROM get_last_changeset();";

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(q.lastError().text());
		return;
	}

	q.next();
	*lastChangesetId = q.value(0).toInt();

	return;
}

bool DbWorker::db_getUserData(QSqlDatabase db, int userId, DbUser* user)
{
	if (user == nullptr)
	{
		assert(user != nullptr);
		return false;
	}

	QSqlQuery query(db);

	bool result = query.exec(QString("SELECT * FROM Users WHERE UserID = %1").arg(userId));

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

	user->setUserId(userId);

	user->setUsername(query.value("Username").toString());
	user->setFirstName(query.value("FirstName").toString());
	user->setLastName(query.value("LastName").toString());
	user->setPassword(query.value("Password").toString());
	user->setAdministrator(query.value("Administrator").toBool());
	user->setReadonly(query.value("ReadOnly").toBool());
	user->setDisabled(query.value("Disabled").toBool());

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

	if (projectVersion < 4)		// Since version 4 database has stored procedure get_user_id
	{
		// Check by query
		//
		QSqlQuery query(db);

		bool result = query.exec(
			QString("SELECT UserID FROM Users WHERE Username='%1' AND Password='%2'").arg(username).arg(password));

		if (result == false)
		{
			return false;
		}

		if (query.next() == false)
		{
			return false;
		}
	}
	else
	{
		// Check by store function
		//
		QSqlQuery query(db);
		bool result = query.exec(QString("SELECT * FROM check_user_password('%1', '%2');").arg(username).arg(password));

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
		emitError(versionQuery.lastError());
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
	file->setFileId(q.value("FileID").toInt());
	file->setDeleted(q.value("Deleted").toBool());
	file->setFileName(q.value("Name").toString());
	file->setParentId(q.value("ParentID").toInt());
	file->setChangeset(q.value("ChangesetID").toInt());
	file->setCreated(q.value("Created").toString());
	file->setLastCheckIn(q.value("CheckOutTime").toString());		// setLastCheckIn BUT TIME IS CheckOutTime

	bool checkedOut = q.value("CheckedOut").toBool();
	file->setState(checkedOut ? VcsState::CheckedOut : VcsState::CheckedIn);

	int action = q.value("Action").toInt();
	file->setAction(static_cast<VcsItemAction::VcsItemActionType>(action));

	file->setUserId(q.value("UserID").toInt());

	file->setDetails(q.value("Details").toString());

	QByteArray data = q.value("Data").toByteArray();
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
	fileInfo->setCreated(q.value(5).toString());
	fileInfo->setSize(q.value(6).toInt());
	fileInfo->setState(q.value(7).toBool() ? VcsState::CheckedOut : VcsState::CheckedIn);
	//fileInfo->setCheckoutTime(q.value(8).toString());
	fileInfo->setUserId(q.value(9).toInt());
	fileInfo->setAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(10).toInt()));
	fileInfo->setDetails(q.value(11).toString());

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
