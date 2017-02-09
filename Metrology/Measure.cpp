#include "Measure.h"

#include "Options.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Measurement::Measurement(int measureType) :
    m_measureType (measureType)
{
    clear();
}

// -------------------------------------------------------------------------------------------------------------------

Measurement::~Measurement()
{
}

// -------------------------------------------------------------------------------------------------------------------

Measurement* Measurement::at(int index)
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
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement::LinearityMeasurement(const MeasureParam &measureParam)
{
    clear();

    if (measureParam.isValid() == false)
    {
        return;
    }

    int outputSignalType = measureParam.outputSignalType();
    if (outputSignalType < 0  || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(0);
        return;
    }

    if (measureParam.calibratorManager() == nullptr)
    {
        assert(0);
        return;
    }

    switch ( outputSignalType )
    {
        case OUTPUT_SIGNAL_TYPE_UNUSED:         set1(measureParam); break;
        case OUTPUT_SIGNAL_TYPE_FROM_INPUT:     set2(measureParam); break;
        case OUTPUT_SIGNAL_TYPE_FROM_TUNING:    set3(measureParam); break;
        default:                                assert(0);
    }
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement::~LinearityMeasurement()
{
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::clear()
{
    setMeasureType(MEASURE_TYPE_LINEARITY);

    m_appSignalID = QString();
    m_customAppSignalID = QString();
    m_caption = QString();

    m_position.clear();

    for(int t = 0; t < VALUE_TYPE_COUNT; t++)
    {
        m_nominal[t] = 0;
        m_measure[t] = 0;

        for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
        {
            m_measureArray[t][m] = 0;
        }

        m_lowLimit[t] = 0;
        m_highLimit[t] = 0;
        m_unit[t] = QString();

        m_valuePrecision[t] = 0;

        m_hasRange[t] = true;
    }

    m_percent = 0;
    m_measureCount = 0;
    m_adjustment = 0;

    for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
    {
        m_errorInput[e] = 0;
        m_errorOutput[e] = 0;
        m_errorLimit[e] = 0;
    }

    m_additionalParamCount = 0;

    for(int a = 0; a < MEASURE_ADDITIONAL_PARAM_COUNT; a++)
    {
        m_additionalParam[a] = 0;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::set1(const MeasureParam &measureParam)
{
    SignalParam param = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
    if (param.isValid() == false)
    {
        assert(false);
        return;
    }

    if (measureParam.calibratorManager() == nullptr)
    {
        assert(0);
        return;
    }

    Calibrator* pCalibrator = measureParam.calibratorManager()->calibrator();
    if (pCalibrator == nullptr)
    {
        assert(false);
        return;
    }

    //
    //
    setMeasureType(MEASURE_TYPE_LINEARITY);
    setSignalHash(param.hash());

    // set ranges
    //
    setHasRange(VALUE_TYPE_PHYSICAL, true);
    setHasRange(VALUE_TYPE_IN_ELECTRIC, true);
    setHasRange(VALUE_TYPE_OUT_ELECTRIC, false);

    // features
    //
    setAppSignalID( param.appSignalID() );
    setCustomAppSignalID( param.customAppSignalID() );
    setCaption( param.caption() );

    setPosition( param.position() );

    // nominal
    //
    double electric = pCalibrator->sourceValue();
    double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, param);

    setNominal(VALUE_TYPE_PHYSICAL, physical);
    setNominal(VALUE_TYPE_IN_ELECTRIC, electric);
    setNominal(VALUE_TYPE_OUT_ELECTRIC, 0);

    setPercent( (( physical - param.inputPhysicalLowLimit()) * 100)/(param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit() ));

    // measure
    //

    double averagePhVal = 0;
    double averageInElVal = 0;
    double averageOutElVal = 0;

    int measureCount = theOptions.linearity().measureCountInPoint();

    setMeasureCount(measureCount);

    for(int index = 0; index < measureCount; index++)
    {
        AppSignalState signalState = theSignalBase.signalState(param.hash());

        double phVal = signalState.value;
        double elVal = conversion(signalState.value, CT_PHYSICAL_TO_ELECTRIC, param);

        setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
        setMeasureItemArray(VALUE_TYPE_IN_ELECTRIC, index, elVal);
        setMeasureItemArray(VALUE_TYPE_OUT_ELECTRIC, index, 0);

        averagePhVal += phVal;
        averageInElVal += elVal;

        QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
    }

    averagePhVal /= measureCount;
    averageInElVal /= measureCount;

    setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
    setMeasure(VALUE_TYPE_IN_ELECTRIC, averageInElVal);
    setMeasure(VALUE_TYPE_OUT_ELECTRIC, averageOutElVal);

    // limits
    //
    setLowLimit(VALUE_TYPE_PHYSICAL, param.inputPhysicalLowLimit());
    setHighLimit(VALUE_TYPE_PHYSICAL, param.inputPhysicalHighLimit());
    setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit( param.inputPhysicalUnitID() ) );

    setLowLimit(VALUE_TYPE_IN_ELECTRIC, param.inputElectricLowLimit());
    setHighLimit(VALUE_TYPE_IN_ELECTRIC, param.inputElectricHighLimit());
    setUnit(VALUE_TYPE_IN_ELECTRIC, theUnitBase.unit( param.inputElectricUnitID() ) );

    setLowLimit(VALUE_TYPE_OUT_ELECTRIC, 0);
    setHighLimit(VALUE_TYPE_OUT_ELECTRIC, 0);
    setUnit(VALUE_TYPE_OUT_ELECTRIC, QString() );

    // precision
    //
    setValuePrecision(VALUE_TYPE_PHYSICAL, param.inputPhysicalPrecision());
    setValuePrecision(VALUE_TYPE_IN_ELECTRIC, param.inputElectricPrecision());
    setValuePrecision(VALUE_TYPE_OUT_ELECTRIC, 0 );

    // calc errors
    //
    setErrorInput(MEASURE_ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL)) );
    setErrorInput(MEASURE_ERROR_TYPE_REDUCE, abs( ((averagePhVal - nominal(VALUE_TYPE_PHYSICAL)) / (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL))) * 100.0) );
    setErrorInput(MEASURE_ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL) ) / nominal(VALUE_TYPE_PHYSICAL)) * 100.0) );

    setErrorLimit(MEASURE_ERROR_TYPE_ABSOLUTE, abs( (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL)) * theOptions.linearity().m_errorValue / 100.0 ));
    setErrorLimit(MEASURE_ERROR_TYPE_REDUCE, theOptions.linearity().m_errorValue);
    setErrorLimit(MEASURE_ERROR_TYPE_RELATIVE, theOptions.linearity().m_errorValue);

    setErrorOutput(MEASURE_ERROR_TYPE_ABSOLUTE, 0 );
    setErrorOutput(MEASURE_ERROR_TYPE_REDUCE, 0 );
    setErrorOutput(MEASURE_ERROR_TYPE_RELATIVE, 0 );

    // calc additional parameters
    //
    calcAdditionalParam(averagePhVal, measureCount, VALUE_TYPE_PHYSICAL);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::set2(const MeasureParam &measureParam)
{
    if (measureParam.isValid() == false)
    {
        assert(false);
        return;
    }

    int outputSignalType = measureParam.outputSignalType();
    if (outputSignalType < 0  || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(0);
        return;
    }

    if (measureParam.calibratorManager() == nullptr)
    {
        assert(0);
        return;
    }

    Calibrator* pCalibrator = measureParam.calibratorManager()->calibrator();
    if (pCalibrator == nullptr)
    {
        assert(false);
        return;
    }

    SignalParam inParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
    SignalParam outParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

    //
    //
    setMeasureType(MEASURE_TYPE_LINEARITY);
    setSignalHash(outParam.hash());

    // set ranges
    //
    setHasRange(VALUE_TYPE_PHYSICAL, true);
    setHasRange(VALUE_TYPE_IN_ELECTRIC, true);
    setHasRange(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput());

    // features
    //
    setAppSignalID( outParam.appSignalID() );
    setCustomAppSignalID( outParam.customAppSignalID() );
    setCaption( outParam.caption() );

    setPosition( outParam.position() );

    // nominal
    //
    double electric = pCalibrator->sourceValue();
    double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, inParam);

    setNominal(VALUE_TYPE_PHYSICAL, physical);
    setNominal(VALUE_TYPE_IN_ELECTRIC, electric);
    setNominal(VALUE_TYPE_OUT_ELECTRIC, 0);

    if (outParam.isOutput() == true)
    {
        double outElectric =    (physical - outParam.inputPhysicalLowLimit()) * (outParam.outputElectricHighLimit() - outParam.outputElectricLowLimit())/
                                (outParam.inputPhysicalHighLimit() - outParam.inputPhysicalLowLimit()) + outParam.outputElectricLowLimit();

        setNominal(VALUE_TYPE_OUT_ELECTRIC, outElectric);
    }

    setPercent( (( physical - inParam.inputPhysicalLowLimit()) * 100)/(inParam.inputPhysicalHighLimit() - inParam.inputPhysicalLowLimit() ));

    // measure
    //

    double averagePhVal = 0;
    double averageInElVal = 0;
    double averageOutElVal = 0;

    int measureCount = theOptions.linearity().measureCountInPoint();

    setMeasureCount(measureCount);

    for(int index = 0; index < measureCount; index++)
    {
        AppSignalState signalState = theSignalBase.signalState(inParam.hash());

        double phVal = signalState.value;
        double elVal = conversion(signalState.value, CT_PHYSICAL_TO_ELECTRIC, inParam);
        double OutElVal = 0;

        if (outParam.isOutput() == true)
        {
            OutElVal = pCalibrator->measureValue();
        }

        setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
        setMeasureItemArray(VALUE_TYPE_IN_ELECTRIC, index, elVal);
        setMeasureItemArray(VALUE_TYPE_OUT_ELECTRIC, index, OutElVal);

        averagePhVal += phVal;
        averageInElVal += elVal;
        averageOutElVal += OutElVal;

        QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
    }

    averagePhVal /= measureCount;
    averageInElVal /= measureCount;
    averageOutElVal /= measureCount;

    setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
    setMeasure(VALUE_TYPE_IN_ELECTRIC, averageInElVal);
    setMeasure(VALUE_TYPE_OUT_ELECTRIC, averageOutElVal);

    // limits
    //
    setLowLimit(VALUE_TYPE_PHYSICAL, inParam.inputPhysicalLowLimit());
    setHighLimit(VALUE_TYPE_PHYSICAL, inParam.inputPhysicalHighLimit());
    setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit( inParam.inputPhysicalUnitID() ) );

    setLowLimit(VALUE_TYPE_IN_ELECTRIC, inParam.inputElectricLowLimit());
    setHighLimit(VALUE_TYPE_IN_ELECTRIC, inParam.inputElectricHighLimit());
    setUnit(VALUE_TYPE_IN_ELECTRIC, theUnitBase.unit( inParam.inputElectricUnitID() ) );

    setLowLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricLowLimit() : 0);
    setHighLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricHighLimit() : 0);
    setUnit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? theUnitBase.unit( outParam.outputElectricUnitID() ) : QString());

    // precision
    //
    setValuePrecision(VALUE_TYPE_PHYSICAL, inParam.inputPhysicalPrecision());
    setValuePrecision(VALUE_TYPE_IN_ELECTRIC, inParam.inputElectricPrecision());
    setValuePrecision(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricPrecision() : 0);

    // calc errors
    //

    setErrorInput(MEASURE_ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL)) );
    setErrorInput(MEASURE_ERROR_TYPE_REDUCE, abs( ((averagePhVal - nominal(VALUE_TYPE_PHYSICAL)) / (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL))) * 100.0) );
    setErrorInput(MEASURE_ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_PHYSICAL)- measure(VALUE_TYPE_PHYSICAL) ) / nominal(VALUE_TYPE_PHYSICAL)) * 100.0) );

    setErrorLimit(MEASURE_ERROR_TYPE_ABSOLUTE, abs( (highLimit(VALUE_TYPE_PHYSICAL) - lowLimit(VALUE_TYPE_PHYSICAL)) * theOptions.linearity().m_errorValue / 100.0 ));
    setErrorLimit(MEASURE_ERROR_TYPE_REDUCE, theOptions.linearity().m_errorValue);
    setErrorLimit(MEASURE_ERROR_TYPE_RELATIVE, theOptions.linearity().m_errorValue);

    if (outParam.isOutput() == true)
    {
        setErrorOutput(MEASURE_ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_OUT_ELECTRIC)- measure(VALUE_TYPE_OUT_ELECTRIC)) );
        setErrorOutput(MEASURE_ERROR_TYPE_REDUCE, abs( ((averageOutElVal - nominal(VALUE_TYPE_OUT_ELECTRIC)) / (highLimit(VALUE_TYPE_OUT_ELECTRIC) - lowLimit(VALUE_TYPE_OUT_ELECTRIC))) * 100.0) );
        setErrorOutput(MEASURE_ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_OUT_ELECTRIC)- measure(VALUE_TYPE_OUT_ELECTRIC) ) / nominal(VALUE_TYPE_OUT_ELECTRIC)) * 100.0) );
    }

    // calc additional parameters
    //
    calcAdditionalParam(averagePhVal, measureCount, VALUE_TYPE_PHYSICAL);
}


// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::set3(const MeasureParam &measureParam)
{
    if (measureParam.isValid() == false)
    {
        assert(false);
        return;
    }

    int outputSignalType = measureParam.outputSignalType();
    if (outputSignalType < 0  || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(0);
        return;
    }

    if (measureParam.calibratorManager() == nullptr)
    {
        assert(0);
        return;
    }

    Calibrator* pCalibrator = measureParam.calibratorManager()->calibrator();
    if (pCalibrator == nullptr)
    {
        assert(false);
        return;
    }

    SignalParam inParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
    SignalParam outParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

    //
    //
    setMeasureType(MEASURE_TYPE_LINEARITY);
    setSignalHash(outParam.hash());

    // set ranges
    //
    setHasRange(VALUE_TYPE_PHYSICAL, true);
    setHasRange(VALUE_TYPE_IN_ELECTRIC, false);
    setHasRange(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput());

    // features
    //
    setAppSignalID( outParam.appSignalID() );
    setCustomAppSignalID( outParam.customAppSignalID() );
    setCaption( outParam.caption() );

    setPosition( outParam.position() );

    // nominal
    //
    double physical = theSignalBase.signalState(outParam.hash()).value;

    setNominal(VALUE_TYPE_PHYSICAL, physical);
    setNominal(VALUE_TYPE_IN_ELECTRIC, 0);
    setNominal(VALUE_TYPE_OUT_ELECTRIC, 0);

    if (outParam.isOutput() == true)
    {
        double outElectric =    (physical - outParam.inputPhysicalLowLimit()) * (outParam.outputElectricHighLimit() - outParam.outputElectricLowLimit())/
                                (outParam.inputPhysicalHighLimit() - outParam.inputPhysicalLowLimit()) + outParam.outputElectricLowLimit();

        setNominal(VALUE_TYPE_OUT_ELECTRIC, outElectric);
    }

    setPercent( (( physical - outParam.inputPhysicalLowLimit()) * 100)/(outParam.inputPhysicalHighLimit() - outParam.inputPhysicalLowLimit() ));

    // measure
    //

    if (outParam.isOutput() == true)
    {
        measureParam.calibratorManager()->value();
        while(measureParam.calibratorManager()->isReadyForManage() != true);
    }

    double averagePhVal = 0;
    double averageOutElVal = 0;

    int measureCount = theOptions.linearity().measureCountInPoint();

    setMeasureCount(measureCount);

    for(int index = 0; index < measureCount; index++)
    {
        double phVal = theSignalBase.signalState(outParam.hash()).value;
        double OutElVal = 0;

        if (outParam.isOutput() == true)
        {
            OutElVal = pCalibrator->measureValue();
        }

        setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
        setMeasureItemArray(VALUE_TYPE_IN_ELECTRIC, index, 0);
        setMeasureItemArray(VALUE_TYPE_OUT_ELECTRIC, index, OutElVal);

        averagePhVal += phVal;
        averageOutElVal += OutElVal;

        QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
    }

    averagePhVal /= measureCount;
    averageOutElVal /= measureCount;

    setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
    setMeasure(VALUE_TYPE_IN_ELECTRIC, 0);
    setMeasure(VALUE_TYPE_OUT_ELECTRIC, averageOutElVal);

    // limits
    //
    setLowLimit(VALUE_TYPE_PHYSICAL, outParam.inputPhysicalLowLimit());
    setHighLimit(VALUE_TYPE_PHYSICAL, outParam.inputPhysicalHighLimit());
    setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit( outParam.inputPhysicalUnitID() ) );

    setLowLimit(VALUE_TYPE_IN_ELECTRIC, 0);
    setHighLimit(VALUE_TYPE_IN_ELECTRIC, 0);
    setUnit(VALUE_TYPE_IN_ELECTRIC, QString() );

    setLowLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricLowLimit() : 0);
    setHighLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricHighLimit() : 0);
    setUnit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? theUnitBase.unit( outParam.outputElectricUnitID() ) : QString());

    // precision
    //
    setValuePrecision(VALUE_TYPE_PHYSICAL, outParam.inputPhysicalPrecision());
    setValuePrecision(VALUE_TYPE_IN_ELECTRIC, 0);
    setValuePrecision(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricPrecision() : 0);

    // calc errors
    //
    setErrorInput(MEASURE_ERROR_TYPE_ABSOLUTE, 0 );
    setErrorInput(MEASURE_ERROR_TYPE_REDUCE, 0 );
    setErrorInput(MEASURE_ERROR_TYPE_RELATIVE, 0 );

    setErrorLimit(MEASURE_ERROR_TYPE_ABSOLUTE, abs( (highLimit(VALUE_TYPE_OUT_ELECTRIC) - lowLimit(VALUE_TYPE_OUT_ELECTRIC)) * theOptions.linearity().m_errorValue / 100.0 ));
    setErrorLimit(MEASURE_ERROR_TYPE_REDUCE, theOptions.linearity().m_errorValue);
    setErrorLimit(MEASURE_ERROR_TYPE_RELATIVE, theOptions.linearity().m_errorValue);

    if (outParam.isOutput() == true)
    {
        setErrorOutput(MEASURE_ERROR_TYPE_ABSOLUTE, abs( nominal(VALUE_TYPE_OUT_ELECTRIC)- measure(VALUE_TYPE_OUT_ELECTRIC)) );
        setErrorOutput(MEASURE_ERROR_TYPE_REDUCE, abs( ((averageOutElVal - nominal(VALUE_TYPE_OUT_ELECTRIC)) / (highLimit(VALUE_TYPE_OUT_ELECTRIC) - lowLimit(VALUE_TYPE_OUT_ELECTRIC))) * 100.0) );
        setErrorOutput(MEASURE_ERROR_TYPE_RELATIVE, abs( ((nominal(VALUE_TYPE_OUT_ELECTRIC)- measure(VALUE_TYPE_OUT_ELECTRIC) ) / nominal(VALUE_TYPE_OUT_ELECTRIC)) * 100.0) );
    }

    // calc additional parameters
    //
    calcAdditionalParam(averageOutElVal, measureCount, VALUE_TYPE_OUT_ELECTRIC);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcAdditionalParam(double averageVal, int measureCount, int type)
{
    if (measureCount < 1 || measureCount >= MAX_MEASUREMENT_IN_POINT)
    {
        return;
    }

    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        return;
    }

    // calc additional parameters
    //
    double maxDeviation = 0;
    int maxDeviationIndex = 0;

    for(int index = 0; index < measureCount; index++)
    {
        if ( maxDeviation < abs(averageVal - measureItemArray(type, index)))
        {
            maxDeviation = abs(averageVal - measureItemArray(type, index));
            maxDeviationIndex = index;
        }
    }

    setAdditionalParam(MEASURE_ADDITIONAL_PARAM_MAX_VALUE,  measureItemArray(type, maxDeviationIndex));

        // according to GOST 8.508-84 paragraph 3.4.1 formula 42
        //
    double systemError = abs (averageVal - nominal(type));

    setAdditionalParam(MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR, systemError);

        // according to GOST 8.736-2011 paragraph 5.3 formula 3
        //
    double sumDeviation = 0;

    for(int index = 0; index < measureCount; index++)
    {
        sumDeviation += pow( averageVal -  measureItemArray(type, index), 2 );	// 1. sum of deviations
    }

    sumDeviation /= (double) measureCount - 1;                                                  // 2. divide on (count of measure - 1)
    double sco = sqrt(sumDeviation);                                                            // 3. sqrt

    setAdditionalParam(MEASURE_ADDITIONAL_PARAM_MSE, sco);

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

    setAdditionalParam(MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER, border);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::nominalStr(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    if (m_hasRange[type] == false)
    {
        return QString("N/A");
    }

    return QString("%1 %2").arg(QString::number(m_nominal[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureStr(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    if (m_hasRange[type] == false)
    {
        return QString("N/A");
    }

    return QString("%1 %2").arg(QString::number(m_measure[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::limitStr(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    if (m_hasRange[type] == false)
    {
        return QString("N/A");
    }

    QString low = QString::number(m_lowLimit[type], 10, m_valuePrecision[type]);
    QString high = QString::number(m_highLimit[type], 10, m_valuePrecision[type]);

    return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[type]);;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::errorInputStr(int type) const
{
    if (type < 0 || type >= MEASURE_ERROR_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    if ( m_hasRange[VALUE_TYPE_IN_ELECTRIC] == false)
    {
        return QString("N/A");
    }

    QString str;

    if (type == MEASURE_ERROR_TYPE_ABSOLUTE)
    {
        str = QString::number(m_errorInput[type], 10, m_valuePrecision[VALUE_TYPE_PHYSICAL]) + " " + m_unit[VALUE_TYPE_PHYSICAL];
    }
    else
    {
        str = QString::number(m_errorInput[type], 10, 2) ;
    }

    return str;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::errorOutputStr(int type) const
{
    if (type < 0 || type >= MEASURE_ERROR_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    if ( m_hasRange[VALUE_TYPE_OUT_ELECTRIC] == false)
    {
        return QString("N/A");
    }

    QString str;

    if (type == MEASURE_ERROR_TYPE_ABSOLUTE)
    {
        str = QString::number(m_errorOutput[type], 10, m_valuePrecision[VALUE_TYPE_OUT_ELECTRIC]) + " " + m_unit[VALUE_TYPE_OUT_ELECTRIC];
    }
    else
    {
        str = QString::number(m_errorOutput[type], 10, 2) ;
    }

    return str;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::errorLimitStr(int type) const
{
    if (type < 0 || type >= MEASURE_ERROR_TYPE_COUNT)
    {
        assert(0);
        return QString();
    }

    QString str;

    if (type == MEASURE_ERROR_TYPE_ABSOLUTE)
    {
        if ( m_hasRange[VALUE_TYPE_IN_ELECTRIC] == true)
        {
            str = QString::number(m_errorLimit[type], 10, m_valuePrecision[VALUE_TYPE_PHYSICAL]) + " " + m_unit[VALUE_TYPE_PHYSICAL];;
        }
        else
        {
            str = QString::number(m_errorLimit[type], 10, m_valuePrecision[VALUE_TYPE_OUT_ELECTRIC]) + " " + m_unit[VALUE_TYPE_OUT_ELECTRIC];;
        }
    }
    else
    {
        str = QString::number(m_errorLimit[type], 10, 2);
    }

    return str;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureItemStr(int type, int index) const
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

    m_measureCount = pLinearityMeasureItem->measureCount();

    for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
    {
        m_measureArray[valueType][m] = pLinearityMeasureItem->measureItemArray(valueType, m);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::updateAdditionalParam(Measurement* pMeasurement)
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

    for(int a = 0; a < MEASURE_ADDITIONAL_PARAM_COUNT; a++)
    {
        m_additionalParam[a] = pLinearityMeasureItem->additionalParam(a);
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

        m_hasRange[v] = from.m_hasRange[v];
    }

    m_percent = from.m_percent;
    m_measureCount = from.m_measureCount;

    m_adjustment = from.m_adjustment;

    for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
    {
        m_errorInput[e] = from.m_errorInput[e];;
        m_errorOutput[e] = from.m_errorOutput[e];;
        m_errorLimit[e] = from.m_errorLimit[e];;
    }

    m_additionalParamCount = from.m_additionalParamCount;

    for(int a = 0; a < MEASURE_ADDITIONAL_PARAM_COUNT; a++)
    {
        m_additionalParam[a] = from.m_additionalParam[a];;
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
