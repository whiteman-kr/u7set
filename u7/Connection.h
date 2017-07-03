#ifndef CONNECTION_H
#define CONNECTION_H

#include "../lib/DbController.h"
#include "DbObjectStorage.h"

namespace Hardware
{
	class Connection :
			public PropertyObject,
			public Proto::ObjectSerialization<Connection>
	{
		Q_OBJECT

	public:
		enum class Type
		{
			PortToPort,
			SinglePort
		};
		Q_ENUM(Type)

		enum class SerialMode
		{
			RS232,
			RS485
		};
		Q_ENUM(SerialMode)

	public:
		Connection();

		// Serialization
		//
	public:
		friend Proto::ObjectSerialization<Connection>;	// for call CreateObject from Proto::ObjectSerialization

	protected:
		// Implementing Proto::ObjectSerialization<DeviceObject>::SaveData, LoadData
		//
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this function only while serialization, as when object is created is not fully initialized
		// and must be read before use
		//
		static std::shared_ptr<Connection> CreateObject(const Proto::Envelope& message);

	public:

        // Load connection from depreated XML document
        //
        bool loadFromXml(QXmlStreamReader& reader);

        // Properties
		//
	public:

        QUuid uuid() const;
        void setUuid(const QUuid& value);

		QString connectionID() const;
		void setConnectionID(const QString& value);

		QString port1EquipmentID() const;
		void setPort1EquipmentID(const QString& value);

		QString port2EquipmentID() const;
		void setPort2EquipmentID(const QString& value);

		quint16 getID() const;
		quint16 linkID() const { return getID(); }

		//
		//
		//

		int port1ManualTxStartAddress() const;
		void setPort1ManualTxStartAddress(int value);

		int port1ManualTxWordsQuantity() const;
		void setPort1ManualTxWordsQuantity(int value);

		int port1ManualRxWordsQuantity() const;
		void setPort1ManualRxWordsQuantity(int value);

		int port1TxRxOptoID() const;
		void setPort1TxRxOptoID(int value);

		quint32 port1TxRxOptoDataUID() const;
		void setPort1TxRxOptoDataUID(quint32 value);

		int port1TxRsID() const;
		void setPort1TxRsID(int value);

		quint32 port1TxRsDataUID() const;
		void setPort1TxRsDataUID(quint32 value);

		QString port1RawDataDescription() const;
		void setPort1RawDataDescription(const QString& value);

		//
		//
		//

		int port2ManualTxStartAddress() const;
		void setPort2ManualTxStartAddress(int value);

		int port2ManualTxWordsQuantity() const;
		void setPort2ManualTxWordsQuantity(int value);

		int port2ManualRxWordsQuantity() const;
		void setPort2ManualRxWordsQuantity(int value);

		int port2TxRxOptoID() const;
		void setPort2TxRxOptoID(int value);

		quint32 port2TxRxOptoDataUID() const;
		void setPort2TxRxOptoDataUID(quint32 value);

		int port2TxRsID() const;
		void setPort2TxRsID(int value);

		quint32 port2TxRsDataUID() const;
		void setPort2TxRsDataUID(quint32 value);

		QString port2RawDataDescription() const;
		void setPort2RawDataDescription(const QString& value);

		//
		//
		//

		static QString serialModeStr(const Connection::SerialMode value);

		Type type() const;
		void setType(const Type value);

		QString typeStr() const;
		static QString typeStr(Connection::Type t);

		bool isPortToPort() const;
		bool isSinglePort() const;

		bool port1EnableSerial() const;
		void setPort1EnableSerial(bool value);

		bool port2EnableSerial() const;
		void setPort2EnableSerial(bool value);

		SerialMode port1SerialMode() const;
		void setPort1SerialMode(const SerialMode value);

		SerialMode port2SerialMode() const;
		void setPort2SerialMode(const SerialMode value);

		bool port1EnableDuplex() const;
		void setPort1EnableDuplex(bool value);

		bool port2EnableDuplex() const;
		void setPort2EnableDuplex(bool value);

		bool manualSettings() const;
		void setManualSettings(bool value);

		bool disableDataId() const;
		void setDisableDataID(bool value);

		bool generateVHDFile() const;
		void setGenerateVHDFile(bool value);

	private:
        QUuid m_uuid;
		QString m_connectionID;

		int m_port1TxStartAddress = 0;
		QString m_port1EquipmentID;
		int m_port1ManualTxWordsQuantity = 479;
		int m_port1ManualRxWordsQuantity = 479;
		int m_port1TxRxOptoID = 0;
		quint32 m_port1TxRxOptoDataUID = 0;
		int m_port1TxRsID = 0;
		quint32 m_port1TxRsDataUID = 0;
		QString m_port1RawDataDescription;

		int m_port2TxStartAddress = 0;
		QString m_port2EquipmentID;
		int m_port2ManualTxWordsQuantity = 479;
		int m_port2ManualRxWordsQuantity = 479;
		int m_port2TxRxOptoID = 0;
		quint32 m_port2TxRxOptoDataUID = 0;
		int m_port2TxRsID = 0;
		quint32 m_port2TxRsDataUID = 0;
		QString m_port2RawDataDescription;

		bool m_port1EnableSerial = false;
		bool m_port2EnableSerial = false;

		SerialMode m_port1SerialMode = SerialMode::RS232;
		SerialMode m_port2SerialMode = SerialMode::RS232;

		bool m_port1EnableDuplex = false;
		bool m_port2EnableDuplex = false;

		Type m_type = Type::PortToPort;


		bool m_manualSettings = false;

		bool m_disableDataID = false;
		bool m_generateVHDFile = false;
    };

	class ConnectionStorage : public DbObjectStorage<std::shared_ptr<Connection>>
	{
	public:
        ConnectionStorage(DbController* db, QWidget* parentWidget);
        virtual ~ConnectionStorage();

		using DbObjectStorage::get;

		std::vector<std::shared_ptr<Connection>> get(const QStringList& masks) const;

		std::shared_ptr<Connection> getPortConnection(QString portEquipmentId) const;

		bool load() override;
		bool save(const QUuid& uuid) override;

		//

		bool loadFromConnectionsFolder();
        bool loadFromXmlDeprecated(QString &errorString);

        bool deleteXmlDeprecated();
	};
}

Q_DECLARE_METATYPE (std::shared_ptr<Hardware::Connection>)

#endif // CONNECTION_H
