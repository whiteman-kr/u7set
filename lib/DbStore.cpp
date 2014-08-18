#include "../include/DbStore.h"

// Upgrade database
//
const UpgradeItem DbStore::upgradeItems[] = {
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


//
//	DbStore
//
DbStore::DbStore() :
	QObject(nullptr),
	m_pThread(nullptr),
	m_host("127.0.0.1"),
	m_port(5432),
	m_serverUsername("u7"),
	m_serverPassword(""),
	m_operationMutex(QMutex::NonRecursive)
{
	qDebug() << Q_FUNC_INFO;

	m_pThread = new QThread();

	this->moveToThread(m_pThread);

	connect(m_pThread, &QThread::started, [](){qDebug() << "Database communication thread has been started";});
	connect(m_pThread, &QThread::finished, [](){qDebug() << "Database communication thread has been finished";});

	// --
	//
	//connect(this, SIGNAL(signal_openConnection(QString, int, QString, QString)), this, SLOT(slot_openConnection(QString, int, QString, QString)));
	//connect(this, SIGNAL(signal_closeConnection()), this, SLOT(slot_closeConnection()));

	// Non blocking calls
	//
	connect(this, &DbStore::signal_upgradeProject, this, &DbStore::slot_upgradeProject);

	// Blocking calls
	//
	connect(this, &DbStore::signal_debug, this, &DbStore::slot_debug, Qt::BlockingQueuedConnection);

	connect(this, &DbStore::signal_isProjectOpen, this, &DbStore::slot_isProjectOpen, Qt::BlockingQueuedConnection);
	connect(this, &DbStore::signal_openProject, this, &DbStore::slot_openProject, Qt::BlockingQueuedConnection);
	connect(this, &DbStore::signal_closeProject, this, &DbStore::slot_closeProject, Qt::BlockingQueuedConnection);
	connect(this, &DbStore::signal_getProjectList, this, &DbStore::slot_getProjectList);
	connect(this, &DbStore::signal_createProject, this, &DbStore::slot_createProject, Qt::BlockingQueuedConnection);

	connect(this, &DbStore::signal_createUser, this, &DbStore::slot_createUser, Qt::BlockingQueuedConnection);
	connect(this, &DbStore::signal_updateUser, this, &DbStore::slot_updateUser, Qt::BlockingQueuedConnection);
	connect(this, &DbStore::signal_getUserList, this, &DbStore::slot_getUserList, Qt::BlockingQueuedConnection);

	connect(this, &DbStore::signal_getFileList, this, &DbStore::slot_getFileList, Qt::BlockingQueuedConnection);
	connect(this, &DbStore::signal_getFileHistory, this, &DbStore::slot_getFileHistory);
	connect(this, &DbStore::signal_addFiles, this, &DbStore::slot_addFiles);

	connect(this, &DbStore::signal_undoFilesPendingChanges, this, &DbStore::slot_undoFilesPendingChanges);
	connect(this, &DbStore::signal_checkInFiles, this, &DbStore::slot_checkInFiles);
	connect(this, &DbStore::signal_checkOutFiles, this, &DbStore::slot_checkOutFiles);

	connect(this, &DbStore::signal_getWorkcopy, this, &DbStore::slot_getWorkcopy);
	connect(this, &DbStore::signal_setWorkcopy, this, &DbStore::slot_setWorkcopy);

	connect(this, &DbStore::signal_getLatestCopy, this, &DbStore::slot_getLatestCopy);
	connect(this, &DbStore::signal_getSpecificCopy, this, &DbStore::slot_getSpecificCopy);

	connect(this, &DbStore::signal_addSystem, this, &DbStore::slot_addSystem);
	connect(this, &DbStore::signal_getEquipmentWorkcopy, this, &DbStore::slot_getEquipmentWorkcopy);

	// Starts an event loop, and emits workerThread->started()
	//
	m_pThread->start();

	return;
}

DbStore::~DbStore()
{
	qDebug() << Q_FUNC_INFO;

	// Stop thread
	//
	if (m_pThread != nullptr)
	{
		m_pThread->exit();
		m_pThread->wait(10000);
		m_pThread = nullptr;
	}
	else
	{
		assert(m_pThread != nullptr);
	}

	return;
}

DbStore* DbStore::create()
{
	DbStore* object = new DbStore();
	return object;
}

void DbStore::destroy()
{
	deleteLater();
}

int DbStore::databaseVersion() const
{
	return sizeof(upgradeItems) / sizeof(upgradeItems[0]);
}

void DbStore::upgradeProject(const QString& projectName, QWidget* parentWidget)
{
/*	assert(parentWidget);

	DbProgress progress;

	QProgressDialog dp(tr("Upgrading project %1").arg(projectName), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_upgradeProject("u7_" + projectName, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}*/

	return;
}


bool DbStore::debug()
{
	emit signal_debug();
	return true;
}

bool DbStore::isProjectOpened()
{
	bool result = emit signal_isProjectOpen();
	return result;
}

// Get u7 project list from the database, BLOCKING CONNECTION
//
bool DbStore::getProjectList(std::vector<DbProject>* projects, QWidget* parentWidget)
{
	assert(parentWidget != nullptr);

	// --
	//
	bool ok = initProgress(parentWidget, tr("Getting projects list"), 1);
	if (ok == true)
	{
		emit signal_getProjectList(projects);

		bool result = runProgress();

		qDebug() << "DbStore::getProjectList run stop";
		return result;
	}

	return false;
}

void DbStore::openProject(const QString& projectName, const QString &username, const QString &password)
{
	emit signal_openProject(projectName, username, password);
}

void DbStore::closeProject()
{
	emit signal_closeProject();
}

// Create new project
//
bool DbStore::createProject(const QString& projectName, const QString& administratorPassword)
{
	if (projectName.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		return false;
	}

	emit signal_createProject(projectName, administratorPassword);
	return true;
}

void DbStore::createUser(const DbUser& user)
{
	emit signal_createUser(user);
	return;
}

void DbStore::updateUser(const DbUser& user)
{
	emit signal_updateUser(user);
	return;
}

void DbStore::getUserList(std::vector<DbUser>& users)
{
	emit signal_getUserList(users);
	return;
}

void DbStore::getFileList(std::vector<DbFileInfo> &files)
{
	emit signal_getFileList(files, false, QString());
	return;
}

void DbStore::getFileList(std::vector<DbFileInfo>& files, bool justCheckedInState, const QString& filter)
{
	emit signal_getFileList(files, justCheckedInState, filter);
	return;
}

void DbStore::getFileInfo(int fileId, DbFileInfo* out)
{
	if (fileId == -1 ||
		out == nullptr)
	{
		assert(fileId != -1);
		assert(out != nullptr);
		*out = DbFileInfo();
		out->setFileId(fileId);
		return;
	}

	// Function must be optimized in in future, ask for fileId file, not all files
	//
	std::vector<DbFileInfo> files;
	getFileList(files);

	for (auto f = files.begin(); f != files.end(); ++f)
	{
		if (f->fileId() == fileId)
		{
			*out = *f;
			return;
		}
	}

	qDebug() << "DbStore::getFileInfo: File with id " << fileId << " is not found.";

	*out = DbFileInfo();
	out->setFileId(fileId);

	return;
}

void DbStore::getFileHistory(const DbFileInfo& file, std::vector<DbChangesetInfo>* out, QWidget* parentWidget)
{
/*	if (file.fileId() == -1 || out == nullptr || parentWidget == nullptr)
	{
		assert(file.fileId() != -1);
		assert(out != nullptr);
		assert(parentWidget != nullptr);

		return;
	}

	DbProgress progress;
	progress.setMaxValue(1);

	QProgressDialog dp(tr("Getting file history..."), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_getFileHistory(file, out, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}*/

	return;
}

// Select all checked-in files or checked out by current user
//
void DbStore::getCurrentUserAndCheckedInFileList(std::vector<DbFileInfo>& files, const QString& filter)
{
/*	std::vector<DbFileInfo> allFiles;
	std::vector<DbFileInfo> checkedInFiles;

	emit signal_getFileList(allFiles, false, filter);			// Get all files
	emit signal_getFileList(checkedInFiles, true, filter);		// Get latest check-in files

	files.clear();

	for (unsigned int i = 0; i < allFiles.size(); i++)
	{
		const DbFileInfo& file = allFiles[i];

		if (file.state() == VcsState::CheckedIn)
		{
			files.push_back(file);
			continue;
		}

		if (file.state() == VcsState::CheckedOut && file.user() == currentUser())
		{
			files.push_back(file);
			continue;
		}

		// File checked-out by other user, get the latest check-in file version (if there is any)
		//
		for (unsigned int f = 0; f < checkedInFiles.size(); f++)
		{
			if (checkedInFiles[f].fileId() == file.fileId())
			{
				files.push_back(checkedInFiles[f]);
				break;
			}
		}

		// --
		//
	}

	return;*/
}

void DbStore::addFiles(std::vector<std::shared_ptr<DbFile>>* files, QWidget* parentWidget)
{
/*	assert(files != nullptr);
	assert(files->size() != 0);
	assert(parentWidget != nullptr);

	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files->size()));

	QProgressDialog dp(tr("Adding files to the database..."), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_addFiles(files, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}

	return;*/
}

bool DbStore::undoFilesPendingChanges(const std::vector<DbFileInfo>& files, QWidget* parentWidget)
{
/*	assert(files.size() != 0);
	assert(parentWidget != nullptr);

	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("Undo pending changes"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_undoFilesPendingChanges(files, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
		return false;
	}

	if (progress.wasCanceled())
	{
		return false;
	}
*/
	return true;
}

bool DbStore::checkInFiles(const std::vector<DbFileInfo>& files, const QString& comment, QWidget* parentWidget)
{
/*	assert(files.size() != 0);
	assert(parentWidget != nullptr);

	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("CheckingIn files"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_checkInFiles(files, comment, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
		return false;
	}

	if (progress.wasCanceled())
	{
		return false;
	}
*/
	return true;
}

bool DbStore::checkOutFiles(const std::vector<DbFileInfo>& files, QWidget* parentWidget)
{
/*	assert(files.size() != 0);
	assert(parentWidget != nullptr);

	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("CheckingOut files"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_checkOutFiles(files, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
		return false;
	}

	if (progress.wasCanceled())
	{
		return false;
	}
*/
	return true;
}

bool DbStore::getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget)
{
/*	assert(files.size() != 0);
	assert(out != nullptr);
	assert(parentWidget != nullptr);

	qDebug() << Q_FUNC_INFO;

	// --
	//
	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("Getting workcopy"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_getWorkcopy(files, out, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}

	return !progress.hasError();*/

	return false;
}

