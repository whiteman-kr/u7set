#ifndef SIGNALBASE_H
#define SIGNALBASE_H

// This is class was designed to save and distribute signals fo measure
//
// Algorithm:
//
// - fill base SignalList
// - find all types of Case from base SignalList
// - select one of CaseType to measure
// - find all CaseNo for selected CaseType
//          Note:
//          caseNo and the caseIndex may be different
//          for example, system SNETO_RAES where were 1-th, 2-th, 3-th and 7-th SHFS,
//          but no exist 4-th, 5-th and 6-th SHFS (cunnig plan of ALMAX)
// - find signals for selected CaseType
// - fill signal list for measure
//

#include <QObject>

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalState.h"

// ==============================================================================================

union DevicePositionHandle
{
    struct
    {
        quint64             caseNo      : 8;
        quint64             caseType    : 8;
        quint64             channel     : 8;
        quint64             subblock    : 8;
        quint64             block       : 8;
        quint64             entry       : 8;

    };

    quint64 handle;
};

// ==============================================================================================

class DevicePosition
{
public:

                            DevicePosition();
    explicit                DevicePosition(const QString& equipmentID);
                            ~DevicePosition();

private:

    QString                 m_equipmentID;

    int                     m_caseNo = -1;

    int                     m_caseType = -1;        // depend from m_caseCaption
    QString                 m_caseCaption;

    int                     m_channel = -1;
    int                     m_subblock = -1;
    int                     m_block = -1;
    int                     m_entry = -1;

    DevicePositionHandle    m_position;

public:

    void                    clear();

    void                    setFromID(const QString& equipmentID);

    QString                 equipmentID() const { return m_equipmentID; }
    void                    setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

    int                     caseNo() const { return m_caseNo; }
    void                    setCaseNo(const int caseNo) { m_caseNo = caseNo; }

    int                     caseType() const { return m_caseType; }
    void                    setCaseType(const int type) { m_caseType = type; }

    QString                 caseCaption() const { return m_caseCaption; }
    void                    setCaseCaption(const QString& caption) { m_caseCaption = caption; }

    QString                 caseStr() const;

    int                     channel() const { return m_channel; }
    void                    setChannel(const int channel) { m_channel = channel; }

    QString                 channelString() const;

    int                     subblock() const { return m_subblock; }
    void                    setSubblock(const int subblock) { m_subblock = subblock; }

    QString                 subblockStr() const;

    int                     block() const { return m_block; }
    void                    setBlock(const int block) { m_block = block; }

    QString                 blockStr() const;

    int                     entry() const { return m_entry; }
    void                    setEntry(const int entry) { m_entry = entry; }

    QString                 entryStr() const;

    quint64                 handle() const { return m_position.handle; }

    DevicePosition&         operator=(const DevicePosition& from);
};

// ==============================================================================================

const char* const       StatisticStateStr[] =
{
                        QT_TRANSLATE_NOOP("SignalBase.h", "Invalid"),
                        QT_TRANSLATE_NOOP("SignalBase.h", "Ok"),
};

const int               STATISTIC_STATE_COUNT = sizeof(StatisticStateStr)/sizeof(StatisticStateStr[0]);

const int               STATISTIC_STATE_INVALID   = 0,
                        STATISTIC_STATE_SUCCESS   = 1;

// ==============================================================================================

class StatisticItem
{
public:

                        StatisticItem();
    explicit            StatisticItem(const Hash& signalHash);
                        ~StatisticItem();

private:


    Hash                m_signalHash = 0;

    int                 m_measureCount = 0;
    int                 m_state = STATISTIC_STATE_SUCCESS;

public:

    Hash                signalHash() const { return m_signalHash; }
    void                setSignalHash(const Hash& hash) { m_signalHash = hash; }

    int                 incrementMeasureCount() { m_measureCount++; return m_measureCount; }
    int                 measureCount() const { return m_measureCount; }
    QString             measureCountStr() const;

    int                 state() const { return m_state; }
    QString             stateStr() const;
    void                setState(const bool state) { m_state = state; }
};

// ==============================================================================================

class MeasureSignalParam
{
public:

                            MeasureSignalParam();
                            MeasureSignalParam(const Signal& param);
                            ~MeasureSignalParam();

private:

