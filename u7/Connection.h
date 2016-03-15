#ifndef CONNECTION_H
#define CONNECTION_H

#include "../include/DbController.h"

namespace Hardware
{
    class Connection : public PropertyObject
    {
        Q_OBJECT

    public:
        enum class SerialMode
        {
            RS232,
            RS485
        };
        Q_ENUM(SerialMode)

        enum class Type
		{
            Optical,
            Serial
		};
        Q_ENUM(Type)

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

		QString ocmPortStrID() const;
		void setOcmPortStrID(const QString& value);

		QString device1StrID() const;
        void setDevice1StrID(const QString& value);

		QString device2StrID() const;
        void setDevice2StrID(const QString& value);

        //
        //
        //

        int device1TxWordsQuantity() const;
        void setDevice1TxWordsQuantity(int value);

        int device1RxWordsQuantity() const;
        void setDevice1RxWordsQuantity(int value);

        int device1TxRxOptoID() const;
        void setDevice1TxRxOptoID(int value);

        quint32 device1TxRxOptoDataUID() const;
        void setDevice1TxRxOptoDataUID(quint32 value);

        int device1TxRsID() const;
        void setDevice1TxRsID(int value);

        quint32 device1TxRsDataUID() const;
        void setDevice1TxRsDataUID(quint32 value);

        //
        //
        //

        int device2TxWordsQuantity() const;
        void setDevice2TxWordsQuantity(int value);

        int device2RxWordsQuantity() const;
        void setDevice2RxWordsQuantity(int value);

        int device2TxRxOptoID() const;
        void setDevice2TxRxOptoID(int value);

        quint32 device2TxRxOptoDataUID() const;
        void setDevice2TxRxOptoDataUID(quint32 value);

        int device2TxRsID() const;
        void setDevice2TxRsID(int value);

        quint32 device2TxRsDataUID() const;
        void setDevice2TxRsDataUID(quint32 value);

        //
        //
        //

        SerialMode serialMode() const;
        void setSerialMode(const SerialMode& value);

        Type type() const;
        void setType(const Type& value);

		bool enable() const;
        void setEnable(bool value);

        bool enableDuplex() const;
        void setEnableDuplex(bool value);

        QStringList signalList() const;
		void setSignalList(const QStringList& value);

        Hardware::Connection& operator = (const Hardware::Connection& that)
        {
            m_index = that.m_index;
            m_caption = that.m_caption;
			m_ocmPortStrID = that.m_ocmPortStrID;
            m_device1StrID = that.m_device1StrID;
            m_device2StrID = that.m_device2StrID;

            m_device1TxWordsQuantity = that.m_device1TxWordsQuantity;
            m_device1RxWordsQuantity = that.m_device1RxWordsQuantity;
            m_device1TxRxOptoID = that.m_device1TxRxOptoID;
            m_device1TxRxOptoDataUID = that.m_device1TxRxOptoDataUID;
            m_device1TxRsID = that.m_device1TxRsID;
            m_device1TxRsDataUID = that.m_device1TxRsDataUID;

            m_device2TxWordsQuantity = that.m_device2TxWordsQuantity;
            m_device2RxWordsQuantity = that.m_device2RxWordsQuantity;
            m_device2TxRxOptoID = that.m_device2TxRxOptoID;
            m_device2TxRxOptoDataUID = that.m_device2TxRxOptoDataUID;
            m_device2TxRsID = that.m_device2TxRsID;
            m_device2TxRsDataUID = that.m_device2TxRsDataUID;

            m_serialMode = that.m_serialMode;
            m_enable = that.m_enable;
            m_enableDuplex = that.m_enableDuplex;

            setType(that.type());
			setSignalList(that.signalList());

            return *this;
        }

    private:
        int m_index = -1;
        QString m_caption;
		QString m_ocmPortStrID;

        QString m_device1StrID;
        int m_device1TxWordsQuantity = 479;
        int m_device1RxWordsQuantity = 479;
        int m_device1TxRxOptoID = 0;
        quint32 m_device1TxRxOptoDataUID = 0;
        int m_device1TxRsID = 0;
        quint32 m_device1TxRsDataUID = 0;

        QString m_device2StrID;
        int m_device2TxWordsQuantity = 479;
        int m_device2RxWordsQuantity = 479;
        int m_device2TxRxOptoID = 0;
        quint32 m_device2TxRxOptoDataUID = 0;
        int m_device2TxRsID = 0;
        quint32 m_device2TxRsDataUID = 0;


        SerialMode m_serialMode = SerialMode::RS232;
        Type m_type = Type::Optical;

        bool m_enable = false;
        bool m_enableDuplex = false;

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

        bool setLMConnectionParams(const QString& deviceStrID, int m_txWordsQuantity, int m_rxWordsQuantity,
                                 int m_txRxOptoID, quint32 m_txRxOptoDataUID);

        bool setOCMConnectionParams(const QString& deviceStrID, int m_txWordsQuantity, int m_rxWordsQuantity,
                                 int m_txRxOptoID, quint32 m_txRxOptoDataUID, int m_txRsID, quint32 m_txRsDataUID);

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
