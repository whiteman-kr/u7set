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

#include "CalibratorManager.h"

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

class SignalPosition
{
public:

                            SignalPosition();
    explicit                SignalPosition(const QString& equipmentID);
                            ~SignalPosition();

private:

    QString                 m_equipmentID;

    int                     m_rackNo = -1;

    int                     m_rackType = -1;        // depend from m_caseCaption
    QString                 m_rackCaption;

    int                     m_channel = -1;

    int                     m_chassis = -1;
    int                     m_module = -1;
    int                     m_place = -1;
    QString                 m_contact;

public:

    void                    clear();

    void                    setFromID(const QString& equipmentID);
    bool                    readFromXml(XmlReadHelper& xml);

    QString                 equipmentID() const { return m_equipmentID; }
    void                    setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

    int                     caseNo() const { return m_rackNo; }
    void                    setCaseNo(int caseNo) { m_rackNo = caseNo; }

    int                     caseType() const { return m_rackType; }
    void                    setCaseType(int type) { m_rackType = type; }

    QString                 caseCaption() const { return m_rackCaption; }
    void                    setCaseCaption(const QString& caption) { m_rackCaption = caption; }

    QString                 caseStr() const;

    int                     channel() const { return m_channel; }
    QString                 channelStr() const;
    void                    setChannel(int channel) { m_channel = channel; }

    int                     subblock() const { return m_chassis; }
    QString                 subblockStr() const;
    void                    setSubblock(int subblock) { m_chassis = subblock; }

    int                     block() const { return m_module; }
    QString                 blockStr() const;
    void                    setBlock(int block) { m_module = block; }

    int                     entry() const { return m_place; }
    QString                 entryStr() const;
    void                    setEntry(int entry) { m_place = entry; }

    QString                 contact() const { return m_contact; }
    void                    setContact(const QString& contact) { m_contact = contact; }

};

// ==============================================================================================

class SignalParam
{
public:
                            SignalParam();
                            SignalParam(const Signal& signal);
                            ~SignalParam();
private:

    Hash                    m_hash = 0;                        // hash calcHash from AppSignalID

    QString                 m_appSignalID;
    QString                 m_customAppSignalID;
    QString                 m_caption;

    E::SignalType           m_signalType = E::SignalType::Analog;
    E::SignalInOutType      m_inOutType = E::SignalInOutType::Internal;

    SignalPosition          m_position;

    int                     m_lowADC = 0;
    int                     m_highADC = 0;

    double                  m_inputElectricLowLimit = 0;
    double                  m_inputElectricHighLimit = 0;
    E::InputUnit            m_inputElectricUnitID = E::InputUnit::NoInputUnit;
    E::SensorType           m_inputElectricSensorType = E::SensorType::NoSensorType;
    int                     m_inputElectricPrecision = 3;

    double                  m_inputPhysicalLowLimit = 0;
    double                  m_inputPhysicalHighLimit = 0;
    int                     m_inputPhysicalUnitID = NO_UNIT_ID;
    int                     m_inputPhysicalPrecision = 2;

    double                  m_outputElectricLowLimit = 0;
    double                  m_outputElectricHighLimit = 0;
    E::InputUnit            m_outputElectricUnitID  = E::InputUnit::NoInputUnit;
    E::SensorType           m_outputElectricSensorType = E::SensorType::NoSensorType;
    int                     m_outputElectricPrecision = 3;

    double                  m_outputPhysicalLowLimit = 0;
    double                  m_outputPhysicalHighLimit = 0;
    int                     m_outputPhysicalUnitID = NO_UNIT_ID;
    int                     m_outputPhysicalPrecision = 2;

    bool                    m_enableTuning = false;
    double                  m_tuningDefaultValue = 0;


    StatisticItem           m_statistic;

public:

    bool                    isValid() const;

    void                    setParam(const Signal& signal);
	bool                    readFromXml(XmlReadHelper& xml);

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

    bool                    isAnalog() const { return m_signalType == E::SignalType::Analog; }
    bool                    isDiscrete() const { return m_signalType == E::SignalType::Discrete; }