std::shared_ptr<DbFile> DbStore::getWorkcopy(const DbFileInfo& file, QWidget* parentWidget)
{
/*	std::vector<DbFileInfo> files;
	files.push_back(file);

	std::vector<std::shared_ptr<DbFile>> out;

	bool result = getWorkcopy(files, &out, parentWidget);

	if (result == true && out.size() == 1)
	{
		return out.front();
	}
	else
	{
		return std::shared_ptr<DbFile>();
	}
	*/


	return std::shared_ptr<DbFile>();
}

bool DbStore::setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, QWidget* parentWidget)
{
/*	assert(files.empty() == false);
	assert(parentWidget != nullptr);

	qDebug() << Q_FUNC_INFO;

	// --
	//
	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("Setting workcopy"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_setWorkcopy(files, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();

		return false;
	}

	if (progress.wasCanceled() == true)
	{
		return false;
	}

	return true;*/

	return false;
}

bool DbStore::setWorkcopy(const std::shared_ptr<DbFile>& file, QWidget* parentWidget)
{
	std::vector<std::shared_ptr<DbFile>> files;
	files.push_back(file);

	return setWorkcopy(files, parentWidget);
}

void DbStore::getLatestCopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget)
{
/*	assert(files.size() != 0);
	assert(out != nullptr);
	assert(parentWidget != nullptr);

	// --
	//
	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("Getting latest copy"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_getLatestCopy(files, out, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}
*/
	return;
}

bool DbStore::getSpecificCopy(int changesetId, const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget)
{
/*	if (changesetId < 0 ||
		files.empty() == true ||
		out == nullptr ||
		parentWidget == nullptr)
	{
		assert(changesetId >= 0);
		assert(files.size() != 0);
		assert(out != nullptr);
		assert(parentWidget != nullptr);

		return false;
	}

	// --
	//
	DbProgress progress;
	progress.setMaxValue(static_cast<int>(files.size()));

	QProgressDialog dp(tr("Getting file copy"), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_getSpecificCopy(changesetId, files, out, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}

	return !progress.hasError();*/

	return false;
}

bool DbStore::addSystem(DeviceSystem* system, QWidget* parentWidget)
{
/*	if (system == nullptr ||
		parentWidget == nullptr)
	{
		assert(system != nullptr);
		assert(parentWidget != nullptr);

		return false;
	}

	// --
	//
	DbProgress progress;
	progress.setMaxValue(1);

	QProgressDialog dp(tr("Adding system..."), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_addSystem(system, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}

	return !progress.hasError();*/

	return false;
}

bool DbStore::getEquipmentWorkcopy(DeviceRoot* out, QWidget* parentWidget)
{
/*	if (out == nullptr ||
		parentWidget == nullptr)
	{
		assert(out != nullptr);
		assert(parentWidget != nullptr);

		return false;
	}

	// --
	//
	DbProgress progress;
	progress.setMaxValue(1);

	QProgressDialog dp(tr("Getting equipment workcopy..."), tr("Cancel"), 0, progress.maxValue(), parentWidget);
	dp.setWindowModality(Qt::WindowModal);
	dp.setMinimumDuration(100);

	emit signal_getEquipmentWorkcopy(out, &progress);

	while (progress.completed() == false)
	{
		QThread::yieldCurrentThread();
		QCoreApplication::processEvents();

		dp.setValue(progress.value());
		if (dp.wasCanceled() == true)
		{
			progress.setCancel(true);
		}
	}

	dp.reset();

	if (progress.hasError() == true)
	{
		QMessageBox mb;
		mb.setText(progress.errorMessage());
		mb.exec();
	}

	return !progress.hasError();*/

	return false;
}

QSqlDatabase DbStore::openPostgresDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "postgres");
	if (db.lastError().isValid() == true)
	{
		return db;
	}

	db.setHostName(host());
	db.setPort(port());
	db.setDatabaseName("postgres");
	db.setUserName(serverUsername());
	db.setPassword(serverPassword());

	bool ok = db.open();

	if (ok == true)
	{
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


		if (hasTransaction == false ||
			hasQuerySize == false ||
			hasBlob == false ||
			hasUnicode == false ||
			hasPreparedQueries == false)
		{
			emitError(tr("Database driver doesn't have required features."));

			db.close();
			return QSqlDatabase();	// !!!!!!!!!!!!!!!!!!!!!!11 we return the empty database, cant get error from it!
		}
	}

	return db;
}

bool DbStore::closePostgresDatabase()
{
	if (QSqlDatabase::contains("postgres") == false)
	{
		assert(QSqlDatabase::contains("postgres"));
		return false;
	}

	{
		QSqlDatabase db = QSqlDatabase::database("postgres");
		if (db.isOpen() == true)
		{
			db.close();
		}
	}

	QSqlDatabase::removeDatabase("postgres");
	return true;
}


const QString& DbStore::projectConnectionName() const
{
	static const QString pcn = "ProjectConnection";
	return pcn;
}

QSqlDatabase DbStore::projectDatabase()
{
	assert(QSqlDatabase::contains(projectConnectionName()));
	return QSqlDatabase::database(projectConnectionName(), false);
}

// Must be called from the GUI thread
//
bool DbStore::initProgress(QWidget* parentWidget, const QString& description, int maxValue)
{
	if (m_operationMutex.tryLock() == false)		// MUST BE UNLOCKED LATER (in DbStore::runProgress!!!)
	{
		qDebug() << "DbStore: Another operation is in progress!";
		return false;
	}

	return m_progress.init(parentWidget, description, maxValue);
}

bool DbStore::runProgress()
{
	// Must be called from the GUI thread
	//
	bool result = m_progress.run();
	m_operationMutex.unlock();						// WAS LOCKED IN DbStore::initProgress
	return result;
}

void DbStore::emitError(const QSqlError& err)
{
	emitError(err.text());
}

void DbStore::emitError(const QString& err)
{
	qDebug() << err;
	m_progress.setErrorMessage(err);
}

void DbStore::slot_debug()
{
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		return;
	}

	{
//		QString savefilerequest = "INSERT INTO binary_file (data) VALUES (E'\\\\x";
//		savefilerequest.reserve(100001000);
//		for (int i = 0; i < 100000000; i++)
//		{
//			int b = i % 256;
//			savefilerequest += QString::number(b, 16).leftJustified(2, '0');
//		}

//		savefilerequest += "');";

//		//--
//		//
//		QSqlQuery query(db);
//		bool result = query.exec(savefilerequest);

//		if (result == false)
//		{
//			qDebug() << Q_FUNC_INFO << db.lastError();
//			return;
//		}
	}

	// get data
	//
	{
//		QString getfile = "SELECT data FROM binary_file WHERE id='41a66b74-a0e5-11e2-9945-080027a27470';";

//		//--
//		//
//		QSqlQuery query(db);
//		bool result = query.exec(getfile);

//		if (result == false)
//		{
//			qDebug() << Q_FUNC_INFO << db.lastError();
//			return;
//		}

//		if (query.next() == true)
//		{
//			QByteArray data = query.value("data").toByteArray();
//			int size = data.size();

//			int bytet255 = static_cast<unsigned int>(data[255]);
//			int bytet254 = data[254];
//			int i = 0;
//		}
	}

	return;
}

