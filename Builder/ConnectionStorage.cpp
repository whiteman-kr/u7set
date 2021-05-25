#include "ConnectionStorage.h"

namespace Builder
{
	//
	// ConnectionStorage
	//
	ConnectionStorage::ConnectionStorage(DbController* db)
		 : DbObjectStorage(db, db->systemFileId(DbDir::ConnectionsDir))
	{
	}

	ConnectionStorage::~ConnectionStorage()
	{
	}

	std::vector<std::shared_ptr<Hardware::Connection>> ConnectionStorage::get(const QStringList& masks) const
	{
		if (masks.empty() == true)
		{
			return m_objectsVector;
		}

		std::vector<std::shared_ptr<Hardware::Connection>> result;
		result.reserve(m_objectsVector.size());

		for (std::shared_ptr<Hardware::Connection> connection : m_objectsVector)
		{
			for (const QString& mask : masks)
			{
				if (connection->connectionID().contains(mask, Qt::CaseInsensitive) == true ||
					connection->port1EquipmentID().contains(mask, Qt::CaseInsensitive) == true ||
					connection->port2EquipmentID().contains(mask, Qt::CaseInsensitive) == true)
				{
					result.push_back(connection);
				}
			}
		}

		return result;
	}

	QStringList ConnectionStorage::connectionIds() const
	{
		QStringList result;

		for (const std::shared_ptr<Hardware::Connection>& connection : m_objectsVector)
		{
			assert(connection);
			result.push_back(connection->connectionID());
		}

		return result;
	}

	QStringList ConnectionStorage::filterByMoudules(const QStringList& modules) const
	{
		std::set<QString> chassisConnectios;

		for (QString lm : modules)
		{
			// Let's assume that module (LM) has a Chassis parent, and LM's id is like SYSTEM_RACK_CHASSIS_LM.
			// Try to cut ID to chassis
			//
			int lastUnderscoreIndex = lm.lastIndexOf('_');
			if (lastUnderscoreIndex != -1)
			{
				lm = lm.left(lastUnderscoreIndex + 1);
			}

			std::vector<std::shared_ptr<Hardware::Connection>> lmConnections;
			lmConnections = get(QStringList() << lm.trimmed());

			for (const std::shared_ptr<Hardware::Connection>& c : lmConnections)
			{
				chassisConnectios.insert(c->connectionID());
			}
		}

		// --
		//
		QStringList result;
		result.reserve(static_cast<int>(chassisConnectios.size()));

		for (const QString& c : chassisConnectios)
		{
			result.push_back(c);
		}

		std::sort(result.begin(), result.end());

		return result;
	}

	std::shared_ptr<Hardware::Connection> ConnectionStorage::getPortConnection(QString portEquipmentId) const
	{
		for (const std::shared_ptr<Hardware::Connection>& connection : m_objectsVector)
		{
			assert(connection);

			if (connection != nullptr &&
				(connection->port1EquipmentID() == portEquipmentId || connection->port2EquipmentID() == portEquipmentId))
			{
				return connection;
			}
		}

		return std::shared_ptr<Hardware::Connection>();
	}

	std::vector<std::shared_ptr<Hardware::Connection>> ConnectionStorage::getConnections() const
	{
		return m_objectsVector;
	}

	bool ConnectionStorage::load(QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		bool ok = loadFromConnectionsFolder(errorMessage);
		if (ok == false)
		{
			return false;
		}

		ok = loadFromXmlDeprecated(errorMessage);
		if (ok == false)
		{
			return false;
		}

		return true;
	}

