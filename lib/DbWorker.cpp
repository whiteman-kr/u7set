#include "../include/DbWorker.h"

// Upgrade database
//
const UpgradeItem DbWorker::upgradeItems[] = {
	{"Create project", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0001.sql"},
	{"Add Changeset table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0002.sql"},
	{"Add Disabled column to User table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0003.sql"},
	{"Add GetUserID fucntion", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0004.sql"},
	{"Add File table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0005.sql"},
	{"Add File to Changeset table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0006.sql"},
	{"Add UUID extension, cretae FileInstance table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0007.sql"},
	{"Add CheckOut table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0008.sql"},
	{"Add UndoFilePendingChanges function", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0009.sql"},
	{"Add UNIQUE(FileID), UNIQUE(SignalID) to the CheckOut table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0010.sql"},
	{"Add SysttemInst, CaseInst, SubblockInst and BlockInst tables", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0011.sql"},
	{"Add SystemID, CaseID, SubblockID, BlockID columns to CheckOut table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0012.sql"},
	{"Add GetFileList stored procedure", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0013.sql"},
	{"Add AddFile stored procedure", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0014.sql"},
	{"Add fields ParentID, Deleted to File table", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0015.sql"},
	{"Replace AddFile function", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0016.sql"},
	{"Add GetWorkcopy function", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0017.sql"},
	{"Add CheckIn function", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0018.sql"},
	{"Add system folders (AFBL, AL, HC, WVS, DVS)", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0019.sql"},
	{"Add tables for storing application signals", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0020.sql"},
	{"Add AddSystem function", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0021.sql"},
	{"Add add_device function, drop AddSystem", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0022.sql"},
	{"Add is_admin function", ":/DatabaseUpgrade/DatabaseUpgrade/Upgrade0023.sql"},
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
	m_afblFileId(-1),
	m_alFileId(-1),
	m_hcFileId(-1),
	m_wvsFileId(-1),
	m_dvsFileId(-1),
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

			QString createVersionTableSql = QString("SELECT max(\"VersionNo\") FROM \"Version\";");

			QSqlQuery versionQuery(projectDb);
			result = versionQuery.exec(createVersionTableSql);

			if (result == false)
			{
				emitError(versionQuery.lastError());
				versionQuery.clear();
				projectDb.close();
				continue;
			}

			if (versionQuery.next())
			{
				int projectVersion = versionQuery.value(0).toInt();
				pi->setVersion(projectVersion);
			}

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
					"CREATE TABLE \"Version\" ("
					"\"VersionId\" SERIAL PRIMARY KEY,"
					"\"VersionNo\" integer NOT NULL,"
					"\"Date\" timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,"
					"\"Reasone\" text NOT NULL"
					");");

		result = newDbQuery.exec(createVersionTableSql);
		if (result == false)
		{
			emitError(newDbQuery.lastError());
			newDatabase.close();
			return;
		}

		// Add first record to version table
		//
		QString addFirstVersionRecord = QString(
			"INSERT INTO \"Version\" (\"VersionNo\", \"Reasone\")"
			"VALUES (1, 'Create project');");

		result = newDbQuery.exec(addFirstVersionRecord);
		if (result == false)
		{
			emitError(newDbQuery.lastError());
			newDbQuery.clear();
			newDatabase.close();
			return;
		}

		// Create User table
		//
		QString createUserTableSql = QString(
			"CREATE TABLE \"User\""
			"("
				"\"UserID\" serial PRIMARY KEY NOT NULL,"
				"\"Date\" timestamp with time zone NOT NULL DEFAULT now(),"
				"\"Username\" text NOT NULL,"
				"\"FirstName\" text NOT NULL,"
				"\"LastName\" text NOT NULL,"
				"\"Password\" text NOT NULL,"
				"\"Administrator\" boolean NOT NULL DEFAULT FALSE,"
				"\"ReadOnly\" boolean NOT NULL DEFAULT TRUE"
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
			"INSERT INTO \"User\"(\"Username\", \"FirstName\", \"LastName\", \"Password\", \"Administrator\", \"ReadOnly\")"
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
	result = query.exec(QString("SELECT \"GetUserID\"('%1', '%2');").arg(username).arg(password));

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

	slot_getFileList(&systemFiles, rootFileId(), "%");

	m_mutex.lock();
	m_afblFileId = -1;
	m_alFileId = -1;
	m_hcFileId = -1;
	m_wvsFileId = -1;
	m_dvsFileId = -1;
	m_mutex.unlock();

	for (const DbFileInfo& fi : systemFiles)
	{
		if (fi.fileName() == "AFBL")
		{
			QMutexLocker locker(&m_mutex);
			m_afblFileId = fi.fileId();
			continue;
		}

		if (fi.fileName() == "AL")
		{
			QMutexLocker locker(&m_mutex);
			m_alFileId = fi.fileId();
			continue;
		}

		if (fi.fileName() == "HC")
		{
			QMutexLocker locker(&m_mutex);
			m_hcFileId = fi.fileId();
			continue;
		}

		if (fi.fileName() == "WVS")
		{
			QMutexLocker locker(&m_mutex);
			m_wvsFileId = fi.fileId();
			continue;
		}

		if (fi.fileName() == "DVS")
		{
			QMutexLocker locker(&m_mutex);
			m_dvsFileId = fi.fileId();
			continue;
		}
	}


	m_mutex.lock();
	result = m_afblFileId != -1;
	result &= m_alFileId != -1;
	result &= m_hcFileId != -1;
	result &= m_wvsFileId != -1;
	result &= m_dvsFileId != -1;
	m_mutex.unlock();

	if (result == false)
	{
		emitError(tr("Can't get system floder.") + db.lastError().text());
		query.clear();
		db.close();

		// Lock is nit necessare, we will crash anyway!
		assert(m_afblFileId != -1);
		assert(m_alFileId != -1);
		assert(m_hcFileId != -1);
		assert(m_wvsFileId != -1);
		assert(m_dvsFileId != -1);

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

void DbWorker::slot_deleteProject(QString projectName, QString password)
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

	// Rename project from the template u7_[projectname] to u7deleted_[projectname]
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

		QSqlQuery query(db);
		QString createDatabaseSql = QString("ALTER DATABASE %1 RENAME TO u7deleted_%2;").arg(databaseName).arg(projectName.toLower());

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

void DbWorker::slot_upgradeProject(QString projectName, QString password)
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

	// Copy project from the template u7_[projectname] to u7upgrade[oldversion]_[projectname]
	//
	{
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

		QSqlQuery query(db);
		QString copyDatabaseSql =
			QString("CREATE DATABASE u7upgrade%1_%2 WITH TEMPLATE %3 OWNER %4;")
				.arg(projectVersion)
				.arg(projectName.toLower())
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
			result = versionQuery.exec("LOCK TABLE \"Version\" IN ACCESS EXCLUSIVE MODE NOWAIT;");

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
						"INSERT INTO \"Version\" (\"VersionNo\", \"Reasone\")"
						"VALUES (%1, '%2');").arg(i + 1).arg(ui.text);

					QSqlQuery versionQuery(db);

					result = versionQuery.exec(addVersionRecord);

					if (result == false)
					{
						emitError(versionQuery.lastError());
						break;
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

	// Start transaction
	//
	bool result = db.transaction();

	if (result == false)
	{
		emitError(db.lastError());
		return;
	}

	{
		std::shared_ptr<int*> finishTransaction(nullptr, [this, &db, &result](void*)
		{
			if (result == true)
			{
				qDebug() << "CreateUser: Commit changes.";
				result = db.commit();
				if (result == false)
				{
					emitError(db.lastError());
				}
			}
			else
			{
				qDebug() << "CreateUser: Rollback changes.";
				db.rollback();
			}
			});


		// Check if such user already exists
		// SELECT "UserID" FROM "User" WHERE "Username"='Administrator';
		//
		QSqlQuery query(db);
		result = query.exec(QString("SELECT \"UserID\" FROM \"User\" WHERE \"Username\"='%1';").arg(user.username()));

		if (result == false)
		{
			emitError(tr("Can't create user %1, error: %2").arg(user.username()).arg(db.lastError().text()));
			return;
		}

		if (query.size() > 0)
		{
			result = query.next();
			assert(result);

			int userID = query.value("UserID").toInt();

			if (userID != 0)
			{
				emitError(tr("User %1 already exists").arg(user.username()));
				return;
			}
		}

		// Create User
		// INSERT INTO "User"("Username", "FirstName", "LastName", "Password", "Administrator", "ReadOnly", "Disabled")
		//		VALUES (:username, :firstName, :lastName, :password, :administrator, :readonly, :disabled);
		//

		query.prepare("INSERT INTO \"User\"(\"Username\", \"FirstName\", \"LastName\", \"Password\", \"Administrator\", \"ReadOnly\", \"Disabled\") "
					  "VALUES (:username, :firstName, :lastName, :password, :administrator, :readonly, :disabled);");

		query.bindValue(":username", user.username());
		query.bindValue(":firstName", user.firstName());
		query.bindValue(":lastName", user.lastName());
		query.bindValue(":password", user.newPassword());
		query.bindValue(":administrator", user.isAdminstrator());
		query.bindValue(":readonly", user.isReadonly());
		query.bindValue(":disabled", user.isDisabled());

		result = query.exec();

		if (result == false)
		{
			emitError(tr("Create user error: %1").arg(query.lastError().text()));
			return;
		}
	}	// commit will happen here, by auto variable finishTransaction


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
		emitError(tr("Cannot get user list. Database connection is not openned."));
		return;
	}

	if (currentUser().username() != user.username() && currentUser().isAdminstrator() == false)
	{
		assert(false);
		emitError(tr("Only administrators can change other users' details."));
		return;
	}


	// Start transaction
	//
	bool result = db.transaction();

	if (result == false)
	{
		emitError(db.lastError());
		return;
	}

	// Operation
	//
	{
		std::shared_ptr<int*> finishTransaction(nullptr, [this, &db, &result](void*)
		{
			if (result == true)
			{
				qDebug() << "UpdateUser: Commit changes.";
				result = db.commit();
				if (result == false)
				{
					emitError(db.lastError());
				}
			}
			else
			{
				qDebug() << "UpdateUser: Rollback changes.";
				db.rollback();
			}
			});

		// Check if such user already exists
		// SELECT "UserID", "Password" FROM "User" WHERE "Username"=user.username();
		//
		QSqlQuery query(db);
		result = query.exec(QString("SELECT \"UserID\", \"Password\" FROM \"User\" WHERE \"Username\"='%1';").arg(user.username()));

		if (result == false || query.size() != 1)
		{
			emitError(tr("Can't update user data, error: %1").arg(db.lastError().text()));
			result = false;
			return;
		}

		result = query.next();
		assert(result);

		int userID = query.value("UserID").toInt();
		QString password = query.value("Password").toString();

		if (userID == 0)
		{
			emitError(tr("User %1 is not exists").arg(user.username()));
			result = false;
			return;
		}

		bool updatePassword = false;

		if (user.newPassword().isEmpty() == false)
		{
			if (user.password() != password)
			{
				emitError(tr("Wrong old password."));
				result = false;
				return;
			}
			else
			{
				updatePassword = true;
			}
		}

		// Update User request
		// UPDATE "User"
		//		SET "UserID"=?, "Date"=?, "Username"=?, "FirstName"=?, "LastName"=?,
		//		"Password"=?, "Administrator"=?, "ReadOnly"=?, "Disabled"=?
		//		WHERE <condition>;

		QString updateQurery;

		if (updatePassword == true)
		{
			updateQurery = QString(
				"UPDATE \"User\" "
					"SET \"FirstName\"='%1', \"LastName\"='%2', \"Password\"='%3', "
					"\"Administrator\"=%4, \"ReadOnly\"=%5, \"Disabled\"=%6 "
					"WHERE \"UserID\" = %7;")
				.arg(user.firstName())
				.arg(user.lastName())
				.arg(user.newPassword())
				.arg(user.isAdminstrator() ? "TRUE" : "FALSE")
				.arg(user.isReadonly() ? "TRUE" : "FALSE")
				.arg(user.isDisabled() ? "TRUE" : "FALSE")
				.arg(userID);

		}
		else
		{
			updateQurery = QString(
				"UPDATE \"User\" "
					"SET \"FirstName\"='%1', \"LastName\"='%2', "
					"\"Administrator\"=%3, \"ReadOnly\"=%4, \"Disabled\"=%5 "
					"WHERE \"UserID\" = %6;")
				.arg(user.firstName())
				.arg(user.lastName())
				.arg(user.isAdminstrator() ? "TRUE" : "FALSE")
				.arg(user.isReadonly() ? "TRUE" : "FALSE")
				.arg(user.isDisabled() ? "TRUE" : "FALSE")
				.arg(userID);
		}

		result = query.exec(updateQurery);

		if (result == false)
		{
			emitError(tr("Update user data error: %1").arg(query.lastError().text()));
			return;
		}
	}
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

	// SELECT "UserID" FROM "User" ORDER BY "Username";
	//
	QSqlQuery q(db);

	bool result = q.exec("SELECT \"UserID\" FROM \"User\" ORDER BY \"Username\";");
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

void DbWorker::slot_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

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

	QString request = QString("SELECT * FROM GetFileList(%1, '%%%2');")
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

		fileInfo.setFileName(q.value("Name").toString());
		fileInfo.setFileId(q.value("FileID").toInt());
		fileInfo.setParentId(q.value("ParentID").toInt());
		fileInfo.setSize(q.value("Size").toInt());
		fileInfo.setChangeset(q.value("ChangesetID").toInt());
		fileInfo.setCreated(q.value("Created").toString());
		fileInfo.setLastCheckIn(q.value("ChangesetTime").toString());
		fileInfo.setState(q.value("CheckedOut").toBool() ? VcsState::CheckedOut : VcsState::CheckedIn);

		DbUser user;
		user.setUserId(q.value("UserID").toInt());
		fileInfo.setUser(user);

		files->push_back(fileInfo);
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
		QString request = QString("SELECT * FROM AddFile(%1,'%2', %3, %4, ")
				.arg(currentUser().userId())
				.arg(file->fileName())
				.arg(parentId)
				.arg(file->size());

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);
		data.clear();

		request += ");";

		QSqlQuery q(db);

		bool result = q.exec(request);

		if (result == false)
		{
			emitError(tr("Can't add file. Error: ") +  q.lastError().text());
			return;
		}

		if (q.next() == false)
		{
			emitError(tr("Can't get FileID"));
			return;
		}

		int fileId = q.value(0).toInt();

		file->setFileId(fileId);
		file->setUser(currentUser());
		file->setState(VcsState::CheckedOut);		// Set file state to CheckedOut
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
		QString request = QString("SELECT * FROM GetWorkcopy(%1);")
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

		file->setFileId(q.value("FileID").toInt());

		file->setFileName(q.value("Name").toString());
		file->setParentId(q.value("ParentID").toInt());
		file->setChangeset(0);
		file->setCreated(q.value("Created").toString());
		file->setLastCheckIn(q.value("CheckOutTime").toString());		// setLastCheckIn BUT TIME IS CheckOutTime
		file->setState(VcsState::CheckedOut);

		DbUser user;
		bool ok = db_getUserData(db, q.value("UserID").toInt(), &user);

		if (ok == false)
		{
			emitError(tr("Can not get user info. userID=%1").arg(q.value("UserID").toInt()));
			continue;
		}

		file->setUser(user);

		QByteArray data = q.value("Data").toByteArray();
		file->swapData(data);

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
		QString request = QString("SELECT * FROM SetWorkcopy(%1, ")
				.arg(file->fileId());

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);
		data.clear();

		request += ");";

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

	QString request = "SELECT * FROM checkin(ARRAY[";

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

	request += QString("], %1, '%2');")
			.arg(currentUser().userId())
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

	// Set file state to CheckedIn
	//
	for (auto& fi : *files)
	{
		fi.setState(VcsState::CheckedIn);
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

	QString request = "SELECT * FROM checkout(ARRAY[";

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

	request += QString("], %1);")
			.arg(currentUser().userId());

	// request
	//
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't undo changes. Error: ") +  q.lastError().text());
		return;
	}

	// Set file state to CheckedOut
	//
	for (auto& fi : *files)
	{
		fi.setState(VcsState::CheckedOut);
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

	QString request = "SELECT * FROM undoChanges(ARRAY[";

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

	request += QString("], %1);")
			.arg(currentUser().userId());

	// request
	//
	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't check out. Error: ") +  q.lastError().text());
		return;
	}

	// Set file state to CheckedIn
	//
	for (auto& fi : *files)
	{
		fi.setState(VcsState::CheckedIn);
	}

	return;
}

void DbWorker::slot_addDeviceObject(DbFile* file, int parentId, QString fileExtension)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

	// Check parameters
	//
	if (file == nullptr)
	{
		assert(file != nullptr);
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

	// request
	// FUNCTION add_device(user_id integer, file_data bytea, parent_id integer, file_extension text)
	//
	QString data;
	file->convertToDatabaseString(&data);

	QString request = QString("SELECT * FROM add_device(%1, %2, %3, '%4');")
		.arg(currentUser().userId())
		.arg(data)
		.arg(parentId)
		.arg(fileExtension);

	data.clear();

	QSqlQuery q(db);
	bool result = q.exec(request);

	if (result == false)
	{
		emitError(tr("Can't add system. Error: ") +  q.lastError().text());
		return;
	}

	if (q.next() == false)
	{
		emitError(tr("Can't get FileID"));
		return;
	}

	int fileId = q.value(0).toInt();

	file->setFileId(fileId);
	file->setUser(currentUser());
	file->setState(VcsState::CheckedOut);		// Set file state to CheckedOut

	return;
}


void DbWorker::slot_getSignalsIDs(QSet<int>* signalsIDs)
{
	// Init automitic varaiables
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
		{
			this->m_progress->setCompleted(true);			// set complete flag on return
		});

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
		emitError(tr("Cannot get signals' IDs. Database connection is not openned."));
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

		qDebug() << signalID;

		signalsIDs->insert(signalID);
	}

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

	bool result = query.exec(QString("SELECT * FROM \"User\" WHERE \"UserID\" = %1").arg(userId));

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

	if (projectVersion < 4)		// Since version 4 database has stored procedure GetUserID
	{
		// Check by query
		//
		QSqlQuery query(db);

		bool result = query.exec(
			QString("SELECT \"UserID\" FROM \"User\" WHERE \"Username\"='%1' AND \"Password\"='%2'").arg(username).arg(password));

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
		bool result = query.exec(QString("SELECT \"GetUserID\"('%1', '%2');").arg(username).arg(password));

		if (result == false)
		{
			return false;
		}

		if (query.next() == false)
		{
			return false;
		}
	}

	return true;
}

int DbWorker::db_getProjectVersion(QSqlDatabase db)
{
	if (db.isOpen() == false)
	{
		return -1;
	}

	QString createVersionTableSql = QString("SELECT max(\"VersionNo\") FROM \"Version\";");

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
