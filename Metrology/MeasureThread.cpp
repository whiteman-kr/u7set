#include "MeasureThread.h"

#include <assert.h>

#include <QTime>

#include "Options.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QObject *parent) :
    QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::~MeasureThread()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::init(QWidget* parent)
{
    m_parent = parent;

    connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &MeasureThread::updateSignalParam, Qt::QueuedConnection);

    connect(this, &MeasureThread::showMsgBox, this, &MeasureThread::msgBox);
    connect(this, &MeasureThread::finished, this, &MeasureThread::stopMeasure);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
    // Timeout for Measure
    //
    for(int t = 0; t < theOptions.toolBar().measureTimeout(); t += MEASURE_THREAD_TIMEOUT_STEP )
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

bool MeasureThread::prepareCalibrator(CalibratorManager* pManager, const int calibratorMode, const E::InputUnit signalInputUnit, const double inputElectricHighLimit)
{
    if (calibratorIsValid(pManager) == false)
    {
        return false;
    }

    if (calibratorMode < 0 || calibratorMode >= CALIBRATOR_MODE_COUNT)
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
                if (inputElectricHighLimit <= 400)
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

    // set command for exit (stop measure) in state = FALSE
    //
    m_cmdStopMeasure = false;

    // create param list
    //
    for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
    {
        Hash signalHash = theSignalBase.activeSignal().hash(i);
        if (signalHash == 0)
        {
            continue;
        }

        m_activeSignalParam[i] = theSignalBase.signalParam(signalHash);
    }

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
            for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
            {
                MeasureSignalParam param = m_activeSignalParam[i];
                if (param.isValid() == false)
                {
                    continue;
                }

                CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);
                if (calibratorIsValid(pManager) == false)
                {
                    continue;
                }

                if (prepareCalibrator(pManager, CALIBRATOR_MODE_SOURCE, param.inputElectricUnitID(), param.inputElectricHighLimit() ) == false)
                {
                    emit showMsgBox(QString("Calibrator: %1 - can not set source mode.").arg(pManager->portName()));
                    continue;
                }
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

        for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
        {
            MeasureSignalParam param = m_activeSignalParam[i];
            if (param.isValid() == false)
            {
                continue;
            }

            CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);
            if (calibratorIsValid(pManager) == false)
            {
                continue;
            }


            double physicalVal = (point.percent() * (param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) / 100) + param.inputPhysicalLowLimit();
            double electricVal = conversion(physicalVal, CT_PHYSICAL_TO_ELECTRIC, param);

            pManager->setValue( electricVal );
        }

        // wait ready all calibrators,
        // wait until all calibrators will has fixed electric value
        //

        for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
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

        for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
        {
            MeasureSignalParam param = m_activeSignalParam[i];
            if (param.isValid() == false)
            {
                continue;
            }

            CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(i);
            if (calibratorIsValid(pManager) == false)
            {
                continue;
            }

            LinearityMeasurement* pMeasurement = new LinearityMeasurement(pManager->calibrator(), param);
            if (pMeasurement == nullptr )
            {
                continue;
            }

            emit measureComplite( pMeasurement );
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

void MeasureThread::updateSignalParam(const Hash& signalHash)
{
    if (signalHash == 0)
    {
        assert(signalHash != 0);
        return;
    }

    for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
    {
        if (m_activeSignalParam[i].hash() == signalHash)
        {
            m_activeSignalParam[i] = theSignalBase.signalParam(signalHash);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::stopMeasure()
{
    setMeasureType(MEASURE_TYPE_UNKNOWN);
    m_cmdStopMeasure = true;
}

// -------------------------------------------------------------------------------------------------------------------
