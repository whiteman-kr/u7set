#include "DbMetrologyConnection.h"

namespace Metrology
{

	DbConnectionBase::DbConnectionBase(QWidget* parent) :
		ConnectionBase(parent),
		m_parentWidget(parent)
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

	void DbConnectionBase::setDbController(DbController* dbController)
	{
		TEST_PTR_RETURN(dbController);

		m_dbController = dbController;
	}

	// -------------------------------------------------------------------------------------------------------------------

	std::shared_ptr<DbFile> DbConnectionBase::getConnectionFile()
	{
		TEST_PTR_RETURN_NULLPTR(m_dbController);

		bool result = false;

		std::shared_ptr<DbFile> file;
		std::vector<DbFileInfo> fileList;

		int etcFileId = m_dbController->systemFileId(DbDir::EtcDir);

		result = m_dbController->getFileList(&fileList, etcFileId, File::METROLOGY_CONNECTIONS_CSV, true, m_parentWidget);

		if (result == false || fileList.size() != 1)
		{
			// if it does not exists, then create a file
			//
			std::shared_ptr<DbFile> newFile = std::make_shared<DbFile>();
			newFile->setFileName(File::METROLOGY_CONNECTIONS_CSV);

			result = m_dbController->addFile(newFile, etcFileId, nullptr);

			if (result == false)
			{
				return nullptr;
			}

			result = m_dbController->getFileList(&fileList, etcFileId, File::METROLOGY_CONNECTIONS_CSV, true, m_parentWidget);

			if (result == false || fileList.size() != 1)
			{
				return nullptr;
			}

			// RPCT-3927
			//
			// Checkin file MetrologyConnections.csv immediately after creation
			// to show this file for all users

			result = m_dbController->checkIn(fileList[0], QString("File %1 created").arg(File::METROLOGY_CONNECTIONS_CSV), m_parentWidget);

			if (result == false)
			{
				return nullptr;
			}

			// ... and immediately checkout for editing by current user

			result = m_dbController->checkOut(fileList[0], m_parentWidget);

			if (result == false)
			{
				return nullptr;
			}
		}

		result = m_dbController->getLatestVersion(fileList[0], &file, nullptr);

		if (result == false || file == nullptr)
		{
			return nullptr;
		}

		file->setState(fileList[0].state());

		return file;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::load()
	{
		TEST_PTR_RETURN_FALSE(m_dbController);

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile();
		TEST_PTR_RETURN_FALSE(file);

		//
		//
		if (file->state() == E::VcsState::CheckedOut)
		{
			int userId = file->userId();

			// currentUser
			//
			if (userId != m_dbController->currentUser().userId())
			{
				m_enableEditBase = false;
			}

			m_userIsAdmin = m_dbController->currentUser().isAdministrator();

			// user of file
			//
			m_userName = m_dbController->username(userId);
		}

		// read CSV-data from file
		//
		QByteArray data;
		file->swapData(data);

		// load connections from CSV-data
		//
		m_connectionList = connectionsFromCsvData(data);

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::save(bool checkIn, const QString& comment)
	{
		TEST_PTR_RETURN_FALSE(m_dbController);

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
		std::shared_ptr<DbFile> file = getConnectionFile();
		TEST_PTR_RETURN_FALSE(file);

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
		if (m_dbController->setWorkcopy(file, nullptr) == false)
		{
			return false;
		}

		// check in file
		//
		if (checkIn == true)
		{
			if (m_dbController->checkIn(*file, comment, nullptr) == false)
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
		TEST_PTR_RETURN_FALSE(m_dbController);

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile();
		TEST_PTR_RETURN_FALSE(file);

		// check out file
		//
		if (file->state() == E::VcsState::CheckedOut)
		{
			return true;
		}

		bool result = m_dbController->checkOut(*file, nullptr);
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool DbConnectionBase::isCheckIn()
	{
		TEST_PTR_RETURN_FALSE(m_dbController);

		// open connection file
		//
		std::shared_ptr<DbFile> file = getConnectionFile();
		TEST_PTR_RETURN_FALSE(file);

		// test checked in
		//
		bool result = file->state() == E::VcsState::CheckedIn;
		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::setAction(int index, const E::VcsItemAction& type)
	{
		if (index < 0 || index >= TO_INT(m_connectionList.size()))
		{
			return;
		}

		m_connectionList[static_cast<quint64>(index)].setAction(type);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void DbConnectionBase::removeAllMarked()
	{
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
		quint64 connectionCount = m_connectionList.size();
		for(quint64 index = 0; index < connectionCount; index++)
		{
			m_connectionList[index].setRestoreID(static_cast<int>(index));
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	Connection DbConnectionBase::connectionFromChekedIn(int restoreID)
	{
		TEST_PTR_RETURN_VALUE(m_dbController, Connection());

		// file
		//
		std::shared_ptr<DbFile> file = getConnectionFile();
		TEST_PTR_RETURN_VALUE(file, Connection());

		// get last changeset of file
		//
		std::vector<DbChangeset> changesetList;
		m_dbController->getFileHistory(*file, &changesetList, nullptr);

		if (changesetList.size() == 0)
		{
			return Connection();
		}

		// read last Checked In file
		//
		std::shared_ptr<DbFile> fileOut;
		bool result = m_dbController->getSpecificCopy(*file, changesetList[0].changeset(), &fileOut, nullptr);
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
