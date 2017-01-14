#include "MeasureThread.h"

#include <assert.h>

#include <QTime>

#include "Options.h"
#include "MeasureBase.h"
#include "Conversion.h"

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

    // set events on disconnect of calibrators
    // in case if in process of measuring, our calibrator will disconnect we will be known about it
    //
    int calibratorCount = theCalibratorBase.count();
    for(int c = 0; c < calibratorCount; c ++)
    {
        CalibratorManager* pManager = theCalibratorBase.at(c);
        if (pManager == nullptr)
        {
            continue;
        }

        Calibrator* pCalibrator = pManager->calibrator();
        if (pCalibrator == nullptr)
        {
            continue;
        }

        connect(pCalibrator, &Calibrator::disconnected, this, &MeasureThread::calibratorDisconnected);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::calibratorDisconnected()
{
    if (m_cmdStopMeasure == true)
    {
        return;
    }

    // calibrator has been disconnected
    // if no connected calibrators then stop measuring
    //

    int calibratorDisconnectedCount = theCalibratorBase.connectedCalibratorsCount();

    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            if ( calibratorDisconnectedCount == 0)
            {
                m_cmdStopMeasure = true;
            }

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            if ( calibratorDisconnectedCount < CALIBRATOR_COUNT_FOR_CC )
            {
                m_cmdStopMeasure = true;
            }

            break;

        default:
            assert(0);
            break;
    }

    // tell about problem
    //
    Calibrator* pDisconnectedCalibrator = dynamic_cast<Calibrator*> (sender());
    if (pDisconnectedCalibrator != nullptr)
    {
        emit showMsgBox(tr("Calibrator: %1 - disconnected.").arg(pDisconnectedCalibrator->portName()));
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
    // Timeout for Measure
    //
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

bool MeasureThread::calibratorIsValid(CalibratorManager* pManager)
{
    if (pManager == nullptr)
    {
        return false;
    }

    if (pManager->calibratorIsConnected() == false)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::prepareCalibrator(CalibratorManager* pManager, const int& calibratorMode, const E::InputUnit& signalInputUnit, const double& highInputLimit)
{
    if (calibratorIsValid(pManager) == false)
    {
        return false;
    }

    if (calibratorMode < 0 || calibratorMode >= CALIBRATOR_MODE_COUNT)
    {
        return false;
    }

    if (signalInputUnit < 0 || signalInputUnit >= INPUT_UNIT_COUNT)
    {
        return false;
    }

    int calibratorUnit = CALIBRATOR_UNIT_UNKNOWN;

    switch(signalInputUnit)
    {
        case E::InputUnit::mA:  calibratorUnit = CALIBRATOR_UNIT_MA;    break;
        case E::InputUnit::mV:  calibratorUnit = CALIBRATOR_UNIT_MV;    break;
        case E::InputUnit::V:   calibratorUnit = CALIBRATOR_UNIT_V;     break;
        case E::InputUnit::Ohm:
            {
                // Minimal range for calibrators TRX-II and Calys75 this is 400 Ohm
                //
                if (highInputLimit <= 400)
                {
                    calibratorUnit = CALIBRATOR_UNIT_LOW_OHM;
                }
                else
                {
                    calibratorUnit = CALIBRATOR_UNIT_HIGH_OHM;
                }
            }

            break;


        default: assert(0);
    }

    if (calibratorUnit < 0 || calibratorUnit >= CALIBRATOR_UNIT_COUNT)
    {
        return false;
    }

    emit measureInfo(QString("Prepare calibrator: %1 ").arg(pManager->portName()));

    return pManager->setUnit(calibratorMode, calibratorUnit);;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    int calibratorCount = theCalibratorBase.count();
    if (calibratorCount == 0)
    {
        emit showMsgBox(QString("Proccess of measure can not start, because no connected calibrators!\nPlease, make initialization calibrators"));
        return;
    }

    m_activeSignal = theSignalBase.activeSignal();
    if (m_activeSignal.isEmpty() == true)
    {
        return;
    }

    m_cmdStopMeasure = false;

    // select calibrator mode and calibrator unit
    // depend from analog signal
    //
    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            // if m_measureKind == MEASURE_KIND_ONE (i.e measure in the one channel) then take first connected calibrator
            // if m_measureKind == MEASURE_KIND_MULTI (i.e measure all channels) then attempt to take all calibrators
            //
            for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i ++)
            {
                Hash signalHash = m_activeSignal.hash(i);
                if (signalHash == 0)
                {
                    continue;
                }

                Signal param = theSignalBase.signalParam(signalHash);
                if (param.appSignalID().isEmpty() == true || param.hash() == 0)
                {
                    continue;
                }

                CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);
                if (calibratorIsValid(pManager) == false)
                {
                    continue;
                }

                if (prepareCalibrator(pManager, CALIBRATOR_MODE_SOURCE, param.inputUnitID(), param.inputHighLimit() ) == false)
                {
                    emit showMsgBox(QString("Calibrator: %1 - can not set source mode.").arg(pManager->portName()));
                    continue;
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

            break;

        default:
            assert(false);
    }

    // start measure function
    //
    switch (m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
            measureLinearity();
            break;

        case MEASURE_TYPE_COMPARATOR:
            measureComprators();
            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:
            measureComplexComprators();
            break;

        default:
            assert(0);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureLinearity()
{
    int calibratorCount = theCalibratorBase.connectedCalibratorsCount();
    if (calibratorCount == 0)
    {
        emit showMsgBox("Proccess of measure can not start, because no connected calibrators!\nPlease, make initialization calibrators");
        return;
    }

    int pointCount = theOptions.linearity().m_pointBase.count();
    if (pointCount == 0)
    {
        emit showMsgBox("No points for measure");
        return;
    }

    MeasureMultiSignal& multiSignal = theSignalBase.activeSignal();
    if (multiSignal.isEmpty() == true)
    {
        return;
    }

    for(int p = 0; p < pointCount; p++)
    {
        if (m_cmdStopMeasure == true)
        {
            break;
        }

        LinearityPoint point = theOptions.linearity().m_pointBase.at(p);

        emit measureInfo(tr("Set point %1 / %2 ").arg(p + 1).arg(pointCount));

        // set electric value on calibrators
        //

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i ++)
        {
            Hash signalHash = m_activeSignal.hash(i);
            if (signalHash == 0)
            {
                continue;
            }

            Signal param = theSignalBase.signalParam(signalHash);
            if (param.appSignalID().isEmpty() == true || param.hash() == 0)
            {
                continue;
            }

            CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);
            if (calibratorIsValid(pManager) == false)
            {
                continue;
            }


            double physicalVal = (point.percent() * (param.highEngeneeringUnits() - param.lowEngeneeringUnits()) / 100) + param.lowEngeneeringUnits();
            double electricVal = conversion(physicalVal, CT_PHYSICAL_TO_ELECTRIC, param);

            pManager->setValue( electricVal );
        }

        // wait ready all calibrators,
        // wait until all calibrators will has fixed electric value
        //

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i ++)
        {
            CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);

            if (calibratorIsValid(pManager) == false)
            {
                continue;
            }

            while(pManager->isReady() != true)
            {
                if (m_cmdStopMeasure == true)
                {
                    break;
                }
            }
        }

        // wait timeout for measure
        //

        emit measureInfo(tr("Wait timeout %1 / %2 ").arg(p + 1).arg(pointCount));

        waitMeasureTimeout();

        // save measurement
        //

        emit measureInfo(tr("Save measurements "));

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i ++)
        {
            Hash signalHash = m_activeSignal.hash(i);
            if (signalHash == 0)
            {
                continue;
            }

            CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);
            if (calibratorIsValid(pManager) == false)
            {
                continue;
            }

            LinearetyMeasureItem* pMeasure = new LinearetyMeasureItem(pManager->calibrator(), signalHash);
            if (pMeasure == nullptr )
            {
                continue;
            }

            emit measureComplite( pMeasure );
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComprators()
{
    int calibratorCount = theCalibratorBase.connectedCalibratorsCount();
    if (calibratorCount == 0)
    {
        emit showMsgBox("Proccess of measure can not start, because no connected calibrators!\nPlease, make initialization calibrators");
        m_cmdStopMeasure = true;
        return;
    }

    emit measureInfo(tr("Comprators "));

    waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComplexComprators()
{
    int calibratorCount = theCalibratorBase.connectedCalibratorsCount();
    if (calibratorCount < CALIBRATOR_COUNT_FOR_CC)
    {
        emit showMsgBox("Proccess of measure can not start, because no connected calibrators!\nPlease, make initialization calibrators");
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
}

// -------------------------------------------------------------------------------------------------------------------
