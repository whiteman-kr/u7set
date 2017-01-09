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

#include "Measure.h"

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalState.h"

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
    QString             adjustmentString();
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

    MeasureSignal*      m_signal[MEASURE_MULTI_SIGNAL_COUNT];

    int                 m_caseNo = -1;
    int                 m_subblock = -1;
    int                 m_block = -1;
    int                 m_entry = -1;

public:

    void                clear();
    bool                isEmpty() const;

    MeasureSignal*      signal(const int& index) const;
    void                setSignal(const int& index, MeasureSignal* pSignal);

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
    MeasureSignal           operator [] (int index);

    int                     appendSignal(const Signal &param);

    Signal*                 signalParam(const Hash& hash);
    Signal*                 signalParam(const int& index);

    AppSignalState*         signalState(const Hash& hash);
    AppSignalState*         signalState(const int& index);

    bool                    setSignalState(const Hash& hash, const AppSignalState& state);
    bool                    setSignalState(const int& index, const AppSignalState& state);

    // Units
    //
    void                    appendUnit(const int& unitID, const QString& unit);
    QString                 unit(const int& unitID);

    // hashs for update signal state
    //

    int                     hashForRequestStateCount() const;
    Hash                    hashForRequestState(int index);

    // Signals and Cases for measure
    //
    int                     createCaseTypeList();
    int                     caseTypeCount() const;
    QString                 caseTypeCaption(const int& type);

    int                     caseNoCount() const;
    int                     caseNoByCaseIndex(const int& caseIndex);
    //int                     caseIndexByCaseNo(const int& caseNo);

    int                     createSignalListForMeasure(const int& caseType, const int &measureKind);
    int                     signalForMeasureCount() const;
    bool                    signalForMeasure(const int& index, MeasureMultiSignal& signal);

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
