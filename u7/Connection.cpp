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
		ADD_PROPERTY_GETTER_SETTER(QString, Device2StrID, true, Connection::device2StrID, Connection::setDevice2StrID);

		ADD_PROPERTY_GETTER_SETTER(QString, OcmPortStrID, false, Connection::ocmPortStrID, Connection::setOcmPortStrID);

        ADD_PROPERTY_GETTER_SETTER(int, Device1TxWordsQuantity, true, Connection::device1TxWordsQuantity, Connection::setDevice1TxWordsQuantity);
        ADD_PROPERTY_GETTER_SETTER(int, Device1RxWordsQuantity, true, Connection::device1RxWordsQuantity, Connection::setDevice1RxWordsQuantity);

        ADD_PROPERTY_GETTER_SETTER(int, Device1TxRxOptoID, false, Connection::device1TxRxOptoID, Connection::setDevice1TxRxOptoID);
        ADD_PROPERTY_GETTER_SETTER(quint32, Device1TxRxOptoDataUID, false, Connection::device1TxRxOptoDataUID, Connection::setDevice1TxRxOptoDataUID);

        ADD_PROPERTY_GETTER_SETTER(int, Device1TxRsID, false, Connection::device1TxRsID, Connection::setDevice1TxRsID);
        ADD_PROPERTY_GETTER_SETTER(quint32, Device1TxRsDataUID, false, Connection::device1TxRsDataUID, Connection::setDevice1TxRsDataUID);

        ADD_PROPERTY_GETTER_SETTER(int, Device2TxWordsQuantity, true, Connection::device2TxWordsQuantity, Connection::setDevice2TxWordsQuantity);
        ADD_PROPERTY_GETTER_SETTER(int, Device2RxWordsQuantity, true, Connection::device2RxWordsQuantity, Connection::setDevice2RxWordsQuantity);

        ADD_PROPERTY_GETTER_SETTER(int, Device2TxRxOptoID, false, Connection::device2TxRxOptoID, Connection::setDevice2TxRxOptoID);
        ADD_PROPERTY_GETTER_SETTER(quint32, Device2TxRxOptoDataUID, false, Connection::device2TxRxOptoDataUID, Connection::setDevice2TxRxOptoDataUID);

        ADD_PROPERTY_GETTER_SETTER(int, Device2TxRsID, false, Connection::device2TxRsID, Connection::setDevice2TxRsID);
        ADD_PROPERTY_GETTER_SETTER(quint32, Device2TxRsDataUID, false, Connection::device2TxRsDataUID, Connection::setDevice2TxRsDataUID);

        ADD_PROPERTY_GETTER_SETTER(ConnectionMode, ConnectionMode, false, Connection::connectionMode, Connection::setConnectionMode);
		ADD_PROPERTY_GETTER_SETTER(bool, Enable, false, Connection::enable, Connection::setEnable);
	}

    Connection::Connection(const Connection& that):Connection()
    {
        *this = that;
    }

	bool Connection::save(QXmlStreamWriter& writer)
	{
		writer.writeAttribute("Index", QString::number(index()));
		writer.writeAttribute("Caption", caption());
		writer.writeAttribute("OcmPortStrID", ocmPortStrID());
		writer.writeAttribute("Device1StrID", device1StrID());
		writer.writeAttribute("Device2StrID", device2StrID());
		writer.writeAttribute("ConnectionMode", QString::number(static_cast<int>(connectionMode())));
		writer.writeAttribute("ConnectionType", QString::number(static_cast<int>(connectionType())));
		writer.writeAttribute("Enable", enable() ? "true" : "false");
		writer.writeAttribute("SignalList", signalList().join(';'));

        // Tx/Rx words quantity
        //
        writer.writeAttribute("Device1TxWordsQuantity", QString::number(device1TxWordsQuantity()));
        writer.writeAttribute("Device1RxWordsQuantity", QString::number(device1RxWordsQuantity()));
        writer.writeAttribute("Device2TxWordsQuantity", QString::number(device2TxWordsQuantity()));
        writer.writeAttribute("Device2RxWordsQuantity", QString::number(device2RxWordsQuantity()));

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

		if (reader.attributes().hasAttribute("OcmPortStrID"))
		{
			setOcmPortStrID(reader.attributes().value("OcmPortStrID").toString());
		}

		if (reader.attributes().hasAttribute("Device1StrID"))
		{
			setDevice1StrID(reader.attributes().value("Device1StrID").toString());
		}

		if (reader.attributes().hasAttribute("Device2StrID"))
		{
			setDevice2StrID(reader.attributes().value("Device2StrID").toString());
		}

		if (reader.attributes().hasAttribute("ConnectionMode"))
		{
			setConnectionMode(static_cast<ConnectionMode>(reader.attributes().value("ConnectionMode").toInt()));
		}

		if (reader.attributes().hasAttribute("ConnectionType"))
		{
			setConnectionType(static_cast<ConnectionType>(reader.attributes().value("ConnectionType").toInt()));
		}

		if (reader.attributes().hasAttribute("Enable"))
		{
			setEnable(reader.attributes().value("Enable").toString() == "true" ? true : false);
		}

		if (reader.attributes().hasAttribute("SignalList"))
		{
			setSignalList(reader.attributes().value("SignalList").toString().split(';', QString::SkipEmptyParts));
		}

        // Tx/Rx words quantity, may be overloaded later
        //

        if (reader.attributes().hasAttribute("Device1TxWordsQuantity"))
        {
            setDevice1TxWordsQuantity(reader.attributes().value("Device1TxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Device1RxWordsQuantity"))
        {
            setDevice1RxWordsQuantity(reader.attributes().value("Device1RxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Device2TxWordsQuantity"))
        {
            setDevice2TxWordsQuantity(reader.attributes().value("Device2TxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Device2RxWordsQuantity"))
        {
            setDevice2RxWordsQuantity(reader.attributes().value("Device2RxWordsQuantity").toInt());
        }


		QXmlStreamReader::TokenType endToken = reader.readNext();
		if (endToken != QXmlStreamReader::EndElement && endToken != QXmlStreamReader::Invalid)
		{
			Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);
			return false;
		}

        // Generate Connection IDs as a hash of caption or ocmPortStrID
        //
        quint16 hashOpto = CUtils::calcHash16(m_caption.data(), m_caption.size() * sizeof(QChar));
        setDevice1TxRxOptoID(hashOpto);
        setDevice2TxRxOptoID(hashOpto);

        quint16 hashRs = CUtils::calcHash16(m_ocmPortStrID.data(), m_ocmPortStrID.size() * sizeof(QChar));
        setDevice1TxRsID(hashRs);
        setDevice2TxRsID(hashRs);

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

	QString Connection::ocmPortStrID() const
	{
		return m_ocmPortStrID;
	}

	void Connection::setOcmPortStrID(const QString& value)
	{
		m_ocmPortStrID = value;
	}

	QString Connection::device1StrID() const
    {
        return m_device1StrID;
    }

    void Connection::setDevice1StrID(const QString& value)
    {
        m_device1StrID = value;
    }

	QString Connection::device2StrID() const
    {
        return m_device2StrID;
    }

    void Connection::setDevice2StrID(const QString& value)
    {
        m_device2StrID = value;
    }

    //
    // Device1
    //

    int Connection::device1TxWordsQuantity() const
    {
        return m_device1TxWordsQuantity;
    }

    void Connection::setDevice1TxWordsQuantity(int value)
    {
        m_device1TxWordsQuantity = value;
    }

    int Connection::device1RxWordsQuantity() const
    {
        return m_device1RxWordsQuantity;
    }

    void Connection::setDevice1RxWordsQuantity(int value)
    {
        m_device1RxWordsQuantity = value;
    }

    //
    //

    int Connection::device1TxRxOptoID() const
    {
        return m_device1TxRxOptoID;
    }

    void Connection::setDevice1TxRxOptoID(int value)
    {
        m_device1TxRxOptoID = value;
    }

    quint32 Connection::device1TxRxOptoDataUID() const
    {
        return m_device1TxRxOptoDataUID;

    }

    void Connection::setDevice1TxRxOptoDataUID(quint32 value)
    {
        m_device1TxRxOptoDataUID = value;

    }

    int Connection::device1TxRsID() const
    {
        return m_device1TxRsID;
    }

    void Connection::setDevice1TxRsID(int value)
    {
        m_device1TxRsID = value;
    }

    quint32 Connection::device1TxRsDataUID() const
    {
        return m_device1TxRsDataUID;
    }

    void Connection::setDevice1TxRsDataUID(quint32 value)
    {
        m_device1TxRsDataUID = value;
    }

    //
    // Device2
    //

    int Connection::device2TxWordsQuantity() const
    {
        return m_device2TxWordsQuantity;
    }

    void Connection::setDevice2TxWordsQuantity(int value)
    {
        m_device2TxWordsQuantity = value;
    }

    int Connection::device2RxWordsQuantity() const
    {
        return m_device2RxWordsQuantity;
    }

    void Connection::setDevice2RxWordsQuantity(int value)
    {
        m_device2RxWordsQuantity = value;
    }

    //
    //

    int Connection::device2TxRxOptoID() const
    {
        return m_device2TxRxOptoID;
    }

    void Connection::setDevice2TxRxOptoID(int value)
    {
        m_device2TxRxOptoID = value;
    }

    quint32 Connection::device2TxRxOptoDataUID() const
    {
        return m_device2TxRxOptoDataUID;

    }

    void Connection::setDevice2TxRxOptoDataUID(quint32 value)
    {
        m_device2TxRxOptoDataUID = value;

    }

    int Connection::device2TxRsID() const
    {
        return m_device2TxRsID;
    }

    void Connection::setDevice2TxRsID(int value)
    {
        m_device2TxRsID = value;
    }

    quint32 Connection::device2TxRsDataUID() const
    {
        return m_device2TxRsDataUID;
    }

    void Connection::setDevice2TxRsDataUID(quint32 value)
    {
        m_device2TxRsDataUID = value;
    }

    Connection::ConnectionMode Connection::connectionMode() const
    {
        return m_connectionMode;
    }

    void Connection::setConnectionMode(const ConnectionMode& value)
    {
		m_connectionMode = value;
	}

	Connection::ConnectionType Connection::connectionType() const
	{
		return m_connectionType;
	}

	void Connection::setConnectionType(const ConnectionType &value)
	{
		m_connectionType = value;
		auto propertyVisibilityChanger = [this](const char* propertyName, bool visible) {
			auto property = propertyByCaption(propertyName);
			if (property != nullptr)
			{
				property->setVisible(visible);
			}
		};

		switch(value)
		{
			case ConnectionType::OpticalConnectionType:
				propertyVisibilityChanger("OcmPortStrID", false);
				propertyVisibilityChanger("ConnectionMode", false);
				propertyVisibilityChanger("Enable", false);
				propertyVisibilityChanger("Device1StrID", true);
				propertyVisibilityChanger("Device2StrID", true);
				break;
			case ConnectionType::SerialConnectionType:
				propertyVisibilityChanger("OcmPortStrID", true);
				propertyVisibilityChanger("ConnectionMode", true);
				propertyVisibilityChanger("Enable", true);
				propertyVisibilityChanger("Device1StrID", false);
				propertyVisibilityChanger("Device2StrID", false);
				break;
			default:
				assert(false);
		}
	}

	bool Connection::enable() const
    {
        return m_enable;
    }

    void Connection::setEnable(bool value)
    {
		m_enable = value;
	}

	QStringList Connection::signalList() const
	{
		return m_signalList;
	}

	void Connection::setSignalList(const QStringList &value)
	{
		m_signalList = value;
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
		if (editObject->connectionType() != Hardware::Connection::ConnectionType::OpticalConnectionType)
		{
			return true;
		}

		if (editObject->device1StrID() == editObject->device2StrID())
        {
            return false;
        }

        for (std::shared_ptr<Hardware::Connection> c : m_connections)
        {
            if (editObject->index() == c->index())
            {
                continue;
            }

            if (editObject->device1StrID() == c->device1StrID())
            {
                return false;
            }

            if (editObject->device2StrID() == c->device2StrID())
            {
                return false;
            }
        }

        return true;
    }

    bool ConnectionStorage::setLMConnectionParams(const QString& deviceStrID, int m_txWordsQuantity, int m_rxWordsQuantity,
                             int m_txRxOptoID, quint32 m_txRxOptoDataUID)
    {
        return setOCMConnectionParams(deviceStrID, m_txWordsQuantity, m_rxWordsQuantity, m_txRxOptoID, m_txRxOptoDataUID, 0, 0);
    }

    bool ConnectionStorage::setOCMConnectionParams(const QString& deviceStrID, int m_txWordsQuantity, int m_rxWordsQuantity,
                             int m_txRxOptoID, quint32 m_txRxOptoDataUID, int m_txRsID, quint32 m_txRsDataUID)
    {
        for (std::shared_ptr<Hardware::Connection> c : m_connections)
        {
            if (c->device1StrID() == deviceStrID)
            {
                c->setDevice1TxWordsQuantity(m_txWordsQuantity);
                c->setDevice1RxWordsQuantity(m_rxWordsQuantity);
                c->setDevice1TxRxOptoID(m_txRxOptoID);
                c->setDevice1TxRxOptoDataUID(m_txRxOptoDataUID);
                c->setDevice1TxRsID(m_txRsID);
                c->setDevice1TxRsDataUID(m_txRsDataUID);
                return true;
            }

            if (c->device2StrID() == deviceStrID)
            {
                c->setDevice2TxWordsQuantity(m_txWordsQuantity);
                c->setDevice2RxWordsQuantity(m_rxWordsQuantity);
                c->setDevice2TxRxOptoID(m_txRxOptoID);
                c->setDevice2TxRxOptoDataUID(m_txRxOptoDataUID);
                c->setDevice2TxRsID(m_txRsID);
                c->setDevice2TxRsDataUID(m_txRsDataUID);
                return true;
            }
        }

        assert(false);
        return false;
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

