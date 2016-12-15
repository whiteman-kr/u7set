#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "../lib/CUtils.h"
#include "../lib/ProtoSerialization.h"

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
        auto propConnId = ADD_PROPERTY_GETTER_SETTER(QString, "ConnectionID", true, Connection::connectionID, Connection::setConnectionID);
        propConnId->setValidator("^[A-Za-z0-9_]+$");

        auto propFileName = ADD_PROPERTY_GETTER(QString, "FileName", true, Connection::fileName);
        propFileName->setExpert(true);

        auto propFileID = ADD_PROPERTY_GETTER(int, "FileID", true, Connection::fileID);
        propFileID->setExpert(true);

        ADD_PROPERTY_GETTER_SETTER(QString, "Port1EquipmentID", true, Connection::port1EquipmentID, Connection::setPort1EquipmentID);
		ADD_PROPERTY_GETTER_SETTER(QString, "Port2EquipmentID", true, Connection::port2EquipmentID, Connection::setPort2EquipmentID);

		auto propPort1RawDataDescription = ADD_PROPERTY_GETTER_SETTER(QString, "Port1RawDataDescription", true, Connection::port1RawDataDescription, Connection::setPort1RawDataDescription);
		propPort1RawDataDescription->setCategory(tr("Miscellaneous"));

		auto propPort2RawDataDescription = ADD_PROPERTY_GETTER_SETTER(QString, "Port2RawDataDescription", true, Connection::port2RawDataDescription, Connection::setPort2RawDataDescription);
		propPort2RawDataDescription->setCategory(tr("Miscellaneous"));

		auto proptx1sa = ADD_PROPERTY_GETTER_SETTER(int, "Port1TxStartAddress", true, Connection::port1ManualTxStartAddress, Connection::setPort1ManualTxStartAddress);
		proptx1sa->setCategory(tr("Manual Settings"));

		auto proptx1wq = ADD_PROPERTY_GETTER_SETTER(int, "Port1TxWordsQuantity", true, Connection::port1ManualTxWordsQuantity, Connection::setPort1ManualTxWordsQuantity);
		proptx1wq->setCategory(tr("Manual Settings"));

		auto proprx1wq = ADD_PROPERTY_GETTER_SETTER(int, "Port1RxWordsQuantity", true, Connection::port1ManualRxWordsQuantity, Connection::setPort1ManualRxWordsQuantity);
		proprx1wq->setCategory(tr("Manual Settings"));

		ADD_PROPERTY_GETTER_SETTER(int, "Port1TxRxOptoID", false, Connection::port1TxRxOptoID, Connection::setPort1TxRxOptoID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port1TxRxOptoDataUID", false, Connection::port1TxRxOptoDataUID, Connection::setPort1TxRxOptoDataUID);

		ADD_PROPERTY_GETTER_SETTER(int, "Port1TxRsID", false, Connection::port1TxRsID, Connection::setPort1TxRsID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port1TxRsDataUID", false, Connection::port1TxRsDataUID, Connection::setPort1TxRsDataUID);

		auto proptx2sa = ADD_PROPERTY_GETTER_SETTER(int, "Port2TxStartAddress", true, Connection::port2ManualTxStartAddress, Connection::setPort2ManualTxStartAddress);
		proptx2sa->setCategory(tr("Manual Settings"));

		auto proptx2wq = ADD_PROPERTY_GETTER_SETTER(int, "Port2TxWordsQuantity", true, Connection::port2ManualTxWordsQuantity, Connection::setPort2ManualTxWordsQuantity);
		proptx2wq->setCategory(tr("Manual Settings"));

		auto proprx2wq = ADD_PROPERTY_GETTER_SETTER(int, "Port2RxWordsQuantity", true, Connection::port2ManualRxWordsQuantity, Connection::setPort2ManualRxWordsQuantity);
		proprx2wq->setCategory(tr("Manual Settings"));

		ADD_PROPERTY_GETTER_SETTER(int, "Port2TxRxOptoID", false, Connection::port2TxRxOptoID, Connection::setPort2TxRxOptoID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port2TxRxOptoDataUID", false, Connection::port2TxRxOptoDataUID, Connection::setPort2TxRxOptoDataUID);

		ADD_PROPERTY_GETTER_SETTER(int, "Port2TxRsID", false, Connection::port2TxRsID, Connection::setPort2TxRsID);
		ADD_PROPERTY_GETTER_SETTER(quint32, "Port2TxRsDataUID", false, Connection::port2TxRsDataUID, Connection::setPort2TxRsDataUID);

        auto propEnableSerial = ADD_PROPERTY_GETTER_SETTER(bool, "EnableSerial", true, Connection::enableSerial, Connection::setEnableSerial);
		propEnableSerial->setCategory(tr("Serial Communications (OCM)"));

        auto propSerialMode = ADD_PROPERTY_GETTER_SETTER(OptoPort::SerialMode, "SerialMode", true, Connection::serialMode, Connection::setSerialMode);
		propSerialMode->setCategory(tr("Serial Communications (OCM)"));

        auto propEnableDuplex = ADD_PROPERTY_GETTER_SETTER(bool, "EnableDuplex", true, Connection::enableDuplex, Connection::setEnableDuplex);
		propEnableDuplex->setCategory(tr("Serial Communications (OCM)"));

		auto propManual = ADD_PROPERTY_GETTER_SETTER(bool, "EnableManualSettings", true, Connection::manualSettings, Connection::setManualSettings);
		propManual->setCategory(tr("Manual Settings"));

        auto propMode = ADD_PROPERTY_GETTER_SETTER(OptoPort::Mode, "Mode", true, Connection::mode, Connection::setMode);
        propMode->setCategory(tr("Serial Communications (OCM)"));

		auto propDisableDataID = ADD_PROPERTY_GETTER_SETTER(bool, "Disable DataID Control", true, Connection::disableDataID, Connection::setDisableDataID);
		propDisableDataID->setCategory(tr("Miscellaneous"));

		auto propGenerateVHD = ADD_PROPERTY_GETTER_SETTER(bool, "Generate connection VHD file", true, Connection::generateVHDFile, Connection::setGenerateVHDFile);
		propGenerateVHD->setCategory(tr("Miscellaneous"));
	}

    Connection::Connection(const Connection& that):Connection()
    {
        *this = that;
    }

    bool Connection::save(DbController *db)
    {
        ::Proto::Connection connectionProto;

        connectionProto.set_index(m_index);
        connectionProto.set_connectionid(m_connectionID.toUtf8());
        connectionProto.set_port1equipmentid(m_port1EquipmentID.toUtf8());
        connectionProto.set_port2equipmentid(m_port2EquipmentID.toUtf8());
        connectionProto.set_port1rawdatadescription(m_port1RawDataDescription.toUtf8());
        connectionProto.set_port2rawdatadescription(m_port2RawDataDescription.toUtf8());

        connectionProto.set_serialmode(static_cast<int>(serialMode()));
        connectionProto.set_mode(static_cast<int>(mode()));
        connectionProto.set_enableserial(m_enableSerial);
        connectionProto.set_enableduplex(m_enableDuplex);
        connectionProto.set_manualsettings(m_manualSettings);
        connectionProto.set_disabledataid(m_disableDataID);
        connectionProto.set_generatevhdfile(m_generateVHDFile);

        connectionProto.set_port1txstartaddress(m_port1TxStartAddress);
        connectionProto.set_port1txwordsquantity(m_port1ManualTxWordsQuantity);
        connectionProto.set_port1rxwordsquantity(m_port1ManualRxWordsQuantity);

        connectionProto.set_port2txstartaddress(m_port2TxStartAddress);
        connectionProto.set_port2txwordsquantity(m_port2ManualTxWordsQuantity);
        connectionProto.set_port2rxwordsquantity(m_port2ManualRxWordsQuantity);

        QByteArray data;

        int size = connectionProto.ByteSize();
        data.resize(size);

        connectionProto.SerializeToArray(data.data(), size);

        // save to db
        //

        if (m_fileName.isEmpty() == true)
        {
            QString sUUID = QUuid::createUuid().toString();
            sUUID.remove('{');
            sUUID.remove('}');

            m_fileName = tr("connection-%1.%2").arg(sUUID).arg(::OclFileExtension);
        }

        std::vector<DbFileInfo> fileList;

        std::shared_ptr<DbFile> file = nullptr;

        bool ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            // create a file, if it does not exists
            //
            file = std::make_shared<DbFile>();
            file->setFileName(m_fileName);

            file->swapData(data);

            if (db->addFile(file, db->connectionsFileId(), nullptr) == false)
            {
                return false;
            }

        }
        else
        {
            // save to existing file
            //
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
        }

        return true;

    }

    bool Connection::load(const QByteArray& data)
    {
        ::Proto::Connection connectionProto;

        bool result = connectionProto.ParseFromArray(data.data(), data.size());
        if (result == false)
        {
            return false;
        }

        m_index = connectionProto.index();

        m_connectionID = connectionProto.connectionid().c_str();
        m_port1EquipmentID = connectionProto.port1equipmentid().c_str();
        m_port2EquipmentID = connectionProto.port2equipmentid().c_str();
        m_port1RawDataDescription = connectionProto.port1rawdatadescription().c_str();
        m_port2RawDataDescription = connectionProto.port2rawdatadescription().c_str();

        m_serialMode = static_cast<OptoPort::SerialMode>(connectionProto.serialmode());
        m_mode = static_cast<OptoPort::Mode>(connectionProto.mode());
        m_enableSerial = connectionProto.enableserial();
        m_enableDuplex = connectionProto.enableduplex();
        m_manualSettings = connectionProto.manualsettings();
        m_disableDataID = connectionProto.disabledataid();
        m_generateVHDFile = connectionProto.generatevhdfile();

        m_port1TxStartAddress = connectionProto.port1txstartaddress();
        m_port1ManualTxWordsQuantity = connectionProto.port1txwordsquantity();
        m_port1ManualRxWordsQuantity = connectionProto.port1rxwordsquantity();

        m_port2TxStartAddress = connectionProto.port2txstartaddress();
        m_port2ManualTxWordsQuantity = connectionProto.port2txwordsquantity();
        m_port2ManualRxWordsQuantity = connectionProto.port2rxwordsquantity();


        return true;
    }


    bool Connection::loadFromXml(QXmlStreamReader& reader)
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
			setEnableSerial(reader.attributes().value("Enable").toString() == "true" ? true : false);
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

        if (reader.attributes().hasAttribute("DisableDataID"))
		{
			setDisableDataID(reader.attributes().value("DisableDataID").toString() == "true" ? true : false);
		}

		if (reader.attributes().hasAttribute("GenerateVHDFile"))
		{
			setGenerateVHDFile(reader.attributes().value("GenerateVHDFile").toString() == "true" ? true : false);
		}

		if (reader.attributes().hasAttribute("Port1RawDataDescription"))
		{
			setPort1RawDataDescription(reader.attributes().value("Port1RawDataDescription").toString());
		}

		if (reader.attributes().hasAttribute("Port2RawDataDescription"))
		{
			setPort2RawDataDescription(reader.attributes().value("Port2RawDataDescription").toString());
		}

		// Tx/Rx words quantity, may be overloaded later
        //

		if (reader.attributes().hasAttribute("Port1TxStartAddress"))
		{
			setPort1ManualTxStartAddress(reader.attributes().value("Port1TxStartAddress").toInt());
		}

		if (reader.attributes().hasAttribute("Port1TxWordsQuantity"))
        {
            setPort1ManualTxWordsQuantity(reader.attributes().value("Port1TxWordsQuantity").toInt());
        }

        if (reader.attributes().hasAttribute("Port1RxWordsQuantity"))
        {
            setPort1ManualRxWordsQuantity(reader.attributes().value("Port1RxWordsQuantity").toInt());
        }

		if (reader.attributes().hasAttribute("Port2TxStartAddress"))
		{
			setPort2ManualTxStartAddress(reader.attributes().value("Port2TxStartAddress").toInt());
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

    bool Connection::remove(DbController *db, bool &fileRemoved)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }

        fileRemoved = false;

        // Load the file from the database
        //
        std::vector<DbFileInfo> fileList;
        bool ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
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

        if (file->state() != VcsState::CheckedOut)
        {
            if (db->checkOut(fileList[0], nullptr) == false)
            {
                return false;
            }
        }

        ok = db->deleteFiles(&fileList, nullptr);
        if (ok == false)
        {
            return false;
        }

        // checkin file if it exists


        ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            fileRemoved = true;
        }

        return true;
    }

    bool Connection::checkOut(DbController *db)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
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

    bool Connection::checkIn(DbController *db, const QString& comment)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
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

    bool Connection::undo(DbController *db, bool& fileRemoved)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }

        fileRemoved = false;

        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
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

        // after undo operation, file can be removed, check this

        ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            fileRemoved = true;

            return true;
        }

        // read previous data from file

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        QByteArray data;
        file->swapData(data);

        if (load(data) == false)
        {
            return false;
        }

        return true;
    }

    bool Connection::vcsStateAndAction(DbController *db, VcsState& state, VcsItemAction &action)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }
        std::shared_ptr<DbFile> file = nullptr;

        std::vector<DbFileInfo> fileList;

        bool ok = db->getFileList(&fileList, db->connectionsFileId(), m_fileName, true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        ok = db->getLatestVersion(fileList[0], &file, nullptr);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        state = file->state();

        action = file->action();

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

    int Connection::fileID() const
    {
        return m_fileID;
    }

    void Connection::setFileID(int value)
    {
        m_fileID = value;
    }

    QString Connection::fileName() const
    {
        return m_fileName;
    }

    void Connection::setFileName(const QString& value)
    {
        m_fileName = value;
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

	int Connection::port1ManualTxStartAddress() const
	{
		return m_port1TxStartAddress;
	}

	void Connection::setPort1ManualTxStartAddress(int value)
	{
		m_port1TxStartAddress = value;
	}

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

	QString Connection::port1RawDataDescription() const
	{
		return m_port1RawDataDescription;
	}

	void Connection::setPort1RawDataDescription(const QString& value)
	{
		m_port1RawDataDescription = value;
	}

    //
    // port2
    //

	int Connection::port2ManualTxStartAddress() const
	{
		return m_port2TxStartAddress;
	}

	void Connection::setPort2ManualTxStartAddress(int value)
	{
		m_port2TxStartAddress = value;
	}

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

	QString Connection::port2RawDataDescription() const
	{
		return m_port2RawDataDescription;
	}

	void Connection::setPort2RawDataDescription(const QString& value)
	{
		m_port2RawDataDescription = value;
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
				propertyVisibilityChanger("EnableSerial", false);
                propertyVisibilityChanger("EnableDuplex", false);

				propertyVisibilityChanger("Port2EquipmentID", true);
				propertyVisibilityChanger("Port2TxStartAddress", true);
                propertyVisibilityChanger("Port2TxWordsQuantity", true);
                propertyVisibilityChanger("Port2RxWordsQuantity", true);


                break;
            case OptoPort::Mode::Serial:
                propertyVisibilityChanger("SerialMode", true);
				propertyVisibilityChanger("EnableSerial", true);
                propertyVisibilityChanger("EnableDuplex", true);

				propertyVisibilityChanger("Port2EquipmentID", false);
				propertyVisibilityChanger("Port2TxStartAddress", false);
                propertyVisibilityChanger("Port2TxWordsQuantity", false);
                propertyVisibilityChanger("Port2RxWordsQuantity", false);
                break;
            default:
                assert(false);
        }
    }

	bool Connection::enableSerial() const
    {
		return m_enableSerial;
    }

	void Connection::setEnableSerial(bool value)
    {
		m_enableSerial = value;
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

    bool Connection::disableDataID() const
	{
		return m_disableDataID;
	}

	void Connection::setDisableDataID(bool value)
	{
		m_disableDataID = value;
	}

	bool Connection::generateVHDFile() const
	{
		return m_generateVHDFile;
	}

	void Connection::setGenerateVHDFile(bool value)
	{
		m_generateVHDFile = value;
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

		int i = 1;
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
			int i = 1;
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
        bool ok = db->getFileList(&fileList, db->connectionsFileId(), true, nullptr);
        if (ok == false)
        {
            return true;
        }

        for (DbFileInfo& dbfi : fileList)
        {
            std::shared_ptr<DbFile> file = nullptr;
            ok = db->getLatestVersion(dbfi, &file, nullptr);
            if (ok == false || file == nullptr)
            {
                errorCode = tr("Failed to load file %1").arg(dbfi.fileName());
                return false;
            }

            std::shared_ptr<Hardware::Connection> s = std::make_shared<Hardware::Connection>();

            QByteArray data;
            file->swapData(data);

            if (s->load(data) == true)
            {
                s->setFileName(dbfi.fileName());
                s->setFileID(dbfi.fileId());

                m_connections.push_back(s);
            }
            else
            {
                return false;
            }
        }

        return true;
    }


    bool ConnectionStorage::loadFromXmlDeprecated(DbController* db, QString &errorCode)
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
        bool ok = db->getFileList(&fileList, db->mcFileId(), "Connections.xml", true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
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

                if (s->loadFromXml(reader) == true)
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

    bool ConnectionStorage::deleteXmlDeprecated(DbController* db)
    {
        if (db == nullptr)
        {
            assert(db);
            return false;
        }

        // Load the file from the database
        //
        std::vector<DbFileInfo> fileList;
        bool ok = db->getFileList(&fileList, db->mcFileId(), "Connections.xml", true, nullptr);
        if (ok == false || fileList.size() != 1)
        {
            return true;
        }

        std::shared_ptr<DbFile> file = nullptr;

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

        ok = db->deleteFiles(&fileList, nullptr);
        if (ok == false)
        {
            return false;
        }

        if (db->checkIn(fileList[0], "Deleted deprecated file Connections.xml", nullptr) == false)
        {
            return false;
        }

        if (ok == false)
        {
            return false;
        }

        return true;
    }

}