bool DbStore::slot_isProjectOpen()
{
	return QSqlDatabase::contains(projectConnectionName());
}

void DbStore::slot_getProjectList(std::vector<DbProject>* projects)
{
	assert(projects != nullptr);

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	DbProgress& progress = m_progress;
	std::shared_ptr<int*> progressCompleted(nullptr, [&progress](void*)
		{
			progress.setCompleted(true);
		});


	// Open database "postgres", and get project list
	//
	{
		QSqlDatabase db = openPostgresDatabase();
		if (db.isOpen() == false)
		{
			emitError(db.lastError());
			return;
		}

		QSqlQuery query(db);

		bool result = query.exec("SELECT datname FROM pg_database;");
		if (result == false)
		{
			emitError(query.lastError());
			return;
		}

		while (query.next())
		{
			QString databaseName = query.value(0).toString();
			QString projectName = databaseName;

			// filter database list
			//
			if (projectName.left(3) != "u7_" &&
				projectName.left(3) != "U7_")
			{
				continue;
			}

			projectName.replace("u7_", "", Qt::CaseInsensitive);

			// --
			//
			DbProject p;
			p.setDatabaseName(databaseName);
			p.setProjectName(projectName);

			projects->push_back(p);
		}
	}

	closePostgresDatabase();

	// Open each project and get it's version
	//

	for (auto pi = projects->begin(); pi != projects->end(); ++pi)
	{
		QString projectDatabaseConnectionName = QString("%1 connection").arg(pi->projectName());

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
				QSqlDatabase::removeDatabase(projectDatabaseConnectionName);
				break;
			}

			// Get project version
			//

			// Request is:
			//	SELECT max("VersionNo") FROM "Version";
			//

			QString createVersionTableSql = QString("SELECT max(\"VersionNo\") FROM \"Version\";");

			QSqlQuery versionQuery(projectDb);
			result = versionQuery.exec(createVersionTableSql);

			if (result == false)
			{
				//			qDebug() << versionQuery.lastError();
				//			emit error(versionQuery.lastError().text());

				//			versionQuery.clear();
				//			projectDb.close();
				QSqlDatabase::removeDatabase(projectDatabaseConnectionName);
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

		QSqlDatabase::removeDatabase(projectDatabaseConnectionName);
	}

	// --
	//
	return;
}

void DbStore::slot_openProject(QString projectName, QString username, QString password)
{
/*	projectName = projectName.trimmed();
	QString projectDatabaseName = "u7_" + projectName.toLower();
	username = username.trimmed();

	if (projectName.isEmpty() || username.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(username.isEmpty() == false);

		emit error(tr("OpenProject error, projectname and username musn't be empty. ProjectName is %1, Username is %2").arg(projectName).arg(username));
		return;
	}

	if (QSqlDatabase::contains(projectConnectionName()) == true)
	{
		emit error(tr("OpenProject error, another project is opened. To open a new project, please close the current project."));
		return;
	}

	// Open database
	//
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", projectConnectionName());

	db.setHostName(m_host);
	db.setDatabaseName(projectDatabaseName);
	db.setUserName(m_serverUsername);
	db.setPassword(m_serverPassword);

	bool result = db.open();

	if (result == false)
	{
		emit error(tr("Open Project %1 error. %2").arg(projectName).arg(db.lastError().text()));

		QSqlDatabase::removeDatabase(projectConnectionName());
		return;
	}

	// Check username and password
	//
	QSqlQuery query(db);
	result = query.exec(QString("SELECT \"GetUserID\"('%1', '%2');").arg(username).arg(password));

	if (result == false)
	{
		emit error(query.lastError().text());
		QSqlDatabase::removeDatabase(projectConnectionName());
		return;
	}

	if (query.next() == false)
	{
		emit error(tr("Internal error. Can't check username and password."));

		db.close();
		QSqlDatabase::removeDatabase(projectConnectionName());
		return;
	}

	int userId = query.value(0).toInt();

	if (userId <= 0)
	{
		emit error(tr("Can't open project. Wrong username or password, please try agin with correct values"));

		db.close();
		QSqlDatabase::removeDatabase(projectConnectionName());
		return;
	}

	// Set user data
	//
	DbUser user;
	result = db_getUserData(db, userId, &user);

	if (result == false)
	{
		assert(result);

		emit error(tr("Can't read user data ") + db.lastError().text());

		db.close();
		QSqlDatabase::removeDatabase(projectConnectionName());
		return;
	}

	setCurrentUser(user);

	// Set project data
	//
	DbProject project;

	project.setDatabaseName(projectDatabaseName);
	project.setProjectName(projectName);

	setCurrentProject(project);

	// Send notifications
	//
	emit projectOpened();*/
	return;
}

void DbStore::slot_closeProject()
{
/*	setCurrentUser(DbUser());
	setCurrentProject(DbProject());

	if (QSqlDatabase::contains(projectConnectionName()) == false)
	{
		emit error(tr("Project is not opened."));
		return;
	}

	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		emit error(tr("Project database connection is closed."));
		return;
	}

	db.close();
	QSqlDatabase::removeDatabase(projectConnectionName());

	emit projectClosed();*/
}

// Create databse for new project.
// The database name is u7_projectName.
// The first version of the database consist of two tables, with one record in each.
// Version table has one record with value 1.
// User table has has user 'admin'.
//
void DbStore::slot_createProject(const QString projectName, const QString administratorPassword)
{
	// Open database "postgres", and get project list
	//
/*	QSqlDatabase db = openPostgresDatabase();
	if (db.isOpen() == false)
	{
		return;
	}

	// --
	//

	QString databaseName = "u7_" + projectName;
	databaseName = databaseName.toLower();

	QSqlQuery query(db);

	// Create databse
	//
	QString createDatabaseSql = QString("CREATE DATABASE %1 WITH ENCODING='UTF8' CONNECTION LIMIT=-1;").arg(databaseName);

	bool result = query.exec(createDatabaseSql);

	if (result == false)
	{
		qDebug() << query.lastError();
		emit error(query.lastError().text());
		closePostgresDatabase();
		return;
	}

	// connect to newly created database
	//
	QString newDatabaseConnectionName = QString("%1 connection").arg(databaseName);

	QSqlDatabase newDatabase = QSqlDatabase::addDatabase("QPSQL", newDatabaseConnectionName);
	newDatabase.setHostName(db.hostName());
	newDatabase.setPort(db.port());
	newDatabase.setDatabaseName(databaseName);
	newDatabase.setUserName(db.userName());
	newDatabase.setPassword(db.password());
	result = newDatabase.open();

	if (result == false)
	{
		qDebug() << newDatabase.lastError();
		emit error(newDatabase.lastError().text());

		query.exec(QString("DROP DATABASE %1;").arg(databaseName));

		QSqlDatabase::removeDatabase(newDatabaseConnectionName);
		closePostgresDatabase();
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
		qDebug() << newDbQuery.lastError();
		emit error(newDbQuery.lastError().text());

		newDatabase.close();
		QSqlDatabase::removeDatabase(newDatabaseConnectionName);

		query.exec(QString("DROP DATABASE %1;").arg(databaseName));

		closePostgresDatabase();
		return;
	}

	// Add first record to version table
	//
	QString addFirstVersionRecord = QString(
		"INSERT INTO \"Version\" (\"VersionNo\", \"Reasone\")"
		"VALUES (1, 'Create a project');");

	result = newDbQuery.exec(addFirstVersionRecord);
	if (result == false)
	{
		qDebug() << newDbQuery.lastError();
		emit error(newDbQuery.lastError().text());

		newDatabase.close();
		QSqlDatabase::removeDatabase(newDatabaseConnectionName);

		query.exec(QString("DROP DATABASE %1;").arg(databaseName));

		closePostgresDatabase();
		return;
	}

	// Create User table
	//


//	CREATE TABLE "User"
//	(
//		"UserID" serial PRIMARY KEY NOT NULL,
//		"Date" timestamp with time zone NOT NULL DEFAULT now(),
//		"Username" text NOT NULL,
//		"FirstName" text NOT NULL,
//		"LastName" text NOT NULL,
//		"Password" text NOT NULL,
//		"Administrator" boolean NOT NULL DEFAULT FALSE,
//		"ReadOnly" boolean NOT NULL DEFAULT TRUE
//	);


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
		qDebug() << newDbQuery.lastError();
		emit error(newDbQuery.lastError().text());

		newDatabase.close();
		QSqlDatabase::removeDatabase(newDatabaseConnectionName);

		query.exec(QString("DROP DATABASE %1;").arg(databaseName));

		closePostgresDatabase();
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
		qDebug() << newDbQuery.lastError();
		emit error(newDbQuery.lastError().text());

		newDbQuery.clear();
		newDatabase.close();
		QSqlDatabase::removeDatabase(newDatabaseConnectionName);

		query.exec(QString("DROP DATABASE %1;").arg(databaseName));

		closePostgresDatabase();
		return;
	}

	// Close all databases
	//
	newDbQuery.clear();
	newDatabase.close();
	QSqlDatabase::removeDatabase(newDatabaseConnectionName);
	closePostgresDatabase();*/
	return;
}

