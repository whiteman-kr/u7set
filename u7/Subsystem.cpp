#include "Subsystem.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Hardware
{
	//
	//
	// Subsystem
	//
	//
	Subsystem::Subsystem():
		m_index(0),
		m_key(5)
	{

	}

	Subsystem::Subsystem(int index, int key, const QString& strId, const QString& caption):
		m_index(index),
		m_key(key),
		m_strId(strId),
		m_caption(caption)
	{

	}

	bool Subsystem::save(QXmlStreamWriter& writer)
	{
		writer.writeAttribute("Index", QString::number(index()));
		writer.writeAttribute("Key", QString::number(key()));
		writer.writeAttribute("StrID", strId());
		writer.writeAttribute("Caption", caption());
		return true;
	}


	bool Subsystem::load(QXmlStreamReader& reader)
	{
		if (reader.attributes().hasAttribute("Index"))
		{
			setIndex(reader.attributes().value("Index").toInt());
		}

		if (reader.attributes().hasAttribute("Key"))
		{
			setKey(reader.attributes().value("Key").toInt());
		}

		if (reader.attributes().hasAttribute("StrID"))
		{
			setStrId(reader.attributes().value("StrID").toString());
		}

		if (reader.attributes().hasAttribute("Caption"))
		{
			setCaption(reader.attributes().value("Caption").toString());
		}

		QXmlStreamReader::TokenType endToken = reader.readNext();
		Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

		return true;
	}


	const QString& Subsystem::strId() const
	{
		return m_strId;
	}

	void Subsystem::setStrId(const QString& value)
	{
		m_strId = value;
	}

	const QString& Subsystem::caption() const
	{
		return m_caption;
	}

	void Subsystem::setCaption(const QString& value)
	{
		m_caption = value;
	}

	int Subsystem::index() const
	{
		return m_index;
	}

	void Subsystem::setIndex(int value)
	{
		m_index = value;
	}

	int Subsystem::key() const
	{
		return m_key;
	}

	void Subsystem::setKey(int value)
	{
		m_key = value;
	}

	//
	//
	// SubsystemStorage
	//
	//
	SubsystemStorage::SubsystemStorage()
	{

	}

	void SubsystemStorage::add(std::shared_ptr<Subsystem> subsystem)
	{
		m_subsystems.push_back(subsystem);
	}

	int SubsystemStorage::count() const
	{
		return static_cast<int>(m_subsystems.size());
	}

	std::shared_ptr<Subsystem> SubsystemStorage::get(int index) const
	{
		if (index < 0 || index >= count())
		{
			assert(false);
			return std::make_shared<Subsystem>();
		}
		return m_subsystems[index];
	}

	void SubsystemStorage::clear()
	{
		m_subsystems.clear();
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
		bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, nullptr);
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

		bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			// create a file, if it does not exists
			//
			std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
			pf->setFileName(fileName);

			if (db->addFile(pf, db->mcFileId(), nullptr) == false)
			{
				return false;
			}

			ok = db->getFileList(&fileList, db->mcFileId(), fileName, nullptr);
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

		if (file->state() != VcsState::CheckedOut)
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
			if (s->strId() == subsysId)
			{
				return s->key();
			}
		}

		return -1;
	}
}
