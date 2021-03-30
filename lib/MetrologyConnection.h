#pragma once

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"
#include "../lib/DbController.h"
#include "../lib/SignalSetProvider.h"
#include "../lib/Crc.h"

#include <QMutex>
#include <QVector>

namespace Metrology
{
	// ==============================================================================================

	const char* const	CONNECTIONS_FILE_NAME = "MetrologyConnections.csv";

	// ==============================================================================================

	enum ConnectionType
	{
		NoConnectionType	= -1,
		Unused				= 0,
		Input_Internal		= 1,
		Input_Output		= 2,
		Input_DP_Internal_F	= 3,
		Input_DP_Output_F	= 4,
		Input_C_Internal_F	= 5,
		Input_C_Output_F	= 6,
		Tuning_Output		= 7,
	};
	const int ConnectionTypeCount = 8;			// count of ...ConnectionType elements

	#define ERR_METROLOGY_CONNECTION_TYPE(type) (TO_INT(type) < 0 || TO_INT(type) >= Metrology::ConnectionTypeCount)

	QString ConnectionTypeCaption(ConnectionType type);

	// ==============================================================================================

	enum ConnectionIoType
	{
		Source		= 0,
		Destination	= 1,
	};
	const int ConnectionIoTypeCount = 2;		// count of ...ConnectionIoType elements

	#define ERR_METROLOGY_CONNECTION_IO_TYPE(type) (TO_INT(type) < 0 || TO_INT(type) >= Metrology::ConnectionIoTypeCount)

	// ==============================================================================================

	class ConnectionSignal
	{
	public:

		ConnectionSignal();
		virtual ~ConnectionSignal() {}

	public:

		void clear();

		QString appSignalID() const { return m_appSignalID; }
		void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

		bool isExist() const { return m_exist; }

		void set(::Signal* pSignal);
		void set(Metrology::Signal* pSignal);

		Metrology::Signal* metrologySignal() const { return m_pMetrologySignal; }

	private:

		QString m_appSignalID;								// AppSignalID from connections file
		bool m_exist = false;								// signal has been found in SignalSetProvider

		Metrology::Signal* m_pMetrologySignal = nullptr;	// only for software Metrology
	};


	// ==============================================================================================

	class Connection
	{
	public:

		Connection();
		virtual ~Connection() {}

	public:

		bool isValid() const;
		void clear();

		//
		//
		QString strID() const;

		static bool signalIsOk(const ::Signal& signal);

		//
		//
		ConnectionSignal connectionSignal(int ioType) const;

		ConnectionType type() const { return m_type; }
		QString typeStr() const;
		void setType(ConnectionType connectionType) { m_type = connectionType; }
		void setType(int connectionType) { m_type = static_cast<ConnectionType>(connectionType); }

		QString appSignalID(int ioType) const;
		void setAppSignalID(int ioType, const QString& appSignalID);

		bool isExist(int ioType) const;							// signal has not been found in SignalSetProvider

		void setSignal(int ioType, ::Signal* pSignal);
		void setSignal(int ioType, Metrology::Signal* pSignal);

		Metrology::Signal* metrologySignal(int ioType) const;

		//
		//
		const VcsItemAction& action() const { return m_action; }
		void setAction(const VcsItemAction& action) { m_action = action; }

		//
		//
		int restoreID() const { return m_restoreID; }
		void setRestoreID(int restoreID) { m_restoreID = restoreID; }	// will be update on check in during save

		// serialize for Build
		//
		bool readFromXml(XmlReadHelper& xml);
		void writeToXml(XmlWriteHelper& xml);

		//
		//
		bool operator == (const Connection& connection) const;

	private:

		ConnectionType m_type = ConnectionType::NoConnectionType;
		ConnectionSignal m_connectionSignal[ConnectionIoTypeCount];

		VcsItemAction m_action;
		int m_restoreID = -1;
	};

	// ==============================================================================================

	class ConnectionBase : public QObject
	{
		Q_OBJECT

	public:

		ConnectionBase(QObject* parent = nullptr);
		virtual ~ConnectionBase() override {}

	public:

		//
		//
		void clear();
		int count() const;

		//
		//
		void setSignalSetProvider(SignalSetProvider* signalSetProvider);

		//
		//
		std::shared_ptr<DbFile> getConnectionFile(DbController* db);

		bool load(DbController* db);
		bool save(bool checkIn, const QString &comment);

		bool checkOut();
		bool isCheckIn();

		void findSignal_in_signalSet();

		// resolution on edit
		//
		QString userName() { return m_userName; }
		bool enableEditBase() { return m_enableEditBase; }
		bool userIsAdmin() { return m_userIsAdmin; }

		// modify
		//
		int append(const Connection& connection);

		Connection connection(int index) const;
		Connection* connectionPtr(int index);
		void setConnection(int index, const Connection& connection);

		void remove(int index);
		void removeAllMarked();

		//
		//
		void setAction(int index, const VcsItemAction::VcsItemActionType& type);

		//
		//
		void updateRestoreIDs();									// will be update all resotoreID  on check in during save connection
		Connection connectionFromChekedIn(int restoreID);			// get connection from last check in
		int restoreConnection(int restoreID);						// restore connection from last check in, return index of restore connection

		//
		//
		void sort();

		int findConnectionIndex(const Connection& connection) const;
		int findConnectionIndex(int ioType, Metrology::Signal* pSignal) const;
		int findConnectionIndex(int ioType, ConnectionType connectionType, Metrology::Signal* pSignal) const;

		std::vector<Metrology::Signal*> destinationSignals(const QString& sourceAppSignalID, ConnectionType connectionType) const;

		// CSV-data
		//
		QByteArray csvDataFromConnections(bool full);
		QVector<Connection> connectionsFromCsvData(const QByteArray& data) const;

		bool exportConnectionsToFile(const QString& fileName);

		//
		//
		ConnectionBase&	operator=(const ConnectionBase& from);

	private:

		mutable QMutex m_connectionMutex;
		QVector<Connection> m_connectionList;

		SignalSetProvider* m_signalSetProvider = nullptr;

		QString m_userName;
		bool m_enableEditBase = true;
		bool m_userIsAdmin = false;
	};

	// ==============================================================================================
}