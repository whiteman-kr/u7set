#include "MeasureThread.h"

#include <assert.h>
#include <QMessageBox>
#include <QTime>
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QObject *parent) :
    QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
    for(int t = 0; t < theOptions.getToolBar().m_measureTimeout; t += MT_TIMEOUT_STEP )
    {
        if (m_cmdStopMeasure == true)
        {
            break;
        }

        QThread::msleep(MT_TIMEOUT_STEP);

        emit measureInfo(t + MT_TIMEOUT_STEP);
    }
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::prepareCalibrator(CalibratorManager* manager, int mode, int unit)
{
    if (manager == nullptr)
    {
        return false;
    }

    if (manager->calibratorIsConnected() == false)
    {
        return false;
    }

    if (mode < 0 || mode >= CALIBRATOR_MODE_COUNT)
    {
        return false;
    }

    if (unit < 0 || unit >= CALIBRATOR_UNIT_COUNT)
    {
        return false;
    }

    emit measureInfo(QString("Prepare calibrator %1 ").arg(manager->getIndex() + 1));

    return manager->setUnit(mode, unit);;
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

    // Prepare calibreators
    //
    int channelCount = theOptions.getChannelCount();

    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            // select mode and unit
            //
            for (int c = 0; c < channelCount; c++)
            {
                CalibratorManager* manager = theCalibratorBase.getCalibratorManager(c);
                if (manager == nullptr)
                {
                    continue;
                }

                if (manager->calibratorIsConnected() == false)
                {
                    continue;
                }

                // if analog signal has output range
                //
                if (prepareCalibrator(manager, CALIBRATOR_MODE_MEASURE, CALIBRATOR_UNIT_MA) == false)
                {
                    // meassage box
                }

                if (prepareCalibrator(manager, CALIBRATOR_MODE_SOURCE, CALIBRATOR_UNIT_LOW_OHM) == false)
                {
                    // meassage box
                }
            }

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            break;

        default:
            assert(false);
            break;
    }

    // additional delay for the calibrator model TRX-II
    // as its hardware is too slow and without delay don't have time to execute commands
    //
    // QThread::msleep(5000);

    // start measure function
    //
    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            measureLinearity();         break;
        case MEASURE_TYPE_COMPARATOR:           measureComprators();        break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   measureComplexComprators(); break;
        default:                                assert(false);              break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureLinearity()
{
    QTime responseTime;
    int channelCount = theOptions.getChannelCount();

    int pointCount = theOptions.getLinearity().m_pointBase.count();
    if (pointCount == 0)
    {
//        QMessageBox msg;
//        msg.setText(tr("Don't set points for measure"));
//        msg.exec();

        return;
    }

    for(int p = 0; p < pointCount; p++)
    {
        if (m_cmdStopMeasure == true)
        {
            break;
        }

        LinearityPoint* pPoint = theOptions.getLinearity().m_pointBase.at(p);
        if (pPoint == nullptr)
        {
            continue;
        }

        emit measureInfo(tr("Set point %1 / %2 ").arg(p + 1).arg(pointCount));

        // set electric value
        //
        for (int c = 0; c < channelCount; c++)
        {
            CalibratorManager* manager = theCalibratorBase.getCalibratorManager(c);
            if (manager == nullptr)
            {
                continue;
            }

            if (manager->calibratorIsConnected() == false)
            {
                continue;
            }

            manager->setValue( pPoint->getSensorValue(POINT_SENSOR_I_0_5_MA) );
        }

        // wait ready all calibrators
        //
        responseTime.start();

        for (int c = 0; c < channelCount; c++)
        {
            CalibratorManager* manager = theCalibratorBase.getCalibratorManager(c);
            if (manager == nullptr)
            {
                continue;
            }

            while(manager->isReady() != true)
            {
                if (m_cmdStopMeasure == true)
                {
                    break;
                }
            }
        }

        qDebug("Function: %s, time waiting ready: %d ms", __FUNCTION__, responseTime.elapsed());

        // wait measure timeout
        //
        waitMeasureTimeout();

        emit measureInfo(tr("Save measurement "));

        emit measureComplite();
    }
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

