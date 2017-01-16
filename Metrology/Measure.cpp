#include "Measure.h"

#include "Options.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Measurement::Measurement(int type)
{
    m_measureType = type;
}

// -------------------------------------------------------------------------------------------------------------------

Measurement* Measurement::at(int index)
{
    Measurement* pMeasurement = nullptr;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            pMeasurement = static_cast<LinearityMeasurement*> (this) + index;           break;
        case MEASURE_TYPE_COMPARATOR:           pMeasurement = static_cast<ComparatorMeasurement*> (this) + index;          break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   pMeasurement = static_cast<ComplexComparatorMeasurement*> (this) + index;   break;
        default:                                assert(0);
    }

    return pMeasurement;
}

// -------------------------------------------------------------------------------------------------------------------

Measurement& Measurement::operator=(Measurement& from)
{
    m_measureType = from.m_measureType;
    m_signalHash = from.m_signalHash;

    m_measureID = from.m_measureID;
    m_filter = from.m_filter;

    m_measureTime = from.m_measureTime;
    m_reportType = from.m_reportType;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            *static_cast<LinearityMeasurement*> (this) = *static_cast <LinearityMeasurement*> (&from);                  break;
        case MEASURE_TYPE_COMPARATOR:           *static_cast<ComparatorMeasurement*> (this) = *static_cast <ComparatorMeasurement*> (&from);                break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   *static_cast<ComplexComparatorMeasurement*> (this) = *static_cast <ComplexComparatorMeasurement*> (&from);  break;
        default:                                assert(0);
    }

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement::LinearityMeasurement() :
    Measurement(MEASURE_TYPE_LINEARITY)
{
    for(int v = 0; v < VALUE_TYPE_COUNT; v++)
    {
        m_nominal[v] = 0;
        m_measure[v] = 0;

        for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
        {
            m_measureArray[v][m] = 0;
        }

        m_lowLimit[v] = 0;
        m_highLimit[v] = 0;

        m_valuePrecision[v] = 0;
    }

    for(int e = 0; e < ERROR_TYPE_COUNT; e++)
    {
        m_errorInput[e] = 0;
        m_errorOutput[e] = 0;
        m_errorLimit[e] = 0;

        m_errorPrecision[e] = 0;
    }

    for(int a = 0; a < ADDITIONAL_VALUE_COUNT; a++)
    {
        m_additionalValue[a] = 0;
    }
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement::LinearityMeasurement(Calibrator* pCalibrator, const Hash& signalHash)
{
    if (pCalibrator == nullptr)
    {
        assert(false);
        return;
    }

    if (signalHash == 0)
    {
        assert(false);
        return;
    }

    MeasureSignal signal = theSignalBase.signal(signalHash);
    if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
    {
        assert(false);
        return;
    }

    setMeasureType(MEASURE_TYPE_LINEARITY);

    setSignalHash(signalHash);

    // features
    //

    setAppSignalID( signal.param().appSignalID() );
    setCustomAppSignalID( signal.param().customAppSignalID() );
    setCaption( signal.param().caption() );

    setPosition( signal.position() );

    setValuePrecision(VALUE_TYPE_ELECTRIC, 3);
    setValuePrecision(VALUE_TYPE_PHYSICAL, signal.param().decimalPlaces());
    setValuePrecision(VALUE_TYPE_OUTPUT, 3);

    // nominal
    //

    double electric = pCalibrator->sourceValue();
    double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, signal.param());

    setNominal(VALUE_TYPE_ELECTRIC, electric);
    setNominal(VALUE_TYPE_PHYSICAL, physical);

    if (signal.param().isOutput() == true)
    {
        setNominal(VALUE_TYPE_OUTPUT, 0);
    }

    setPercent( (( physical - signal.param().lowEngeneeringUnits()) * 100)/(signal.param().highEngeneeringUnits() - signal.param().lowEngeneeringUnits() ));

    // measure
    //

    int measureCount = theOptions.linearity().m_measureCountInPoint > MEASUREMENT_IN_POINT ? MEASUREMENT_IN_POINT : theOptions.linearity().m_measureCountInPoint;

    setMeasureArrayCount(measureCount);

    double averageElVal = 0;
    double averagePhVal = 0;

    for(int index = 0; index < measureCount; index++)
    {
        AppSignalState signalState = theSignalBase.signalState(signalHash);

        double elVal = conversion(signalState.value, CT_PHYSICAL_TO_ELECTRIC, signal.param());
        double phVal = signalState.value;

        setMeasureItemArray(VALUE_TYPE_ELECTRIC, index, elVal);
        setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
        setMeasureItemArray(VALUE_TYPE_OUTPUT, index, 0);

        averageElVal += elVal;
        averagePhVal += phVal;

        QThread::msleep((theOptions.linearity().m_measureTimeInPoint*1000)/measureCount);
    }

    averageElVal /= measureCount;
    averagePhVal /= measureCount;

    setMeasure(VALUE_TYPE_ELECTRIC, averageElVal);
    setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
    setMeasure(VALUE_TYPE_OUTPUT, pCalibrator->measureValue());

    // limits
    //
    setLowLimit(VALUE_TYPE_ELECTRIC, signal.param().inputLowLimit());
    setHighLimit(VALUE_TYPE_ELECTRIC, signal.param().inputHighLimit());

    if ( signal.param().inputUnitID() >= 0 && signal.param().inputUnitID() < INPUT_UNIT_COUNT)
    {
        setUnit(VALUE_TYPE_ELECTRIC, InputUnitStr[ signal.param().inputUnitID() ] );
    }

    setLowLimit(VALUE_TYPE_PHYSICAL, signal.param().lowEngeneeringUnits());
    setHighLimit(VALUE_TYPE_PHYSICAL, signal.param().highADC());
    setUnit(VALUE_TYPE_PHYSICAL, theSignalBase.unit( signal.param().unitID() ) );

    if (signal.param().isOutput() == true)
    {
        switch(signal.param().outputMode())
        {
            case E::OutputMode::Plus0_Plus5_V:     setLowLimit(VALUE_TYPE_OUTPUT, 0);      setHighLimit(VALUE_TYPE_OUTPUT, 5);     setUnit(VALUE_TYPE_OUTPUT, "V");     break;
            case E::OutputMode::Plus4_Plus20_mA:   setLowLimit(VALUE_TYPE_OUTPUT, 4);      setHighLimit(VALUE_TYPE_OUTPUT, 20);    setUnit(VALUE_TYPE_OUTPUT, "mA");    break;
            case E::OutputMode::Minus10_Plus10_V:  setLowLimit(VALUE_TYPE_OUTPUT, -10);    setHighLimit(VALUE_TYPE_OUTPUT, 10);    setUnit(VALUE_TYPE_OUTPUT, "V");     break;
            case E::OutputMode::Plus0_Plus5_mA:    setLowLimit(VALUE_TYPE_OUTPUT, 0);      setHighLimit(VALUE_TYPE_OUTPUT, 5);     setUnit(VALUE_TYPE_OUTPUT, "mA");    break;
        }
    }

    setHasOutput(false);
    setAdjustment(0);

    // calc errors
    //

    setErrorInput(ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL)) );
    setErrorInput(ERROR_TYPE_REDUCE, abs( ((averagePhVal - nominal(VALUE_TYPE_PHYSICAL)) / (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL))) * 100.0) );
    setErrorInput(ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL) ) / nominal(VALUE_TYPE_PHYSICAL)) * 100.0) );

    setErrorLimit(ERROR_TYPE_ABSOLUTE, abs( (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL)) * theOptions.linearity().m_errorValue / 100.0 ));
    setErrorLimit(ERROR_TYPE_REDUCE, theOptions.linearity().m_errorValue);
    setErrorLimit(ERROR_TYPE_RELATIVE, theOptions.linearity().m_errorValue);

    if (signal.param().isOutput() == true)
    {
        setErrorOutput(ERROR_TYPE_ABSOLUTE, 0);
        setErrorOutput(ERROR_TYPE_REDUCE, 0);
        setErrorOutput(ERROR_TYPE_RELATIVE, 0);
    }

    setErrorPrecision(ERROR_TYPE_ABSOLUTE, signal.param().decimalPlaces());
    setErrorPrecision(ERROR_TYPE_REDUCE, 2);
    setErrorPrecision(ERROR_TYPE_RELATIVE, 2);

    // calc additional parameters
    //

    double maxDeviation = 0;
    int maxDeviationIndex = 0;

    for(int index = 0; index < measureCount; index++)
    {
        if ( maxDeviation < abs(averagePhVal - measureItemArray(VALUE_TYPE_PHYSICAL, index)))
        {
            maxDeviation = abs(averagePhVal - measureItemArray(VALUE_TYPE_PHYSICAL, index));
            maxDeviationIndex = index;
        }
    }

    setAdditionalValue(ADDITIONAL_VALUE_MEASURE_MAX,  measureItemArray(VALUE_TYPE_PHYSICAL, maxDeviationIndex));

        // according to GOST 8.508-84 paragraph 3.4.1 formula 42
        //
    double systemError = abs (averagePhVal - nominal(VALUE_TYPE_PHYSICAL));

    setAdditionalValue(ADDITIONAL_VALUE_SYSTEM_ERROR, systemError);

        // according to GOST 8.736-2011 paragraph 5.3 formula 3
        //
    double sumDeviation = 0;

    for(int index = 0; index < measureCount; index++)
    {
        sumDeviation += pow( averagePhVal -  measureItemArray(VALUE_TYPE_PHYSICAL, index), 2 );	// 1. sum of deviations
    }

    sumDeviation /= (double) measureCount - 1;                                                  // 2. divide on (count of measure - 1)
    double sco = sqrt(sumDeviation);                                                            // 3. sqrt

    setAdditionalValue(ADDITIONAL_VALUE_MSE, sco);

        // according to GOST 8.207-76 paragraph 2.4
        //
    double estimateSCO = sco / sqrt( (double) measureCount );

        // Student's rate according to GOST 27.202 on P = 0.95
        //
    double k_student = 0;

    if ( measureCount >= 0 && measureCount < K_STUDENT_COUNT )
    {
        k_student = K_STUDENT[measureCount];
    }

        // according to GOST 8.207-76 paragraph 3.2
        //
    double border = k_student * estimateSCO;

    setAdditionalValue(ADDITIONAL_VALUE_LOW_HIGH_BORDER, border);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::limitString(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    QString low = QString::number(m_lowLimit[type], 10, m_valuePrecision[type]);
    QString high = QString::number(m_highLimit[type], 10, m_valuePrecision[type]);

    return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[type]);;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::nominalString(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    return QString("%1 %2").arg(QString::number(m_nominal[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureString(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    return QString("%1 %2").arg(QString::number(m_measure[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureItemString(int type, int index) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    if (index < 0 || index >= MEASUREMENT_IN_POINT)
    {
        assert(0);
        return "";
    }

    return QString::number(m_measureArray[type][index], 10, m_valuePrecision[type]);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::updateMeasureArray(int valueType, Measurement* pMeasurement)
{
    if (pMeasurement == nullptr)
    {
        return;
    }

    int measureType = pMeasurement->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    LinearityMeasurement* pLinearityMeasureItem = static_cast <LinearityMeasurement*> (pMeasurement);

    m_measureArrayCount = pLinearityMeasureItem->measureArrayCount();

    for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
    {
        m_measureArray[valueType][m] = pLinearityMeasureItem->measureItemArray(valueType, m);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::updateAdditionalValue(Measurement* pMeasurement)
{
    if (pMeasurement == nullptr)
    {
        return;
    }

    int measureType = pMeasurement->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    LinearityMeasurement* pLinearityMeasureItem = static_cast <LinearityMeasurement*> (pMeasurement);

    for(int a = 0; a < ADDITIONAL_VALUE_COUNT; a++)
    {
        m_additionalValue[a] = pLinearityMeasureItem->additionalValue(a);
    }
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement& LinearityMeasurement::operator=(const LinearityMeasurement& from)
{
    m_appSignalID = from.m_appSignalID;
    m_customAppSignalID = from.m_customAppSignalID;
    m_caption = from.m_caption;

    m_position = from.m_position;

    for(int v = 0; v < VALUE_TYPE_COUNT; v++)
    {
        m_nominal[v] = from.m_nominal[v];
        m_measure[v] = from.m_measure[v];

        for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
        {
            m_measureArray[v][m] = from.m_measureArray[v][m];
        }

        m_lowLimit[v] = from.m_lowLimit[v];;
        m_highLimit[v] = from.m_highLimit[v];;
        m_unit[v] = from.m_unit[v];;

        m_valuePrecision[v] = from.m_valuePrecision[v];
    }

    m_percent = from.m_percent;
    m_measureArrayCount = from.m_measureArrayCount;

    m_hasOutput = from.m_hasOutput;
    m_adjustment = from.m_adjustment;

    for(int e = 0; e < ERROR_TYPE_COUNT; e++)
    {
        m_errorInput[e] = from.m_errorInput[e];;
        m_errorOutput[e] = from.m_errorOutput[e];;
        m_errorLimit[e] = from.m_errorLimit[e];;

        m_errorPrecision[e] = from.m_errorPrecision[e];;
    }

    m_additionalValueCount = from.m_additionalValueCount;

    for(int a = 0; a < ADDITIONAL_VALUE_COUNT; a++)
    {
        m_additionalValue[a] = from.m_additionalValue[a];;
    }

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement::ComparatorMeasurement() :
    Measurement(MEASURE_TYPE_COMPARATOR)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement::ComparatorMeasurement(Calibrator* pCalibrator)
{
    if (pCalibrator == nullptr)
    {
        return;
    }

    setMeasureType(MEASURE_TYPE_COMPARATOR);

    // features
    //
    setStrID("#IDMPS");
    setExtStrID("IDMPS");
    setName("This is signal of the block MPS");

    position().setCaseNo(0);
    position().setCaseCaption("CASE-1");
    position().setChannel(0);
    position().setBlock(0);
    position().setSubblock(0);
    position().setEntry(0);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasurement::updateHysteresis(Measurement* pMeasurement)
{
    if (pMeasurement == nullptr)
    {
        return;
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


ComplexComparatorMeasurement::ComplexComparatorMeasurement() :
    Measurement(MEASURE_TYPE_COMPLEX_COMPARATOR)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComplexComparatorMeasurement::ComplexComparatorMeasurement(Calibrator* pMainCalibrator, Calibrator* pSubCalibrator)
{
    if (pMainCalibrator == nullptr || pSubCalibrator == nullptr)
    {
        return;
    }

    setMeasureType(MEASURE_TYPE_COMPLEX_COMPARATOR);

    // features
    //
    setStrID("#IDMPS");
    setExtStrID("IDMPS");
    setName("This is signal of the block MPS");

    position().setCaseNo(0);
    position().setCaseCaption("CASE-1");
    position().setChannel(0);
    position().setBlock(0);
    position().setSubblock(0);
    position().setEntry(0);
}

// -------------------------------------------------------------------------------------------------------------------

void ComplexComparatorMeasurement::updateHysteresis(Measurement* pMeasurement)
{
    if (pMeasurement == nullptr)
    {
        return;
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

