#ifndef OUTPUTSIGNALBASE_H
#define OUTPUTSIGNALBASE_H

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"

// ==============================================================================================

const char* const			MeasureIoSignalType[] =
{
							QT_TRANSLATE_NOOP("SignalBase.h", "Input"),
							QT_TRANSLATE_NOOP("SignalBase.h", "Output"),
};

const int					MEASURE_IO_SIGNAL_TYPE_COUNT = sizeof(MeasureIoSignalType)/sizeof(MeasureIoSignalType[0]);

const int					MEASURE_IO_SIGNAL_TYPE_UNKNOWN	= -1,
							MEASURE_IO_SIGNAL_TYPE_INPUT	= 0,
							MEASURE_IO_SIGNAL_TYPE_OUTPUT	= 1;

// ==============================================================================================

const char* const			OutputSignalType[] =
{
							QT_TRANSLATE_NOOP("SignalBase.h", "Not output"),
							QT_TRANSLATE_NOOP("SignalBase.h", "Input → Output"),
							QT_TRANSLATE_NOOP("SignalBase.h", "Tuning → Output"),
};

const int					OUTPUT_SIGNAL_TYPE_COUNT = sizeof(OutputSignalType)/sizeof(OutputSignalType[0]);

const int					OUTPUT_SIGNAL_TYPE_UNUSED		= 0,
							OUTPUT_SIGNAL_TYPE_FROM_INPUT	= 1,
							OUTPUT_SIGNAL_TYPE_FROM_TUNING	= 2;

// ==============================================================================================

class OutputSignal
{
public:

	OutputSignal();
	OutputSignal(const OutputSignal& from);
	virtual ~OutputSignal() {}

private:

	int						m_signalID = -1;
	Hash					m_hash = 0;

	int						m_type = OUTPUT_SIGNAL_TYPE_UNUSED;

	mutable QMutex			m_signalMutex;

	QString					m_appSignalID[MEASURE_IO_SIGNAL_TYPE_COUNT];
	Metrology::SignalParam	m_param[MEASURE_IO_SIGNAL_TYPE_COUNT];

public:

	void					clear();
	bool					isValid() const;

	int						signalID() const { return m_signalID; }
	void					setSignalID(int id) { m_signalID = id; }

	Hash					hash() const { return m_hash; }
	bool					setHash();

	int						type() const { return m_type; }
	QString					typeStr() const;
	void					setType(int type) { m_type = type; }

	QString					appSignalID(int type) const;
	void					setAppSignalID(int type, const QString& appSignalID);

	Metrology::SignalParam	param(int type) const;
	void					setParam(int type, const Metrology::SignalParam& param);
	void					updateParam();

	OutputSignal&			operator=(const OutputSignal& from);
};

// ==============================================================================================

class OutputSignalBase : public QObject
{
	Q_OBJECT

public:

	explicit OutputSignalBase(QObject *parent = 0);
	virtual ~OutputSignalBase() {}

private:

	mutable QMutex			m_signalMutex;
	QVector<OutputSignal>	m_signalList;

public:

	void					clear();
	int						signalCount() const;
	void					sort();

	int						load();
	bool					save();

	int						appendSignal(const OutputSignal& signal);

	OutputSignal			signal(int index) const;
	void					setSignal(int index, const OutputSignal& signal);

	void					remove(const OutputSignal& signal);
	void					remove(int index);

	int						find(int measureIoType, const Hash& hash, int outputSignalType);
	int						find(const OutputSignal& signal);

	OutputSignalBase&		operator=(const OutputSignalBase& from);

signals:

public slots:

};

// ==============================================================================================

#endif // OUTPUTSIGNALBASE_H
