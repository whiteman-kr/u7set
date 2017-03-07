#ifndef SIGNALBASE_H
#define SIGNALBASE_H

// This is class was designed to save and distribute signals for measure
//
// Algorithm:
//
// - fill base SignalList
// - find all types of Case from base SignalList
// - select one of CaseType to measure
// - find all CaseNo for selected CaseType
// - find signals for selected CaseType
// - fill signal list for measure
//

#include <QObject>

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalState.h"
#include "../lib/MetrologySignal.h"

#include "CalibratorManager.h"

// ==============================================================================================

class SignalState
{
public:

                            SignalState() {}
    explicit                SignalState(const AppSignalState& state) { setState(state); }
                            ~SignalState() {}

private:

    AppSignalState          m_state;

public:


    AppSignalState          state() const { return m_state; }
    void                    setState(const AppSignalState& state) { m_state = state; }
};


// ==============================================================================================

const char* const           StatisticStateStr[] =
{
							QT_TRANSLATE_NOOP("SignalBase.h", "Invalid"),
							QT_TRANSLATE_NOOP("SignalBase.h", "Ok"),
};

const int                   STATISTIC_STATE_COUNT = sizeof(StatisticStateStr)/sizeof(StatisticStateStr[0]);

const int                   STATISTIC_STATE_INVALID   = 0,
							STATISTIC_STATE_SUCCESS   = 1;

// ==============================================================================================

class StatisticItem
{
public:

							StatisticItem();
	explicit                StatisticItem(const Hash& signalHash);
							~StatisticItem();

private:

	Hash                    m_signalHash = 0;

	int                     m_measureCount = 0;
	int                     m_state = STATISTIC_STATE_SUCCESS;

public:

	Hash                    signalHash() const { return m_signalHash; }
	void                    setSignalHash(const Hash& hash) { m_signalHash = hash; }

	int                     incrementMeasureCount() { m_measureCount++; return m_measureCount; }
	int                     measureCount() const { return m_measureCount; }
	QString                 measureCountStr() const;

	int                     state() const { return m_state; }
	QString                 stateStr() const;
	void                    setState(bool state) { m_state = state; }
};

// ==============================================================================================

class MetrologySignal
{
public:

                            MetrologySignal();
	explicit                MetrologySignal(const Metrology::SignalParam& param);
                            ~MetrologySignal();

private:

	Metrology::SignalParam	m_param;
    AppSignalState          m_state;

	StatisticItem           m_statistic;

public:

	Metrology::SignalParam&	param() { return m_param; }
	void                    setParam(const Metrology::SignalParam& param) { m_param = param; }

    AppSignalState&         state() { return m_state; }
    void                    setState(const AppSignalState& state) { m_state = state; }

	StatisticItem&          statistic() { return m_statistic; }
	void                    setStatistic(const StatisticItem& statistic) { m_statistic = statistic; }

    MetrologySignal&        operator=(const MetrologySignal& from);
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(MetrologySignal)
                            Q_DECLARE_METATYPE(MetrologySignal*)

// ==============================================================================================

const int                   CHANNEL_0               = 0,
                            CHANNEL_1               = 1,
                            CHANNEL_2               = 2,
                            CHANNEL_3               = 3,
                            CHANNEL_4               = 4,
                            CHANNEL_5               = 5;

const int                   MAX_CHANNEL_COUNT       = 6;

// ==============================================================================================

class MetrologyMultiSignal
{
public:

                            MetrologyMultiSignal();
                            MetrologyMultiSignal(const MetrologyMultiSignal& from);
                            ~MetrologyMultiSignal();
private:

    mutable QMutex          m_mutex;

    Hash                    m_signalHash[MAX_CHANNEL_COUNT];

	Metrology::SignalLocation	m_location;

    QString                 m_strID;

public:

    void                    clear();
    bool                    isEmpty() const;

    Hash                    hash(int channel) const;
	bool                    setSignal(int channel, int measureKind, const Metrology::SignalParam& param);

	Metrology::SignalLocation& location() { return m_location; }
	void                    setLocation(const Metrology::SignalLocation& location) { m_location = location; }

    QString&                strID() { return m_strID; }

