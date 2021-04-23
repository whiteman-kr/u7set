#include "SubsystemStorage.h"
#include "../DbLib/DbController.h"

namespace Builder
{
	//
	// SubsystemStorage
	//
	SubsystemStorage::SubsystemStorage()
	{
	}

	void SubsystemStorage::add(std::shared_ptr<Hardware::Subsystem> subsystem)
	{
		m_subsystems.push_back(subsystem);
	}

	int SubsystemStorage::count() const
	{
		return static_cast<int>(m_subsystems.size());
	}

	std::shared_ptr<Hardware::Subsystem> SubsystemStorage::get(int index) const
	{
		if (index < 0 || index >= count())
		{
			assert(false);
			return std::make_shared<Hardware::Subsystem>();
		}
		return m_subsystems[index];
	}

	void SubsystemStorage::clear()
	{
		m_subsystems.clear();
	}

	const std::vector<std::shared_ptr<Hardware::Subsystem>>& SubsystemStorage::subsystems()
	{
		return m_subsystems;
	}

	bool SubsystemStorage::load(DbController *db, QString& errorCode)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;
		int mcFileId = db->systemFileId(DbDir::ModuleConfigurationDir);

		bool ok = db->getFileList(&fileList, mcFileId, fileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			return false;
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

		if (reader.name() != "Subsystems")
		{
			reader.raiseError(QObject::tr("The file is not an Subsystems file."));
			errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			if (reader.name() == "Subsystem")
			{
				std::shared_ptr<Hardware::Subsystem> s = std::make_shared<Hardware::Subsystem>();

				if (s->load(reader) == true)
				{
					m_subsystems.push_back(s);
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

	bool SubsystemStorage::save(DbController *db, const QString& comment)
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

		writer.writeStartElement("Subsystems");
		for (auto s : m_subsystems)
		{
			writer.writeStartElement("Subsystem");
			s->save(writer);
			writer.writeEndElement();
		}
		writer.writeEndElement();

		writer.writeEndDocument();

		// save to db
		//
		std::shared_ptr<DbFile> file = nullptr;
		std::vector<DbFileInfo> fileList;
		int mcFileId = db->systemFileId(DbDir::ModuleConfigurationDir);

		bool ok = db->getFileList(&fileList, mcFileId, fileName, true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			// create a file, if it does not exists
			//
			std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
			pf->setFileName(fileName);

			if (db->addFile(pf, mcFileId, nullptr) == false)
			{
				return false;
			}

			ok = db->getFileList(&fileList, mcFileId, fileName, true, nullptr);
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

	int SubsystemStorage::ssKey(QString subsysId)
	{
		for (auto s : m_subsystems)
		{
			if (s->subsystemId() == subsysId)
			{
				return s->key();
			}
		}

		return -1;
	}

	int SubsystemStorage::subsystemKey(const QString& subsystemID)
	{
		return ssKey(subsystemID);
	}
}