// Upgrade the project database to the appropriate version
//
void DbStore::slot_upgradeProject(const QString databaseName, DbProgress* progress)
{
/*	assert(progress != nullptr);

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	// Open database
	//
	QString dbConnectionName = QString("%1 connection").arg(databaseName);

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", dbConnectionName);
	db.setHostName(host());
	db.setPort(port());
	db.setDatabaseName(databaseName);
	db.setUserName(serverUsername());
	db.setPassword(serverPassword());

	bool result = db.open();

	if (result == false)
	{
		qDebug() << db.lastError();

		errorMessage += tr("\r\nDatabase connection is closed. Error: %1").arg(db.lastError().text());
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	// Start transaction
	//
	result = db.transaction();
	if (result == false)
	{
		errorMessage += tr(" Cannot begin transaction for database %1.").arg(databaseName);
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		db.close();
		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	// Get project version
	//

	// Lock Version table
	// LOCK TABLE "Version" IN ACCESS EXCLUSIVE MODE NOWAIT;
	//
	QSqlQuery versionQuery(db);

	result = versionQuery.exec("LOCK TABLE \"Version\" IN ACCESS EXCLUSIVE MODE NOWAIT;");

	if (result == false)
	{
		qDebug() << versionQuery.lastError();

		errorMessage += versionQuery.lastError().text();
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		versionQuery.clear();

		db.rollback();
		db.close();
		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	// Request is:
	//	SELECT max("VersionNo") FROM "Version";
	//
	result = versionQuery.exec("SELECT max(\"VersionNo\") FROM \"Version\";");

	if (result == false)
	{
		qDebug() << versionQuery.lastError();

		errorMessage += versionQuery.lastError().text();
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		versionQuery.clear();

		db.rollback();
		db.close();
		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	if (versionQuery.next() == false)
	{
		qDebug() << "Cannot get project database version.";

		errorMessage += tr("Cannot get project database version.");
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		versionQuery.clear();

		db.rollback();
		db.close();
		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	int projectVersion = versionQuery.value(0).toInt();

	progress->setMaxValue(std::max(databaseVersion() - projectVersion, 0));
	progress->setValue(0);

	versionQuery.clear();

	// Some checks
	//
	if (projectVersion == databaseVersion())
	{
		errorMessage = tr("Database %1 is up to date.").arg(databaseName);
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		db.rollback();
		db.close();
		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	if (projectVersion > databaseVersion())
	{
		errorMessage = tr("Database %1 is newer than the software version.").arg(databaseName);
		if (progress != nullptr)
		{
			progress->setErrorMessage(errorMessage);
		}

		db.rollback();
		db.close();
		QSqlDatabase::removeDatabase(dbConnectionName);
		return;
	}

	// Upgrade database
	//

	bool upgradeResult = true;

	for (int i = projectVersion; i < databaseVersion(); i++)
	{
		progress->setValue(i - projectVersion);

		const UpgradeItem& ui = upgradeItems[i];

		// Perform upgade action
		//
//        if (QFile::exists(ui.upgradeFileName) == false)
//        {
//            qDebug() << "File " << ui.upgradeFileName << " doesn't exists.";

//            errorMessage += QString("File %1 doesn't exists.").arg(ui.upgradeFileName);

//            upgradeResult = false;
//            break;
//        }

        QFile upgradeFile(ui.upgradeFileName);

        if (upgradeFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        {
            qDebug() << "Can't open file " << ui.upgradeFileName << " " << upgradeFile.errorString();

            errorMessage += QString("Can't open file %1.").arg(ui.upgradeFileName);

            upgradeResult = false;
            break;
        }

        QString upgradeScript = upgradeFile.readAll();

        QSqlQuery upgradeQuery(db);

        if (upgradeQuery.exec(upgradeScript) == false)
        {
            qDebug() << upgradeQuery.lastError().text();
            errorMessage += upgradeQuery.lastError().text();

            upgradeResult = false;
            break;
        }

        // Add record to Version table
		//
		QString addVersionRecord = QString(
			"INSERT INTO \"Version\" (\"VersionNo\", \"Reasone\")"
			"VALUES (%1, '%2');").arg(i + 1).arg(ui.text);

		QSqlQuery versionQuery(db);

		result = versionQuery.exec(addVersionRecord);
		if (result == false)
		{
			qDebug() << versionQuery.lastError().text();
			errorMessage += versionQuery.lastError().text();

			upgradeResult = false;
			break;
		}
	}

	// Commit transaction.
	// Note: For some databases, the commit/rollback will fail and return false if there is an active query using the database for a SELECT. Make the query inactive before doing the commit.
	//
	if (upgradeResult == true)
	{
		result = db.commit();	// Commit upgrade transaction

		if (result == false)
		{
			qDebug() << db.lastError().text();

			errorMessage += db.lastError().text();
			if (progress != nullptr)
			{
				progress->setErrorMessage(errorMessage);
			}

			db.close();
			QSqlDatabase::removeDatabase(dbConnectionName);
			return;
		}

		// Ok
		//
		errorMessage = tr("Database %1 has been successfully upgraded.").arg(databaseName);
	}
	else
	{
		result = db.rollback();	// Rollback upgrade transaction

		if (result == false)
		{
			qDebug() << db.lastError().text();

			errorMessage += db.lastError().text();
			if (progress != nullptr)
			{
				progress->setErrorMessage(errorMessage);
			}

			db.close();
			QSqlDatabase::removeDatabase(dbConnectionName);
			return;
		}
	}

	if (progress != nullptr)
	{
		progress->setErrorMessage(errorMessage);
	}

	// --
	//
	db.close();
	QSqlDatabase::removeDatabase(dbConnectionName);

	return;*/
}

void DbStore::slot_createUser(DbUser user)
{
/*	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		return;
	}

	if (currentUser().isAdminstrator() == false)
	{
		emit error(tr("Current user doesn't have administrator privileges."));
		return;
	}

	// Start transaction
	//
	bool result = db.transaction();

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();

		emit error(tr("Can't create user %1, error: %2").arg(user.username()).arg(db.lastError().text()));
		return;
	}

	// Check if such user already exists
	// SELECT "UserID" FROM "User" WHERE "Username"='Administrator';
	//
	QSqlQuery query(db);
	result = query.exec(QString("SELECT \"UserID\" FROM \"User\" WHERE \"Username\"='%1';").arg(user.username()));

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		emit error(tr("Can't create user %1, error: %2").arg(user.username()).arg(db.lastError().text()));

		db.rollback();
		return;
	}

	if (query.size() > 0)
	{
		result = query.next();
		assert(result);

		int userID = query.value("UserID").toInt();

		if (userID != 0)
		{
			emit error(tr("User %1 already exists").arg(user.username()));

			db.rollback();
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
		qDebug() << query.lastError().text();
		emit error(tr("Create user error: %1").arg(query.lastError().text()));

		db.rollback();
		return;
	}

	// Commit transaction
	//
	result = db.commit();

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		emit error(tr("Can't create user %1, error: %2").arg(user.username()).arg(db.lastError().text()));
	}
*/
	return;
}

void DbStore::slot_updateUser(DbUser user)
{
/*	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		return;
	}

	if (currentUser().username() != user.username() && currentUser().isAdminstrator() == false)
	{
		assert(false);
		emit error("Only administrators can change other users' details.");
		return;
	}

	// Start transaction
	//
	bool result = db.transaction();

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		emit error(tr("Can't start user update transaction, error details: %1").arg(db.lastError().text()));
		return;
	}

	// Check if such user already exists
	// SELECT "UserID", "Password" FROM "User" WHERE "Username"=user.username();
	//
	QSqlQuery query(db);
	result = query.exec(QString("SELECT \"UserID\", \"Password\" FROM \"User\" WHERE \"Username\"='%1';").arg(user.username()));

	if (result == false || query.size() != 1)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		emit error(tr("Can't update user data, error: %1").arg(db.lastError().text()));

		db.rollback();
		return;
	}

	result = query.next();
	assert(result);

	int userID = query.value("UserID").toInt();
	QString password = query.value("Password").toString();

	if (userID == 0)
	{
		emit error(tr("User %1 is not exists").arg(user.username()));

		db.rollback();
		return;
	}

	bool updatePassword = false;

	if (user.newPassword().isEmpty() == false)
	{
		if (user.password() != password)
		{
			emit error(tr("Wrong old password."));

			db.rollback();
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
		qDebug() << query.executedQuery();
		qDebug() << query.lastError().text();
		emit error(tr("Update user data error: %1").arg(query.lastError().text()));

		db.rollback();
		return;
	}

	// Commit transaction
	//
	result = db.commit();

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		emit error(tr("Can't update user %1 data, error: %2").arg(user.username()).arg(db.lastError().text()));
	}

	return;*/
}