    Hash                    m_hash = 0;                        // hash calcHash from AppSignalID

    QString                 m_appSignalID;
    QString                 m_customAppSignalID;
    QString                 m_caption;

    E::SignalType           m_signalType = E::SignalType::Analog;
    E::SignalInOutType      m_inOutType = E::SignalInOutType::Internal;

    DevicePosition          m_position;

    int                     m_lowADC = 0;
    int                     m_highADC = 0;

    double                  m_inputElectricLowLimit = 0;
    double                  m_inputElectricHighLimit = 0;
    E::InputUnit            m_inputElectricUnitID = E::InputUnit::NoInputUnit;
    E::SensorType           m_inputElectricSensorType = E::SensorType::NoSensorType;
    int                     m_inputElectricPrecise = 3;

    double                  m_inputPhysicalLowLimit = 0;
    double                  m_inputPhysicalHighLimit = 0;
    int                     m_inputPhysicalUnitID = NO_UNIT_ID;
    int                     m_inputPhysicalPrecise = 2;

    double                  m_adjustment = 0;

    bool                    m_hasOutput = false;

    double                  m_outputElectricLowLimit = 0;
    double                  m_outputElectricHighLimit = 0;
    E::InputUnit            m_outputElectricUnitID  = E::InputUnit::NoInputUnit;
    E::SensorType           m_outputElectricSensorType = E::SensorType::NoSensorType;
    int                     m_outputElectricPrecise = 3;

    double                  m_outputPhysicalLowLimit = 0;
    double                  m_outputPhysicalHighLimit = 0;
    int                     m_outputPhysicalUnitID = NO_UNIT_ID;
    int                     m_outputPhysicalPrecise = 2;

    bool                    m_acquire = true;
    int                     m_normalState = 0;

    StatisticItem           m_statistic;

public:

    bool                    isValid() const;

    void                    setParam(const Signal& param);

    Hash                    hash() const { return m_hash; }

    QString                 appSignalID() const { return m_appSignalID; }
    void                    setAppSignalID(const QString& appSignalID);

    QString                 customAppSignalID() const { return m_customAppSignalID; }
    void                    setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

    QString                 caption() const { return m_caption; }
    void                    setCaption(const QString& caption) { m_caption = caption; }

    E::SignalType           signalType() const { return m_signalType; }
    void                    setSignalType(const E::SignalType& type) { m_signalType = type; }

    E::SignalInOutType      inOutType() const { return m_inOutType; }
    void                    setInOutType(const E::SignalInOutType& inOutType) { m_inOutType = inOutType; }

    bool                    isRegistered() const { return acquire(); }

    bool                    isAnalog() const { return m_signalType == E::SignalType::Analog; }
    bool                    isDiscrete() const { return m_signalType == E::SignalType::Discrete; }