    MetrologyMultiSignal&   operator=(const MetrologyMultiSignal& from);
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(MetrologyMultiSignal)
                            Q_DECLARE_METATYPE(MetrologyMultiSignal*)

// ==============================================================================================

const char* const           MeasureIoSignalType[] =
{
                            QT_TRANSLATE_NOOP("SignalBase.h", "Input"),
                            QT_TRANSLATE_NOOP("SignalBase.h", "Output"),
};

const int                   MEASURE_IO_SIGNAL_TYPE_COUNT = sizeof(MeasureIoSignalType)/sizeof(MeasureIoSignalType[0]);

const int                   MEASURE_IO_SIGNAL_TYPE_UNKNOWN			= -1,
                            MEASURE_IO_SIGNAL_TYPE_INPUT			= 0,
                            MEASURE_IO_SIGNAL_TYPE_OUTPUT			= 1;

// ----------------------------------------------------------------------------------------------

#define                     ERR_MEASURE_IO_SIGNAL_TYPE(type) (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
#define                     TEST_MEASURE_IO_SIGNAL_TYPE(type)			if (ERR_MEASURE_IO_SIGNAL_TYPE(type)) { return; }
#define                     TEST_MEASURE_IO_SIGNAL_TYPE1(type, retVal)	if (ERR_MEASURE_IO_SIGNAL_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const           OutputSignalType[] =
{
                            QT_TRANSLATE_NOOP("SignalBase.h", "Not output"),
                            QT_TRANSLATE_NOOP("SignalBase.h", "Input → Output"),
                            QT_TRANSLATE_NOOP("SignalBase.h", "Tuning → Output"),
};

const int                   OUTPUT_SIGNAL_TYPE_COUNT = sizeof(OutputSignalType)/sizeof(OutputSignalType[0]);

const int                   OUTPUT_SIGNAL_TYPE_UNUSED       = 0,
                            OUTPUT_SIGNAL_TYPE_FROM_INPUT   = 1,
                            OUTPUT_SIGNAL_TYPE_FROM_TUNING  = 2;

// ----------------------------------------------------------------------------------------------

#define                     ERR_OUTPUT_SIGNAL_TYPE(type) (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
#define                     TEST_OUTPUT_SIGNAL_TYPE(type)			if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return; }
#define                     TEST_OUTPUT_SIGNAL_TYPE1(type, retVal)	if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return retVal; }

// ==============================================================================================

class MeasureParam
{
public:
                            MeasureParam();
                            MeasureParam(const MeasureParam& from);
                            ~MeasureParam();
private:

    mutable QMutex          m_mutex;

	Metrology::SignalParam	m_param[MEASURE_IO_SIGNAL_TYPE_COUNT];

    int                     m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

	CalibratorManager*      m_pCalibratorManager = nullptr;

	bool                    m_equalPhysicalRange = false;

public:

    void                    clear();
    bool                    isValid() const;

	Metrology::SignalParam	param(int type) const;
	bool                    setParam(int type, const Metrology::SignalParam& param);

    int                     outputSignalType() const { return m_outputSignalType; }
    void                    setOutputSignalType(int type) { m_outputSignalType = type; }

	bool                    equalPhysicalRange() const { return m_equalPhysicalRange; }
    bool                    testPhysicalRange();

	QString                 rackCaption() const;
    QString                 signalID(bool showCustomID, const QString& divider) const;
	QString                 chassisStr() const;
	QString                 moduleStr() const;
	QString                 placeStr() const;
    QString                 caption(const QString &divider) const;
    QString                 physicalRangeStr(const QString& divider) const;
    QString                 electricRangeStr(const QString& divider) const;
    QString                 electricSensorStr(const QString& divider) const;

    CalibratorManager*      calibratorManager() const { return m_pCalibratorManager; }
    QString                 calibratorStr() const;
    void                    setCalibratorManager(CalibratorManager* pCalibratorManager) { m_pCalibratorManager = pCalibratorManager; }

    MeasureParam&           operator=(const MeasureParam& from);
};

// ==============================================================================================

class MeasureSignal
{
public:
                            MeasureSignal();
                            MeasureSignal(const MeasureSignal& from);
                            ~MeasureSignal();
private:

    mutable QMutex          m_mutex;

    int                     m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

    MetrologyMultiSignal    m_signal[MEASURE_IO_SIGNAL_TYPE_COUNT];

public:

    void                    clear();
    bool                    isEmpty() const;

    int                     outputSignalType() const { return m_outputSignalType; }

    MetrologyMultiSignal    signal(int type) const;
    Hash                    signalHash(int type, int channel) const;

    bool                    setSignal(int type, const MetrologyMultiSignal& signal);
	bool                    setSignal(int channel, int measureKind, int outputSignalType, const Metrology::SignalParam& param);

    MeasureSignal&          operator=(const MeasureSignal& from);
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(MeasureSignal)
                            Q_DECLARE_METATYPE(MeasureSignal*)

// ==============================================================================================

class SignalBase : public QObject
{
    Q_OBJECT

public:
    explicit                SignalBase(QObject *parent = 0);
                            ~SignalBase();

    void                    clear();
    void                    sortByPosition();

    // Signals
    //

    int                     signalCount() const;
    void                    clearSignalList();

	int						appendSignal(const Metrology::SignalParam& param);

    MetrologySignal         signal(const QString& appSignalID);
    MetrologySignal         signal(const Hash& hash);
    MetrologySignal         signal(int index);

	Metrology::SignalParam	signalParam(const QString& appSignalID);
	Metrology::SignalParam	signalParam(const Hash& hash);
	Metrology::SignalParam  signalParam(int index);

	void                    setSignalParam(const QString& appSignalID, const Metrology::SignalParam& param);
	void                    setSignalParam(const Hash& hash, const Metrology::SignalParam& param);
	void                    setSignalParam(int index, const Metrology::SignalParam& param);

    AppSignalState          signalState(const QString& appSignalID);
    AppSignalState          signalState(const Hash& hash);
    AppSignalState          signalState(int index);

    void                    setSignalState(const QString& appSignalID, const AppSignalState& state);
    void                    setSignalState(const Hash& hash, const AppSignalState& state);
    void                    setSignalState(int index, const AppSignalState& state);

    // hashs for update signal state
    //

    int                     hashForRequestStateCount() const;
    Hash                    hashForRequestState(int index);

	// Racks and Signals for measure
    //
							// racks
                            //
	int                     createRackList(int outputSignalType);
	void                    clearRackList();

	int                     rackCount() const;
	Metrology::RackParam	rack(int index);

                            // signals
                            //
	void					initSignals();

	int                     createMeasureSignalList(int measureKind, int outputSignalType, Hash rackHash);
    void                    clearMeasureSignalList();

    int                     measureSignalCount() const;
    MeasureSignal           measureSignal(int index);



    // Main signal for measure
    //
    MeasureSignal           activeSignal() const;
    void                    setActiveSignal(const MeasureSignal& signal);
    void                    clearActiveSignal();

private:

    // all signals that received form AppDataSrv
    //
    mutable QMutex          m_signalMutex;
    QMap<Hash, int>         m_signalHashMap;
    QVector<MetrologySignal>  m_signalList;

    // list of hashes in order to receive signal state form AppDataSrv
    //
    mutable QMutex          m_stateMutex;
    QVector<Hash>           m_requestStateList;

	// list of racks in order to select signal for measure
    //
	mutable QMutex          m_rackMutex;
	QVector<Metrology::RackParam> m_rackList;

    // list of signals for measure
    //
    mutable QMutex          m_signalMesaureMutex;
    QVector<MeasureSignal>  m_signalMesaureList;

    // main signal that are measuring at the current moment
    //
    mutable QMutex          m_activeSignalMutex;
    MeasureSignal           m_activeSignal;

signals:

    void                    updatedSignalParam(Hash signalHash);

    void                    activeSignalChanged(const MeasureSignal& signal);

public slots:

};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================

class UnitBase : public QObject
{
    Q_OBJECT

public:
    explicit                UnitBase(QObject *parent = 0);
                            ~UnitBase();

    void                    clear();

    int                     unitCount() const;

    void                    appendUnit(int unitID, const QString& unit);

    bool                    hasUnit(int unitID);
    bool                    hasSensorType(int sensorType);

    QString                 unit(int unitID);

private:

    // all units that received form AppDataSrv
    //
    mutable QMutex          m_unitMutex;
    QMap<int, QString>      m_unitMap;


signals:

public slots:

};

// ==============================================================================================

extern UnitBase theUnitBase;

// ==============================================================================================

class RackBase : public QObject
{
    Q_OBJECT

public:
    explicit                RackBase(QObject *parent = 0);
                            ~RackBase();