void DbStore::slot_getUserList(std::vector<DbUser> &users)
{
/*	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
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

	users.clear();

	while (q.next())
	{
		DbUser user;
		int userId = q.value(0).toInt();

		bool ok = db_getUserData(db, userId, &user);

		if (ok == true)
		{
			users.push_back(user);
		}
	}

	return;*/
}

bool DbStore::db_getUserData(QSqlDatabase& db, int userId, DbUser* pUser)
{
/*	if (pUser == nullptr)
	{
		assert(pUser != nullptr);
		return false;
	}

	QSqlQuery query(db);

	QString sql = QString(
		"SELECT \"Date\", \"Username\", \"FirstName\", \"LastName\", \"Password\", \"Administrator\", \"ReadOnly\", \"Disabled\" "
			"FROM \"User\" "
			"WHERE \"UserID\"=%1;"
		).arg(userId);

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

	pUser->setUserId(userId);

	pUser->setUsername(query.value("Username").toString());
	pUser->setFirstName(query.value("FirstName").toString());
	pUser->setLastName(query.value("LastName").toString());
	pUser->setPassword(query.value("Password").toString());
	pUser->setAdministrator(query.value("Administrator").toBool());
	pUser->setReadonly(query.value("ReadOnly").toBool());
	pUser->setDisabled(query.value("Disabled").toBool());

	*/
	return true;
}

//
//
void DbStore::slot_getFileList(std::vector<DbFileInfo> &files, bool justCheckedInState, QString filter)
{
/*	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		return;
	}

	// SELECT FileID, Name, Created FROM File WHERE Name ILIKE '%filter' ORDER BY name;
	//
	QString request;

	if (filter.isEmpty() == true)
	{
		request = "SELECT FileID, Name, Created FROM File ORDER BY name;";
	}
	else
	{
		request = QString("SELECT FileID, Name, Created FROM File WHERE Name ILIKE '%%1' ORDER BY name;").arg(filter);
	}

	QSqlQuery q(db);

	bool result = q.exec(request);

	if (result == false)
	{
		qDebug() << Q_FUNC_INFO << q.lastError();
		emit error(tr("Can't get file list. Error: ") +  q.lastError().text());
		return;
	}

	files.clear();
	while (q.next())
	{
		DbFileInfo fileInfo;

		int fileId = q.value("FileID").toInt();
		QString fileName  = q.value("Name").toString();
		QString fileCreated = q.value("Created").toString();

		int fileSize = 0;
		///int changesetId = 0;
		QString lastCheckIn;


		// Get last FileInstance
		//
		QString fileInstanceRequest;

		if (justCheckedInState == true)
		{
			fileInstanceRequest = QString(
				" SELECT "
				"	FileInstanceID, FileID, ChangesetID, Size, Created "
				" FROM "
				"	FileInstance "
				" WHERE "
				"   ChangesetID IS NOT NULL AND"
				"   Sequence = (SELECT max(Sequence) FROM FileInstance WHERE ChangesetID IS NOT NULL AND FileID=%1); "
				).arg(fileId);
		}
		else
		{
			fileInstanceRequest = QString(
				" SELECT "
				"	FileInstanceID, FileID, ChangesetID, Size, Created "
				" FROM "
				"	FileInstance "
				" WHERE "
				"	Sequence = (SELECT max(Sequence) FROM FileInstance WHERE FileID=%1); "
				).arg(fileId);
		}

		QSqlQuery fq(db);

		if (fq.exec(fileInstanceRequest) == false)
		{
			qDebug() << fq.lastError();
			assert(false);
			continue;
		}

		if (fq.next() == true)
		{
			fileSize = fq.value("Size").toInt();
			///changesetId = fq.value("ChangesetID").toInt();
			lastCheckIn = fq.value("Created").toString();
		}
		else
		{
			// File was not check in yet
			//
			continue;
		}

		// Get file state
		//

		// The file lateset checkIn
		//

//			SELECT * FROM Changeset
//			WHERE
//				ChangesetID =
//					(SELECT ChangesetID FROM FileInstance
//						WHERE
//							FileID=6 AND
//							ChangesetID IS NOT NULL AND
//							Sequence = (SELECT max(Sequence) FROM FileInstance WHERE FileID=6 AND ChangesetID IS NOT NULL)
//					);


		QString lastCheckInRequest = QString(
			"SELECT * FROM Changeset "
			"	WHERE "
			"		ChangesetID = "
			"			(SELECT ChangesetID FROM FileInstance "
			"				WHERE "
			"					FileID=%1 AND "
			"					ChangesetID IS NOT NULL AND "
			"					Sequence = (SELECT max(Sequence) FROM FileInstance WHERE FileID=%1 AND ChangesetID IS NOT NULL) "
			"			 ); "
			).arg(fileId);

		QSqlQuery lc(db);

		if (lc.exec(lastCheckInRequest) == false)
		{
			qDebug() << lc.lastError();
			assert(false);
			return;
		}

		bool hasCheckIn = lc.next();		// Now lc points to the last check in record for the file

		// The file's Checked Out state
		//
		if (justCheckedInState == true)
		{
			// COPY PASTE TO THE BOTTOM !!!!!!!!!!!!!!!!!!! HERE
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			if (hasCheckIn == false)
			{
				assert(hasCheckIn == true);		// There is a file which does not have any record in CheckOut table, so it must be checked-in at least once
			}
			else
			{
				int userId = lc.value("UserID").toInt();
				DbUser user;

				db_getUserData(db, userId, &user);
				fileInfo.setUser(user);

				fileInfo.setState(VcsState::CheckedIn);
				lastCheckIn = lc.value("time").toString();
			}
		}
		else
		{
			QString stateRequest = QString("SELECT UserID FROM CheckOut WHERE FileID=%1;").arg(fileId);
			QSqlQuery sq(db);

			if (sq.exec(stateRequest) == false)
			{
				qDebug() << sq.lastError();
				assert(false);
				return;
			}

			if (sq.next() == true)
			{
				// File currently is checked Out
				//
				int userId = sq.value("UserID").toInt();
				DbUser user;

				db_getUserData(db, userId, &user);
				fileInfo.setUser(user);

				fileInfo.setState(VcsState::CheckedOut);
			}
			else
			{
				// COPY PASTE TO THE BOTTOM !!!!!!!!!!!!!!!!!!! HERE
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (hasCheckIn == false)
				{
					assert(hasCheckIn == true);		// There is a file which does not have any record in CheckOut table, so it must be checked-in at least once
				}
				else
				{
					int userId = lc.value("UserID").toInt();
					DbUser user;

					db_getUserData(db, userId, &user);
					fileInfo.setUser(user);

					fileInfo.setState(VcsState::CheckedIn);
					lastCheckIn = lc.value("time").toString();
				}
			}
		}

		// Add file to the list
		//
		fileInfo.setFileId(fileId);
		fileInfo.setFileName(fileName);
		fileInfo.setSize(fileSize);
		fileInfo.setCreated(fileCreated);
		fileInfo.setLastCheckIn(lastCheckIn);

		files.push_back(fileInfo);
	}

	return;*/
}

void DbStore::slot_getFileHistory(DbFileInfo file, std::vector<DbChangesetInfo>* out, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (file.fileId() == -1 || out == nullptr)
	{
		assert(file.fileId() != -1);
		assert(out != nullptr);
		return;
	}

	out->clear();

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());

		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setMaxValue(1);
	progress->setValue(0);

	// Get data from the database
	//
	QString request;
	QSqlQuery q(db);

	if (progress->wasCanceled() == true)
	{
		return;
	}

//	SELECT
//		Changeset.ChangesetID AS ChangesetID,
//		Changeset.UserID AS UserID,
//		Changeset.Time AS Time,
//		Changeset.Comment AS Comment
//	FROM
//		FileInstance, Changeset
//	WHERE
//		FileInstance.FileID = 8 AND
//		FileInstance.ChangesetID =  Changeset.ChangesetID
//	ORDER BY Changeset.ChangesetID DESC;

	request = QString(
		" SELECT "
		"   Changeset.ChangesetID AS ChangesetID, "
		"   Changeset.UserID AS UserID, "
		"   Changeset.Time AS Time, "
		"   Changeset.Comment AS Comment "
		" FROM "
		"   FileInstance, Changeset "
		" WHERE "
		"   FileInstance.FileID = %1 AND "
		"   FileInstance.ChangesetID =  Changeset.ChangesetID "
		" ORDER BY Changeset.ChangesetID DESC; "
		).arg(file.fileId());

	if (q.exec(request) == false)
	{
		qDebug() << Q_FUNC_INFO << q.lastError();

		progress->setErrorMessage(tr("\r\nFile %1 was not pocessed. Reasone: %2").arg(file.fileName()).arg(q.lastError().text()));
		return;
	}

	while (q.next() == true)
	{
		DbChangesetInfo ci;

		ci.setChangeset(q.value("ChangesetID").toInt());
		ci.setDate(q.value("Time").toString());
		ci.setComment(q.value("Comment").toString());

		DbUser user;
		db_getUserData(db, q.value("UserID").toInt(), &user);
		ci.setUser(user);

		out->push_back(ci);
	}

	// --
	//
	progress->setErrorMessage(errorMessage);

	return;*/
}

