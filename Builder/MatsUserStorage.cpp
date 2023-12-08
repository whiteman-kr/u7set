#include "MatsUserStorage.h"
#include "../DbLib/DbController.h"

namespace Builder
{
	//
	// MatsUserStorage
	//
	MatsUserStorage::MatsUserStorage()
	{
	}

	void MatsUserStorage::add(const DbMatsUser& user)
	{
		m_users.push_back(user);
	}

	int MatsUserStorage::count() const
	{
		return static_cast<int>(m_users.size());
	}

	const DbMatsUser& MatsUserStorage::get(int index) const
	{
		if (index < 0 || index >= count())
		{
			static DbMatsUser err;
			assert(false);
			return err;
		}
		return m_users[index];
	}

	void MatsUserStorage::clear()
	{
		m_users.clear();
	}

	const std::vector<DbMatsUser>& MatsUserStorage::users() const
	{
		return m_users;
	}

	QString MatsUserStorage::tuningTags(const QString& login, bool* found) const
	{

		auto it = std::find_if(m_users.begin(), m_users.end(), [&login](const DbMatsUser& user)
							{
								return user.login() == login;
							});
		if (it == m_users.end())
		{
			if (found != nullptr)
			{
				*found = false;
			}
			return QString();
		}
		if (found != nullptr)
		{
			*found = true;
		}
		return it->tuningTags();
	}

	bool MatsUserStorage::load(DbController *db, QString& errorCode)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;
		int fileId = db->systemFileId(DbDir::EtcDir);

		bool ok = db->getFileList(&fileList, fileId, fileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			// No such file, no users
			return true;
		}

		std::shared_ptr<DbFile> file = nullptr;
		ok = db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			return false;
		}

		QByteArray data;
		file->swapData(data);

		// Load subsystems from XML
		//

		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			reader.raiseError(QObject::tr("Failed to load root element."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		if (reader.name() != QLatin1String("MatsUsers"))
		{
			reader.raiseError(QObject::tr("The file is not an MatsUsers file."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			if (reader.name() == QLatin1String("User"))
			{
				DbMatsUser user;

				if (user.load(reader) == true)
				{
					m_users.push_back(user);
				}
			}
			else
			{
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				errorCode = reader.errorString();
				reader.skipCurrentElement();
			}
		}
		return !reader.hasError();
	}

	bool MatsUserStorage::save(DbController *db, const QString& comment) const
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// save data to XML
		//
		QByteArray data;
		QXmlStreamWriter writer(&data);

		writer.setAutoFormatting(true);
		writer.writeStartDocument();

		writer.writeStartElement("MatsUsers");
		for (const auto& s : m_users)
		{
			writer.writeStartElement("User");
			s.save(writer);
			writer.writeEndElement();
		}
		writer.writeEndElement();

		writer.writeEndDocument();

		// save to db
		//
		std::shared_ptr<DbFile> file = nullptr;
		std::vector<DbFileInfo> fileList;
		int fileId = db->systemFileId(DbDir::EtcDir);

		bool ok = db->getFileList(&fileList, fileId, fileName, true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			// create a file, if it does not exists
			//
			std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
			pf->setFileName(fileName);

			if (db->addFile(pf, fileId, nullptr) == false)
			{
				return false;
			}

			ok = db->getFileList(&fileList, fileId, fileName, true, nullptr);
			if (ok == false || fileList.size() != 1)
			{
				return false;
			}
		}

		ok = db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			return false;
		}

		if (file->state() != E::VcsState::CheckedOut)
		{
			if (db->checkOut(fileList[0], nullptr) == false)
			{
				return false;
			}
		}

		file->swapData(data);

		if (db->setWorkcopy(file, nullptr) == false)
		{
			return false;
		}

		if (db->checkIn(fileList[0], comment, nullptr) == false)
		{
			return false;
		}

		return true;
	}
}
