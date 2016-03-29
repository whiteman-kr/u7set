#ifndef CONNECTION_H
#define CONNECTION_H

#include "../include/DbController.h"
#include "./Builder/OptoModule.h"

namespace Hardware
{
    class Connection : public PropertyObject
    {
        Q_OBJECT

    public:
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

        QString port1StrID() const;
        void setPort1StrID(const QString& value);

        QString port2StrID() const;
        void setPort2StrID(const QString& value);

        quint16 getID() const;

        //
        //
        //

        int port1TxWordsQuantity() const;
        void setPort1TxWordsQuantity(int value);

        int port1RxWordsQuantity() const;
        void setPort1RxWordsQuantity(int value);

        int port1TxRxOptoID() const;
        void setPort1TxRxOptoID(int value);

        quint32 port1TxRxOptoDataUID() const;
        void setPort1TxRxOptoDataUID(quint32 value);

        int port1TxRsID() const;
        void setPort1TxRsID(int value);

        quint32 port1TxRsDataUID() const;
        void setPort1TxRsDataUID(quint32 value);

        //
        //
        //

        int port2TxWordsQuantity() const;
        void setPort2TxWordsQuantity(int value);

        int port2RxWordsQuantity() const;
        void setPort2RxWordsQuantity(int value);

        int port2TxRxOptoID() const;
        void setPort2TxRxOptoID(int value);

        quint32 port2TxRxOptoDataUID() const;
        void setPort2TxRxOptoDataUID(quint32 value);

        int port2TxRsID() const;
        void setPort2TxRsID(int value);

        quint32 port2TxRsDataUID() const;
        void setPort2TxRsDataUID(quint32 value);

        //
        //
        //

        OptoPort::SerialMode serialMode() const;
        void setSerialMode(const OptoPort::SerialMode value);

        OptoPort::Mode mode() const;
        void setMode(const OptoPort::Mode value);

        bool enable() const;
        void setEnable(bool value);

        bool enableDuplex() const;
        void setEnableDuplex(bool value);

        bool manualSettings() const;
        void setManualSettings(bool value);

        QStringList signalList() const;
        void setSignalList(const QStringList& value);

        Hardware::Connection& operator = (const Hardware::Connection& that)
        {
            m_index = that.m_index;
            m_caption = that.m_caption;
            m_port1StrID = that.m_port1StrID;
            m_port2StrID = that.m_port2StrID;

            m_port1TxWordsQuantity = that.m_port1TxWordsQuantity;
            m_port1RxWordsQuantity = that.m_port1RxWordsQuantity;
            m_port1TxRxOptoID = that.m_port1TxRxOptoID;
            m_port1TxRxOptoDataUID = that.m_port1TxRxOptoDataUID;
            m_port1TxRsID = that.m_port1TxRsID;
            m_port1TxRsDataUID = that.m_port1TxRsDataUID;

            m_port2TxWordsQuantity = that.m_port2TxWordsQuantity;
            m_port2RxWordsQuantity = that.m_port2RxWordsQuantity;
            m_port2TxRxOptoID = that.m_port2TxRxOptoID;
            m_port2TxRxOptoDataUID = that.m_port2TxRxOptoDataUID;
            m_port2TxRsID = that.m_port2TxRsID;
            m_port2TxRsDataUID = that.m_port2TxRsDataUID;

            m_serialMode = that.m_serialMode;
            m_enable = that.m_enable;
            m_enableDuplex = that.m_enableDuplex;
            m_manualSettings = that.m_manualSettings;

            setMode(that.mode());
            setSignalList(that.signalList());

            return *this;
        }

    private:
        int m_index = -1;
        QString m_caption;

        QString m_port1StrID;
        int m_port1TxWordsQuantity = 479;
        int m_port1RxWordsQuantity = 479;
        int m_port1TxRxOptoID = 0;
        quint32 m_port1TxRxOptoDataUID = 0;
        int m_port1TxRsID = 0;
        quint32 m_port1TxRsDataUID = 0;

        QString m_port2StrID;
        int m_port2TxWordsQuantity = 479;
        int m_port2RxWordsQuantity = 479;
        int m_port2TxRxOptoID = 0;
        quint32 m_port2TxRxOptoDataUID = 0;
        int m_port2TxRsID = 0;
        quint32 m_port2TxRsDataUID = 0;

        OptoPort::SerialMode m_serialMode = OptoPort::SerialMode::RS232;
        OptoPort::Mode m_mode = OptoPort::Mode::Optical;

        bool m_enable = false;
        bool m_enableDuplex = false;
        bool m_manualSettings = false;

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

        bool setLMConnectionParams(const QString& portStrID, int m_txWordsQuantity, int m_rxWordsQuantity,
                                 int m_txRxOptoID, quint32 m_txRxOptoDataUID);

        bool setOCMConnectionParams(const QString& portStrID, int m_txWordsQuantity, int m_rxWordsQuantity,
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
