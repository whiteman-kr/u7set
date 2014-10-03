#include "MeasureThread.h"

#include <assert.h>
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QObject *parent) :
    QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
    for(int t = 0; t < theOptions.getToolBar().m_measureTimeout; t += MEASURE_THREAD_TIMEOUT_STEP )
    {
        if (m_cmdStopMeasure == true)
        {
            break;
        }

        QThread::msleep(MEASURE_THREAD_TIMEOUT_STEP);

        emit measureInfo(t);
    }
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::prepareCalibrator()
{
    emit measureInfo("Prepare calibrator ");

    QThread::msleep(1000);

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

void MeasureThread::measureLinearity()
{
    emit measureInfo("Linearity ");

    waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComprators()
{
    emit measureInfo("Comprators ");

    waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComplexComprators()
{
    emit measureInfo("Complex Comprators ");

    waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

