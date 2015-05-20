#include "MeasureSignalBase.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalBase theMeasureSignalBase;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam::MeasureSignalParam()
{
}

MeasureSignalParam::~MeasureSignalParam()
{
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignalState::MeasureSignalState()
{
}

MeasureSignalState::~MeasureSignalState()
{
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


MeasureSignal::MeasureSignal()
{
}

MeasureSignal::~MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComplexMeasureSignal::ComplexMeasureSignal()
{
    clear();
}

ComplexMeasureSignal::~ComplexMeasureSignal()
{
    clear();
}

void ComplexMeasureSignal::clear()
{
    m_mutex.lock();

        for(int i = 0; i < COMPLEX_MEASURE_SIGNAL_COUNT; i++)
        {
            m_signal[i] = nullptr;
        }

     m_mutex.unlock();
}

MeasureSignal* ComplexMeasureSignal::signal(int index) const
{
    if (index < 0 || index >= COMPLEX_MEASURE_SIGNAL_COUNT)
    {
        return nullptr;
    }

    return m_signal[index];
}

void ComplexMeasureSignal::setSignal(int index, MeasureSignal *s)
{
    if (index < 0 || index >= COMPLEX_MEASURE_SIGNAL_COUNT)
    {
        return;
    }

    m_mutex.lock();

        m_signal[index] = s;

    m_mutex.unlock();
}

ComplexMeasureSignal& ComplexMeasureSignal::operator=(const ComplexMeasureSignal& from)
{
    for(int i = 0; i < COMPLEX_MEASURE_SIGNAL_COUNT; i++)
    {
        setSignal(i, from.signal(i) );
    }

    return *this;
}

bool ComplexMeasureSignal::operator==(const ComplexMeasureSignal& from)
{
    for(int i = 0; i < COMPLEX_MEASURE_SIGNAL_COUNT; i++)
    {
        if ( signal(i) != from.signal(i) )
        {
            return false;
        }
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignalBase::MeasureSignalBase()
{
}

MeasureSignalBase::~MeasureSignalBase()
{
}


// -------------------------------------------------------------------------------------------------------------------
