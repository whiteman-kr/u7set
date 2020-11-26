#ifndef SIGNALCONNECTIONBASE_H
#define SIGNALCONNECTIONBASE_H

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"

// ==============================================================================================

const char* const			MeasureIoSignalType[] =
{
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Output"),
};

const int					MEASURE_IO_SIGNAL_TYPE_COUNT = sizeof(MeasureIoSignalType)/sizeof(MeasureIoSignalType[0]);

const int					MEASURE_IO_SIGNAL_TYPE_UNDEFINED	= -1,
							MEASURE_IO_SIGNAL_TYPE_INPUT		= 0,
							MEASURE_IO_SIGNAL_TYPE_OUTPUT		= 1;

// ==============================================================================================

const char* const			SignalConnectionType[] =
{
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "No connections"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input -> Internal"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input -> Output"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input dP -> Internal F"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input dP -> Output F"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input °С -> Internal °F"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input °С -> Output °F"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Tuning -> Output"),
};

const int					SIGNAL_CONNECTION_TYPE_COUNT = sizeof(SignalConnectionType)/sizeof(SignalConnectionType[0]);

const int					SIGNAL_CONNECTION_TYPE_UNDEFINED				= -1,
							SIGNAL_CONNECTION_TYPE_UNUSED					= 0,
							SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL			= 1,
							SIGNAL_CONNECTION_TYPE_INPUT_OUTPUT				= 2,
							SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F	= 3,
							SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F		= 4,
							SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F	= 5,
							SIGNAL_CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F		= 6,
							SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT			= 7;

// ==============================================================================================

#pragma pack(push, 1)

	union SignalConnectionHandle
	{
		struct
		{
			quint64 outputID : 30;
			quint64 inputID : 30;
			quint64 type : 4;
		};

		quint64 uint64;
	};

#pragma pack(pop)

// ==============================================================================================

class SignalConnection
{
public:

	SignalConnection();
	SignalConnection(const SignalConnection& from);
	virtual ~SignalConnection() {}

private:

	int						m_index = -1;									// for database

	SignalConnectionHandle	m_handle;
	QString					m_appSignalID[MEASURE_IO_SIGNAL_TYPE_COUNT];
	Metrology::Signal*		m_pSignal[MEASURE_IO_SIGNAL_TYPE_COUNT];

public:

	bool					isValid() const;
	void					clear();

	int						index() const { return m_index; }
	void					setIndex(int index) { m_index = index; }

	void					createHandle();
	SignalConnectionHandle	handle() const { return m_handle; }

	int						type() const { return m_handle.type; }
	QString					typeStr() const;
	void					setType(int type) { m_handle.type = type; }

	QString					appSignalID(int ioType) const;
	void					setAppSignalID(int ioType, const QString& appSignalID);

	Metrology::Signal*		signal(int ioType) const;
	void					setSignal(int ioType, Metrology::Signal* pSignal);
	void					initSignals();										// set Metrology::Signal* from SignalBase

	SignalConnection&		operator=(const SignalConnection& from);
};

// ==============================================================================================

class SignalConnectionBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalConnectionBase(QObject *parent = nullptr);
	virtual ~SignalConnectionBase() {}

private:

	mutable QMutex			m_connectionMutex;
	QVector<SignalConnection>	m_connectionList;

public:

	void					clear();
	int						count() const;
	void					sort();

	int						load();
	bool					save();

	void					initSignals();									// set all Metrology::Signal* from SignalBase
	void					clearSignals();									// set all Metrology::Signal* value nullptr

	int						append(const SignalConnection& connection);

	SignalConnection		connection(int index) const;
	void					setConnection(int index, const SignalConnection& connection);

	void					remove(int index);

	int						findIndex(int ioType, Metrology::Signal* pSignal) const;
	int						findIndex(int connectionType, int ioType, Metrology::Signal* pSignal) const;
	int						findIndex(const SignalConnection& connection) const;

	QVector<Metrology::Signal*> getOutputSignals(int connectionType, const QString& InputAppSignalID) const;

	SignalConnectionBase&	operator=(const SignalConnectionBase& from);

signals:

public slots:

};

// ==============================================================================================

#endif // SIGNALCONNECTIONBASE_H