void DbStore::slot_addFiles(std::vector<std::shared_ptr<DbFile>>* files, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (files == nullptr || files->empty() == true)
	{
		assert(files != nullptr);
		assert(files->empty() != true);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());

		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setMaxValue(static_cast<int>(files->size()));
	progress->setValue(0);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files->size(); i++)
	{
		QString request;
		QSqlQuery q(db);

		std::shared_ptr<DbFile> file = files->at(i);

		progress->setValue(i);
		progress->setCurrentOperation(tr("Adding file %1").arg(file->fileName()));

		if (progress->wasCanceled() == true)
		{
			break;
		}

		// Start transaction
		//
		if (db.transaction() == false)
		{
			qDebug() << Q_FUNC_INFO << db.lastError();
			errorMessage += tr("\r\nFile %1 was not pocessed. Reasone: Can't start database transaction.").arg(file->fileName());

			continue;
		}

		// Insert record to the file table
		// INSERT INTO File (Name) VALUES ('file.fileName()');
		//
		request = QString("INSERT INTO File (Name) VALUES ('%1');").arg(file->fileName());

		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not pocessed. Reasone: %2").arg(file->fileName()).arg(q.lastError().text());

			continue;
		}
		q.clear();

		// Get FileID from created record
		//
		//
		request = QString("SELECT FileID FROM File WHERE Name='%1';").arg(file->fileName());

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not pocessed. Reasone: %2").arg(file->fileName()).arg(q.lastError().text());

			db.rollback();
			continue;
		}

		if (q.next() == false)
		{
			qDebug() << Q_FUNC_INFO << "Can't find just created file record.";
			errorMessage += tr("\r\nFile %1 was not pocessed. Reasone: Can't find just created file record.").arg(file->fileName());

			db.rollback();
			continue;
		}

		int fileId = q.value("FileID").toInt();
		q.clear();

		// Add record to the CheckOut table
		// INSERT INTO CheckOut (UserID, FileID) VALUES (%1, %2);
		//
		request = QString("INSERT INTO CheckOut (UserID, FileID) VALUES (%1, %2);").arg(currentUser().userId()).arg(fileId);

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not pocessed. Reasone: %2").arg(file->fileName()).arg(q.lastError().text());

			db.rollback();
			continue;
		}
		q.clear();

		// Add record to the FileInstance table
		// INSERT INTO FileInstance (FileID, Size, Data) VALUES (%1, %2, E'\\x....');
		//
		request = QString("INSERT INTO FileInstance (FileID, Size, data) VALUES (%1, %2, ").arg(fileId).arg(file->size());

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);
		data.clear();

		request += ");";

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not pocessed. Reasone: %2").arg(file->fileName()).arg(q.lastError().text());

			db.rollback();
			continue;
		}
		q.clear();

		// Commit transaction;
		//
		db.commit();

		// Set file descriptions
		//
		file->setFileId(fileId);
		file->setUser(currentUser());
		file->setState(VcsState::CheckedOut);		// Set file state to CheckedOut
	}

	progress->setErrorMessage(errorMessage);

	return;*/
}

void DbStore::slot_undoFilesPendingChanges(const std::vector<DbFileInfo>& files, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (files.empty() == true)
	{
		assert(files.empty() != true);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());

		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	// Progress
	//
	progress->setMaxValue(static_cast<int>(files.size()));
	progress->setValue(0);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		progress->setValue(i);

		if (progress->wasCanceled() == true)
		{
			break;
		}

		// --
		//
		QString request;
		QSqlQuery q(db);

		int fileId = files[i].fileId();

		// Start transaction
		//
		if (db.transaction() == false)
		{
			qDebug() << Q_FUNC_INFO << db.lastError();
			errorMessage += tr("\r\nFileId %1 was not pocessed. Reasone: Can't start database transaction.").arg(files[i].fileName());
			return;
		}

		// Insert record to the file table
		// SELECT UndoFilePendingChanges(%1, %2);
		//
		request = QString("SELECT UndoFilePendingChanges(%1, %2);").arg(fileId).arg(m_currentUser.userId());

		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not processed. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());

			continue;
		}
		q.clear();

		// Commit transaction;
		//
		db.commit();
	}

	progress->setErrorMessage(errorMessage);

	return;*/
}

void DbStore::slot_checkInFiles(const std::vector<DbFileInfo>& files, QString comment, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (files.empty() == true)
	{
		assert(files.empty() != true);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());

		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	// Set progress
	//
	progress->setMaxValue(static_cast<int>(files.size()));
	progress->setValue(0);

	// Start transaction
	//
	if (db.transaction() == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		progress->setErrorMessage(tr("Can't start database transaction. Reasone: %1").arg(db.lastError().text()));
		return;
	}

	// INSERT INTO Changeset (UserID, Comment, File) VALUES (:UserID, :Comment, TRUE) RETURNING ChangesetID;
	//
	QString request = QString("INSERT INTO Changeset (UserID, Comment, File) VALUES (%1, '%2', TRUE) RETURNING ChangesetID;").
			  arg(m_currentUser.userId()).
			  arg(comment);

	QSqlQuery q(db);

	if (q.exec(request) == false)
	{
		db.rollback();

		qDebug() << Q_FUNC_INFO << q.lastError();
		progress->setErrorMessage(tr("Can't CheckIn File(s). Reasone: %1").arg(q.lastError().text()));
		return;
	}

	if (q.next() == false)
	{
		db.rollback();

		progress->setErrorMessage("Can't find ChangesetID.");
		return;
	}

	int changesetId = q.value(0).toInt();		// From the RETURNING clause.....

	q.clear();

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		progress->setValue(i);
		if (progress->wasCanceled() == true)
		{
			db.rollback();
			break;
		}

		QString request;
		int fileId = files[i].fileId();

		// Set ChangesetID in FileInstance table
		//
		request = QString("UPDATE FileInstance SET ChangesetID=%1 WHERE FileID=%2 AND ChangesetID IS NULL RETURNING FileInstanceID;").arg(changesetId).arg(fileId);
		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("Can't CheckIn file %1. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());

			progress->setErrorMessage(errorMessage);
			return;
		}

		if (q.size() != 1)
		{
			// Record was not updated
			//
			db.rollback();

			errorMessage += tr("Can't CheckIn file %1. Can't update FileInstance record.").arg(files[i].fileName());
			progress->setErrorMessage(errorMessage);

			return;
		}

		// Delete row frowm the CheckOut table
		//
		request = QString("DELETE FROM CheckOut WHERE FileID=%1;").arg(fileId);
		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("Can't CheckIn file %1. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());

			progress->setErrorMessage(errorMessage);
			return;
		}

		q.clear();
	}

	// Commit transaction;
	//
	db.commit();

	progress->setErrorMessage(errorMessage);
	return;*/
}

void DbStore::slot_checkOutFiles(const std::vector<DbFileInfo>& files, DbProgress* progress)
{
/*	assert(progress != nullptr);

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (files.empty() == true)
	{
		assert(files.empty() != true);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());

		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	// Set progress
	//
	progress->setMaxValue(static_cast<int>(files.size()));
	progress->setValue(0);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		progress->setValue(i);

		if (progress->wasCanceled() == true)
		{
			db.rollback();
			break;
		}

		// Start transaction
		//
		if (db.transaction() == false)
		{
			qDebug() << Q_FUNC_INFO << db.lastError();
			progress->setErrorMessage(tr("Can't start database transaction. Reasone: %1").arg(db.lastError().text()));
			return;
		}

		QString request;
		QSqlQuery q(db);

		int fileId = files[i].fileId();

		// Check if file is not checked out already
		// SELECT CheckOutID FROM CheckOut WHERE FileID=:FileID;
		//
		request = QString("SELECT CheckOutID FROM CheckOut WHERE FileID=%1;").arg(fileId);

		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("Can't CheckOut File %1. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());

			continue;
		}

		if (q.size() != 0)
		{
			// Apparentkly file already CheckedOut;
			//
			db.rollback();

			errorMessage += tr("File %1 already has been Checked Out.").arg(files[i].fileName());

			continue;
		}

		// Add record to the CheckOutTable
		// INSERT INTO CheckOut (UserID, FileID) VALUES (:UserID, :FileID);
		//
		request = QString("INSERT INTO CheckOut (UserID, FileID) VALUES (%1, %2)").arg(currentUser().userId()).arg(fileId);

		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("Can't CheckOut File %1. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());

			continue;
		}

		// Add record to the FileInstance table;
		// INSERT INTO
		//		FileInstance (Data, Size, FileID, ChangesetID)
		// VALUES
		// (
		//		(SELECT Data FROM FileInstance WHERE FileID=%1 AND Sequence=(SELECT MAX(Sequence) FROM FileInstance WHERE FileID=%1)),
		//		(SELECT Size FROM FileInstance WHERE FileID=%1 AND Sequence=(SELECT MAX(Sequence) FROM FileInstance WHERE FileID=%1)),
		//		%1,
		//		NULL
		// );
		//

		request = QString(
			" INSERT INTO																												   "
			"	FileInstance (Data, Size, FileID, ChangesetID)																			   "
			" VALUES																													   "
			" (																															   "
			" 		(SELECT Data FROM FileInstance WHERE FileID=%1 AND Sequence=(SELECT MAX(Sequence) FROM FileInstance WHERE FileID=%1)), "
			"		(SELECT Size FROM FileInstance WHERE FileID=%1 AND Sequence=(SELECT MAX(Sequence) FROM FileInstance WHERE FileID=%1)), "
			"		%1,																													   "
			"		NULL																												   "
			" );																														   "
			).arg(fileId);

		if (q.exec(request) == false)
		{
			db.rollback();

			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("Can't CheckOut File %1. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());

			continue;
		}

		// Commit transaction;
		//
		db.commit();
	}

	if (progress != nullptr)
	{
		progress->setErrorMessage(errorMessage);
	}

	return;*/
}



