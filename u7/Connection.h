#ifndef CONNECTION_H
#define CONNECTION_H

#include "../include/DbController.h"

namespace Hardware
{
    class Connection : public PropertyObject
    {
        Q_OBJECT

    public:
        enum class ConnectionMode
        {
            ModeRS232,
            ModeRS485
        };
        Q_ENUM(ConnectionMode)

		enum class ConnectionType
		{
			DeviceConnectionType,
			SerialPortSignalListType
		};
		Q_ENUM(ConnectionType)

        Connection();
        Connection(const Connection& that);

        bool save(QXmlStreamWriter& writer);
        bool load(QXmlStreamReader& reader);


		// Properties
        //
    public:

		int index() const;
        void setIndex(int value);

		QString caption() const;
		void setCaption(const QString& value);

		QString osmStrID() const;
		void setOsmStrID(const QString& value);

		QString device1StrID() const;
        void setDevice1StrID(const QString& value);

		QString device2StrID() const;
        void setDevice2StrID(const QString& value);

        //
        //
        //

        int device1TxStartAddress() const;
        void setDevice1TxStartAddress(int value);

        int device1TxWordsQuantity() const;
        void setDevice1TxWordsQuantity(int value);

        int device1RxWordsQuantity() const;
        void setDevice1RxWordsQuantity(int value);

        int device1TxRxOptoID() const;
        void setDevice1TxRxOptoID(int value);

        quint32 device1TxRxOptoDataUID() const;
        void setDevice1TxRxOptoDataUID(quint32 value);

        int device1TxRxID() const;
        void setDevice1TxRxID(int value);

        quint32 device1TxRxDataUID() const;
        void setDevice1TxRxDataUID(quint32 value);

        //
        //
        //

        int device2TxStartAddress() const;
        void setDevice2TxStartAddress(int value);

        int device2TxWordsQuantity() const;
        void setDevice2TxWordsQuantity(int value);

        int device2RxWordsQuantity() const;
        void setDevice2RxWordsQuantity(int value);

        int device2TxRxOptoID() const;
        void setDevice2TxRxOptoID(int value);

        quint32 device2TxRxOptoDataUID() const;
        void setDevice2TxRxOptoDataUID(quint32 value);

        int device2TxRxID() const;
        void setDevice2TxRxID(int value);

        quint32 device2TxRxDataUID() const;
        void setDevice2TxRxDataUID(quint32 value);

        //
        //
        //

        ConnectionMode connectionMode() const;
        void setConnectionMode(const ConnectionMode& value);

		ConnectionType connectionType() const;
		void setConnectionType(const ConnectionType& value);

		bool enable() const;
        void setEnable(bool value);

		QStringList signalList() const;
		void setSignalList(const QStringList& value);

        Hardware::Connection& operator = (const Hardware::Connection& that)
        {
            m_index = that.m_index;
            m_caption = that.m_caption;
			m_osmStrID = that.m_osmStrID;
            m_device1StrID = that.m_device1StrID;
            m_device2StrID = that.m_device2StrID;



            m_device1TxStartAddress = that.m_device1TxStartAddress;
            m_device1TxWordsQuantity = that.m_device1TxWordsQuantity;
            m_device1RxWordsQuantity = that.m_device1RxWordsQuantity;
            m_device1TxRxOptoID = that.m_device1TxRxOptoID;
            m_device1TxRxOptoDataUID = that.m_device1TxRxOptoDataUID;
            m_device1TxRxID = that.m_device1TxRxID;
            m_device1TxRxDataUID = that.m_device1TxRxDataUID;

            m_device2TxStartAddress = that.m_device2TxStartAddress;
            m_device2TxWordsQuantity = that.m_device2TxWordsQuantity;
            m_device2RxWordsQuantity = that.m_device2RxWordsQuantity;
            m_device2TxRxOptoID = that.m_device2TxRxOptoID;
            m_device2TxRxOptoDataUID = that.m_device2TxRxOptoDataUID;
            m_device2TxRxID = that.m_device2TxRxID;
            m_device2TxRxDataUID = that.m_device2TxRxDataUID;

            m_connectionMode = that.m_connectionMode;
            m_enable = that.m_enable;

			setConnectionType(that.connectionType());
			setSignalList(that.signalList());

            return *this;
        }

    private:
        int m_index = -1;
        QString m_caption;
		QString m_osmStrID;



        QString m_device1StrID;
        int m_device1TxStartAddress = 0;
        int m_device1TxWordsQuantity = 0;
        int m_device1RxWordsQuantity = 0;
        int m_device1TxRxOptoID = 0;
        quint32 m_device1TxRxOptoDataUID = 0;
        int m_device1TxRxID = 0;
        quint32 m_device1TxRxDataUID = 0;

        QString m_device2StrID;
        int m_device2TxStartAddress = 0;
        int m_device2TxWordsQuantity = 0;
        int m_device2RxWordsQuantity = 0;
        int m_device2TxRxOptoID = 0;
        quint32 m_device2TxRxOptoDataUID = 0;
        int m_device2TxRxID = 0;
        quint32 m_device2TxRxDataUID = 0;


        ConnectionMode m_connectionMode = ConnectionMode::ModeRS232;
		ConnectionType m_connectionType = ConnectionType::DeviceConnectionType;
        bool m_enable = false;

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
        bool checkUniqueConnections(Connection *editObject);

        bool setLMConnectionParams(const QString& deviceStrID, int m_txStartAddress, int m_txWordsQuantity, int m_rxWordsQuantity,
                                 int m_txRxOptoID, quint32 m_txRxOptoDataUID);

        bool setOCMConnectionParams(const QString& deviceStrID, int m_txStartAddress, int m_txWordsQuantity, int m_rxWordsQuantity,
                                 int m_txRxOptoID, quint32 m_txRxOptoDataUID, int m_txRxRsID, quint32 m_txRxRsDataUID);

        bool load(DbController* db, QString &errorCode);
        bool save(DbController* db);

        bool checkOut(DbController* db);
        bool checkIn(DbController* db, const QString &comment);
        bool undo(DbController* db);
        bool isCheckedOut(DbController* db);

    private:
        std::vector<std::shared_ptr<Connection>> m_connections;
        const QString fileName = "Connections.xml";
    };
}
Q_DECLARE_METATYPE (std::shared_ptr<Hardware::Connection>)

#endif // CONNECTION_H