    bool                    isInput() const { return m_inOutType == E::SignalInOutType::Input; }
    bool                    isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
    bool                    isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }

    DevicePosition          position() const { return m_position; }
    void                    setPosition(const DevicePosition& position) { m_position = position; }

    void                    setCaseType(const int type) { m_position.setCaseType(type); }

    int                     adcLowLimit() const { return m_lowADC; }
    void                    setLowADC(const int lowADC) { m_lowADC = lowADC; }

    int                     adcHighLimit() const { return m_highADC; }
    void                    setHighADC(const int highADC) { m_highADC = highADC;}

    QString                 adcRangeStr(const bool showInHex) const;

    double                  inputElectricLowLimit() const { return m_inputElectricLowLimit; }
    void                    setInputElectricLowLimit(const double lowLimit) { m_inputElectricLowLimit = lowLimit; }

    double                  inputElectricHighLimit() const { return m_inputElectricHighLimit; }
    void                    setInputElectricHighLimit(const double highLimit) { m_inputElectricHighLimit = highLimit; }

    E::InputUnit            inputElectricUnitID() const { return m_inputElectricUnitID; }
    void                    setInputElectricUnitID(const E::InputUnit unit) { m_inputElectricUnitID = unit; }

    E::SensorType           inputElectricSensorType() const { return m_inputElectricSensorType; }
    void                    setInputElectricSensorType(const E::SensorType sensorType) { m_inputElectricSensorType = sensorType; }

    int                     inputElectricPrecise() const { return m_inputElectricPrecise; }
    void                    setInputElectricPrecise(const int precise) { m_inputElectricPrecise = precise; }

    QString                 inputElectricRangeStr() const;

    double                  inputPhysicalLowLimit() const { return m_inputPhysicalLowLimit; }
    void                    setInputPhysicalLowLimit(const double lowLimit) { m_inputPhysicalLowLimit = lowLimit; }

    double                  inputPhysicalHighLimit() const { return m_inputPhysicalHighLimit; }
    void                    setInputPhysicalHighLimit(const double highLimit) { m_inputPhysicalHighLimit = highLimit; }

    int                     inputPhysicalUnitID() const { return m_inputPhysicalUnitID; }
    void                    setInputPhysicalUnitID(const int unit) { m_inputPhysicalUnitID = unit; }

    int                     inputPhysicalPrecise() const { return m_inputPhysicalPrecise; }
    void                    setInputPhysicalPrecise(const int precise) { m_inputPhysicalPrecise = precise; }

    QString                 inputPhysicalRangeStr() const;

    double                  adjustment() const { return m_adjustment; }
    void                    setAdjustment(const double adjustment) { m_adjustment = adjustment; }

    bool                    hasOutput() const { return m_hasOutput; }
    void                    setHasOutput(const bool hasOutput) { m_hasOutput = hasOutput; }

    double                  outputElectricLowLimit() const { return m_outputElectricLowLimit; }
    void                    setOutputElectricLowLimit(const double lowLimit) { m_outputElectricLowLimit = lowLimit; }

    double                  outputElectricHighLimit() const { return m_outputElectricHighLimit; }
    void                    setOutputElectricHighLimit(const double highLimit) { m_outputElectricHighLimit = highLimit; }

    E::InputUnit            outputElectricUnitID() const { return m_outputElectricUnitID; }
    void                    setOutputElectricUnitID(const E::InputUnit unit) { m_outputElectricUnitID = unit; }

    E::SensorType           outputElectricSensorType() const { return m_outputElectricSensorType; }
    void                    setOutputElectricSensorType(const E::SensorType sensorType) { m_outputElectricSensorType = sensorType; }

    int                     outputElectricPrecise() const { return m_outputElectricPrecise; }
    void                    setOutputElectricPrecise(const int precise) { m_outputElectricPrecise = precise; }

    QString                 outputElectricRangeStr() const;

    double                  outputPhysicalLowLimit() const { return m_outputPhysicalLowLimit; }
    void                    setOutputPhysicalLowLimit(const double lowLimit) { m_outputPhysicalLowLimit = lowLimit; }

    double                  outputPhysicalHighLimit() const { return m_outputPhysicalHighLimit; }
    void                    setOutputPhysicalHighLimit(const double highLimit) { m_outputPhysicalHighLimit = highLimit; }

    int                     outputPhysicalUnitID() const { return m_outputPhysicalUnitID; }
    void                    setOutputPhysicalUnitID(const int unit) { m_outputPhysicalUnitID = unit; }

    int                     outputPhysicalPrecise() const { return m_outputPhysicalPrecise; }
    void                    setOutputPhysicalPrecise(const int precise) { m_outputPhysicalPrecise = precise; }

    QString                 outputPhysicalRangeStr() const;

    bool                    acquire() const { return m_acquire; }
    void                    setAcquire(const bool acquire) { m_acquire = acquire; }

    int                     normalState() const { return m_normalState; }
    void                    setNormalState(const int normalState) { m_normalState = normalState; }

    QString                 calibratorIndexStr(const int index) const;

    StatisticItem           statistic() const { return m_statistic; }
    void                    setStatistic(const StatisticItem& statistic) { m_statistic = statistic; }
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(MeasureSignalParam)
                            Q_DECLARE_METATYPE(MeasureSignalParam*)


// ==============================================================================================

class MeasureSignalState
{
public:

                            MeasureSignalState() {}
    explicit                MeasureSignalState(const AppSignalState& state) { setState(state); }
                            ~MeasureSignalState() {}

private:

    AppSignalState          m_state;

public:


    AppSignalState          state() const { return m_state; }
    void                    setState(const AppSignalState& state) { m_state = state; }
};

