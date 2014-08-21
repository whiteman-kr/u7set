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

		bool result = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' OR datname LIKE 'U7_%';");

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
			qDebug() << postgresConnectionName() << "S";
			QSqlDatabase::removeDatabase(postgresConnectionName());		// remove database
			qDebug() << postgresConnectionName() << "F";
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

void DbWorker::slot_upgradeProject(const QString databaseName)
{
	assert(false);
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
