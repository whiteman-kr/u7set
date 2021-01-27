#pragma once

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"
#include "../lib/DbController.h"

#include <QMutex>
#include <QVector>

namespace Metrology
{
	// ==============================================================================================

	const char* const	IoConnectionType[] =
	{
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Input"),
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "Output"),
	};

	const int			IO_SIGNAL_CONNECTION_TYPE_COUNT = sizeof(IoConnectionType)/sizeof(IoConnectionType[0]);

	const int			IO_SIGNAL_CONNECTION_TYPE_UNDEFINED	= -1,
						IO_SIGNAL_CONNECTION_TYPE_INPUT		= 0,
						IO_SIGNAL_CONNECTION_TYPE_OUTPUT	= 1;

	// ==============================================================================================

	const char* const	ConnectionType[] =
	{
						QT_TRANSLATE_NOOP("MetrologyConnectionBase.h", "No connections"),
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

	const quint64		EMPTY_CONNECTION_HANDLE					= 0;
	const quint64		SIGNAL_ID_IS_NOT_FOUND					= 0;

    #pragma pack(push, 1)

	    union ConnectionHandle
		{
			struct
			{
				quint64 outputID : 30;
				quint64 inputID : 30;
				quint64 type : 4;
			};

			quint64 state;
		};

    #pragma pack(pop)

	// ==============================================================================================

	class SignalConnection
	{
	public:

		SignalConnection();
		virtual ~SignalConnection() {}

	private:

		ConnectionHandle m_handle;

		QString m_appSignalID[IO_SIGNAL_CONNECTION_TYPE_COUNT];
		Metrology::Signal* m_pSignal[IO_SIGNAL_CONNECTION_TYPE_COUNT];

	public:

		bool isValid() const;
		void clear();

		ConnectionHandle handle() const { return m_handle; }

		int type() const { return m_handle.type; }
		QString typeStr() const;
		void setType(int type) { m_handle.type = static_cast<quint64>(type); }

		QString appSignalID(int ioType) const;
		void setAppSignalID(int ioType, const QString& appSignalID);

		Metrology::Signal* signal(int ioType) const;
		void setSignal(int ioType, ::Signal* pSignal);
		void setSignal(int ioType, Metrology::Signal* pSignal);

		SignalConnection& operator=(const SignalConnection& from);

		// serialize
		//
		bool readFromXml(XmlReadHelper& xml);
		void writeToXml(XmlWriteHelper& xml);
	};

	// ==============================================================================================

	const char* const CONNECTIONS_FILE_NAME = "MetrologyConnections.csv";

	// ==============================================================================================

	class ConnectionBase : public QObject
	{
		Q_OBJECT

	public:

		ConnectionBase(QObject *parent = nullptr);
		ConnectionBase(DbController* db, SignalSet* signalSet, QObject *parent = nullptr);
		virtual ~ConnectionBase() {}

	private:

		mutable QMutex m_connectionMutex;
		QVector<SignalConnection> m_connectionList;

	public:

		void clear();
		int count() const;
		void sort();

		//
		//
		bool load(DbController* db, SignalSet* signalSet);
		bool save(DbController* db, bool checkIn, const QString &comment);

		bool isCheckIn(DbController* db);
		bool сheckOut(DbController* db);

		//
		//
		int append(const SignalConnection& connection);
		void remove(int index);

		SignalConnection connection(int index) const;
		SignalConnection* connectionPtr(int index);
		void setConnection(int index, const SignalConnection& connection);

		int findConnectionIndex(int ioType, Metrology::Signal* pSignal) const;
		int findConnectionIndex(int connectionType, int ioType, Metrology::Signal* pSignal) const;
		int findConnectionIndex(const SignalConnection& connection) const;

		int getOutputSignalCount(int connectionType, const QString& InputAppSignalID) const;
		QVector<Metrology::Signal*> getOutputSignals(int connectionType, const QString& InputAppSignalID) const;

		//
		//
		QString getCSVdata();
		bool exportToFile(const QString& fileName);

		//
		//
		ConnectionBase&	operator=(const ConnectionBase& from);
	};

	// ==============================================================================================
}

