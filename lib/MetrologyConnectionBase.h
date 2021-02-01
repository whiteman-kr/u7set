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

	const char* const	ConnectionType[] =
	{
	                    QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "No connections             "),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input -> Internal"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input -> Output"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input dP -> Internal F"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input dP -> Output F"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input °С -> Internal °F"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input °С -> Output °F"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Tuning -> Output"),
	};

	const int			CONNECTION_TYPE_COUNT = sizeof(ConnectionType)/sizeof(ConnectionType[0]);

	const int			CONNECTION_TYPE_UNDEFINED				= -1,
						CONNECTION_TYPE_UNUSED					= 0,
						CONNECTION_TYPE_INPUT_INTERNAL			= 1,
						CONNECTION_TYPE_INPUT_OUTPUT			= 2,
						CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F	= 3,
						CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F	= 4,
						CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F	= 5,
						CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F		= 6,
						CONNECTION_TYPE_TUNING_OUTPUT			= 7;

	// ==============================================================================================

	enum ConnectionIoType
	{
		Source = 0,
		Destination = 1,
		Count = 2			// count of ...ConnectionIoType elements
	};

	// ==============================================================================================

	const int SIGNAL_ID_IS_EMPTY = 0;

	class ConnectionSignal
	{
	public:

		ConnectionSignal();
		virtual ~ConnectionSignal() {}

	public:

		void clear();

		void set(::Signal* pSignal);
		void set(Metrology::Signal* pSignal);	// only for software Metrology

		QString appSignalID() const { return m_appSignalID; }
		void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

		int signalID() const { return m_signalID; }
		void setSignalID(int id) { m_signalID = id; }

		Metrology::Signal* metrologySignal() const { return m_pMetrologySignal; }

	private:

		QString m_appSignalID;
		int m_signalID = SIGNAL_ID_IS_EMPTY;

		Metrology::Signal* m_pMetrologySignal = nullptr; // only for software Metrology
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

		QString strID() const { return m_strID; }
		void createStrID();

		ConnectionSignal connectionSignal(int ioType) const;

		int type() const { return m_type; }
		QString typeStr() const;
		void setType(int type) { m_type = type; }

		QString appSignalID(int ioType) const;
		void setAppSignalID(int ioType, const QString& appSignalID);

		int signalID(int ioType) const;
		void setSignalID(int ioType, int id);

		void setSignal(int ioType, ::Signal* pSignal);
		void setSignal(int ioType, Metrology::Signal* pSignal);	// only for software Metrology

		const VcsItemAction& action() const { return m_action; }
		void setAction(const VcsItemAction& action) { m_action = action; }

		Metrology::Signal* metrologySignal(int ioType) const;

		// serialize for Build
		//
		bool readFromXml(XmlReadHelper& xml);
		void writeToXml(XmlWriteHelper& xml);

	private:

		QString m_strID;

		int m_type = CONNECTION_TYPE_UNUSED;

		ConnectionSignal m_connectionSignal[ConnectionIoType::Count];

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

		void setSignalSetProvider(SignalSetProvider* signalSetProvider) { m_signalSetProvider = signalSetProvider; }

		//
		//
		void clear();
		int count() const;

		//
		//
		std::shared_ptr<DbFile> getConnectionFile(DbController* db);

		bool load(DbController* db);
		bool save(bool checkIn, const QString &comment);

		bool checkOut();
		bool isCheckIn();

		void setSignalIDs();

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
	};

	// ==============================================================================================
}

