#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include <QObject>

#include "Measure.h"

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalState.h"


// ==============================================================================================

struct MEASURE_SIGNAL
{
    MEASURE_SIGNAL()
    {

    }

    MEASURE_SIGNAL(Signal param)
    {
        m_param = param;

        if (param.equipmentID().isEmpty() == false && param.hash() != 0)
        {
            m_position.setFromID(param.equipmentID());
        }
    }

    MEASURE_SIGNAL(Signal param, AppSignalState state)
    {
        m_param = param;
        m_state = state;

        if (param.equipmentID().isEmpty() == false && param.hash() != 0)
        {
            m_position.setFromID(param.equipmentID());
        }
    }

    Signal m_param;
    AppSignalState m_state;
    DevicePosition m_position;
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

    int                     appendSignal(const Signal &param);

    bool                    signalParam(const int& index, Signal& param);
    bool                    signalParam(const Hash& hash, Signal& param);

    bool                    signalState(const Hash& hash, AppSignalState& state);
    bool                    signalState(const int& index, AppSignalState& state);

    bool                    setSignalState(const Hash& hash, const AppSignalState& state);
    bool                    setSignalState(const int& index, const AppSignalState& state);

    // Units
    //

    void                    appendUnit(const int& id, const QString& unit);

    QString                 signalUnit(const Hash& hash);
    QString                 signalUnit(const int& index);

    QString                 unit(const int& id);

    // hashs for update signal state
    //

    int                     hashForRequestStateCount() const;
    Hash                    hashForRequestState(int index);

private:

    mutable QMutex          m_paramMutex;
    QMap<Hash, int>         m_hashMap;
    QVector<MEASURE_SIGNAL> m_signalList;

    mutable QMutex          m_unitMutex;
    UnitList                m_unitList;

    mutable QMutex          m_stateMutex;
    QVector<Hash>           m_requestStateList;

signals:

public slots:

};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================


#endif // SIGNALBASE_H
