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

class DevicePosition
{
private:

    QString                 m_equipmentID;

    int                     m_caseNo = -1;

    int                     m_caseType = -1;        // depend from m_caseCaption
    QString                 m_caseCaption;

    int                     m_channel = -1;
    int                     m_subblock = -1;
    int                     m_block = -1;
    int                     m_entry = -1;

public:

    void                    setFromID(const QString& equipmentID);

    QString                 equipmentID() const { return m_equipmentID; }
    void                    setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

    int                     caseNo() const { return m_caseNo; }
    void                    setCaseNo(int caseNo) { m_caseNo = caseNo; }

    int                     caseType() const { return m_caseType; }
    void                    setCaseType(int type) { m_caseType = type; }

    QString                 caseCaption() const { return m_caseCaption; }
    void                    setCaseCaption(const QString& caption) { m_caseCaption = caption; }

    QString                 caseString() const;

    int                     channel() const { return m_channel; }
    void                    setChannel(int channel) { m_channel = channel; }

    QString                 channelString() const;

    int                     subblock() const { return m_subblock; }
    void                    setSubblock(int subblock) { m_subblock = subblock; }

    QString                 subblockString() const;

    int                     block() const { return m_block; }
    void                    setBlock(int block) { m_block = block; }

    QString                 blockString() const;

    int                     entry() const { return m_entry; }
    void                    setEntry(int entry) { m_entry = entry; }

    QString                 entryString() const;

    DevicePosition&         operator=(const DevicePosition& from);
};

// ==============================================================================================

class MeasureSignal
{
public:

    explicit            MeasureSignal();
    explicit            MeasureSignal(Signal param);
                        ~MeasureSignal();

private:

    Signal              m_param;
    AppSignalState      m_state;
    DevicePosition      m_position;

public:

    Signal              param() const { return m_param; }
    void                setParam(Signal param) { m_param = param; }

    AppSignalState      state() const { return m_state; }
    void                setState(AppSignalState state) { m_state = state; }

    DevicePosition      position() const { return m_position; }
    void                setPosition(DevicePosition position) { m_position = position; }

    void                setCaseType(const int& type) { m_position.setCaseType(type); }

    MeasureSignal&      operator=(const MeasureSignal& from);

    QString             stateString();
    QString             adcRange(const bool& showInHex);
    QString             inputPhysicalRange();
    QString             inputElectricRange();
    QString             outputPhysicalRange();
    QString             outputElectricRange();
    QString             calibratorIndexString(const int& index);
};

// ==============================================================================================

                        Q_DECLARE_METATYPE(MeasureSignal)
                        Q_DECLARE_METATYPE(MeasureSignal*)

// ==============================================================================================

const int               MEASURE_MULTI_SIGNAL_0            = 0,
                        MEASURE_MULTI_SIGNAL_1            = 1,
                        MEASURE_MULTI_SIGNAL_2            = 2,
                        MEASURE_MULTI_SIGNAL_3            = 3,
                        MEASURE_MULTI_SIGNAL_4            = 4,
                        MEASURE_MULTI_SIGNAL_5            = 5;

const int               MEASURE_MULTI_SIGNAL_COUNT        = 6;

// ----------------------------------------------------------------------------------------------

class MeasureMultiSignal
{
public:

    explicit            MeasureMultiSignal();
    explicit            MeasureMultiSignal(const MeasureMultiSignal& from) { *this = from; }
                        ~MeasureMultiSignal();

private:

    mutable QMutex      m_mutex;

    Hash                m_signalHash[MEASURE_MULTI_SIGNAL_COUNT];

    int                 m_caseNo = -1;
    int                 m_subblock = -1;
    int                 m_block = -1;
    int                 m_entry = -1;

public:

    void                clear();
    bool                isEmpty() const;

    Hash                hash(const int& index) const;
    void                setSignal(const int& index, const MeasureSignal& signal);

    int                 caseNo() const { return m_caseNo; }
    void                setCaseNo(int caseNo) { m_caseNo = caseNo; }

    int                 subblock() const { return m_subblock; }
    void                setSubblock(int subblock) { m_subblock = subblock; }

    int                 block() const { return m_block; }
    void                setBlock(int block) { m_block = block; }

    int                 entry() const { return m_entry; }
    void                setEntry(int entry) { m_entry = entry; }

    MeasureMultiSignal& operator=(const MeasureMultiSignal& from);
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
    bool                    hashIsValid(const Hash& hash);

    // Signals
    //

    int                     appendSignal(const Signal &param);

    MeasureSignal           signal(const Hash& hash);
    MeasureSignal           signal(const int& index);

    Signal                  signalParam(const Hash& hash);
    Signal                  signalParam(const int& index);

    void                    setSignalParam(const Hash& hash, const Signal& param);
    void                    setSignalParam(const int& index, const Signal& param);

    AppSignalState          signalState(const Hash& hash);
    AppSignalState          signalState(const int& index);

    void                    setSignalState(const Hash& hash, const AppSignalState& state);
    void                    setSignalState(const int& index, const AppSignalState& state);

    // Units
    //
    void                    appendUnit(const int& unitID, const QString& unit);
    int                     unitCount() const;
    QString                 unit(const int& unitID);

    // hashs for update signal state
    //

    int                     hashForRequestStateCount() const;
    Hash                    hashForRequestState(int index);

    // Signals and Cases for measure
    //
    int                     createCaseTypeList();

                            // cases
                            //
    int                     caseTypeCount() const;
    QString                 caseTypeCaption(const int& type);

    int                     caseNoCount() const;
    int                     caseNoByCaseIndex(const int& caseIndex);

                            // signals
                            //
    int                     createSignalListForMeasure(const int& caseType, const int &measureKind);
    int                     signalForMeasureCount() const;
    bool                    signalForMeasure(const int& index, MeasureMultiSignal& multiSignal);

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

    // all units that received form AppDataSrv
    //
    mutable QMutex          m_unitMutex;
    QMap<int, QString>      m_unitMap;

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

public slots:

};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================


#endif // SIGNALBASE_H