// ==============================================================================================

class MeasureSignal
{
public:

                            MeasureSignal();
    explicit                MeasureSignal(const Signal& param);
                            ~MeasureSignal();

private:

    MeasureSignalParam      m_param;
    AppSignalState          m_state;

public:

    MeasureSignalParam      param() const { return m_param; }
    void                    setParam(const MeasureSignalParam& param) { m_param = param; }

    AppSignalState          state() const { return m_state; }
    void                    setState(const AppSignalState& state) { m_state = state; }

    MeasureSignal&          operator=(const MeasureSignal& from);
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(MeasureSignal)
                            Q_DECLARE_METATYPE(MeasureSignal*)

// ==============================================================================================

const int                   CHANNEL_0               = 0,
                            CHANNEL_1               = 1,
                            CHANNEL_2               = 2,
                            CHANNEL_3               = 3,
                            CHANNEL_4               = 4,
                            CHANNEL_5               = 5;

const int                   MEASURE_CHANNEL_COUNT   = 6;
const int                   MAX_CHANNEL_COUNT       = 6;

// ==============================================================================================

class MeasureMultiSignal
{
public:

                            MeasureMultiSignal();
                            MeasureMultiSignal(const MeasureMultiSignal& from);
                            ~MeasureMultiSignal();

private:

    mutable QMutex          m_mutex;

    Hash                    m_signalHash[MAX_CHANNEL_COUNT];

    int                     m_caseNo = -1;
    int                     m_subblock = -1;
    int                     m_block = -1;
    int                     m_entry = -1;

public:

    void                    clear();
    bool                    isEmpty() const;

    Hash                    hash(const int index) const;
    void                    setSignal(const int index, const MeasureSignalParam& param);

    int                     caseNo() const { return m_caseNo; }
    void                    setCaseNo(const int caseNo) { m_caseNo = caseNo; }

    int                     subblock() const { return m_subblock; }
    void                    setSubblock(const int subblock) { m_subblock = subblock; }

    int                     block() const { return m_block; }
    void                    setBlock(const int block) { m_block = block; }

    int                     entry() const { return m_entry; }
    void                    setEntry(const int entry) { m_entry = entry; }

    MeasureMultiSignal&     operator=(const MeasureMultiSignal& from);
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(MeasureMultiSignal)
                            Q_DECLARE_METATYPE(MeasureMultiSignal*)

// ==============================================================================================

class SignalBase : public QObject
{
    Q_OBJECT

public:
    explicit                SignalBase(QObject *parent = 0);
                            ~SignalBase();

    void                    clear();
    int                     signalCount() const;
    void                    sortByPosition();

    // Signals
    //

    int                     appendSignal(const Signal& param);

    MeasureSignal           signal(const QString& appSignalID);
    MeasureSignal           signal(const Hash& hash);
    MeasureSignal           signal(const int index);

    MeasureSignalParam      signalParam(const QString& appSignalID);
    MeasureSignalParam      signalParam(const Hash& hash);
    MeasureSignalParam      signalParam(const int index);

    void                    setSignalParam(const Hash& hash, const MeasureSignalParam& param);
    void                    setSignalParam(const int index, const MeasureSignalParam& param);

    AppSignalState          signalState(const Hash& hash);
    AppSignalState          signalState(const int index);

    void                    setSignalState(const Hash& hash, const AppSignalState& state);
    void                    setSignalState(const int index, const AppSignalState& state);

    // hashs for update signal state
    //

    int                     hashForRequestStateCount() const;
    Hash                    hashForRequestState(const int index);

    // Signals and Cases for measure
    //
    int                     createCaseTypeList();

                            // cases
                            //
    int                     caseTypeCount() const;
    QString                 caseTypeCaption(const int type);

    int                     caseNoCount() const;
    int                     caseNoByCaseIndex(const int caseIndex);

                            // signals
                            //
    int                     createSignalListForMeasure(const int caseType, const int measureKind);
    int                     signalForMeasureCount() const;
    MeasureMultiSignal      signalForMeasure(const int index);

    // Main signal for measure
    //
    MeasureMultiSignal&     activeSignal() { return m_activeSignal; }
    void                    setActiveSignal(const MeasureMultiSignal& multiSignal);

private:

