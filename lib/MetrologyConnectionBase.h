#pragma once

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"
#include "../lib/DbController.h"
#include "../lib/SignalSetProvider.h"

#include <QMutex>
#include <QVector>

namespace Metrology
{
	// ==============================================================================================

	const char* const	CONNECTIONS_FILE_NAME = "MetrologyConnections.csv";

	// ==============================================================================================

	enum ConnectionType
	{
		Unknown				= -1,
		Unsed				= 0,
		Input_Internal		= 1,
		Input_Output		= 2,
		Input_DP_Internal_F	= 3,
		Input_DP_Output_F	= 4,
		Input_C_Internal_F	= 5,
		Input_C_Output_F	= 6,
		Tuning_Output		= 7,
	};
	const int ConnectionTypeCount = 8;			// count of ...ConnectionType elements

	QString ConnectionTypeCaption(int type);

	// ==============================================================================================

	enum ConnectionIoType
	{
		Source		= 0,
		Destination	= 1,
	};
	const int ConnectionIoTypeCount = 2;		// count of ...ConnectionIoType elements

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

		QString strID(bool full) const;

		ConnectionSignal connectionSignal(int ioType) const;

		ConnectionType type() const { return m_type; }
		QString typeStr() const;
		void setType(ConnectionType type) { m_type = type; }

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

		// serialize for Build
		//
		bool readFromXml(XmlReadHelper& xml);
		void writeToXml(XmlWriteHelper& xml);

	private:

		ConnectionType m_type = ConnectionType::Unknown;

		ConnectionSignal m_connectionSignal[ConnectionIoTypeCount];

		VcsItemAction m_action;
	};

	// ==============================================================================================

	class ConnectionBase : public QObject
	{
		Q_OBJECT

	public:

		ConnectionBase(QObject* parent = nullptr);
		virtual ~ConnectionBase() {}

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

		bool enableEdit() { return m_enableEdit; }
		QString userName() { return m_userName; }

		//
		//
		int append(const Connection& connection);
		void remove(int index);
		void removeAllMarked();

		Connection connection(int index) const;
		Connection* connectionPtr(int index);
		void setConnection(int index, const Connection& connection);

		//
		//
		void setAction(int index, const VcsItemAction::VcsItemActionType& type);

		//
		//
		void sort();

		int findConnectionIndex(int ioType, Metrology::Signal* pSignal) const;
		int findConnectionIndex(int connectionType, int ioType, Metrology::Signal* pSignal) const;
		int findConnectionIndex(const Connection& connection) const;

		int getOutputSignalCount(int connectionType, const QString& InputAppSignalID) const;
		QVector<Metrology::Signal*> getOutputSignals(int connectionType, const QString& InputAppSignalID) const;

		//
		//
		QString getCSVdata();
		bool exportToFile(const QString& fileName);

		//
		//
		ConnectionBase&	operator=(const ConnectionBase& from);

	private:

		mutable QMutex m_connectionMutex;
		QVector<Connection> m_connectionList;

		SignalSetProvider* m_signalSetProvider = nullptr;

		bool m_enableEdit = true;
		QString m_userName;
	};

	// ==============================================================================================
}