void DbStore::slot_getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	if (files.empty() == true)
	{
		assert(files.empty() != true);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setCurrentOperation(tr("Getting files..."));
	progress->setMaxValue(static_cast<int>(files.size()));
	progress->setValue(0);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		progress->setValue(i);
		if (progress->wasCanceled() == true)
		{
			break;
		}

		// --
		//
		QString request;
		QSqlQuery q(db);

		int fileId = files[i].fileId();

		// Select fileinfo and file instance, if ChangesetID is NULL than it's workcopy.
		//
		// SELECT
		//	 File.FileID AS FileID, File.Name AS FileName, File.Created AS Created,
		//	 FileInstance.Size AS Size, FileInstance.Data as Data,
		//	 Checkout.Time As ChechoutTime, Checkout.UserID AS UserID
		// FROM
		//	 File, FileInstance, Checkout
		// WHERE
		//	 File.FileID=%1 AND
		//	 FileInstance.FileID=%1 AND FileInstance.ChangesetID IS NULL AND
		//	 Checkout.FileID=%1;

		request = QString(
			"SELECT "
			"	File.FileID AS FileID, File.Name AS FileName, File.Created AS Created, "
			"	FileInstance.Size AS Size, FileInstance.Data as Data, "
			"	Checkout.Time As ChechoutTime, Checkout.UserID AS UserID "
			"FROM "
			"	File, FileInstance, Checkout "
			"WHERE "
			"	File.FileID=%1 AND "
			"	FileInstance.FileID=%1 AND FileInstance.ChangesetID IS NULL AND "
			"	Checkout.FileID=%1;").arg(fileId);

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not processed. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());
			continue;
		}

		if (q.next() == false)
		{
			errorMessage += tr("Can't find workcopy, file %1").arg(files[i].fileName());
			continue;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		file->setFileName(q.value("FileName").toString());
		file->setFileId(q.value("FileID").toInt());
		file->setChangeset(-1);		// There is no ChangesetID or I have to get latest chngeset for the file?????
		file->setCreated(q.value("Created").toString());
		file->setState(VcsState::CheckedOut);
		QByteArray data = q.value("Data").toByteArray();
		file->swapData(data);

		DbUser user;
		int userId = q.value("UserID").toInt();

		if (db_getUserData(db, userId, &user) == true)
		{
			file->setUser(user);
		}
		else
		{
			errorMessage += tr("Can't get user data, UserID=%1").arg(userId);
			continue;
		}

		out->push_back(file);
	}

	progress->setErrorMessage(errorMessage);
	return;*/
}

void DbStore::slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, DbProgress* progress)
{
/*	assert(progress != nullptr);

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (files.empty() == true)
	{
		assert(files.empty() == false);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	if (progress != nullptr)
	{
		progress->setCurrentOperation(tr("Setting files..."));
		progress->setMaxValue(static_cast<int>(files.size()));
		progress->setValue(0);

	}

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		if (progress != nullptr)
		{
			progress->setValue(i);

			if (progress->wasCanceled() == true)
			{
				break;
			}
		}

		// --
		//
		QString request;
		QSqlQuery q(db);

		const std::shared_ptr<DbFile>& file = files.at(i);

		// Select file instance, if ChangesetID is NULL than it's workcopy.
		//
		// UPDATE FileInstance SET Size=???, Data=E'\\x.......' WHERE FileId=??? AND ChangesetID IS NULL;
		//
		request = QString("UPDATE FileInstance SET Size=%1, Data=").arg(file->size());

		QString data;
		file->convertToDatabaseString(&data);
		request.append(data);

		request += QString(" WHERE FileId=%1 AND ChangesetID IS NULL;").arg(file->fileId());

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFileID %1 was not pocessed. Reasone: %2").arg(file->fileId()).arg(q.lastError().text());
			continue;
		}
	}

	if (progress != nullptr)
	{
		progress->setErrorMessage(errorMessage);
	}

	return;*/
}

void DbStore::slot_getLatestCopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	if (files.empty() == true)
	{
		assert(files.empty() != true);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen());
		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setCurrentOperation(tr("Getting files..."));
	progress->setMaxValue(static_cast<int>(files.size()));
	progress->setValue(0);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		progress->setValue(i);
		if (progress->wasCanceled() == true)
		{
			break;
		}

		// --
		//
		QString request;
		QSqlQuery q(db);

		int fileId = files[i].fileId();

//		SELECT
//			File.FileID AS FileID,
//			File.Name AS FileName,
//			File.Created AS Created,
//			FileInstance.Size AS Size,
//			FileInstance.Data as Data,
//			Changeset.ChangesetID AS ChangesetID,
//			Changeset.Time As ChangesetTime,
//			Changeset.UserID AS UserID
//		FROM
//			 File, FileInstance, Changeset
//		 WHERE
//			 File.FileID=%1 AND
//			 FileInstance.FileID=%1 AND
//			 FileInstance.ChangesetID=(SELECT max(ChangesetID) FROM FileInstance WHERE FileID=%1 AND ChangesetID IS NOT NULL) AND
//			 Changeset.ChangesetID=FileInstance.ChangesetID;

		request = QString(
			" SELECT "
				" File.FileID AS FileID, "
				" File.Name AS FileName, "
				" File.Created AS Created, "
				" FileInstance.Size AS Size, "
				" FileInstance.Data as Data, "
				" Changeset.ChangesetID AS ChangesetID, "
				" Changeset.Time As ChangesetTime, "
				" Changeset.UserID AS UserID "
			" FROM "
				 " File, FileInstance, Changeset "
			" WHERE "
				 " File.FileID=%1 AND "
				 " FileInstance.FileID=%1 AND "
				 " FileInstance.ChangesetID=(SELECT max(ChangesetID) FROM FileInstance WHERE FileID=%1 AND ChangesetID IS NOT NULL) AND "
				 " Changeset.ChangesetID=FileInstance.ChangesetID;"
			).arg(fileId);

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not processed. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());
			continue;
		}

		if (q.next() == false)
		{
			errorMessage += tr("Can't find file %1").arg(files[i].fileName());
			continue;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		file->setFileName(q.value("FileName").toString());
		file->setFileId(q.value("FileID").toInt());
		file->setChangeset(q.value("ChangesetID").toInt());
		file->setCreated(q.value("Created").toString());
		file->setLastCheckIn(q.value("ChangesetTime").toString());
		file->setState(VcsState::CheckedIn);
		QByteArray data = q.value("Data").toByteArray();
		file->swapData(data);

		DbUser user;
		int userId = q.value("UserID").toInt();

		if (db_getUserData(db, userId, &user) == true)
		{
			file->setUser(user);
		}
		else
		{
			errorMessage += tr("Can't get user data, UserID=%1").arg(userId);
			continue;
		}

		out->push_back(file);
	}

	progress->setErrorMessage(errorMessage);
	return;*/
}

