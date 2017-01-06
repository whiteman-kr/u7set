#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include <QObject>

#include "Measure.h"

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalState.h"


// ==============================================================================================

class MeasureSignal
{
public:

    explicit    MeasureSignal();
                MeasureSignal(Signal param);
                ~MeasureSignal();

private:

    Signal m_param;
    AppSignalState m_state;
    DevicePosition m_position;

    QString m_inputPhysicalUnit;
    QString m_inputElectricUnit;
    QString m_outputPhysicalUnit;

public:

    Signal param() const { return m_param; }
    void setParam(Signal param) { m_param = param; }

    AppSignalState state() const { return m_state; }
    void setState(AppSignalState state) { m_state = state; }

    DevicePosition position() const { return m_position; }
    void setState(DevicePosition position) { m_position = position; }

    QString inputPhysicalUnit() const { return m_inputPhysicalUnit; }
    void setInputPhysicalUnit(QString unit) { m_inputPhysicalUnit = unit; }

    QString inputElectricUnit() const { return m_inputElectricUnit; }
    void setInputElectricUnit(QString unit) { m_inputElectricUnit = unit; }

    QString outputPhysicalUnit() const { return m_outputPhysicalUnit; }
    void setOutputPhysicalUnit(QString unit) { m_outputPhysicalUnit = unit; }

    MeasureSignal& operator=(const MeasureSignal& from);

    QString adcRangeString();
    QString inputPhysicalRangeString();
    QString inputElectricRangeString();
    QString outputPhysicalRangeString();
    QString outputElectricRangeString();
};

// ==============================================================================================

class SignalBase : public QObject
{
    Q_OBJECT

public:
    explicit                SignalBase(QObject *parent = 0);
                            ~SignalBase();

    void                    clear();
    int                     size() const;
    bool                    hashIsValid(const Hash& hash);

    // Signals
    //

    MeasureSignal           operator [] (int index);

    int                     appendSignal(const Signal &param);

    bool                    signalParam(const int& index, Signal& param);
    bool                    signalParam(const Hash& hash, Signal& param);

    bool                    signalState(const Hash& hash, AppSignalState& state);
    bool                    signalState(const int& index, AppSignalState& state);

    bool                    setSignalState(const Hash& hash, const AppSignalState& state);
    bool                    setSignalState(const int& index, const AppSignalState& state);

    // Units
    //

    void                    appendUnit(const int& unitID, const QString& unit);
    QString                 unit(const int& unitID);
    void                    updateSignalUnit();


    // hashs for update signal state
    //

    int                     hashForRequestStateCount() const;
    Hash                    hashForRequestState(int index);

private:

    mutable QMutex          m_paramMutex;
    QMap<Hash, int>         m_hashMap;
    QVector<MeasureSignal>  m_signalList;

    mutable QMutex          m_unitMutex;
    QMap<int, QString>      m_unitList;

    mutable QMutex          m_stateMutex;
    QVector<Hash>           m_requestStateList;

signals:

public slots:

};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================


#endif // SIGNALBASE_H
