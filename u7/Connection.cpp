#include "Connection.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Hardware
{

    //
    //
    // Connection
    //
    //
    Connection::Connection()
    {
        ADD_PROPERTY_GETTER_SETTER(QString, Caption, true, Connection::caption, Connection::setCaption);
        ADD_PROPERTY_GETTER_SETTER(QString, Device1StrID, true, Connection::device1StrID, Connection::setDevice1StrID);
        ADD_PROPERTY_GETTER_SETTER(int, Device1Port, true, Connection::device1Port, Connection::setDevice1Port);
        ADD_PROPERTY_GETTER_SETTER(QString, Device2StrID, true, Connection::device2StrID, Connection::setDevice2StrID);
        ADD_PROPERTY_GETTER_SETTER(int, Device2Port, true, Connection::device2Port, Connection::setDevice2Port);

        ADD_PROPERTY_GETTER_SETTER(int, TxStartAddress, false, Connection::txStartAddress, Connection::setTxStartAddress);
        ADD_PROPERTY_GETTER_SETTER(int, TxWordsQuantity, false, Connection::txWordsQuantity, Connection::setTxWordsQuantity);
        ADD_PROPERTY_GETTER_SETTER(int, RxWordsQuantity, false, Connection::rxWordsQuantity, Connection::setRxWordsQuantity);

        ADD_PROPERTY_GETTER_SETTER(ConnectionMode, ConnectionMode, true, Connection::connectionMode, Connection::setConnectionMode);
        ADD_PROPERTY_GETTER_SETTER(bool, Enable, true, Connection::enable, Connection::setEnable);

    }

    Connection::Connection(const Connection& that):Connection()
    {
        *this = that;
    }

    bool Connection::save(QXmlStreamWriter& writer)
    {
        writer.writeAttribute("Index", QString::number(index()));
        writer.writeAttribute("Caption", caption());
		writer.writeAttribute("Device1StrID", device1StrID());
		writer.writeAttribute("Device1Port", QString::number(device1Port()));
		writer.writeAttribute("Device2StrID", device2StrID());
		writer.writeAttribute("Device2Port", QString::number(device2Port()));
        writer.writeAttribute("ConnectionMode", QString::number(static_cast<int>(connectionMode())));
        writer.writeAttribute("Enable", enable() ? "true" : "false");
        return true;
    }


    bool Connection::load(QXmlStreamReader& reader)
    {
        if (reader.attributes().hasAttribute("Index"))
        {
            setIndex(reader.attributes().value("Index").toInt());
        }

        if (reader.attributes().hasAttribute("Caption"))
        {
            setCaption(reader.attributes().value("Caption").toString());
        }

		if (reader.attributes().hasAttribute("Device1StrID"))
        {
			setDevice1StrID(reader.attributes().value("Device1StrID").toString());
        }

		if (reader.attributes().hasAttribute("Device1Port"))
        {
			setDevice1Port(reader.attributes().value("Device1Port").toInt());
        }

		if (reader.attributes().hasAttribute("Device2StrID"))
        {
			setDevice2StrID(reader.attributes().value("Device2StrID").toString());
        }

		if (reader.attributes().hasAttribute("Device2Port"))
        {
			setDevice2Port(reader.attributes().value("Device2Port").toInt());
        }

        if (reader.attributes().hasAttribute("ConnectionMode"))
        {
            setConnectionMode(static_cast<ConnectionMode>(reader.attributes().value("ConnectionMode").toInt()));
        }

        if (reader.attributes().hasAttribute("Enable"))
        {
            setEnable(reader.attributes().value("Enable").toString() == "true" ? true : false);
        }

        QXmlStreamReader::TokenType endToken = reader.readNext();
        if (endToken != QXmlStreamReader::EndElement && endToken != QXmlStreamReader::Invalid)
        {
            Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);
            return false;
        }

        return true;
    }

	int Connection::index() const
    {
        return m_index;
    }

    void Connection::setIndex(int value)
    {
        m_index = value;
    }

	QString Connection::caption() const
    {
        return m_caption;
    }

    void Connection::setCaption(const QString& value)
    {
        m_caption = value;
    }

	QString Connection::device1StrID() const
    {
        return m_device1StrID;
    }

    void Connection::setDevice1StrID(const QString& value)
    {
        m_device1StrID = value;
    }

	int Connection::device1Port() const
    {
        return m_device1Port;
    }

    void Connection::setDevice1Port(int value)
    {
        m_device1Port = value;
    }

	QString Connection::device2StrID() const
    {
        return m_device2StrID;
    }

    void Connection::setDevice2StrID(const QString& value)
    {
        m_device2StrID = value;
    }

	int Connection::device2Port() const
    {
        return m_device2Port;
    }

    void Connection::setDevice2Port(int value)
    {
        m_device2Port = value;
    }

	int Connection::txStartAddress() const
    {
        return m_txStartAddress;
    }

    void Connection::setTxStartAddress(int value)
    {
        m_txStartAddress = value;
    }

	int Connection::txWordsQuantity() const
    {
        return m_txWordsQuantity;
    }

    void Connection::setTxWordsQuantity(int value)
    {
        m_txWordsQuantity = value;
    }

	int Connection::rxWordsQuantity() const
    {
        return m_rxWordsQuantity;
    }

    void Connection::setRxWordsQuantity(int value)
    {
        m_rxWordsQuantity = value;
    }

	Connection::ConnectionMode Connection::connectionMode() const
    {
        return m_connectionMode;
    }

    void Connection::setConnectionMode(const ConnectionMode& value)
    {
        m_connectionMode = value;
    }

	bool Connection::enable() const
    {
        return m_enable;
    }

    void Connection::setEnable(bool value)
    {
        m_enable = value;
    }


    //
    //
    // ConnectionStorage
    //
    //

    ConnectionStorage::ConnectionStorage()
    {
    }

    void ConnectionStorage::add(std::shared_ptr<Connection> connection)
    {
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        m_connections.push_back(connection);

        int i = 0;
        for (auto c : m_connections)
        {
            c->setIndex(i++);
        }
    }

    bool ConnectionStorage::remove(std::shared_ptr<Connection> connection)
    {
        if (connection == nullptr)
        {
            assert(connection);
            return false;
        }

        bool result = false;

        for (auto it = m_connections.begin(); it != m_connections.end(); it++)
        {
            if ((*it)->index() == connection->index())
            {
                m_connections.erase(it);
                result = true;
                break;
            }
        }

        if (result == true)
        {
            int i = 0;
            for (auto c : m_connections)
            {
                c->setIndex(i++);
            }
        }

        if (result == false)
        {
            assert(result);
        }

        return result;
    }

    int ConnectionStorage::count() const
    {
        return static_cast<int>(m_connections.size());
    }

    std::shared_ptr<Connection> ConnectionStorage::get(int index) const
    {
        if (index < 0 || index >= count())
        {
            assert(false);
            return std::make_shared<Connection>();
        }
        return m_connections[index];
    }

    QObject* ConnectionStorage::jsGet(int index) const
    {
        if (index < 0 || index >= count())
        {
            assert(false);
            return nullptr;
        }

        QObject* result = m_connections[index].get();
        QQmlEngine::setObjectOwnership(result, QQmlEngine::CppOwnership);

        return result;
    }

    void ConnectionStorage::clear()
    {
        m_connections.clear();
    }

    bool ConnectionStorage::checkUniqueConnections(Connection* editObject)
    {
        if (editObject->device1StrID() == editObject->device2StrID() && editObject->device1Port() == editObject->device2Port())
        {
            return false;
        }

        for (std::shared_ptr<Hardware::Connection> c : m_connections)
        {
            if (editObject->index() == c->index())
            {
                continue;
            }

            if (editObject->device1StrID() == c->device1StrID() && editObject->device1Port() == c->device1Port())
            {
                return false;
            }

            if (editObject->device1StrID() == c->device2StrID() && editObject->device1Port() == c->device2Port())
            {
                return false;
            }
        }

        return true;
    }

    bool ConnectionStorage::load(DbController *db, QString& errorCode)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }

        // Load the file from the database
        //
        m_connections.clear();

        std::vector<DbFileInfo> fileList;
        bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, true, nullptr);
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

            // save the empty file structure and exit
            //
            save(db);

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

        // Load connections from XML
        //

        QXmlStreamReader reader(data);

        if (reader.readNextStartElement() == false)
        {
            reader.raiseError(QObject::tr("Failed to load root element."));
            errorCode = reader.errorString();
            return !reader.hasError();
        }

        if (reader.name() != "Connections")
        {
            reader.raiseError(QObject::tr("The file is not an Connections file."));
            errorCode = reader.errorString();
            return !reader.hasError();
        }

        // Read signals
        //
        while (reader.readNextStartElement())
        {
            if (reader.name() == "Connection")
            {
                std::shared_ptr<Hardware::Connection> s = std::make_shared<Hardware::Connection>();

                if (s->load(reader) == true)
                {
                    m_connections.push_back(s);
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

    bool ConnectionStorage::save(DbController *db)
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

        writer.writeStartElement("Connections");
        for (auto s : m_connections)
        {
            writer.writeStartElement("Connection");
            s->save(writer);
            writer.writeEndElement();
        }
        writer.writeEndElement();

        writer.writeEndDocument();

        // save to db
        //
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, true, nullptr);

        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() != VcsState::CheckedOut)
        {
            return false;
        }

        file->swapData(data);

        if (db->setWorkcopy(file, nullptr) == false)
        {
            return false;
        }

        return true;
    }

    bool ConnectionStorage::checkOut(DbController *db)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() == VcsState::CheckedOut)
        {
            return false;
        }

        if (db->checkOut(fileList[0], nullptr) == false)
        {
            return false;
        }

        return true;
    }

    bool ConnectionStorage::checkIn(DbController *db, const QString& comment)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() != VcsState::CheckedOut)
        {
            return false;
        }

        if (db->checkIn(fileList[0], comment, nullptr) == false)
        {
            return false;
        }

        return true;
    }

    bool ConnectionStorage::undo(DbController *db)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() != VcsState::CheckedOut)
        {
            return false;
        }

        if (db->undoChanges(fileList[0], nullptr) == false)
        {
            return false;
        }

        return true;

    }

    bool ConnectionStorage::isCheckedOut(DbController *db)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->mcFileId(), fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() != VcsState::CheckedOut)
        {
            return false;
        }

        return true;
    }
}

