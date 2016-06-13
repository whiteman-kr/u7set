#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "../lib/CUtils.h"

#include "Connection.h"

namespace Hardware
{

    //
    //
    // Connection
    //
    //
    Connection::Connection()
    {
		ADD_PROPERTY_GETTER_SETTER(QString, "ConnectionID", true, Connection::connectionID, Connection::setConnectionID);
		ADD_PROPERTY_GETTER_SETTER(QString, "Port1EquipmentID", true, Connection::port1EquipmentID, Connection::setPort1EquipmentID);
		ADD_PROPERTY_GETTER_SETTER(QString, "Port2EquipmentID", true, Connection::port2EquipmentID, Connection::setPort2EquipmentID);

		auto proptx1wq = ADD_PROPERTY_GETTER_SETTER(int, "Port1TxWordsQuantity", true, Connection::port1ManualTxWordsQuantity, Connection::setPort1ManualTxWordsQuantity);
		proptx1wq->setCategory(tr("ManualSettings"));
		auto proprx1wq = ADD_PROPERTY_GETTER_SETTER(int, "Port1RxWordsQuantity", true, Connection::port1ManualRxWordsQuantity, Connection::setPort1ManualRxWordsQuantity);
		proprx1wq->setCategory(tr("ManualSettings"));

		ADD_PROPERTY_GETTER_SETTER(int, "Port1TxRxOptoID", false, Connection::port1TxRxOptoID, Connection::setPort1TxRxOptoID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port1TxRxOptoDataUID", false, Connection::port1TxRxOptoDataUID, Connection::setPort1TxRxOptoDataUID);

		ADD_PROPERTY_GETTER_SETTER(int, "Port1TxRsID", false, Connection::port1TxRsID, Connection::setPort1TxRsID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port1TxRsDataUID", false, Connection::port1TxRsDataUID, Connection::setPort1TxRsDataUID);

		auto proptx2wq = ADD_PROPERTY_GETTER_SETTER(int, "Port2TxWordsQuantity", true, Connection::port2ManualTxWordsQuantity, Connection::setPort2ManualTxWordsQuantity);
		proptx2wq->setCategory(tr("ManualSettings"));
		auto proprx2wq = ADD_PROPERTY_GETTER_SETTER(int, "Port2RxWordsQuantity", true, Connection::port2ManualRxWordsQuantity, Connection::setPort2ManualRxWordsQuantity);
		proprx2wq->setCategory(tr("ManualSettings"));

		ADD_PROPERTY_GETTER_SETTER(int, "Port2TxRxOptoID", false, Connection::port2TxRxOptoID, Connection::setPort2TxRxOptoID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port2TxRxOptoDataUID", false, Connection::port2TxRxOptoDataUID, Connection::setPort2TxRxOptoDataUID);

		ADD_PROPERTY_GETTER_SETTER(int, "Port2TxRsID", false, Connection::port2TxRsID, Connection::setPort2TxRsID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port2TxRsDataUID", false, Connection::port2TxRsDataUID, Connection::setPort2TxRsDataUID);

		ADD_PROPERTY_GETTER_SETTER(OptoPort::Mode, "Mode", false, Connection::mode, Connection::setMode);
		ADD_PROPERTY_GETTER_SETTER(OptoPort::SerialMode, "SerialMode", false, Connection::serialMode, Connection::setSerialMode);
		ADD_PROPERTY_GETTER_SETTER(bool, "Enable", false, Connection::enable, Connection::setEnable);
		ADD_PROPERTY_GETTER_SETTER(bool, "EnableDuplex", false, Connection::enableDuplex, Connection::setEnableDuplex);

		auto propManual = ADD_PROPERTY_GETTER_SETTER(bool, "EnableManualSettings", true, Connection::manualSettings, Connection::setManualSettings);
		propManual->setCategory(tr("ManualSettings"));
    }

    Connection::Connection(const Connection& that):Connection()
    {
        *this = that;
    }

    bool Connection::save(QXmlStreamWriter& writer)
    {
        writer.writeAttribute("Index", QString::number(index()));
		writer.writeAttribute("ConnectionID", connectionID());
		writer.writeAttribute("Port1EquipmentID", port1EquipmentID());
		writer.writeAttribute("Port2EquipmentID", port2EquipmentID());
        writer.writeAttribute("SerialMode", QString::number(static_cast<int>(serialMode())));
        writer.writeAttribute("Mode", QString::number(static_cast<int>(mode())));
        writer.writeAttribute("Enable", enable() ? "true" : "false");
        writer.writeAttribute("EnableDuplex", enableDuplex() ? "true" : "false");
        writer.writeAttribute("ManualSettings", manualSettings() ? "true" : "false");
        writer.writeAttribute("SignalList", signalList().join(';'));

        // Tx/Rx words quantity
        //
        writer.writeAttribute("Port1TxWordsQuantity", QString::number(port1ManualTxWordsQuantity()));
        writer.writeAttribute("Port1RxWordsQuantity", QString::number(port1ManualRxWordsQuantity()));
        writer.writeAttribute("Port2TxWordsQuantity", QString::number(port2ManualTxWordsQuantity()));
        writer.writeAttribute("Port2RxWordsQuantity", QString::number(port2ManualRxWordsQuantity()));

        return true;
    }


    bool Connection::load(QXmlStreamReader& reader)
    {
        if (reader.attributes().hasAttribute("Index"))
        {
            setIndex(reader.attributes().value("Index").toInt());
        }

		if (reader.attributes().hasAttribute("ConnectionID"))
        {
			setConnectionID(reader.attributes().value("ConnectionID").toString());
        }

		if (reader.attributes().hasAttribute("Port1EquipmentID"))
        {
			setPort1EquipmentID(reader.attributes().value("Port1EquipmentID").toString());
        }

		if (reader.attributes().hasAttribute("Port2EquipmentID"))
        {
			setPort2EquipmentID(reader.attributes().value("Port2EquipmentID").toString());
        }

        if (reader.attributes().hasAttribute("SerialMode"))
        {
            setSerialMode(static_cast<OptoPort::SerialMode>(reader.attributes().value("SerialMode").toInt()));
        }

        if (reader.attributes().hasAttribute("Mode"))
        {
            setMode(static_cast<OptoPort::Mode>(reader.attributes().value("Mode").toInt()));
        }

        if (reader.attributes().hasAttribute("Enable"))
        {
            setEnable(reader.attributes().value("Enable").toString() == "true" ? true : false);
        }

        if (reader.attributes().hasAttribute("EnableDuplex"))
        {
            setEnableDuplex(reader.attributes().value("EnableDuplex").toString() == "true" ? true : false);
        }

        if (reader.attributes().hasAttribute("ManualSettings"))
        {
            setManualSettings(reader.attributes().value("ManualSettings").toString() == "true" ? true : false);
        }

        if (reader.attributes().hasAttribute("SignalList"))
        {
            setSignalList(reader.attributes().value("SignalList").toString().split(';', QString::SkipEmptyParts));
        }

        // Tx/Rx words quantity, may be overloaded later
        //

        if (reader.attributes().hasAttribute("Port1TxWordsQuantity"))
        {
            setPort1ManualTxWordsQuantity(reader.attributes().value("Port1TxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Port1RxWordsQuantity"))
        {
            setPort1ManualRxWordsQuantity(reader.attributes().value("Port1RxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Port2TxWordsQuantity"))
        {
            setPort2ManualTxWordsQuantity(reader.attributes().value("Port2TxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Port2RxWordsQuantity"))
        {
            setPort2ManualRxWordsQuantity(reader.attributes().value("Port2RxWordsQuantity").toInt());
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

	QString Connection::connectionID() const
    {
		return m_connectionID;
    }

	void Connection::setConnectionID(const QString& value)
    {
		m_connectionID = value;
    }

	QString Connection::port1EquipmentID() const
    {
        return m_port1EquipmentID;
    }

	void Connection::setPort1EquipmentID(const QString& value)
    {
        m_port1EquipmentID = value;
    }

	QString Connection::port2EquipmentID() const
    {
        return m_port2EquipmentID;
    }

	void Connection::setPort2EquipmentID(const QString& value)
    {
        m_port2EquipmentID = value;
    }

    quint16 Connection::getID() const
    {
        // Connection ID calculation
        // range 1..999
        //

		QString sourceStr = port1EquipmentID() + port2EquipmentID();

        quint16 hash = CUtils::calcHash16(C_STR(sourceStr), sourceStr.length());

        return (hash % 999) + 1;
    }

    //
    // port1
    //

    int Connection::port1ManualTxWordsQuantity() const
    {
        return m_port1ManualTxWordsQuantity;
    }

    void Connection::setPort1ManualTxWordsQuantity(int value)
    {
        m_port1ManualTxWordsQuantity = value;
    }

    int Connection::port1ManualRxWordsQuantity() const
    {
        return m_port1ManualRxWordsQuantity;
    }

    void Connection::setPort1ManualRxWordsQuantity(int value)
    {
        m_port1ManualRxWordsQuantity = value;
    }

    //
    //

    int Connection::port1TxRxOptoID() const
    {
        return m_port1TxRxOptoID;
    }

    void Connection::setPort1TxRxOptoID(int value)
    {
        m_port1TxRxOptoID = value;
    }

    quint32 Connection::port1TxRxOptoDataUID() const
    {
        return m_port1TxRxOptoDataUID;

    }

    void Connection::setPort1TxRxOptoDataUID(quint32 value)
    {
        m_port1TxRxOptoDataUID = value;

    }

    int Connection::port1TxRsID() const
    {
        return m_port1TxRsID;
    }

    void Connection::setPort1TxRsID(int value)
    {
        m_port1TxRsID = value;
    }

    quint32 Connection::port1TxRsDataUID() const
    {
        return m_port1TxRsDataUID;
    }

    void Connection::setPort1TxRsDataUID(quint32 value)
    {
        m_port1TxRsDataUID = value;
    }

    //
    // port2
    //

    int Connection::port2ManualTxWordsQuantity() const
    {
        return m_port2ManualTxWordsQuantity;
    }

    void Connection::setPort2ManualTxWordsQuantity(int value)
    {
        m_port2ManualTxWordsQuantity = value;
    }

    int Connection::port2ManualRxWordsQuantity() const
    {
        return m_port2ManualRxWordsQuantity;
    }

    void Connection::setPort2ManualRxWordsQuantity(int value)
    {
        m_port2ManualRxWordsQuantity = value;
    }

    //
    //

    int Connection::port2TxRxOptoID() const
    {
        return m_port2TxRxOptoID;
    }

    void Connection::setPort2TxRxOptoID(int value)
    {
        m_port2TxRxOptoID = value;
    }

    quint32 Connection::port2TxRxOptoDataUID() const
    {
        return m_port2TxRxOptoDataUID;

    }

    void Connection::setPort2TxRxOptoDataUID(quint32 value)
    {
        m_port2TxRxOptoDataUID = value;

    }

    int Connection::port2TxRsID() const
    {
        return m_port2TxRsID;
    }

    void Connection::setPort2TxRsID(int value)
    {
        m_port2TxRsID = value;
    }

    quint32 Connection::port2TxRsDataUID() const
    {
        return m_port2TxRsDataUID;
    }

    void Connection::setPort2TxRsDataUID(quint32 value)
    {
        m_port2TxRsDataUID = value;
    }

    OptoPort::SerialMode Connection::serialMode() const
    {
        return m_serialMode;
    }

    void Connection::setSerialMode(const OptoPort::SerialMode value)
    {
        m_serialMode = value;
    }

    OptoPort::Mode Connection::mode() const
    {
        return m_mode;
    }

    void Connection::setMode(const OptoPort::Mode value)
    {
        m_mode = value;
        auto propertyVisibilityChanger = [this](const char* propertyName, bool visible) {
            auto property = propertyByCaption(propertyName);
            if (property != nullptr)
            {
                property->setVisible(visible);
            }
        };

        switch(value)
        {
            case OptoPort::Mode::Optical:
                propertyVisibilityChanger("SerialMode", false);
                propertyVisibilityChanger("Enable", false);
                propertyVisibilityChanger("EnableDuplex", false);

				propertyVisibilityChanger("Port2EquipmentID", true);
                propertyVisibilityChanger("Port2TxWordsQuantity", true);
                propertyVisibilityChanger("Port2RxWordsQuantity", true);


                break;
            case OptoPort::Mode::Serial:
                propertyVisibilityChanger("SerialMode", true);
                propertyVisibilityChanger("Enable", true);
                propertyVisibilityChanger("EnableDuplex", true);

				propertyVisibilityChanger("Port2EquipmentID", false);
                propertyVisibilityChanger("Port2TxWordsQuantity", false);
                propertyVisibilityChanger("Port2RxWordsQuantity", false);
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

    bool Connection::enableDuplex() const
    {
        return m_enableDuplex;
    }

    void Connection::setEnableDuplex(bool value)
    {
        m_enableDuplex = value;
    }

    bool Connection::manualSettings() const
    {
        return m_manualSettings;
    }

    void Connection::setManualSettings(bool value)
    {
        m_manualSettings = value;
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

	bool ConnectionStorage::setLMConnectionParams(const QString& portEquipmentID, int m_txWordsQuantity, int m_rxWordsQuantity,
							 int m_txRxOptoID, quint32 m_txRxOptoDataUID)
    {
		return setOCMConnectionParams(portEquipmentID, m_txWordsQuantity, m_rxWordsQuantity, m_txRxOptoID, m_txRxOptoDataUID, 0, 0);
    }

	bool ConnectionStorage::setOCMConnectionParams(const QString& portEquipmentID, int m_txWordsQuantity, int m_rxWordsQuantity,
							 int m_txRxOptoID, quint32 m_txRxOptoDataUID, int m_txRsID, quint32 m_txRsDataUID)
    {
        for (std::shared_ptr<Hardware::Connection> c : m_connections)
        {
			if (c->port1EquipmentID() == portEquipmentID)
            {
                c->setPort1ManualTxWordsQuantity(m_txWordsQuantity);
                c->setPort1ManualRxWordsQuantity(m_rxWordsQuantity);
                c->setPort1TxRxOptoID(m_txRxOptoID);
                c->setPort1TxRxOptoDataUID(m_txRxOptoDataUID);
                c->setPort1TxRsID(m_txRsID);
                c->setPort1TxRsDataUID(m_txRsDataUID);
                return true;
            }

			if (c->port2EquipmentID() == portEquipmentID)
            {
                c->setPort2ManualTxWordsQuantity(m_txWordsQuantity);
                c->setPort2ManualRxWordsQuantity(m_rxWordsQuantity);
                c->setPort2TxRxOptoID(m_txRxOptoID);
                c->setPort2TxRxOptoDataUID(m_txRxOptoDataUID);
                c->setPort2TxRsID(m_txRsID);
                c->setPort2TxRsDataUID(m_txRsDataUID);
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

