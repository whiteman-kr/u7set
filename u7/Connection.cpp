#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "../lib/CUtils.h"
#include "../lib/WUtils.h"
#include "../lib/ProtoSerialization.h"

#include "Connection.h"
#include "./Builder/OptoModule.h"

namespace Hardware
{

    //
    //
    // Connection
    //
    //
    Connection::Connection()
    {

        QUuid uuid = QUuid::createUuid();
        setUuid(uuid);

        QString f = QString("connection-%1.%2").arg(uuid.toString()).arg(::OclFileExtension);
        f.remove('{');
        f.remove('}');
        m_fileInfo.setFileName(f);

        static QString s_conectionId = "ConnectionID";
        static QString s_fileName = "FileName";
        static QString s_fileID = "FileID";
        static QString s_port1EquipmentID = "Port1EquipmentID";
        static QString s_port2EquipmentID = "Port2EquipmentID";
        static QString s_port1RawDataDescription = "Port1RawDataDescription";
        static QString s_port2RawDataDescription = "Port2RawDataDescription";

        static QString s_port1TxStartAddress = "Port1TxStartAddress";
        static QString s_port1TxWordsQuantity = "Port1TxWordsQuantity";
        static QString s_port1RxWordsQuantity = "Port1RxWordsQuantity";
        static QString s_port1TxRxOptoID = "Port1TxRxOptoID";
        static QString s_port1TxRxOptoDataUID = "Port1TxRxOptoDataUID";
        static QString s_port1TxRsID = "Port1TxRsID";
        static QString s_port1TxRsDataUID = "Port1TxRsDataUID";

        static QString s_port2TxStartAddress = "Port2TxStartAddress";
        static QString s_port2TxWordsQuantity = "Port2TxWordsQuantity";
        static QString s_port2RxWordsQuantity = "Port2RxWordsQuantity";
        static QString s_port2TxRxOptoID = "Port2TxRxOptoID";
        static QString s_port2TxRxOptoDataUID = "Port2TxRxOptoDataUID";
        static QString s_port2TxRsID = "Port2TxRsID";
        static QString s_port2TxRsDataUID = "Port2TxRsDataUID";

		static QString s_port1EnableSerial = "Port1EnableSerial";
		static QString s_port1SerialMode = "Port1SerialMode";
		static QString s_port1EnableDuplex = "Port1EnableDuplex";

		static QString s_port2EnableSerial = "Port2EnableSerial";
		static QString s_port2SerialMode = "Port2SerialMode";
		static QString s_port2EnableDuplex = "Port2EnableDuplex";

		static QString s_enableManualSettings = "EnableManualSettings";
        static QString s_mode = "Mode";
        static QString s_disableDataIDControl = "Disable DataID Control";
        static QString s_generateConnectionVHDfile = "Generate connection VHD file";

        static QString s_miscellaneous = "Miscellaneous";
        static QString s_manualSettings = "Manual Settings";
        static QString s_serialCommunicationsOCM = "Serial Communications (OCM)";

		auto propConnId = ADD_PROPERTY_GETTER_SETTER(QString, s_conectionId, true, Connection::connectionID, Connection::setConnectionID);
        propConnId->setValidator("^[A-Z0-9_]+$");

        auto propFileName = ADD_PROPERTY_GETTER(QString, s_fileName, true, Connection::fileName);
        propFileName->setExpert(true);

		auto propFileID = ADD_PROPERTY_GETTER(int, s_fileID, true, Connection::fileId);
        propFileID->setExpert(true);

		ADD_PROPERTY_GETTER_SETTER(QString, s_port1EquipmentID, true, Connection::port1EquipmentID, Connection::setPort1EquipmentID);
		ADD_PROPERTY_GETTER_SETTER(QString, s_port2EquipmentID, true, Connection::port2EquipmentID, Connection::setPort2EquipmentID);

        auto propPort1RawDataDescription = ADD_PROPERTY_GETTER_SETTER(QString, s_port1RawDataDescription, true, Connection::port1RawDataDescription, Connection::setPort1RawDataDescription);
        propPort1RawDataDescription->setCategory(s_miscellaneous);

        auto propPort2RawDataDescription = ADD_PROPERTY_GETTER_SETTER(QString, s_port2RawDataDescription, true, Connection::port2RawDataDescription, Connection::setPort2RawDataDescription);
        propPort2RawDataDescription->setCategory(s_miscellaneous);

        auto proptx1sa = ADD_PROPERTY_GETTER_SETTER(int, s_port1TxStartAddress, true, Connection::port1ManualTxStartAddress, Connection::setPort1ManualTxStartAddress);
        proptx1sa->setCategory(s_manualSettings);

        auto proptx1wq = ADD_PROPERTY_GETTER_SETTER(int, s_port1TxWordsQuantity, true, Connection::port1ManualTxWordsQuantity, Connection::setPort1ManualTxWordsQuantity);
        proptx1wq->setCategory(s_manualSettings);

        auto proprx1wq = ADD_PROPERTY_GETTER_SETTER(int, s_port1RxWordsQuantity, true, Connection::port1ManualRxWordsQuantity, Connection::setPort1ManualRxWordsQuantity);
        proprx1wq->setCategory(s_manualSettings);

        ADD_PROPERTY_GETTER_SETTER(int, s_port1TxRxOptoID, false, Connection::port1TxRxOptoID, Connection::setPort1TxRxOptoID);
        ADD_PROPERTY_GETTER_SETTER(quint32, s_port1TxRxOptoDataUID, false, Connection::port1TxRxOptoDataUID, Connection::setPort1TxRxOptoDataUID);

        ADD_PROPERTY_GETTER_SETTER(int, s_port1TxRsID, false, Connection::port1TxRsID, Connection::setPort1TxRsID);
        ADD_PROPERTY_GETTER_SETTER(quint32, s_port1TxRsDataUID, false, Connection::port1TxRsDataUID, Connection::setPort1TxRsDataUID);

        auto proptx2sa = ADD_PROPERTY_GETTER_SETTER(int, s_port2TxStartAddress, true, Connection::port2ManualTxStartAddress, Connection::setPort2ManualTxStartAddress);
        proptx2sa->setCategory(s_manualSettings);

        auto proptx2wq = ADD_PROPERTY_GETTER_SETTER(int, s_port2TxWordsQuantity, true, Connection::port2ManualTxWordsQuantity, Connection::setPort2ManualTxWordsQuantity);
        proptx2wq->setCategory(s_manualSettings);

        auto proprx2wq = ADD_PROPERTY_GETTER_SETTER(int, s_port2RxWordsQuantity, true, Connection::port2ManualRxWordsQuantity, Connection::setPort2ManualRxWordsQuantity);
        proprx2wq->setCategory(s_manualSettings);

        ADD_PROPERTY_GETTER_SETTER(int, s_port2TxRxOptoID, false, Connection::port2TxRxOptoID, Connection::setPort2TxRxOptoID);
        ADD_PROPERTY_GETTER_SETTER(quint32, s_port2TxRxOptoDataUID, false, Connection::port2TxRxOptoDataUID, Connection::setPort2TxRxOptoDataUID);

        ADD_PROPERTY_GETTER_SETTER(int, s_port2TxRsID, false, Connection::port2TxRsID, Connection::setPort2TxRsID);
        ADD_PROPERTY_GETTER_SETTER(quint32, s_port2TxRsDataUID, false, Connection::port2TxRsDataUID, Connection::setPort2TxRsDataUID);

		auto propEnableSerial = ADD_PROPERTY_GETTER_SETTER(bool, s_port1EnableSerial, true, Connection::port1EnableSerial, Connection::setPort1EnableSerial);
		propEnableSerial->setCategory(s_serialCommunicationsOCM);

		propEnableSerial = ADD_PROPERTY_GETTER_SETTER(bool, s_port2EnableSerial, true, Connection::port2EnableSerial, Connection::setPort2EnableSerial);
		propEnableSerial->setCategory(s_serialCommunicationsOCM);

		auto propSerialMode = ADD_PROPERTY_GETTER_SETTER(Connection::SerialMode, s_port1SerialMode, true, Connection::port1SerialMode, Connection::setPort1SerialMode);
        propSerialMode->setCategory(s_serialCommunicationsOCM);

		propSerialMode = ADD_PROPERTY_GETTER_SETTER(Connection::SerialMode, s_port2SerialMode, true, Connection::port2SerialMode, Connection::setPort2SerialMode);
		propSerialMode->setCategory(s_serialCommunicationsOCM);

		auto propEnableDuplex = ADD_PROPERTY_GETTER_SETTER(bool, s_port1EnableDuplex, true, Connection::port1EnableDuplex, Connection::setPort1EnableDuplex);
        propEnableDuplex->setCategory(s_serialCommunicationsOCM);

		propEnableDuplex = ADD_PROPERTY_GETTER_SETTER(bool, s_port2EnableDuplex, true, Connection::port2EnableDuplex, Connection::setPort2EnableDuplex);
		propEnableDuplex->setCategory(s_serialCommunicationsOCM);

		auto propManual = ADD_PROPERTY_GETTER_SETTER(bool, s_enableManualSettings, true, Connection::manualSettings, Connection::setManualSettings);
        propManual->setCategory(s_manualSettings);

		ADD_PROPERTY_GETTER_SETTER(Connection::Type, s_mode, true, Connection::type, Connection::setType);

		auto propDisableDataID = ADD_PROPERTY_GETTER_SETTER(bool, s_disableDataIDControl, true, Connection::disableDataId, Connection::setDisableDataID);
        propDisableDataID->setCategory(s_miscellaneous);

		auto propGenerateVHD = ADD_PROPERTY_GETTER_SETTER(bool, s_generateConnectionVHDfile, true, Connection::generateVHDFile, Connection::setGenerateVHDFile);
        propGenerateVHD->setCategory(s_miscellaneous);
	}