    void                    clear();

    int                     count() const;

	int						appendRack(const Metrology::RackParam& rack);

	Metrology::RackParam	rack(const QString& rackID);
	Metrology::RackParam    rack(const Hash& hash);
	Metrology::RackParam	rack(int index);

private:

    mutable QMutex          m_rackMutex;
	QMap<Hash, int>			m_rackHashMap;
	QVector<Metrology::RackParam> m_rackList;

signals:

public slots:

};

// ==============================================================================================

extern RackBase theRackBase;

// ==============================================================================================

const char* const OutputSignalSumType[] =
{
                            QT_TRANSLATE_NOOP("Measure.h", "Addition (+)"),
                            QT_TRANSLATE_NOOP("Measure.h", "Subtraction (-)"),
};

const int                   OUTPUT_SIGNAL_SUM_TYPE_COUNT = sizeof(OutputSignalSumType)/sizeof(OutputSignalSumType[0]);

const int                   OUTPUT_SIGNAL_SUM_TYPE_NO_USED  = -1,
                            OUTPUT_SIGNAL_SUM_TYPE_ADD		= 0,
                            OUTPUT_SIGNAL_SUM_TYPE_SUB      = 1;

// ----------------------------------------------------------------------------------------------

#define                     ERR_OUTPUT_SIGNAL_SUM_TYPE(type) (type < 0 || type >= OUTPUT_SIGNAL_SUM_TYPE_COUNT)
#define                     TEST_OUTPUT_SIGNAL_SUM_TYPE(type)			if (ERR_OUTPUT_SIGNAL_SUM_TYPE(type)) { return; }
#define                     TEST_OUTPUT_SIGNAL_SUM_TYPE1(type, retVal)	if (ERR_OUTPUT_SIGNAL_SUM_TYPE(type)) { return retVal; }

// ==============================================================================================

class OutputSignal
{
public:
                            OutputSignal();
                            OutputSignal(const OutputSignal& from);
                            ~OutputSignal();
private:

    int                     m_signalID = -1;
    Hash                    m_hash = 0;

    int                     m_type = OUTPUT_SIGNAL_TYPE_UNUSED;

    mutable QMutex          m_signalMutex;

    QString                 m_appSignalID[MEASURE_IO_SIGNAL_TYPE_COUNT];
	Metrology::SignalParam	m_param[MEASURE_IO_SIGNAL_TYPE_COUNT];

public:

    void                    clear();
    bool                    isValid() const;

    int                     signalID() const { return m_signalID; }
    void                    setSignalID(int id) { m_signalID = id; }

    Hash                    hash() const { return m_hash; }
    bool                    setHash();

    int                     type() const { return m_type; }
    QString                 typeStr() const;
    void                    setType(int type) { m_type = type; }

    QString                 appSignalID(int type) const;
    void                    setAppSignalID(int type, const QString& appSignalID);

	Metrology::SignalParam	param(int type) const;
	void                    setParam(int type, const Metrology::SignalParam& param);
    void                    updateParam();

    OutputSignal&           operator=(const OutputSignal& from);
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(OutputSignal)
                            Q_DECLARE_METATYPE(OutputSignal*)

// ==============================================================================================

class OutputSignalBase : public QObject
{
    Q_OBJECT

public:
    explicit                OutputSignalBase(QObject *parent = 0);
                            ~OutputSignalBase();

    void                    clear();
    int                     signalCount() const;
    void                    sort();

    int                     load();
    bool                    save();

    int                     appendSignal(const OutputSignal& signal);

    OutputSignal            signal(int index) const;
    void                    setSignal(int index, const OutputSignal& signal);

    void                    remove(const OutputSignal& signal);
    void                    remove(int index);

    int                     find(int measureIoType, const Hash& hash, int outputSignalType);
    int                     find(const OutputSignal& signal);

    OutputSignalBase&       operator=(const OutputSignalBase& from);

private:

    mutable QMutex          m_signalMutex;
    QVector<OutputSignal>   m_signalList;

signals:

public slots:

};

// ==============================================================================================

extern OutputSignalBase theOutputSignalBase;

// ==============================================================================================

#endif // SIGNALBASE_H
