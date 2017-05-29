#ifndef CONNECTION_H
#define CONNECTION_H

#include "../lib/DbController.h"
#include "./Builder/OptoModule.h"

namespace Hardware
{
	class Connection :
			public PropertyObject,
			public Proto::ObjectSerialization<Connection>
	{
		Q_OBJECT

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

		const DbFileInfo& fileInfo() const;
		void setFileInfo(const DbFileInfo& value);

        QUuid uuid() const;
        void setUuid(const QUuid& value);

		QString connectionID() const;
		void setConnectionID(const QString& value);

        QString fileName() const;
		int fileId() const;

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

		OptoPort::SerialMode serialMode() const;
		void setSerialMode(const OptoPort::SerialMode value);

		QString serialModeStr() const;

		OptoPort::Mode mode() const;
		void setMode(const OptoPort::Mode value);

		QString modeStr() const;

		bool isSerial() const;

		bool enableDuplex() const;
		void setEnableDuplex(bool value);

		bool manualSettings() const;
		void setManualSettings(bool value);

		bool disableDataId() const;
		void setDisableDataID(bool value);

		bool generateVHDFile() const;
		void setGenerateVHDFile(bool value);

	private:
        QUuid m_uuid;
		QString m_connectionID;

		DbFileInfo m_fileInfo;

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

		OptoPort::SerialMode m_serialMode = OptoPort::SerialMode::RS232;
		OptoPort::Mode m_mode = OptoPort::Mode::Optical;

		bool m_enableDuplex = false;
		bool m_manualSettings = false;

		bool m_disableDataID = false;
		bool m_generateVHDFile = false;
    };

	class ConnectionStorage : public QObject
	{
		Q_OBJECT

	public:
        ConnectionStorage(DbController* db, QWidget* parentWidget);
        virtual ~ConnectionStorage();

        void clear();

        void add(std::shared_ptr<Connection> connection);
        void remove(const QUuid& uuid);
        bool removeFile(const QUuid& uuid, bool &fileRemoved);

        Q_INVOKABLE int count() const;

        std::shared_ptr<Connection> get(const QUuid &uuid) const;
        std::shared_ptr<Connection> get(int index) const;
        Q_INVOKABLE QObject* jsGet(int index) const;

		std::shared_ptr<Connection> getPortConnection(QString portEquipmentId) const;

        bool checkOut(const QUuid& uuid);
        bool checkIn(const QUuid& uuid, const QString &comment, bool &fileRemoved);
        bool undo(const QUuid& uuid, bool &fileRemoved);

        bool load();
        bool loadFromConnectionsFolder();
        bool loadFromXmlDeprecated(QString &errorString);
        bool save(const QUuid& uuid);

        bool deleteXmlDeprecated();

    private:
		std::vector<std::shared_ptr<Connection>> m_connectionsVector;
        std::map<QUuid, std::shared_ptr<Connection>> m_connections;

        DbController* m_db = nullptr;
        QWidget* m_parentWidget = nullptr;
	};
}

Q_DECLARE_METATYPE (std::shared_ptr<Hardware::Connection>)

#endif // CONNECTION_H
