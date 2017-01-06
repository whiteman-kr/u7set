#ifndef MEASURESIGNALBASE_H
#define MEASURESIGNALBASE_H

#include "Measure.h"
#include "ObjectVector.h"
#include "SignalBase.h"

// ==============================================================================================

class MeasureSignalParam
{
public:

    explicit    MeasureSignalParam();
                ~MeasureSignalParam();

private:

    QString m_strID;
    QString m_extStrID;
    QString m_name;

    DevicePosition m_position;

    double m_lowLimit[VALUE_TYPE_COUNT];
    double m_highLimit[VALUE_TYPE_COUNT];
    int m_unitID[VALUE_TYPE_COUNT];

    bool m_hasOutput = false;
    double m_adjustment = 0;

public:

    QString strID() const { return m_strID; }
    void setStrID(const QString& strID) { m_strID = strID; }

    QString extStrID() const { return m_extStrID; }
    void setExtStrID(const QString& extStrID) { m_extStrID = extStrID; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    DevicePosition& position() { return m_position; }

    double lowLimit(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_lowLimit[type]; }
    void setLowLimit(int type, double lowLimit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_lowLimit[type] = lowLimit; }

    double highLimit(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_highLimit[type]; }
    void setHighLimit(int type, double highLimit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_highLimit[type] = highLimit; }

    int unitID(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return -1; } return m_unitID[type]; }
    void setUnitID(int type, int unit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_unitID[type] = unit; }

    bool hasOutput() { return m_hasOutput; }
    void setHasOutput(bool hasOutput) { m_hasOutput = hasOutput; }

    double adjustment() const { return m_adjustment; }
    void setAdjustment(double adjustment) { m_adjustment = adjustment; }
};

// ==============================================================================================

class MeasureSignalState
{
public:

    explicit    MeasureSignalState();
                ~MeasureSignalState();

private:

    double m_value;

public:

    double value() const { return m_value; }
    void setValue(double value) { m_value = value; }
};

// ==============================================================================================

const int       COMPLEX_MEASURE_SIGNAL_0            = 0,
                COMPLEX_MEASURE_SIGNAL_1            = 1,
                COMPLEX_MEASURE_SIGNAL_2            = 2,
                COMPLEX_MEASURE_SIGNAL_3            = 3,
                COMPLEX_MEASURE_SIGNAL_4            = 4,
                COMPLEX_MEASURE_SIGNAL_5            = 5;

const int       COMPLEX_MEASURE_SIGNAL_COUNT        = 6;

// ==============================================================================================

class ComplexMeasureSignal
{
public:

    explicit    ComplexMeasureSignal();
    explicit    ComplexMeasureSignal(const ComplexMeasureSignal& from) { *this = from; }
                ~ComplexMeasureSignal();

private:

    QMutex m_mutex;

    MeasureSignal* m_signal[COMPLEX_MEASURE_SIGNAL_COUNT];

public:

    void clear();

    MeasureSignal* signal(int index) const;
    void setSignal(int index, MeasureSignal *s);

    ComplexMeasureSignal& operator=(const ComplexMeasureSignal& from);
    bool operator==(const ComplexMeasureSignal& from);
};

// ==============================================================================================

class MeasureSignalBase : public PtrObjectVector<ComplexMeasureSignal>
{
public:

    explicit    MeasureSignalBase();
                ~MeasureSignalBase();

private:

    QMutex m_baseMutex;

    ComplexMeasureSignal* m_pActiveSignal = nullptr;

public:

    ComplexMeasureSignal* activeSignal() const { return m_pActiveSignal; }
    void setActiveSignal(ComplexMeasureSignal* s) { m_baseMutex.lock(); m_pActiveSignal = s; m_baseMutex.unlock(); }
};

// ==============================================================================================

extern MeasureSignalBase theMeasureSignalBase;

// ==============================================================================================

#endif // MEASURESIGNALBASE_H
