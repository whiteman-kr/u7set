#include "MeasureThread.h"

#include <assert.h>

#include <QTime>
#include <QMessageBox>
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QWidget *parent) :
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

        emit measureInfo(MEASURE_THREAD_TIMEOUT_STEP + t);
    }

    emit measureInfo(0);
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

    // Prepare calibrators
    //
    m_calibratorManagerList.clear();

    int calibratorCount = theCalibratorBase.count();

    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:
            {

                // create list of calibrators for measure
                //
                switch(theOptions.getToolBar().m_measureKind)
                {
                    case MEASURE_KIND_ONE:

                        // append first connected calibrator
                        //
                        for(int c = 0; c < calibratorCount; c ++)
                        {
                            CalibratorManager* manager = theCalibratorBase.at(c);
                            if (manager == nullptr)
                            {
                                continue;
                            }

                            if (manager->calibratorIsConnected() == false)
                            {
                                continue;
                            }

                            m_calibratorManagerList.append(manager);

                            break;
                        }

                        break;

                    case MEASURE_KIND_MULTI:

                        // append all calibrators
                        //
                        for(int c = 0; c < calibratorCount; c ++)
                        {
                            CalibratorManager* manager = theCalibratorBase.at(c);
                            if (manager == nullptr)
                            {
                                continue;
                            }

                            m_calibratorManagerList.append(manager);
                        }

                        break;

                    default:
                        assert(0);
                        break;
                }

                // select mode and unit
                //
                int count = m_calibratorManagerList.count();
                for (int c = 0; c < count; c++)
                {
                    CalibratorManager* manager = m_calibratorManagerList.at(c);
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
                        //emit showMsgBox(QString("Calibrator %d can not set measure mode").arg(manager->geIndex() + 1));
                    }

                    if (prepareCalibrator(manager, CALIBRATOR_MODE_SOURCE, CALIBRATOR_UNIT_LOW_OHM) == false)
                    {
                        //emit showMsgBox(QString("Calibrator %d can not set source mode").arg(manager->geIndex() + 1));
                    }
                }
            }

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:
            {
                // for verification the complex comparators need two calibrator
                //
                if (theCalibratorBase.getConnectedCalibratorsCount() < CALIBRATOR_COUNT_FOR_CC)
                {
                    break;
                }

                // search two connected calibrators
                //
                int calibratorCount = theCalibratorBase.count();
                for(int c = 0; c < calibratorCount; c ++)
                {
                    CalibratorManager* manager = theCalibratorBase.at(c);
                    if (manager == nullptr)
                    {
                        continue;
                    }

                    if (manager->calibratorIsConnected() == false)
                    {
                        continue;
                    }

                    m_calibratorManagerList.append(manager);

                    if (m_calibratorManagerList.count() == CALIBRATOR_COUNT_FOR_CC)
                    {
                        break;
                    }
                }
            }
            break;

        default:
            assert(false);
            break;
    }

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
    int pointCount = theOptions.getLinearity().m_pointBase.count();
    if (pointCount == 0)
    {
        //emit showMsgBox("The application don't have points of measured");
        return;
    }

    int calibratorCount = m_calibratorManagerList.count();
    if (calibratorCount == 0)
    {
        //emit showMsgBox("The application don't have connected calibrators");
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

        for (int c = 0; c < calibratorCount; c++)
        {
            CalibratorManager* manager = m_calibratorManagerList.at(c);
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
        QTime responseTime;
        responseTime.start();

        for (int c = 0; c < calibratorCount; c++)
        {
            CalibratorManager* manager = m_calibratorManagerList.at(c);
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
        emit measureInfo(tr("Wait timeout %1 / %2 ").arg(p + 1).arg(pointCount));
        waitMeasureTimeout();

        // save measurement
        //
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
