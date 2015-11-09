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

        Connection();
        Connection(const Connection& that);

        bool save(QXmlStreamWriter& writer);
        bool load(QXmlStreamReader& reader);


        //Properties
        //

    public:

        const int index() const;
        void setIndex(int value);

        const QString caption() const;
        void setCaption(const QString& value);

        const QString device1StrID() const;
        void setDevice1StrID(const QString& value);

        const int device1Port() const;
        void setDevice1Port(int value);

        const QString device2StrID() const;
        void setDevice2StrID(const QString& value);

        const int device2Port() const;
        void setDevice2Port(int value);

        //
        //
        //

        const int txStartAddress() const;
        void setTxStartAddress(int value);

        const int txWordsQuantity() const;
        void setTxWordsQuantity(int value);

        const int rxWordsQuantity() const;
        void setRxWordsQuantity(int value);

        const ConnectionMode connectionMode() const;
        void setConnectionMode(const ConnectionMode& value);

        const bool enable() const;
        void setEnable(bool value);

        Hardware::Connection& operator = (const Hardware::Connection& that)
        {
            m_index = that.m_index;
            m_caption = that.m_caption;
            m_device1StrID = that.m_device1StrID;
            m_device1Port = that.m_device1Port;
            m_device2StrID = that.m_device2StrID;
            m_device2Port = that.m_device2Port;

            m_txStartAddress = that.m_txStartAddress;
            m_txWordsQuantity = that.m_txWordsQuantity;
            m_rxWordsQuantity = that.m_rxWordsQuantity;

            m_connectionMode = that.m_connectionMode;
            m_enable = that.m_enable;

            return *this;
        }

    private:
        int m_index = -1;
        QString m_caption;
        QString m_device1StrID;
        int m_device1Port = 0;
        QString m_device2StrID;
        int m_device2Port = 0;

        int m_txStartAddress = 0;
        int m_txWordsQuantity = 0;
        int m_rxWordsQuantity = 0;

        ConnectionMode m_connectionMode = ConnectionMode::ModeRS232;
        bool m_enable = false;


    };


    class ConnectionStorage : public QObject
    {
        Q_OBJECT

    public:

        ConnectionStorage();

        void add(std::shared_ptr<Connection> connection);
        bool remove(std::shared_ptr<Connection> connection);

        int count() const;
        std::shared_ptr<Connection> get(int index) const;
        void clear();
        bool checkUniqueConnections();

        bool load(DbController* db, QString &errorCode);
        bool save(DbController* db, const QString &comment);

    private:
        std::vector<std::shared_ptr<Connection>> m_connections;
        const QString fileName = "Connections.xml";

    };
}
Q_DECLARE_METATYPE (std::shared_ptr<Hardware::Connection>)

#endif // CONNECTION_H