    bool                    isInput() const { return m_inOutType == E::SignalInOutType::Input; }
    bool                    isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
    bool                    isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }

    SignalPosition          position() const { return m_position; }
    void                    setPosition(const SignalPosition& position) { m_position = position; }

    void                    setCaseType(int type) { m_position.setCaseType(type); }
	void                    setCaseNo(int caseNo) { m_position.setCaseNo(caseNo); }

    int                     lowADC() const { return m_lowADC; }
    void                    setLowADC(int lowADC) { m_lowADC = lowADC; }

    int                     highADC() const { return m_highADC; }
    void                    setHighADC(int highADC) { m_highADC = highADC;}

    QString                 adcRangeStr(bool showHex) const;

    double                  inputElectricLowLimit() const { return m_inputElectricLowLimit; }
    void                    setInputElectricLowLimit(double lowLimit) { m_inputElectricLowLimit = lowLimit; }

    double                  inputElectricHighLimit() const { return m_inputElectricHighLimit; }
    void                    setInputElectricHighLimit(double highLimit) { m_inputElectricHighLimit = highLimit; }

    E::InputUnit            inputElectricUnitID() const { return m_inputElectricUnitID; }
    void                    setInputElectricUnitID(const E::InputUnit unit) { m_inputElectricUnitID = unit; }

    QString                 inputElectricRangeStr() const;

    E::SensorType           inputElectricSensorType() const { return m_inputElectricSensorType; }
    QString                 inputElectricSensorStr() const;
    void                    setInputElectricSensorType(const E::SensorType sensorType) { m_inputElectricSensorType = sensorType; }

    int                     inputElectricPrecision() const { return m_inputElectricPrecision; }
    void                    setInputElectricPrecision(int precision) { m_inputElectricPrecision = precision; }

    double                  inputPhysicalLowLimit() const { return m_inputPhysicalLowLimit; }
    void                    setInputPhysicalLowLimit(double lowLimit) { m_inputPhysicalLowLimit = lowLimit; }

    double                  inputPhysicalHighLimit() const { return m_inputPhysicalHighLimit; }
    void                    setInputPhysicalHighLimit(double highLimit) { m_inputPhysicalHighLimit = highLimit; }

    int                     inputPhysicalUnitID() const { return m_inputPhysicalUnitID; }
    void                    setInputPhysicalUnitID(int unit) { m_inputPhysicalUnitID = unit; }

    QString                 inputPhysicalRangeStr() const;

    int                     inputPhysicalPrecision() const { return m_inputPhysicalPrecision; }
    void                    setInputPhysicalPrecision(int precision) { m_inputPhysicalPrecision = precision; }

    double                  outputElectricLowLimit() const { return m_outputElectricLowLimit; }
    void                    setOutputElectricLowLimit(double lowLimit) { m_outputElectricLowLimit = lowLimit; }

    double                  outputElectricHighLimit() const { return m_outputElectricHighLimit; }
    void                    setOutputElectricHighLimit(double highLimit) { m_outputElectricHighLimit = highLimit; }

    E::InputUnit            outputElectricUnitID() const { return m_outputElectricUnitID; }
    void                    setOutputElectricUnitID(const E::InputUnit unit) { m_outputElectricUnitID = unit; }

    QString                 outputElectricRangeStr() const;

    E::SensorType           outputElectricSensorType() const { return m_outputElectricSensorType; }
    QString                 outputElectricSensorStr() const;
    void                    setOutputElectricSensorType(const E::SensorType sensorType) { m_outputElectricSensorType = sensorType; }

    int                     outputElectricPrecision() const { return m_outputElectricPrecision; }
    void                    setOutputElectricPrecision(int precision) { m_outputElectricPrecision = precision; }

    double                  outputPhysicalLowLimit() const { return m_outputPhysicalLowLimit; }
    void                    setOutputPhysicalLowLimit(double lowLimit) { m_outputPhysicalLowLimit = lowLimit; }

    double                  outputPhysicalHighLimit() const { return m_outputPhysicalHighLimit; }
    void                    setOutputPhysicalHighLimit(double highLimit) { m_outputPhysicalHighLimit = highLimit; }

    int                     outputPhysicalUnitID() const { return m_outputPhysicalUnitID; }
    void                    setOutputPhysicalUnitID(int unit) { m_outputPhysicalUnitID = unit; }

    QString                 outputPhysicalRangeStr() const;

    int                     outputPhysicalPrecision() const { return m_outputPhysicalPrecision; }
    void                    setOutputPhysicalPrecision(int precision) { m_outputPhysicalPrecision = precision; }

    bool                    enableTuning() const { return m_enableTuning; }
    QString                 enableTuningStr() const;
    void                    setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

    double                  tuningDefaultValue() const { return m_tuningDefaultValue; }
    QString                 tuningDefaultValueStr() const;
    void                    setTuningDefaultValue(double value) { m_tuningDefaultValue = value; }

    StatisticItem           statistic() const { return m_statistic; }
    void                    setStatistic(const StatisticItem& statistic) { m_statistic = statistic; }
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(SignalParam)
                            Q_DECLARE_METATYPE(SignalParam*)

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

