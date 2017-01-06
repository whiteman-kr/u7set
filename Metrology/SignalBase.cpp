#include "SignalBase.h"


// -------------------------------------------------------------------------------------------------------------------

SignalBase theSignalBase;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal(Signal param)
{
    m_param = param;

    // temporary solution
    //

    if (param.equipmentID().isEmpty() == false && param.hash() != 0)
    {
        if (param.isAnalog() == true && param.isInput() == true)
        {
            m_position.setFromID(param.equipmentID());
        }
    }

    m_param.setInputLowLimit(4);
    m_param.setInputHighLimit(20);
    m_param.setInputUnitID(15);
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::~MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal& MeasureSignal::operator=(const MeasureSignal& from)
{
    m_param = from.m_param;
    m_state = from.m_state;
    m_position = from.m_position;

    m_inputPhysicalUnit = from.m_inputPhysicalUnit;
    m_inputElectricUnit = from.m_inputElectricUnit;
    m_outputPhysicalUnit = from.m_outputPhysicalUnit;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::adcRangeString()
{
    QString range;

    range.sprintf("%04X .. %04X", m_param.lowADC(), m_param.highADC());

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::inputPhysicalRangeString()
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_param.decimalPlaces() );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_param.lowEngeneeringUnits(), m_param.highEngeneeringUnits());

    range.append(m_inputPhysicalUnit);

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::inputElectricRangeString()
{
    QString range;

    range.sprintf("%.3f .. %.3f ", m_param.inputLowLimit(), m_param.inputHighLimit());

    range.append(m_inputElectricUnit);

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::outputPhysicalRangeString()
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_param.decimalPlaces() );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_param.outputLowLimit(), m_param.outputHighLimit());

    range.append(m_outputPhysicalUnit);

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::outputElectricRangeString()
{

    int mode = m_param.outputMode();

    if (mode < 0 || mode >= OUTPUT_MODE_COUNT)
    {
        assert(false);
        return "";
    }

    return OutputModeStr[ m_param.outputMode() ];
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

 SignalBase::~SignalBase()
 {
 }

 // -------------------------------------------------------------------------------------------------------------------

void SignalBase::clear()
{
    m_paramMutex.lock();

        m_hashMap.clear();

        m_signalList.clear();

    m_paramMutex.unlock();

    m_unitMutex.lock();

        m_unitList.clear();

    m_unitMutex.unlock();

    m_stateMutex.lock();

        m_requestStateList.clear();

    m_stateMutex.unlock();

}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::size() const
{
    int size = 0;

    m_paramMutex.lock();

        size = m_signalList.size();

    m_paramMutex.unlock();

    return size;
}
// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::hashIsValid(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return false;
    }

    bool valid = false;

    m_paramMutex.lock();

        if (m_hashMap.contains(hash) == true)
        {
            valid = true;
        }

    m_paramMutex.unlock();

    return valid;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::operator [](int index)
{
    MeasureSignal signal;

    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            signal = m_signalList[index];
        }

    m_paramMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Signal& param)
{
    if (param.appSignalID().isEmpty() == true || param.hash() == 0)
    {
        assert(false);
        return -1;
    }

    int index = -1;

    m_paramMutex.lock();

        if (m_hashMap.contains(param.hash()) == false)
        {
            MeasureSignal signal(param);

            m_signalList.append(signal);
            index = m_signalList.size() - 1;

            m_hashMap.insert(param.hash(), index);
        }

     m_paramMutex.unlock();

     return index;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalParam(const Hash& hash, Signal& param)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return signalParam(index, param);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalParam(const int& index, Signal& param)
{
    bool found = false;

    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].param();

            found = true;
        }

    m_paramMutex.unlock();

    return found;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalState(const Hash& hash, AppSignalState& state)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return signalState(index, state);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalState(const int& index, AppSignalState& state)
{
    bool found = false;

    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            state = m_signalList[index].state();

            found = true;
        }

    m_paramMutex.unlock();

    return found;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalState(const Hash& hash, const AppSignalState &state)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return setSignalState(index, state);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalState(const int& index, const AppSignalState &state)
{
    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setState(state);
        }

    m_paramMutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::appendUnit(const int& unitID, const QString& unit)
{
    m_unitMutex.lock();

        if (m_unitList.contains(unitID) == false)
        {
            m_unitList.insert(unitID, unit);
        }

    m_unitMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::unit(const int& unitID)
{
    QString strUnit;

    m_unitMutex.lock();

        if (m_unitList.contains(unitID) == true)
        {
            strUnit = m_unitList[unitID];
        }

    m_unitMutex.unlock();

    return strUnit;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::updateSignalUnit()
{
    m_paramMutex.lock();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            MeasureSignal& signal = m_signalList[i];

            if (signal.param().hash() == 0)
            {
                continue;
            }

            signal.setInputPhysicalUnit( unit( signal.param().unitID() ) );

            QString inputUnit = unit( signal.param().inputUnitID() );
            int inputSensorID = signal.param().inputSensorID();

            if (inputSensorID >= 1 && inputSensorID < SENSOR_TYPE_COUNT)
            {
                inputUnit = tr("%1 (%2)").arg(inputUnit).arg( SensorTypeStr[ inputSensorID ] );
            }

            signal.setInputElectricUnit( inputUnit );

            signal.setOutputPhysicalUnit( unit( signal.param().outputUnitID() ) );
        }

    m_paramMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::hashForRequestStateCount() const
{
    int count = 0;

    m_stateMutex.lock();

        count = m_requestStateList.size();

    m_stateMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalBase::hashForRequestState(int index)
{
    Hash hash;

    m_stateMutex.lock();

        if (index >= 0 && index < m_requestStateList.size())
        {
            hash = m_requestStateList[index];
        }

    m_stateMutex.unlock();

    return hash;
}

// -------------------------------------------------------------------------------------------------------------------