void DbStore::slot_getSpecificCopy(int changesetId, const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (out == nullptr ||
		files.empty() == true ||
		changesetId < 0)
	{
		assert(out != nullptr);
		assert(files.empty() != true);
		assert(changesetId >= 0);

		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen() == true);
		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setCurrentOperation(tr("Getting files..."));
	progress->setMaxValue(static_cast<int>(files.size()));
	progress->setValue(0);

	// Iterate through files
	//
	for (unsigned int i = 0; i < files.size(); i++)
	{
		progress->setValue(i);
		progress->setCurrentOperation(tr("Getting file %1").arg(files[i].fileName()));
		if (progress->wasCanceled() == true)
		{
			break;
		}

		// --
		//
		QString request;
		QSqlQuery q(db);

		int fileId = files[i].fileId();

//		SELECT
//			File.FileID AS FileID,
//			File.Name AS FileName,
//			File.Created AS Created,
//			FileInstance.Size AS Size,
//			FileInstance.Data as Data,
//			Changeset.ChangesetID AS ChangesetID,
//			Changeset.Time As ChangesetTime,
//			Changeset.UserID AS UserID
//		FROM
//			 File, FileInstance, Changeset
//		 WHERE
//			 File.FileID=:FileID AND
//			 FileInstance.FileID=:FileID AND
//			 FileInstance.ChangesetID=(SELECT max(ChangesetID) FROM FileInstance WHERE FileID=:FileID AND ChangesetID <= :ChangesetID) AND
//			 Changeset.ChangesetID=FileInstance.ChangesetID;

		request = QString(
			" SELECT "
				" File.FileID AS FileID, "
				" File.Name AS FileName, "
				" File.Created AS Created, "
				" FileInstance.Size AS Size, "
				" FileInstance.Data as Data, "
				" Changeset.ChangesetID AS ChangesetID, "
				" Changeset.Time As ChangesetTime, "
				" Changeset.UserID AS UserID "
			" FROM "
				 " File, FileInstance, Changeset "
			" WHERE "
				 " File.FileID = %1 AND "
				 " FileInstance.FileID = %1 AND "
				 " FileInstance.ChangesetID = (SELECT max(ChangesetID) FROM FileInstance WHERE FileID = %1 AND ChangesetID <= %2) AND "
				 " Changeset.ChangesetID = FileInstance.ChangesetID;"
			).arg(fileId).arg(changesetId);

		if (q.exec(request) == false)
		{
			qDebug() << Q_FUNC_INFO << q.lastError();
			errorMessage += tr("\r\nFile %1 was not processed. Reasone: %2").arg(files[i].fileName()).arg(q.lastError().text());
			continue;
		}

		if (q.next() == false)
		{
			errorMessage += tr("Can't find file %1").arg(files[i].fileName());
			continue;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		file->setFileName(q.value("FileName").toString());
		file->setFileId(q.value("FileID").toInt());
		file->setChangeset(q.value("ChangesetID").toInt());
		file->setCreated(q.value("Created").toString());
		file->setLastCheckIn(q.value("ChangesetTime").toString());
		file->setState(VcsState::CheckedIn);
		QByteArray data = q.value("Data").toByteArray();
		file->swapData(data);

		DbUser user;
		int userId = q.value("UserID").toInt();

		if (db_getUserData(db, userId, &user) == true)
		{
			file->setUser(user);
		}
		else
		{
			errorMessage += tr("Can't get user data, UserID=%1").arg(userId);
			continue;
		}

		out->push_back(file);
	}

	progress->setErrorMessage(errorMessage);
	return;*/
}

void DbStore::slot_addSystem(DeviceSystem* system, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (system == nullptr)
	{
		assert(system != nullptr);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());

    if (db.isOpen() == false)
	{
		assert(db.isOpen() == true);
		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setCurrentOperation(tr("Adding system..."));
	progress->setMaxValue(1);
	progress->setValue(0);

    QString request;

    request = QString("SELECT addsystem(%1, '%2', '%3')")
        .arg(currentUser().userId())
        //.arg(VcsItemAction::Added)
        .arg(system->strId())
        .arg(system->caption());

    QSqlQuery q(db);

    if (q.exec(request) == false)
    {
        qDebug() << Q_FUNC_INFO << q.lastError();
        errorMessage += tr("\r\nCan't add system, %1").arg(q.lastError().text());
        progress->setErrorMessage(errorMessage);
        return;
    }



	// Start transaction
	//
	if (db.transaction() == false)
	{
		qDebug() << Q_FUNC_INFO << db.lastError();
		errorMessage += tr("Can't start database transaction, %1").arg(db.lastError().text());
		progress->setErrorMessage(errorMessage);
		return;
	}
	// Add system to the database
	//

	QString request;
	QSqlQuery q(db);

	// Get max SystemID
	//
	if (q.exec("SELECT max(SystemID) AS MaxSystemID FROM SystemInst;") == false)
	{
		db.rollback();

		qDebug() << Q_FUNC_INFO << q.lastError();
		errorMessage += tr("\r\nCan't get max(SystemID) %1").arg(q.lastError().text());
		progress->setErrorMessage(errorMessage);
		return;
	}

	int systemId = 1;
	if (q.size() == 1)
	{
		q.next();
		systemId = q.value("MaxSystemID").toInt() + 1;
	}

	// Add SystemInstance
	//
	request = QString(
		" INSERT INTO "
		"	 SystemInst (SystemID, ChangesetID, State, StrID, Caption) "
		" VALUES "
		" (%1, NULL, %2, '%3', '%4') "
		" RETURNING SystemInstID;")
		.arg(systemId)
		.arg(VcsItemAction::Added)
		.arg(system->strId())
		.arg(system->caption());

	if (q.exec(request) == false)
	{
		db.rollback();

		qDebug() << Q_FUNC_INFO << q.lastError();
		errorMessage += tr("\r\nCan't add system, %1").arg(q.lastError().text());
		progress->setErrorMessage(errorMessage);
		return;
	}

	int systemInstId = 1;
	if (q.size() == 1)
	{
		q.next();
		systemInstId = q.value("SystemInstID").toInt();
	}
	else
	{
		db.rollback();

		assert(q.size() != 1);
		progress->setErrorMessage("Can't add system, there is no SystemInstID.");
		return;
	}

	// Add SystemInstance
	//
	request = QString("INSERT INTO CheckOut(UserID, SystemInstID)  VALUES (%1, %2);").arg(currentUser().userId()).arg(systemInstId);
	if (q.exec(request) == false)
	{
		db.rollback();

		qDebug() << Q_FUNC_INFO << q.lastError();
		errorMessage += tr("\r\nCan't checkout system, %1").arg(q.lastError().text());
		progress->setErrorMessage(errorMessage);
		return;
	}

	db.commit();

	progress->setErrorMessage(errorMessage);
	return;*/
}

void DbStore::slot_getEquipmentWorkcopy(DeviceRoot* out, DbProgress* progress)
{
/*	if (progress == nullptr)
	{
		assert(progress != nullptr);
		return;
	}

	// Automatic scope variable to perform progress->setCompleted(true)
	//
	std::shared_ptr<int*> progressCompleted(nullptr, [progress](void*)
		{
			progress->setCompleted(true);
		});

	QString errorMessage;

	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	// --
	//
	QSqlDatabase db = QSqlDatabase::database(projectConnectionName());
	if (db.isOpen() == false)
	{
		assert(db.isOpen() == true);
		errorMessage += tr("\r\nDatabase connection is closed.");
		return;
	}

	progress->setCurrentOperation(tr("Adding system..."));
	progress->setMaxValue(1);
	progress->setValue(0);

	// ...
	//
	QString request;
	QSqlQuery q(db);

//	// Get max SystemID
//	//
//	if (q.exec("SELECT max(SystemID) AS MaxSystemID FROM SystemInst;") == false)
//	{
//		qDebug() << Q_FUNC_INFO << q.lastError();
//		errorMessage += tr("\r\nCan't get max(SystemID) %1").arg(q.lastError().text());
//		progress->setErrorMessage(errorMessage);
//		return;
//	}

//	int systemId = 1;
//	if (q.size() == 1)
//	{
//		q.next();
//		systemId = q.value("MaxSystemID").toInt() + 1;
//	}

	progress->setErrorMessage(errorMessage);*/
	return;
}


const QString& DbStore::host() const
{
	QMutexLocker locker(&m_mutex);
	return m_host;
}

void DbStore::setHost(const QString& host)
{
	QMutexLocker locker(&m_mutex);
	m_host = host;
}

int DbStore::port() const
{
	QMutexLocker locker(&m_mutex);
	return m_port;
}

void DbStore::setPort(int port)
{
	QMutexLocker locker(&m_mutex);
	m_port = port;
}

const QString& DbStore::serverUsername() const
{
	QMutexLocker locker(&m_mutex);
	return m_serverUsername;
}

void DbStore::setServerUsername(const QString& username)
{
	QMutexLocker locker(&m_mutex);
	m_serverUsername = username;
}

const QString& DbStore::serverPassword() const
{
	QMutexLocker locker(&m_mutex);
	return m_serverPassword;
}

void DbStore::setServerPassword(const QString& password)
{
	QMutexLocker locker(&m_mutex);
	m_serverPassword = password;
}

DbUser DbStore::currentUser() const
{
	QMutexLocker locker(&m_mutex);
	return m_currentUser;
}

void DbStore::setCurrentUser(const DbUser &user)
{
	QMutexLocker locker(&m_mutex);
	m_currentUser = user;
}

DbProject DbStore::currentProject() const
{
	QMutexLocker locker(&m_mutex);
	return m_currentProject;
}

void DbStore::setCurrentProject(const DbProject& project)
{
	QMutexLocker locker(&m_mutex);
	m_currentProject = project;
}

HasDbStore::HasDbStore()
{
	assert(false);
}

HasDbStore::HasDbStore(DbStore* dbstore) :
	m_pDbStore(dbstore)
{
	assert(m_pDbStore);
}

DbStore* HasDbStore::dbstore()
{
	return m_pDbStore;
}
