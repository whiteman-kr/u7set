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

const int					MEASURE_IO_SIGNAL_TYPE_UNKNOWN	= -1,
							MEASURE_IO_SIGNAL_TYPE_INPUT	= 0,
							MEASURE_IO_SIGNAL_TYPE_OUTPUT	= 1;

// ==============================================================================================

const char* const			SignalConnectionType[] =
{
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "No connections"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Input → Output"),
							QT_TRANSLATE_NOOP("SignalConnectionBase.h", "Tuning → Output"),
};

const int					SIGNAL_CONNECTION_TYPE_COUNT = sizeof(SignalConnectionType)/sizeof(SignalConnectionType[0]);

const int					SIGNAL_CONNECTION_TYPE_UNUSED		= 0,
							SIGNAL_CONNECTION_TYPE_FROM_INPUT	= 1,
							SIGNAL_CONNECTION_TYPE_FROM_TUNING	= 2;

// ==============================================================================================

class SignalConnection
{
public:

	SignalConnection();
	SignalConnection(const SignalConnection& from);
	virtual ~SignalConnection() {}

private:

	mutable QMutex			m_signalMutex;

	int						m_index = -1;									// for database

	QString					m_appSignalID[MEASURE_IO_SIGNAL_TYPE_COUNT];
	Hash					m_hash = UNDEFINED_HASH;						// calcHash form m_appSignalID

	int						m_type = SIGNAL_CONNECTION_TYPE_UNUSED;

	Metrology::Signal*		m_pSignal[MEASURE_IO_SIGNAL_TYPE_COUNT];

public:

	bool					isValid() const;
	void					clear();

	int						index() const { return m_index; }
	void					setIndex(int index) { m_index = index; }

	Hash					hash() const { return m_hash; }
	void					setHash();

	int						type() const { return m_type; }
	QString					typeStr() const;
	void					setType(int type) { m_type = type; }

	QString					appSignalID(int type) const;
	void					setAppSignalID(int type, const QString& appSignalID);

	Metrology::Signal*		metrologySignal(int type) const;
	void					setMetrologySignal(int type, Metrology::Signal* pSignal);
	void					initMetrologySignal();		// set Metrology::Signal* from SignalBase by signalHash

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

	void					init();										// set all Metrology::Signal* from SignalBase by signalHash
	void					empty();									// set all Metrology::Signal* value nullptr

	int						append(const SignalConnection& connection);

	SignalConnection		connection(int index) const;
	void					setSignal(int index, const SignalConnection& connection);

	void					remove(int index);

	int						findIndex(int signalConnectionType, int measureIoType, Metrology::Signal* pSignal);
	int						findIndex(const SignalConnection& connection);

	SignalConnectionBase&	operator=(const SignalConnectionBase& from);

signals:

public slots:

};

// ==============================================================================================

#endif // SIGNALCONNECTIONBASE_H
