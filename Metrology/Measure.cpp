#include "Measure.h"

#include "Options.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Measurement::Measurement(const int measureType)
{
    m_measureType = measureType;
}

// -------------------------------------------------------------------------------------------------------------------

Measurement::~Measurement()
{
}

// -------------------------------------------------------------------------------------------------------------------

Measurement* Measurement::at(const int index)
{
    Measurement* pMeasurement = nullptr;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            pMeasurement = static_cast<LinearityMeasurement*> (this) + index;           break;
        case MEASURE_TYPE_COMPARATOR:           pMeasurement = static_cast<ComparatorMeasurement*> (this) + index;          break;
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

        for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
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

LinearityMeasurement::LinearityMeasurement(const Calibrator* pCalibrator, const MeasureSignalParam& param)
{
    if (pCalibrator == nullptr)
    {
        assert(false);
        return;
    }

    if (param.isValid() == false)
    {
        assert(false);
        return;
    }

    setMeasureType(MEASURE_TYPE_LINEARITY);
    setSignalHash(param.hash());

    // features
    //

    setAppSignalID( param.appSignalID() );
    setCustomAppSignalID( param.customAppSignalID() );
    setCaption( param.caption() );

    setPosition( param.position() );

    setValuePrecision(VALUE_TYPE_ELECTRIC, param.inputElectricPrecise());
    setValuePrecision(VALUE_TYPE_PHYSICAL, param.inputPhysicalPrecise());
    setValuePrecision(VALUE_TYPE_OUTPUT, param.outputElectricPrecise());

    // nominal
    //

    double electric =   pCalibrator->sourceValue();
    double physical =   conversion(electric, CT_ELECTRIC_TO_PHYSICAL, param);

    setNominal(VALUE_TYPE_ELECTRIC, electric);
    setNominal(VALUE_TYPE_PHYSICAL, physical);

    if (param.hasOutput() == true)
    {
        double outputEl =   (physical - param.inputPhysicalLowLimit()) * (param.outputElectricHighLimit() - param.outputElectricLowLimit())/
                            (param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit())+ param.outputElectricLowLimit();

        setNominal(VALUE_TYPE_OUTPUT, outputEl);
    }

    setPercent( (( physical - param.inputPhysicalLowLimit()) * 100)/(param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit() ));

    // measure
    //

    int measureCount = theOptions.linearity().m_measureCountInPoint;

    if (measureCount > MAX_MEASUREMENT_IN_POINT)
    {
        measureCount = MAX_MEASUREMENT_IN_POINT;
    }

    setMeasureArrayCount(measureCount);

    double averageElVal = 0;
    double averagePhVal = 0;

    for(int index = 0; index < measureCount; index++)
    {
        AppSignalState signalState = theSignalBase.signalState(param.hash());

        double elVal = conversion(signalState.value, CT_PHYSICAL_TO_ELECTRIC, param);
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
    setLowLimit(VALUE_TYPE_ELECTRIC, param.inputElectricLowLimit());
    setHighLimit(VALUE_TYPE_ELECTRIC, param.inputElectricHighLimit());
    setUnit(VALUE_TYPE_ELECTRIC, theUnitBase.unit( param.inputElectricUnitID() ) );

    setLowLimit(VALUE_TYPE_PHYSICAL, param.inputPhysicalLowLimit());
    setHighLimit(VALUE_TYPE_PHYSICAL, param.inputPhysicalHighLimit());
    setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit( param.inputPhysicalUnitID() ) );

    if (param.hasOutput() == true)
    {
        setLowLimit(VALUE_TYPE_OUTPUT, param.outputElectricLowLimit());
        setHighLimit(VALUE_TYPE_OUTPUT, param.outputElectricHighLimit());
        setUnit(VALUE_TYPE_OUTPUT, theUnitBase.unit( param.outputElectricUnitID() ) );
    }

    setHasOutput( param.hasOutput() );
    setAdjustment( param.adjustment() );

    // calc errors
    //

    setErrorInput(ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL)) );
    setErrorInput(ERROR_TYPE_REDUCE, abs( ((averagePhVal - nominal(VALUE_TYPE_PHYSICAL)) / (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL))) * 100.0) );
    setErrorInput(ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL) ) / nominal(VALUE_TYPE_PHYSICAL)) * 100.0) );

    setErrorLimit(ERROR_TYPE_ABSOLUTE, abs( (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL)) * theOptions.linearity().m_errorValue / 100.0 ));
    setErrorLimit(ERROR_TYPE_REDUCE, theOptions.linearity().m_errorValue);
    setErrorLimit(ERROR_TYPE_RELATIVE, theOptions.linearity().m_errorValue);

    if (param.hasOutput() == true)
    {
        setErrorOutput(ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_OUTPUT)- measure(VALUE_TYPE_OUTPUT)) );
        setErrorOutput(ERROR_TYPE_REDUCE, abs( ((averagePhVal - nominal(VALUE_TYPE_OUTPUT)) / (highLimit(VALUE_TYPE_OUTPUT) - lowLimit(VALUE_TYPE_OUTPUT))) * 100.0) );
        setErrorOutput(ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_OUTPUT)- measure(VALUE_TYPE_OUTPUT) ) / nominal(VALUE_TYPE_OUTPUT)) * 100.0) );
    }

    setErrorPrecision(ERROR_TYPE_ABSOLUTE, param.inputPhysicalPrecise() );
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
        // or GOST 8.207-76 application 2 (last page)
        //
    double k_student = studentK(measureCount, CT_PROPABILITY_95);

        // according to GOST 8.207-76 paragraph 3.2
        //
    double border = k_student * estimateSCO;

    setAdditionalValue(ADDITIONAL_VALUE_LOW_HIGH_BORDER, border);
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement::~LinearityMeasurement()
{
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::limitString(const int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    QString low = QString::number(m_lowLimit[type], 10, m_valuePrecision[type]);
    QString high = QString::number(m_highLimit[type], 10, m_valuePrecision[type]);

    return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[type]);;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::nominalString(const int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    return QString("%1 %2").arg(QString::number(m_nominal[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureString(const int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    return QString("%1 %2").arg(QString::number(m_measure[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureItemString(const int type, const int index) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT)
    {
        assert(0);
        return QString();
    }

    return QString::number(m_measureArray[type][index], 10, m_valuePrecision[type]);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::updateMeasureArray(const int valueType, Measurement* pMeasurement)
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

    for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
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

        for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
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
    setAppSignalID(QString());
    setCustomAppSignalID(QString());
    setCaption(QString());

    position().setCaseNo(0);
    position().setCaseCaption(QString());
    position().setChannel(0);
    position().setBlock(0);
    position().setSubblock(0);
    position().setEntry(0);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement::~ComparatorMeasurement()
{
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