class MetrologySignal
{
public:

                            MetrologySignal();
    explicit                MetrologySignal(const Signal& signal);
    explicit                MetrologySignal(const SignalParam& param);
                            ~MetrologySignal();

private:

    SignalParam             m_param;
    AppSignalState          m_state;

public:

    SignalParam&            param() { return m_param; }
    void                    setParam(const SignalParam& param) { m_param = param; }

    AppSignalState&         state() { return m_state; }
    void                    setState(const AppSignalState& state) { m_state = state; }

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

    SignalPosition          m_position;

    QString                 m_strID;

public:

    void                    clear();
    bool                    isEmpty() const;

    Hash                    hash(int channel) const;
    bool                    setSignal(int channel, int measureKind, const SignalParam& param);

    SignalPosition&         position() { return m_position; }
    void                    setPosition(const SignalPosition& position) { m_position = position; }

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

    SignalParam             m_param[MEASURE_IO_SIGNAL_TYPE_COUNT];

    int                     m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

    bool                    m_equalPhysicalRange = false;

    CalibratorManager*      m_pCalibratorManager = nullptr;

public:

    void                    clear();
    bool                    isValid() const;

    SignalParam             param(int type) const;
    bool                    setParam(int type, const SignalParam& param);

    int                     outputSignalType() const { return m_outputSignalType; }
    void                    setOutputSignalType(int type) { m_outputSignalType = type; }

    bool                    equalPhysicalRange() const { return m_equalPhysicalRange; }
    bool                    testPhysicalRange();

    QString                 caseStr() const;
    QString                 signalID(bool showCustomID, const QString& divider) const;
    QString                 subblockStr() const;
    QString                 blockStr() const;
    QString                 entryStr() const;
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
    bool                    setSignal(int channel, int measureKind, int outputSignalType, const SignalParam& param);

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

    int                     appendSignal(const Signal& signal);
	int						appendSignal(const SignalParam& param);

    MetrologySignal         signal(const QString& appSignalID);
    MetrologySignal         signal(const Hash& hash);
    MetrologySignal         signal(int index);

    SignalParam             signalParam(const QString& appSignalID);
    SignalParam             signalParam(const Hash& hash);
    SignalParam             signalParam(int index);

    void                    setSignalParam(const QString& appSignalID, const SignalParam& param);
    void                    setSignalParam(const Hash& hash, const SignalParam& param);
    void                    setSignalParam(int index, const SignalParam& param);

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

    // Signals and Cases for measure
    //
                            // cases
                            //
    int                     createCaseTypeList(int outputSignalType);
    void                    clearCaseTypeList();

                                    // type of cases
                                    //
    int                     caseTypeCount() const;
    QString                 caseTypeCaption(int type);

                                    // index of cases
                                    //
	void                    setCaseNoForAllSignals();
    int                     caseNoCount() const;
    int                     caseNoByCaseIndex(int caseIndex);

                            // signals
                            //
    int                     createMeasureSignalList(int caseType, int measureKind, int outputSignalType);
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

    // list of cases in order to select signal for measure
    //
    mutable QMutex          m_caseMutex;
    QVector<QString>        m_caseTypeList;
    QVector<int>            m_caseNoList;

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
    SignalParam             m_param[MEASURE_IO_SIGNAL_TYPE_COUNT];

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

    SignalParam             param(int type) const;
    void                    setParam(int type, const SignalParam& param);
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
