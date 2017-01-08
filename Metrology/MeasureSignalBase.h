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

class MeasureSignalBase : public PtrObjectVector<MeasureMultiSignal>
{
public:

    explicit    MeasureSignalBase();
                ~MeasureSignalBase();

private:

    QMutex m_baseMutex;

    MeasureMultiSignal* m_pActiveSignal = nullptr;

public:

    MeasureMultiSignal* activeSignal() const { return m_pActiveSignal; }
    void setActiveSignal(MeasureMultiSignal* s) { m_baseMutex.lock(); m_pActiveSignal = s; m_baseMutex.unlock(); }
};

// ==============================================================================================

extern MeasureSignalBase theMeasureSignalBase;

// ==============================================================================================

#endif // MEASURESIGNALBASE_H
