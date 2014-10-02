#include "MeasureThread.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QObject *parent) :
    QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::prepareCalibrator()
{
    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::stop()
{
    m_cmdStopMeasure = true;

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    m_cmdStopMeasure = false;

    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:

            prepareCalibrator();

            measureLinearity();

            break;

        case MEASURE_TYPE_COMPARATOR:

            prepareCalibrator();

            measureComprators();

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            prepareCalibrator();

            measureComplexComprators();

            break;

        default:
            assert(false);
            break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

