#ifndef CONNECTION_H
#define CONNECTION_H

#include "../lib/DbController.h"
#include "./Builder/OptoModule.h"

namespace Hardware
{
	class Connection : public PropertyObject
	{
		Q_OBJECT

	public:
		Connection();
		Connection(const Connection& that);

        bool save(DbController *db);
        bool load(const QByteArray& data);
        bool loadFromXml(QXmlStreamReader& reader);
        bool remove(DbController *db, bool &fileRemoved);

        bool checkOut(DbController* db);
        bool checkIn(DbController* db, const QString &comment);
        bool undo(DbController* db, bool &fileRemoved);
        bool vcsStateAndAction(DbController* db, VcsState &state, VcsItemAction &action);

        // Properties
		//
	public:

		int index() const;
		void setIndex(int value);

        QString connectionID() const;
        void setConnectionID(const QString& value);

        int fileID() const;
        void setFileID(const int value);

        QString fileName() const;
        void setFileName(const QString& value);

        QString port1EquipmentID() const;
		void setPort1EquipmentID(const QString& value);

		QString port2EquipmentID() const;
		void setPort2EquipmentID(const QString& value);

		quint16 getID() const;

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

		OptoPort::Mode mode() const;
		void setMode(const OptoPort::Mode value);

		bool enableSerial() const;
		void setEnableSerial(bool value);

		bool enableDuplex() const;
		void setEnableDuplex(bool value);

		bool manualSettings() const;
		void setManualSettings(bool value);

        QStringList signalList() const;
        void setSignalList(const QStringList& value);

		bool disableDataID() const;
		void setDisableDataID(bool value);

		bool generateVHDFile() const;
		void setGenerateVHDFile(bool value);

		Hardware::Connection& operator = (const Hardware::Connection& that)
		{
			m_index = that.m_index;
			m_connectionID = that.m_connectionID;
            m_fileID = that.m_fileID;
            m_fileName = that.m_fileName;
            m_port1EquipmentID = that.m_port1EquipmentID;
			m_port2EquipmentID = that.m_port2EquipmentID;

			m_port1ManualTxWordsQuantity = that.m_port1ManualTxWordsQuantity;
			m_port1ManualRxWordsQuantity = that.m_port1ManualRxWordsQuantity;
			m_port1TxRxOptoID = that.m_port1TxRxOptoID;
			m_port1TxRxOptoDataUID = that.m_port1TxRxOptoDataUID;
			m_port1TxRsID = that.m_port1TxRsID;
			m_port1TxRsDataUID = that.m_port1TxRsDataUID;
			m_port1TxStartAddress = that.m_port1TxStartAddress;
			m_port1RawDataDescription = that.m_port1RawDataDescription;


			m_port2ManualTxWordsQuantity = that.m_port2ManualTxWordsQuantity;
			m_port2ManualRxWordsQuantity = that.m_port2ManualRxWordsQuantity;
			m_port2TxRxOptoID = that.m_port2TxRxOptoID;
			m_port2TxRxOptoDataUID = that.m_port2TxRxOptoDataUID;
			m_port2TxRsID = that.m_port2TxRsID;
			m_port2TxRsDataUID = that.m_port2TxRsDataUID;
			m_port2TxStartAddress = that.m_port2TxStartAddress;
			m_port2RawDataDescription = that.m_port2RawDataDescription;

			m_serialMode = that.m_serialMode;
			m_enableSerial = that.m_enableSerial;
			m_enableDuplex = that.m_enableDuplex;
			m_manualSettings = that.m_manualSettings;

			m_disableDataID = that.m_disableDataID;
			m_generateVHDFile = that.m_generateVHDFile;

			setMode(that.mode());
            setSignalList(that.signalList());

			return *this;
		}

	private:
		int m_index = -1;
		QString m_connectionID;

        QString m_fileName;
        int m_fileID = -1;

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

		bool m_enableSerial = false;
		bool m_enableDuplex = false;
		bool m_manualSettings = false;

		bool m_disableDataID = false;
		bool m_generateVHDFile = false;

        QStringList m_signalList;
    };


	class ConnectionStorage : public QObject
	{
		Q_OBJECT

	public:

		ConnectionStorage();

		void add(std::shared_ptr<Connection> connection);
		bool remove(std::shared_ptr<Connection> connection);

		Q_INVOKABLE int count() const;
		std::shared_ptr<Connection> get(int index) const;
		Q_INVOKABLE QObject* jsGet(int index) const;

		void clear();

        bool load(DbController* db, QString &errorCode);

        bool loadFromXmlDeprecated(DbController* db, QString &errorCode);
        bool deleteXmlDeprecated(DbController* db);

	private:
		std::vector<std::shared_ptr<Connection>> m_connections;
	};
}
Q_DECLARE_METATYPE (std::shared_ptr<Hardware::Connection>)

#endif // CONNECTION_H