	bool ConnectionStorage::save(const QUuid &uuid, QString* errorMessage)
	{
		if (m_db == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(errorMessage);
			return false;
		}

		std::shared_ptr<Hardware::Connection> connection = get(uuid);
		if (connection == nullptr)
		{
			assert(connection);
			return false;
		}

		Proto::Envelope message;
		connection->Save(&message);

		QByteArray data;

		int size = static_cast<int>(message.ByteSizeLong());
		data.resize(size);

		message.SerializeToArray(data.data(), size);

		// save to db
		//
		DbFileInfo fi = fileInfo(connection->uuid());

		if (fi.isNull() == true)
		{
			// create a file, if it does not exists
			//
			std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

			QString fileName = QString("connection-%1.%2").arg(connection->uuid().toString()).arg(Db::File::OclFileExtension);
			fileName = fileName.remove('{');
			fileName = fileName.remove('}');

			file->setFileName(fileName);
			file->swapData(data);

			if (m_db->addFile(file, DbDir::ConnectionsDir, nullptr) == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			setFileInfo(connection->uuid(), *file);
		}
		else
		{
			std::shared_ptr<DbFile> file = nullptr;

			// save to existing file
			//
			bool ok = m_db->getLatestVersion(fi, &file, nullptr);
			if (ok == false || file == nullptr)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			if (file->state() != E::VcsState::CheckedOut)
			{
				*errorMessage = QString("File %1 state is not CheckedOut").arg(file->fileName());
				return false;
			}

			file->swapData(data);

			if (m_db->setWorkcopy(file, nullptr) == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			setFileInfo(connection->uuid(), *file);
		}

		return true;
	}

	bool ConnectionStorage::loadFromConnectionsFolder(QString* errorMessage)
	{
		if (m_db == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(errorMessage);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;

		bool ok = m_db->getFileList(&fileList, DbDir::ConnectionsDir, Db::File::OclFileExtension, true, nullptr);
		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		for (DbFileInfo& fi : fileList)
		{
			std::shared_ptr<DbFile> file = nullptr;

			ok = m_db->getLatestVersion(fi, &file, nullptr);
			if (ok == false || file == nullptr)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			std::shared_ptr<Hardware::Connection> c = Hardware::Connection::Create(file->data());

			if (c == nullptr)
			{
				*errorMessage = QString("Create connection file error. Please, inform developers. %1").arg(__FUNCTION__);
				return false;
			}

			setFileInfo(c->uuid(), *file);
			add(c->uuid(), c);
		}

		return true;
	}


	bool ConnectionStorage::loadFromXmlDeprecated(QString* errorMessage)
	{
		if (m_db == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(errorMessage);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;

		bool ok = m_db->getFileList(&fileList, DbDir::ModuleConfigurationDir, "Connections.xml", true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			*errorMessage = m_db->lastError();
			return true;
		}

		std::shared_ptr<DbFile> file = nullptr;
		ok = m_db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		QByteArray data;
		file->swapData(data);

		// Load connections from XML
		//
		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			reader.raiseError(QObject::tr("Failed to load root element."));
			*errorMessage = reader.errorString();
			return !reader.hasError();
		}

		if (reader.name() != "Connections")
		{
			reader.raiseError(QObject::tr("The file is not an Connections file."));
			*errorMessage = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			if (reader.name() == "Connection")
			{
				std::shared_ptr<Hardware::Connection> c = std::make_shared<Hardware::Connection>();

				if (c->loadFromXml(reader) == true)
				{
					add(c->uuid(), c);
				}
			}
			else
			{
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				*errorMessage = reader.errorString();
				reader.skipCurrentElement();
			}
		}

		return !reader.hasError();
	}

	bool ConnectionStorage::deleteXmlDeprecated(QString* errorMessage)
	{
		if (m_db == nullptr ||
			errorMessage == nullptr)
		{
			assert(m_db);
			assert(errorMessage);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;

		bool ok = m_db->getFileList(&fileList, DbDir::ModuleConfigurationDir, "Connections.xml", true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			*errorMessage = m_db->lastError();
			return true;
		}

		std::shared_ptr<DbFile> file = nullptr;

		ok = m_db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		if (file->state() != E::VcsState::CheckedOut)
		{
			if (m_db->checkOut(fileList[0], nullptr) == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}
		}

		ok = m_db->deleteFiles(&fileList, nullptr);
		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		if (m_db->checkIn(fileList[0], "Deleted deprecated file Connections.xml", nullptr) == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		return true;
	}

}