    bool Connection::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);

		::Proto::Connection* mutableConnection = message->mutable_connection();

        Proto::Write(mutableConnection->mutable_uuid(), m_uuid);
		mutableConnection->set_connectionid(m_connectionID.toUtf8());
		mutableConnection->set_port1equipmentid(m_port1EquipmentID.toUtf8());
		mutableConnection->set_port2equipmentid(m_port2EquipmentID.toUtf8());
		mutableConnection->set_port1rawdatadescription(m_port1RawDataDescription.toUtf8());
		mutableConnection->set_port2rawdatadescription(m_port2RawDataDescription.toUtf8());

		mutableConnection->set_type(static_cast<int>(type()));

		mutableConnection->set_port1enableserial(m_port1EnableSerial);
		mutableConnection->set_port1serialmode(static_cast<int>(port1SerialMode()));
		mutableConnection->set_port1enableduplex(m_port1EnableDuplex);

		mutableConnection->set_port2enableserial(m_port2EnableSerial);
		mutableConnection->set_port2serialmode(static_cast<int>(port2SerialMode()));
		mutableConnection->set_port2enableduplex(m_port2EnableDuplex);

		mutableConnection->set_manualsettings(m_manualSettings);
		mutableConnection->set_disabledataid(m_disableDataID);
		mutableConnection->set_generatevhdfile(m_generateVHDFile);

		mutableConnection->set_port1txstartaddress(m_port1TxStartAddress);
		mutableConnection->set_port1txwordsquantity(m_port1ManualTxWordsQuantity);
		mutableConnection->set_port1rxwordsquantity(m_port1ManualRxWordsQuantity);

		mutableConnection->set_port2txstartaddress(m_port2TxStartAddress);
		mutableConnection->set_port2txwordsquantity(m_port2ManualTxWordsQuantity);
		mutableConnection->set_port2rxwordsquantity(m_port2ManualRxWordsQuantity);

		return true;
	}

	bool Connection::LoadData(const Proto::Envelope& message)
	{
		if (message.has_connection() == false)
		{
			assert(message.has_connection());
			return false;
		}

		const ::Proto::Connection& connection = message.connection();

        m_uuid = Proto::Read(connection.uuid());
		m_connectionID = connection.connectionid().c_str();
		m_port1EquipmentID = connection.port1equipmentid().c_str();
		m_port2EquipmentID = connection.port2equipmentid().c_str();
		m_port1RawDataDescription = connection.port1rawdatadescription().c_str();
		m_port2RawDataDescription = connection.port2rawdatadescription().c_str();

		m_type = static_cast<Connection::Type>(connection.type());

		m_port1EnableSerial = connection.port1enableserial();
		m_port1SerialMode = static_cast<Connection::SerialMode>(connection.port1serialmode());
		m_port1EnableDuplex = connection.port1enableduplex();

		m_port2EnableSerial = connection.port2enableserial();
		m_port2SerialMode = static_cast<Connection::SerialMode>(connection.port2serialmode());
		m_port2EnableDuplex = connection.port2enableduplex();

		m_manualSettings = connection.manualsettings();
		m_disableDataID = connection.disabledataid();
		m_generateVHDFile = connection.generatevhdfile();

		m_port1TxStartAddress = connection.port1txstartaddress();
		m_port1ManualTxWordsQuantity = connection.port1txwordsquantity();
		m_port1ManualRxWordsQuantity = connection.port1rxwordsquantity();

		m_port2TxStartAddress = connection.port2txstartaddress();
		m_port2ManualTxWordsQuantity = connection.port2txwordsquantity();
		m_port2ManualRxWordsQuantity = connection.port2rxwordsquantity();

		// obsolete fields, needed to read projects until 30.05.2017

		if (connection.has_obsoletemode())
		{
			//const int Mode_Optical = 0;
			const int Mode_Serial = 1;

			int obsolete_mode = connection.obsoletemode();

			if (obsolete_mode == Mode_Serial)
			{
				setType(Connection::Type::SinglePort);
				m_port1EnableSerial = true;
			}
		}

		if (connection.has_obsoleteserialmode())
		{
			m_port1SerialMode = static_cast<Connection::SerialMode>(connection.obsoleteserialmode());
			m_port2SerialMode = static_cast<Connection::SerialMode>(connection.obsoleteserialmode());
		}
		if (connection.has_obsoleteenableduplex())
		{
			m_port1EnableDuplex = connection.obsoleteenableduplex();
			m_port2EnableDuplex = connection.obsoleteenableduplex();
		}

		return true;
	}

	std::shared_ptr<Connection> Connection::CreateObject(const Proto::Envelope& message)
	{
		// This func can create only one instance
		//
		if (message.has_connection() == false)
		{
			assert(message.has_connection());
			return nullptr;
		}

		std::shared_ptr<Connection> connection = std::make_shared<Connection>();

		connection->LoadData(message);

		return connection;
	}

    bool Connection::loadFromXml(QXmlStreamReader& reader)
    {
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
			setPort1SerialMode(static_cast<Connection::SerialMode>(reader.attributes().value("SerialMode").toInt()));
			setPort2SerialMode(static_cast<Connection::SerialMode>(reader.attributes().value("SerialMode").toInt()));
		}

        if (reader.attributes().hasAttribute("Mode"))
        {
			//const int Mode_Optical = 0;
			const int Mode_Serial = 1;

			int obsolete_mode = reader.attributes().value("Mode").toInt();

			if (obsolete_mode == Mode_Serial)
			{
				setType(Connection::Type::SinglePort);
				m_port1EnableSerial = true;
			}
        }

        if (reader.attributes().hasAttribute("EnableDuplex"))
        {
			setPort1EnableDuplex(reader.attributes().value("EnableDuplex").toString() == "true" ? true : false);
			setPort2EnableDuplex(reader.attributes().value("EnableDuplex").toString() == "true" ? true : false);
		}

        if (reader.attributes().hasAttribute("ManualSettings"))
        {
            setManualSettings(reader.attributes().value("ManualSettings").toString() == "true" ? true : false);
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


	const DbFileInfo& Connection::fileInfo() const
	{
		return m_fileInfo;
	}

	void Connection::setFileInfo(const DbFileInfo& value)
	{
		m_fileInfo = value;
	}

    QUuid Connection::uuid() const
    {
        return m_uuid;
    }

    void Connection::setUuid(const QUuid& value)
    {
        m_uuid = value;
    }

	QString Connection::connectionID() const
    {
		return m_connectionID;
    }

	void Connection::setConnectionID(const QString& value)
    {
		m_connectionID = value;
    }

    QString Connection::fileName() const
    {
        return fileInfo().fileName();
    }

	int Connection::fileId() const
    {
        return fileInfo().fileId();
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

	Connection::SerialMode Connection::port1SerialMode() const
    {
		return m_port1SerialMode;
    }

	void Connection::setPort1SerialMode(const Connection::SerialMode value)
    {
		m_port1SerialMode = value;
    }

	Connection::SerialMode Connection::port2SerialMode() const
	{
		return m_port2SerialMode;
	}

	void Connection::setPort2SerialMode(const Connection::SerialMode value)
	{
		m_port2SerialMode = value;
	}

	QString Connection::serialModeStr(const Connection::SerialMode value)
	{
		switch(value)
		{
		case SerialMode::RS232:
			return "RS232";

		case SerialMode::RS485:
			return "RS485";

		default:
			assert(false);
		}

		return "RS???";
	}

	Connection::Type Connection::type() const
    {
		return m_type;
    }

	void Connection::setType(const Connection::Type value)
    {
		m_type = value;
    }

	QString Connection::typeStr() const
	{
		return typeStr(m_type);
	}

	QString Connection::typeStr(Connection::Type t)
	{
		switch(t)
		{
		case Type::PortToPort:
			return "PortToPort";

		case Type::SinglePort:
			return "SinglePort";

		default:
			assert(false);
		}

		return "Type???";
	}

	bool Connection::isPortToPort() const
	{
		return m_type == Type::PortToPort;
	}

	bool Connection::isSinglePort() const
	{
		return m_type == Type::SinglePort;
	}

	bool Connection::port1EnableSerial() const
	{
		return m_port1EnableSerial;
	}

	void Connection::setPort1EnableSerial(bool value)
	{
		m_port1EnableSerial = value;
	}

	bool Connection::port2EnableSerial() const
	{
		return m_port2EnableSerial;
	}

	void Connection::setPort2EnableSerial(bool value)
	{
		m_port2EnableSerial = value;
	}

	bool Connection::port1EnableDuplex() const
    {
		return m_port1EnableDuplex;
    }

	void Connection::setPort1EnableDuplex(bool value)
    {
		m_port1EnableDuplex = value;
    }

	bool Connection::port2EnableDuplex() const
	{
		return m_port2EnableDuplex;
	}

	void Connection::setPort2EnableDuplex(bool value)
	{
		m_port2EnableDuplex = value;
	}

	bool Connection::manualSettings() const
    {
        return m_manualSettings;
    }

    void Connection::setManualSettings(bool value)
    {
        m_manualSettings = value;
    }

	bool Connection::disableDataId() const
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

    ConnectionStorage::ConnectionStorage(DbController* db, QWidget *parentWidget)
        :QObject(parentWidget),
         m_db(db),
         m_parentWidget(parentWidget)
    {
    }

    ConnectionStorage::~ConnectionStorage()
    {

    }

    void ConnectionStorage::clear()
    {
        m_connections.clear();
        m_connectionsVector.clear();
    }

    void ConnectionStorage::add(std::shared_ptr<Connection> connection)
    {
        if (connection == nullptr)
        {
            assert(connection);
            return;
        }

        if (m_connections.find(connection->uuid()) != m_connections.end())
        {
            assert(false);
            return;
        }

        m_connections[connection->uuid()] = connection;

        m_connectionsVector.push_back(connection);

        assert (m_connectionsVector.size() == m_connections.size());
    }

    void ConnectionStorage::remove(const QUuid &uuid)
    {
        auto it = m_connections.find(uuid);
        if (it == m_connections.end())
        {
            assert(false);
            return;
        }

        m_connections.erase(it);

        bool vectorFound = false;

        int count = (int)m_connectionsVector.size();
        for (int i = 0; i < count; i++)
        {
            if (m_connectionsVector[i]->uuid() == uuid)
            {
                m_connectionsVector.erase(m_connectionsVector.begin() + i);

                vectorFound = true;

                break;
            }
        }

        if (vectorFound == false)
        {
            assert(false);
        }

        assert (m_connectionsVector.size() == m_connections.size());

    }


    bool ConnectionStorage::removeFile(const QUuid &uuid, bool &fileRemoved)
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        auto it = m_connections.find(uuid);
        if (it == m_connections.end())
        {
            assert(false);
            return false;
        }

        std::shared_ptr<Connection> connection = it->second;
        if (connection == nullptr)
        {
            assert(connection);
            return false;
        }

        fileRemoved = false;

        std::vector<DbFileInfo> fileList;
        bool ok = m_db->getFileList(&fileList, m_db->connectionsFileId(), connection->fileName(), true, m_parentWidget);
        if (ok == false || fileList.size() != 1)
        {
            return false;
        }

        std::shared_ptr<DbFile> file = nullptr;

        ok = m_db->getLatestVersion(fileList[0], &file, m_parentWidget);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() != VcsState::CheckedOut)
        {
            if (m_db->checkOut(fileList[0], m_parentWidget) == false)
            {
                return false;
            }
        }

        ok = m_db->deleteFiles(&fileList, m_parentWidget);
        if (ok == false)
        {
            return false;
        }

        // checkin file if it exists

        DbFileInfo& fi = fileList[0];

        if (fi.deleted() == true)
        {
            fileRemoved = true;
        }

        connection->setFileInfo(fi);

        return true;
    }


    int ConnectionStorage::count() const
    {
        return static_cast<int>(m_connections.size());
    }

    std::shared_ptr<Connection> ConnectionStorage::get(const QUuid& uuid) const
    {
        auto it = m_connections.find(uuid);
        if (it == m_connections.end())
        {
            assert(false);
            return std::make_shared<Connection>();
        }

        return it->second;
    }

    std::shared_ptr<Connection> ConnectionStorage::get(int index) const
    {
        if (index < 0 || index >= (int)m_connectionsVector.size())
        {
            assert(false);
            return nullptr;
        }

        return m_connectionsVector[index];
    }

	std::vector<std::shared_ptr<Connection>> ConnectionStorage::get(const QStringList& masks) const
	{
		if (masks.empty() == true)
		{
			return m_connectionsVector;
		}

		std::vector<std::shared_ptr<Connection>> result;

		for (std::shared_ptr<Connection> connection : m_connectionsVector)
		{
			for (const QString& mask : masks)
			{
				if (connection->connectionID().contains(mask, Qt::CaseInsensitive) == true ||
						connection->port1EquipmentID().contains(mask, Qt::CaseInsensitive) == true ||
						connection->port2EquipmentID().contains(mask, Qt::CaseInsensitive) == true ||
						connection->fileName().contains(mask, Qt::CaseInsensitive) == true)
				{
					result.push_back(connection);
				}
			}
		}

		return result;
	}

    QObject* ConnectionStorage::jsGet(int index) const
    {
        if (index < 0 || index >= (int)m_connectionsVector.size())
        {
            assert(false);
            return nullptr;
        }

        if (m_connectionsVector.size() != m_connections.size())
        {
            assert(false);
            nullptr;
        }

        QObject* result = m_connectionsVector[index].get();
        QQmlEngine::setObjectOwnership(result, QQmlEngine::CppOwnership);

        return result;
    }

	std::shared_ptr<Connection> ConnectionStorage::getPortConnection(QString portEquipmentId) const
	{
		for (const std::shared_ptr<Connection>& connection : m_connectionsVector)
		{
			assert(connection);

			if (connection != nullptr &&
				(connection->port1EquipmentID() == portEquipmentId || connection->port2EquipmentID() == portEquipmentId))
			{
				return connection;
			}
		}

		return std::shared_ptr<Connection>();
	}

    bool ConnectionStorage::checkOut(const QUuid &uuid)
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        std::shared_ptr<Connection> connection = get(uuid);
        if (connection == nullptr)
        {
            assert(connection);
            return false;
        }

        DbFileInfo fi = connection->fileInfo();

        if (fi.state() == VcsState::CheckedOut)
        {
            return true;
        }

        if (m_db->checkOut(fi, m_parentWidget) == false)
        {
            return false;
        }

        connection->setFileInfo(fi);

        return true;
    }

    bool ConnectionStorage::checkIn(const QUuid &uuid, const QString& comment, bool &fileRemoved)
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        std::shared_ptr<Connection> connection = get(uuid);
        if (connection == nullptr)
        {
            assert(connection);
            return false;
        }

        fileRemoved = false;

        DbFileInfo fi = connection->fileInfo();

        if (fi.state() == VcsState::CheckedIn)
        {
            return true;
        }

        if (m_db->checkIn(fi, comment, m_parentWidget) == false)
        {
            return false;
        }

        if (fi.deleted() == true)
        {
            fileRemoved = true;
        }

        connection->setFileInfo(fi);

        return true;
    }

    bool ConnectionStorage::undo(const QUuid &uuid, bool &fileRemoved)
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        std::shared_ptr<Connection> connection = get(uuid);
        if (connection == nullptr)
        {
            assert(connection);
            return false;
        }

        fileRemoved = false;

        DbFileInfo fi = connection->fileInfo();

        if (fi.state() != VcsState::CheckedOut)
        {
            return true;
        }

        if (m_db->undoChanges(fi, m_parentWidget) == false)
        {
            return false;
        }

        // after undo operation, file can be removed, check this

        connection->setFileInfo(fi);

        if (fi.deleted() == true)
        {
            fileRemoved = true;

            return true;
        }

        // read previous data from file

        std::shared_ptr<DbFile> file = nullptr;

        bool ok = m_db->getLatestVersion(fi, &file, m_parentWidget);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        QByteArray data;
        file->swapData(data);

        if (connection->Load(data) == false)
        {
            return false;
        }

        return true;
    }

    bool ConnectionStorage::load()
    {
        bool ok = loadFromConnectionsFolder();
        if (ok == false)
        {
            return false;
        }

        QString errorCode;
        ok = loadFromXmlDeprecated(errorCode);
        if (ok == false)
        {
            return false;
        }

        return true;
    }

    bool ConnectionStorage::loadFromConnectionsFolder()
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        // Load the file from the database
        //
		std::vector<DbFileInfo> fileList;
        bool ok = m_db->getFileList(&fileList, m_db->connectionsFileId(), ::OclFileExtension, true, m_parentWidget);
        if (ok == false)
        {
            return true;
        }

		for (DbFileInfo& fi : fileList)
        {
            std::shared_ptr<DbFile> file = nullptr;

            ok = m_db->getLatestVersion(fi, &file, m_parentWidget);
            if (ok == false || file == nullptr)
            {
                return false;
            }

			std::shared_ptr<Hardware::Connection> c = Hardware::Connection::Create(file->data());

			if (c != nullptr)
            {
				c->setFileInfo(*file);
                add(c);
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool ConnectionStorage::save(const QUuid &uuid)
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        auto it = m_connections.find(uuid);
        if (it == m_connections.end())
        {
            assert(false);
            return false;
        }

        std::shared_ptr<Connection> connection = it->second;
        if (connection == nullptr)
        {
            assert(connection);
            return false;
        }

        Proto::Envelope message;
        connection->Save(&message);

        QByteArray data;

        int size = message.ByteSize();
        data.resize(size);

        message.SerializeToArray(data.data(), size);

        // save to db
        //



        if (connection->fileInfo().isNull() == true)
        {
            // create a file, if it does not exists
            //
            std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

            file->setFileName(connection->fileName());

            file->swapData(data);

            if (m_db->addFile(file, m_db->connectionsFileId(), m_parentWidget) == false)
            {
                return false;
            }

            connection->setFileInfo(*file);
        }
        else
        {
            std::shared_ptr<DbFile> file = nullptr;

            // save to existing file
            //
            bool ok = m_db->getLatestVersion(connection->fileInfo(), &file, m_parentWidget);
            if (ok == false || file == nullptr)
            {
                return false;
            }

            if (file->state() != VcsState::CheckedOut)
            {
                return false;
            }

            file->swapData(data);

            if (m_db->setWorkcopy(file, m_parentWidget) == false)
            {
                return false;
            }

            connection->setFileInfo(*file);
        }

        return true;

    }

    bool ConnectionStorage::loadFromXmlDeprecated(QString& errorCode)
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        // Load the file from the database
        //

        std::vector<DbFileInfo> fileList;
        bool ok = m_db->getFileList(&fileList, m_db->mcFileId(), "Connections.xml", true, m_parentWidget);
        if (ok == false || fileList.size() != 1)
        {
            return true;
        }

        std::shared_ptr<DbFile> file = nullptr;
        ok = m_db->getLatestVersion(fileList[0], &file, m_parentWidget);
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
                std::shared_ptr<Hardware::Connection> c = std::make_shared<Hardware::Connection>();

                if (c->loadFromXml(reader) == true)
                {
                    add(c);
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

    bool ConnectionStorage::deleteXmlDeprecated()
    {
        if (m_db == nullptr)
        {
            assert(m_db);
            return false;
        }

        // Load the file from the database
        //
        std::vector<DbFileInfo> fileList;
        bool ok = m_db->getFileList(&fileList, m_db->mcFileId(), "Connections.xml", true, m_parentWidget);
        if (ok == false || fileList.size() != 1)
        {
            return true;
        }

        std::shared_ptr<DbFile> file = nullptr;

        ok = m_db->getLatestVersion(fileList[0], &file, m_parentWidget);
        if (ok == false || file == nullptr)
        {
            return false;
        }

        if (file->state() != VcsState::CheckedOut)
        {
            if (m_db->checkOut(fileList[0], m_parentWidget) == false)
            {
                return false;
            }
        }

        ok = m_db->deleteFiles(&fileList, m_parentWidget);
        if (ok == false)
        {
            return false;
        }

        if (m_db->checkIn(fileList[0], "Deleted deprecated file Connections.xml", m_parentWidget) == false)
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