    // all signals that received form AppDataSrv
    //
    mutable QMutex          m_signalMutex;
    QMap<Hash, int>         m_signalHashMap;
    QVector<MeasureSignal>  m_signalList;

    // list of hashes to receive signal state form AppDataSrv
    //
    mutable QMutex          m_stateMutex;
    QVector<Hash>           m_requestStateList;

    // list of cases to select signal for measure
    //
    mutable QMutex          m_caseMutex;
    QVector<QString>        m_caseTypeList;
    QVector<int>            m_caseNoList;

    // list of signals for measure
    //
    mutable QMutex          m_signalForMesaureMutex;
    QVector<MeasureMultiSignal> m_signalForMesaureList;

    // main signal that are measuring at the current moment
    //
    mutable QMutex          m_activeSignalMutex;
    MeasureMultiSignal      m_activeSignal;

signals:

    void                    updatedSignalParam(Hash signalHash);

    void                    setActiveSignal();

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

    void                    appendUnit(const int unitID, const QString& unit);

    QString                 unit(const int unitID);

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

const char* const OutputSignalKind[] =
{
                            QT_TRANSLATE_NOOP("Measure.h", "Input"),
                            QT_TRANSLATE_NOOP("Measure.h", "Output"),
};

const int                   OUTPUT_SIGNAL_KIND_COUNT = sizeof(OutputSignalKind)/sizeof(OutputSignalKind[0]);

const int                   OUTPUT_SIGNAL_KIND_UNKNOWN			= -1,
                            OUTPUT_SIGNAL_KIND_INPUT			= 0,
                            OUTPUT_SIGNAL_KIND_OUTPUT			= 1;

// ----------------------------------------------------------------------------------------------

#define                     ERR_OUTPUT_SIGNAL_KIND(kind) (kind < 0 || kind >= OUTPUT_SIGNAL_KIND_COUNT)
#define                     TEST_OUTPUT_SIGNAL_KIND(kind)			if (ERR_OUTPUT_SIGNAL_KIND(kind)) { return; }
#define                     TEST_OUTPUT_SIGNAL_KIND1(kind, retVal)	if (ERR_OUTPUT_SIGNAL_KIND(kind)) { return retVal; }

// ==============================================================================================

const char* const           OutputSignalType[] =
{
                            QT_TRANSLATE_NOOP("SignalBase.h", "In → Out "),
};

const int                   OUTPUT_SIGNAL_TYPE_COUNT = sizeof(OutputSignalType)/sizeof(OutputSignalType[0]);

const int                   OUTPUT_SIGNAL_TYPE_NO_USED  = -1,
                            OUTPUT_SIGNAL_TYPE_IN_OUT   = 0;

// ----------------------------------------------------------------------------------------------

#define                     ERR_OUTPUT_SIGNAL_TYPE(type) (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
#define                     TEST_OUTPUT_SIGNAL_TYPE(type)			if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return; }
#define                     TEST_OUTPUT_SIGNAL_TYPE1(type, retVal)	if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return retVal; }

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

    int                     m_type = OUTPUT_SIGNAL_TYPE_NO_USED;

    mutable QMutex          m_signalMutex;

    QString                 m_appSignalID[OUTPUT_SIGNAL_KIND_COUNT];
    MeasureSignalParam      m_param[OUTPUT_SIGNAL_KIND_COUNT];

public:

    void                    clear();
    bool                    isValid() const;

    int                     signalID() const { return m_signalID; }
    void                    setSignalID(const int id) { m_signalID = id; }

    int                     type() const { return m_type; }
    QString                 typeStr() const;
    void                    setType(const int type) { m_type = type; }

    QString                 appSignalID(const int kind) const;
    void                    setAppSignalID(const int kind, const QString& appSignalID);

    MeasureSignalParam      param(const int kind) const;
    void                    setParam(const int kind, const MeasureSignalParam& param);
    void                    updateParam();

    OutputSignal&           operator=(const OutputSignal& from);

    bool                    operator==(const OutputSignal& signal);
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

    OutputSignal            signal(const int index) const;
    void                    setSignal(const int index, const OutputSignal& signal);

    void                    remove(const OutputSignal& signal);
    void                    remove(const int index);

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
