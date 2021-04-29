#include "DbMetrologyConnection.h"

namespace Metrology
{

	DbConnectionBase::DbConnectionBase(QObject* parent) :
	    ConnectionBase(parent)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::clear()
	{
		ConnectionBase::clear();

		m_enableEditBase = true;
		m_userIsAdmin = false;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::setSignalSetProvider(AppSignalSetProvider* signalSetProvider)
	{
		if (signalSetProvider == nullptr)
		{
			Q_ASSERT(signalSetProvider);
			return;
		}

		m_signalSetProvider = signalSetProvider;
	}

	// -------------------------------------------------------------------------------------------------------------------

	std::shared_ptr<DbFile> DbConnectionBase::getConnectionFile(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return nullptr;
		}

		bool result = false;

		std::shared_ptr<DbFile> file;
		std::vector<DbFileInfo> fileList;
		int etcFileId = db->systemFileId(DbDir::EtcDir);

		result = db->getFileList(&fileList, etcFileId, CONNECTIONS_FILE_NAME, true, nullptr);
		if (result == false || fileList.size() != 1)
		{
			// if it does not exists, then create a file
			//
			std::shared_ptr<DbFile> newFile = std::make_shared<DbFile>();
			newFile->setFileName(CONNECTIONS_FILE_NAME);

			result = db->addFile(newFile, etcFileId, nullptr);
			if (result == false)
			{
				return nullptr;
			}

			result = db->getFileList(&fileList, etcFileId, CONNECTIONS_FILE_NAME, true, nullptr);
			if (result == false || fileList.size() != 1)
			{
				return nullptr;
			}
		}

		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false || file == nullptr)
		{
			return nullptr;
		}

		file->setState(fileList[0].state());

		return file;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::load(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		//
		//
		if (file->state() == E::VcsState::CheckedOut)
		{
			int userId = file->userId();

			// currentUser
			//
			if (userId != db->currentUser().userId())
			{
				m_enableEditBase = false;
			}

			m_userIsAdmin = db->currentUser().isAdminstrator();

			// user of file
			//
			m_userName = db->username(userId);

		}

		// read CSV-data from file
		//
		QByteArray data;
		file->swapData(data);

		// load connections from CSV-data
		//
		m_connectionMutex.lock();
			m_connectionList = connectionsFromCsvData(data);
		m_connectionMutex.unlock();

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::save(bool checkIn, const QString& comment)
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return false;
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// only Admin can do changes if base is Checked Out
		//
		if (m_enableEditBase == false)
		{
			if (m_userIsAdmin == false)
			{
				return false;
			}
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		// file must be check out, after save
		//
		if (file->state() != E::VcsState::CheckedOut)
		{
			return true;
		}

		// delete all connections that marked as VcsItemAction::VcsItemActionType::Deleted
		// update all restoreID
		//
		if (checkIn == true)
		{
			removeAllMarked();
			updateRestoreIDs();
		}

		// create CSV-data and write to file
		//
		QByteArray data = csvDataFromConnections(true);
		file->swapData(data);

		// save file to database
		//
		if (db->setWorkcopy(file, nullptr) == false)
		{
			return false;
		}

		// check in file
		//
		if (checkIn == true)
		{
			if (db->checkIn(*file, comment, nullptr) == false)
			{
				return false;
			}
			else
			{
				m_enableEditBase = true;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::checkOut()
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return false;
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		// check out file
		//
		if (file->state() == E::VcsState::CheckedOut)
		{
			return true;
		}

		bool result = db->checkOut(*file, nullptr);
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::isCheckIn()
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return false;
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return false;
		}

		// test checked in
		//
		bool result = file->state() == E::VcsState::CheckedIn;
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::findSignal_in_signalSet()
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return;
		}

		QMutexLocker l(&m_connectionMutex);

		for(Connection& connection : m_connectionList)
		{
			// init signals
			//
			for(int ioType = 0; ioType < ConnectionIoTypeCount; ioType++)
			{
				if (connection.appSignalID(ioType).isEmpty() == true)
				{
					continue;
				}

				::AppSignal* pSignal = m_signalSetProvider->getSignalByStrID(connection.appSignalID(ioType));
				if (pSignal == nullptr)
				{
					continue;
				}

				m_signalSetProvider->loadSignal(pSignal->ID());

				connection.setSignal(ioType, pSignal);
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::setAction(int index, const E::VcsItemAction& type)
	{
		QMutexLocker l(&m_connectionMutex);

		if (index < 0 || index >= TO_INT(m_connectionList.size()))
		{
			return;
		}

		m_connectionList[static_cast<quint64>(index)].setAction(type);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::removeAllMarked()
	{
		QMutexLocker l(&m_connectionMutex);

		auto it = std::remove_if(m_connectionList.begin(), m_connectionList.end(), [](const Connection& connection)
		{
			if (connection.action() != E::VcsItemAction::Deleted)
			{
				return false;
			}

			return true;
		});

		m_connectionList.erase(it, m_connectionList.end());

		for(Connection& connection: m_connectionList)
		{
			connection.setAction(E::VcsItemAction::Unknown);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::updateRestoreIDs()
	{
		QMutexLocker l(&m_connectionMutex);

		quint64 connectionCount = m_connectionList.size();
		for(quint64 index = 0; index < connectionCount; index++)
		{
			m_connectionList[index].setRestoreID(static_cast<int>(index));
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	Connection DbConnectionBase::connectionFromChekedIn(int restoreID)
	{
		if (m_signalSetProvider == nullptr)
		{
			Q_ASSERT(m_signalSetProvider);
			return Connection();
		}

		DbController* db = m_signalSetProvider->dbController();
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return Connection();
		}

		// file
		//
		std::shared_ptr<DbFile> file = getConnectionFile(db);
		if (file == nullptr)
		{
			return Connection();
		}

		// get last changeset of file
		//
		std::vector<DbChangeset> changesetList;
		db->getFileHistory(*file, &changesetList, nullptr);

		if (changesetList.size() == 0)
		{
			return Connection();
		}

		// read last Checked In file
		//
		std::shared_ptr<DbFile> fileOut;

		bool result = db->getSpecificCopy(*file, changesetList[0].changeset(), &fileOut, nullptr);
		if (result == false)
		{
			return Connection();
		}

		// load connections from CSV-data
		//
		QByteArray data;
		fileOut->swapData(data);

		std::vector<Connection> connectionList = connectionsFromCsvData(data);

		// find connection with restoreID
		//
		Connection connectionForRestore;

		for(const Connection& connection: connectionList)
		{
			if (connection.restoreID() == restoreID)
			{
				connectionForRestore = connection;
				break;
			}
		}

		return connectionForRestore;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int DbConnectionBase::restoreConnection(int restoreID)
	{
		// find connection with restoreID
		//
		Connection connectionForRestore = connectionFromChekedIn(restoreID);

		// if connection is empty
		//
		if (connectionForRestore == Connection())
		{
			return -1;
		}

		// find and update connection
		//
		int connectionIndex = -1;

		QMutexLocker l(&m_connectionMutex);

		quint64 connectionCount = m_connectionList.size();
		for(quint64 index = 0; index < connectionCount; index++)
		{
			if (m_connectionList[index].restoreID() == restoreID)
			{
				m_connectionList[index] = connectionForRestore;

				connectionIndex = static_cast<int>(index);

				break;
			}
		}

		return connectionIndex;
	}

	// -------------------------------------------------------------------------------------------------------------------
}
