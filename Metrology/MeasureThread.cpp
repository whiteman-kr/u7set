#include "MeasureThread.h"

#include <assert.h>

#include <QTime>
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QObject *parent) :
    QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::init(QWidget* parent)
{
    m_parentWidget = parent;

    connect(this, &MeasureThread::showMsgBox, this, &MeasureThread::msgBox);
    connect(this, &MeasureThread::finished, this, &MeasureThread::finish);

    // connect calibrators
    //
    int calibratorCount = theCalibratorBase.count();
    for(int c = 0; c < calibratorCount; c ++)
    {
        CalibratorManager* manager = theCalibratorBase.at(c);
        if (manager == nullptr)
        {
            continue;
        }

        Calibrator* calibrator = manager->calibrator();
        if (calibrator == nullptr)
        {
            continue;
        }

        connect(calibrator, &Calibrator::disconnected, this, &MeasureThread::calibratorDisconnected);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
    for(int t = 0; t < theOptions.toolBar().m_measureTimeout; t += MEASURE_THREAD_TIMEOUT_STEP )
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

void MeasureThread::calibratorDisconnected()
{
    if (m_cmdStopMeasure == true)
    {
        return;
    }

    int calibratorDisconnectedCount = 0;

    int calibratorCount = m_calibratorManagerList.count();
    for(int c = 0; c < calibratorCount; c ++)
    {
        CalibratorManager* manager = m_calibratorManagerList.at(c);
        if (manager == nullptr)
        {
            continue;
        }

        if (manager->calibratorIsConnected() == false)
        {
            calibratorDisconnectedCount ++;
        }
    }

    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            if ( calibratorDisconnectedCount == calibratorCount)
            {
                m_cmdStopMeasure = true;
            }

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            if ( calibratorDisconnectedCount != 0 )
            {
                m_cmdStopMeasure = true;
            }

            break;

        default:
            assert(0);
            break;
    }

    Calibrator* disconnectedCalibrator = dynamic_cast<Calibrator*> (sender());
    if (disconnectedCalibrator != nullptr)
    {
        emit showMsgBox(tr("Calibrator: %1 - disconnected.").arg(disconnectedCalibrator->portName()));
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

    emit measureInfo(QString("Prepare calibrator: %1 ").arg(manager->portName()));

    return manager->setUnit(mode, unit);;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    m_cmdStopMeasure = false;

    m_calibratorManagerList.clear();

    int calibratorCount = theCalibratorBase.count();
    if (calibratorCount == 0)
    {
        emit showMsgBox(QString("Unable to start the measurement because it is not connected calibrators").arg(CALIBRATOR_COUNT_FOR_CC));
        return;
    }

    // prepare list calibrators for measusre
    //
    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            // create list of calibrators for measure
            // if m_measureKind == MEASURE_KIND_ONE (measure in one channel) then append first connected calibrator
            // if m_measureKind == MEASURE_KIND_MULTI (measure in all channels) then append all calibrators
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

                if (theOptions.toolBar().m_measureKind == MEASURE_KIND_ONE)
                {
                    break;
                }
            }

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            // for verification the complex comparators need two calibrator
            //
            if (theCalibratorBase.connectedCalibratorsCount() < CALIBRATOR_COUNT_FOR_CC)
            {
                emit showMsgBox(QString("For measure accuracy complex comparator the need for at least %1 calibrators").arg(CALIBRATOR_COUNT_FOR_CC));
                break;
            }

            // search two connected calibrators
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

                if (m_calibratorManagerList.count() == CALIBRATOR_COUNT_FOR_CC)
                {
                    break;
                }
            }

            break;

        default:
            assert(false);
            break;
    }

    // select calibrator mode and calibrator unit
    // depend from analog signal
    //
    calibratorCount = m_calibratorManagerList.count();
    for (int c = calibratorCount - 1; c >= 0; c--)
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
            emit showMsgBox(QString("Calibrator: %1 - can't set measure mode.\nThis is calibrator will be excluded from the measurement process.").arg(manager->portName()));
            m_calibratorManagerList.removeAt(c);
            continue;
        }

        if (prepareCalibrator(manager, CALIBRATOR_MODE_SOURCE, CALIBRATOR_UNIT_LOW_OHM) == false)
        {
            emit showMsgBox(QString("Calibrator: %1 - can't set source mode.\nThis is calibrator will be excluded from the measurement process.").arg(manager->portName()));
            m_calibratorManagerList.removeAt(c);
            continue;
        }
    }

    // start measure function
    //
    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            measureLinearity();         break;
        case MEASURE_TYPE_COMPARATOR:           measureComprators();        break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   measureComplexComprators(); break;
        default:                                assert(0);                  break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureLinearity()
{
    int calibratorCount = m_calibratorManagerList.count();
    if (calibratorCount == 0)
    {
        emit showMsgBox("The measurement process don't have calibrators for measure");
        return;
    }

    int pointCount = theOptions.linearity().m_pointBase.count();
    if (pointCount == 0)
    {
        emit showMsgBox("The measurement process don't have points of linearity");
        return;
    }

    for(int p = 0; p < pointCount; p++)
    {
        if (m_cmdStopMeasure == true)
        {
            break;
        }

        LinearityPoint point = theOptions.linearity().m_pointBase[p];

        emit measureInfo(tr("Set point %1 / %2 ").arg(p + 1).arg(pointCount));

        // set electric value on calibrators
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

            manager->setValue( point.sensorValue(POINT_SENSOR_I_0_5_MA) );
        }

        // wait ready all calibrators,
        // wait until all calibrators will has fixed electric value
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
        emit measureInfo(tr("Save measurements "));

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


            emit measureComplite( new LinearetyMeasureItem(manager->calibrator()) );
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComprators()
{
    int calibratorCount = m_calibratorManagerList.count();
    if (calibratorCount == 0)
    {
        emit showMsgBox(tr("The measurement process don't have calibrators for measure"));
        return;
    }

    emit measureInfo(tr("Comprators "));

    waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComplexComprators()
{
    int calibratorCount = m_calibratorManagerList.count();
    if (calibratorCount < CALIBRATOR_COUNT_FOR_CC)
    {
        emit showMsgBox(tr("The measurement process don't have enough calibrators for measure"));
        m_cmdStopMeasure = true;
        return;
    }

    emit measureInfo(tr("Complex Comprators "));

    waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::finish()
{
    setMeasureType(MEASURE_TYPE_UNKNOWN);
    m_cmdStopMeasure = true;

    m_calibratorManagerList.clear();
}

// -------------------------------------------------------------------------------------------------------------------
