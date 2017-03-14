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
		case MEASURE_TYPE_LINEARITY:	pMeasurement = static_cast<LinearityMeasurement*> (this) + index;	break;
		case MEASURE_TYPE_COMPARATOR:	pMeasurement = static_cast<ComparatorMeasurement*> (this) + index;	break;
		default:						assert(0);
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
		case MEASURE_TYPE_LINEARITY:	*static_cast<LinearityMeasurement*> (this) = *static_cast <LinearityMeasurement*> (&from);		break;
		case MEASURE_TYPE_COMPARATOR:	*static_cast<ComparatorMeasurement*> (this) = *static_cast <ComparatorMeasurement*> (&from);	break;
		default:						assert(0);
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
	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	if (measureParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	switch (outputSignalType)
	{
		case OUTPUT_SIGNAL_TYPE_UNUSED:			set1(measureParam);	break;
		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:		set2(measureParam);	break;
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:	set3(measureParam);	break;
		default:								assert(0);
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

	m_appSignalID.clear();
	m_customAppSignalID.clear();
	m_caption.clear();

	m_location.clear();

	m_percent = 0;

	for(int t = 0; t < VALUE_TYPE_COUNT; t++)
	{
		m_nominal[t] = 0;
		m_measure[t] = 0;

		m_hasLimit[t] = true;
		m_lowLimit[t] = 0;
		m_highLimit[t] = 0;
		m_unit[t].clear();
		m_limitPrecision[t] = 0;

		for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
		{
			m_error[t][e] = 0;
			m_errorLimit[t][e] = 0;
		}

		for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
		{
			m_measureArray[t][m] = 0;
		}
	}

	m_adjustment = 0;

	m_measureCount = 0;

	m_additionalParamCount = 0;

	for(int a = 0; a < MEASURE_ADDITIONAL_PARAM_COUNT; a++)
	{
		m_additionalParam[a] = 0;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::set1(const MeasureParam &measureParam)
{
	Metrology::SignalParam param = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
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

	// features
	//

	setAppSignalID(param.appSignalID());
	setCustomAppSignalID(param.customAppSignalID());
	setCaption(param.caption());

	setLocation(param.location());

	// nominal
	//

	double electric = pCalibrator->sourceValue();
	double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, param);

	setPercent(((physical - param.inputPhysicalLowLimit()) * 100)/(param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()));

	setNominal(VALUE_TYPE_IN_ELECTRIC, electric);
	setNominal(VALUE_TYPE_PHYSICAL, physical);
	setNominal(VALUE_TYPE_OUT_ELECTRIC, 0);

	// measure
	//

	double averageInElVal = 0;
	double averagePhVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		AppSignalState signalState = theSignalBase.signalState(param.hash());

		double elVal = conversion(signalState.value, CT_PHYSICAL_TO_ELECTRIC, param);
		double phVal = signalState.value;

		setMeasureItemArray(VALUE_TYPE_IN_ELECTRIC, index, elVal);
		setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
		setMeasureItemArray(VALUE_TYPE_OUT_ELECTRIC, index, 0);

		averageInElVal += elVal;
		averagePhVal += phVal;

		QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
	}

	averageInElVal /= measureCount;
	averagePhVal /= measureCount;

	setMeasure(VALUE_TYPE_IN_ELECTRIC, averageInElVal);
	setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
	setMeasure(VALUE_TYPE_OUT_ELECTRIC, 0);

	// limits
	//

	setHasLimit(VALUE_TYPE_IN_ELECTRIC, true);
	setHasLimit(VALUE_TYPE_PHYSICAL, true);
	setHasLimit(VALUE_TYPE_OUT_ELECTRIC, false);

	setLowLimit(VALUE_TYPE_IN_ELECTRIC, param.inputElectricLowLimit());
	setHighLimit(VALUE_TYPE_IN_ELECTRIC, param.inputElectricHighLimit());
	setUnit(VALUE_TYPE_IN_ELECTRIC, theUnitBase.unit(param.inputElectricUnitID()));

	setLowLimit(VALUE_TYPE_PHYSICAL, param.inputPhysicalLowLimit());
	setHighLimit(VALUE_TYPE_PHYSICAL, param.inputPhysicalHighLimit());
	setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit(param.inputPhysicalUnitID()));

	setLowLimit(VALUE_TYPE_OUT_ELECTRIC, 0);
	setHighLimit(VALUE_TYPE_OUT_ELECTRIC, 0);
	setUnit(VALUE_TYPE_OUT_ELECTRIC, QString());

	// precision
	//

	setLimitPrecision(VALUE_TYPE_IN_ELECTRIC, param.inputElectricPrecision());
	setLimitPrecision(VALUE_TYPE_PHYSICAL, param.inputPhysicalPrecision());
	setLimitPrecision(VALUE_TYPE_OUT_ELECTRIC, 0);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(VALUE_TYPE_PHYSICAL);
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
	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
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

	Metrology::SignalParam inParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	Metrology::SignalParam outParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

	//
	//
	setMeasureType(MEASURE_TYPE_LINEARITY);
	setSignalHash(outParam.hash());

	// features
	//

	setAppSignalID(outParam.appSignalID());
	setCustomAppSignalID(outParam.customAppSignalID());
	setCaption(outParam.caption());

	setLocation(outParam.location());

	// nominal
	//

	double electric = pCalibrator->sourceValue();
	double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, inParam);

	setPercent(((physical - inParam.inputPhysicalLowLimit()) * 100)/(inParam.inputPhysicalHighLimit() - inParam.inputPhysicalLowLimit()));

	setNominal(VALUE_TYPE_IN_ELECTRIC, electric);
	setNominal(VALUE_TYPE_PHYSICAL, physical);
	setNominal(VALUE_TYPE_OUT_ELECTRIC, 0);

	if (outParam.isOutput() == true)
	{
		double outElectric =	(physical - outParam.inputPhysicalLowLimit()) * (outParam.outputElectricHighLimit() - outParam.outputElectricLowLimit())/
								(outParam.inputPhysicalHighLimit() - outParam.inputPhysicalLowLimit()) + outParam.outputElectricLowLimit();

		setNominal(VALUE_TYPE_OUT_ELECTRIC, outElectric);
	}

	// measure
	//

	double averageInElVal = 0;
	double averagePhVal = 0;
	double averageOutElVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		AppSignalState signalState = theSignalBase.signalState(inParam.hash());

		double elVal = conversion(signalState.value, CT_PHYSICAL_TO_ELECTRIC, inParam);
		double phVal = signalState.value;
		double OutElVal = 0;

		if (outParam.isOutput() == true)
		{
			OutElVal = pCalibrator->measureValue();
		}

		setMeasureItemArray(VALUE_TYPE_IN_ELECTRIC, index, elVal);
		setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
		setMeasureItemArray(VALUE_TYPE_OUT_ELECTRIC, index, OutElVal);

		averageInElVal += elVal;
		averagePhVal += phVal;
		averageOutElVal += OutElVal;

		QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
	}

	averageInElVal /= measureCount;
	averagePhVal /= measureCount;
	averageOutElVal /= measureCount;

	setMeasure(VALUE_TYPE_IN_ELECTRIC, averageInElVal);
	setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
	setMeasure(VALUE_TYPE_OUT_ELECTRIC, averageOutElVal);

	// limits
	//

	setHasLimit(VALUE_TYPE_IN_ELECTRIC, true);
	setHasLimit(VALUE_TYPE_PHYSICAL, true);
	setHasLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput());

	setLowLimit(VALUE_TYPE_IN_ELECTRIC, inParam.inputElectricLowLimit());
	setHighLimit(VALUE_TYPE_IN_ELECTRIC, inParam.inputElectricHighLimit());
	setUnit(VALUE_TYPE_IN_ELECTRIC, theUnitBase.unit(inParam.inputElectricUnitID()));

	setLowLimit(VALUE_TYPE_PHYSICAL, inParam.inputPhysicalLowLimit());
	setHighLimit(VALUE_TYPE_PHYSICAL, inParam.inputPhysicalHighLimit());
	setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit(inParam.inputPhysicalUnitID()));

	setLowLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricLowLimit() : 0);
	setHighLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricHighLimit() : 0);
	setUnit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? theUnitBase.unit(outParam.outputElectricUnitID()) : QString());

	// precision
	//

	setLimitPrecision(VALUE_TYPE_IN_ELECTRIC, inParam.inputElectricPrecision());
	setLimitPrecision(VALUE_TYPE_PHYSICAL, inParam.inputPhysicalPrecision());
	setLimitPrecision(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricPrecision() : 0);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(VALUE_TYPE_PHYSICAL);
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
	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
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

	Metrology::SignalParam inParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	Metrology::SignalParam outParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

	//
	//
	setMeasureType(MEASURE_TYPE_LINEARITY);
	setSignalHash(outParam.hash());

	// features
	//
	setAppSignalID(outParam.appSignalID());
	setCustomAppSignalID(outParam.customAppSignalID());
	setCaption(outParam.caption());

	setLocation(outParam.location());

	// nominal
	//
	double physical = theSignalBase.signalState(outParam.hash()).value;

	setPercent(((physical - outParam.inputPhysicalLowLimit()) * 100)/(outParam.inputPhysicalHighLimit() - outParam.inputPhysicalLowLimit()));

	setNominal(VALUE_TYPE_IN_ELECTRIC, 0);
	setNominal(VALUE_TYPE_PHYSICAL, physical);
	setNominal(VALUE_TYPE_OUT_ELECTRIC, 0);

	if (outParam.isOutput() == true)
	{
		double outElectric =	(physical - outParam.inputPhysicalLowLimit()) * (outParam.outputElectricHighLimit() - outParam.outputElectricLowLimit())/
								(outParam.inputPhysicalHighLimit() - outParam.inputPhysicalLowLimit()) + outParam.outputElectricLowLimit();

		setNominal(VALUE_TYPE_OUT_ELECTRIC, outElectric);
	}

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

		setMeasureItemArray(VALUE_TYPE_IN_ELECTRIC, index, 0);
		setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, phVal);
		setMeasureItemArray(VALUE_TYPE_OUT_ELECTRIC, index, OutElVal);

		averagePhVal += phVal;
		averageOutElVal += OutElVal;

		QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
	}

	setMeasure(VALUE_TYPE_IN_ELECTRIC, 0);
	setMeasure(VALUE_TYPE_PHYSICAL, averagePhVal);
	setMeasure(VALUE_TYPE_OUT_ELECTRIC, averageOutElVal);

	// limits
	//

	setHasLimit(VALUE_TYPE_IN_ELECTRIC, false);
	setHasLimit(VALUE_TYPE_PHYSICAL, true);
	setHasLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput());

	setLowLimit(VALUE_TYPE_IN_ELECTRIC, 0);
	setHighLimit(VALUE_TYPE_IN_ELECTRIC, 0);
	setUnit(VALUE_TYPE_IN_ELECTRIC, QString());

	setLowLimit(VALUE_TYPE_PHYSICAL, outParam.inputPhysicalLowLimit());
	setHighLimit(VALUE_TYPE_PHYSICAL, outParam.inputPhysicalHighLimit());
	setUnit(VALUE_TYPE_PHYSICAL, theUnitBase.unit(outParam.inputPhysicalUnitID()));

	setLowLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricLowLimit() : 0);
	setHighLimit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricHighLimit() : 0);
	setUnit(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? theUnitBase.unit(outParam.outputElectricUnitID()) : QString());

	// precision
	//

	setLimitPrecision(VALUE_TYPE_IN_ELECTRIC, 0);
	setLimitPrecision(VALUE_TYPE_PHYSICAL, outParam.inputPhysicalPrecision());
	setLimitPrecision(VALUE_TYPE_OUT_ELECTRIC, outParam.isOutput() == true ? outParam.outputElectricPrecision() : 0);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(VALUE_TYPE_OUT_ELECTRIC);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcError()
{
	double errorLimit = theOptions.linearity().errorLimit();

	for(int limitType = 0; limitType < VALUE_TYPE_COUNT; limitType++)
	{
		if (hasLimit(limitType) == false)
		{
			continue;
		}

		setError(limitType, MEASURE_ERROR_TYPE_ABSOLUTE,		std::abs(nominal(limitType)- measure(limitType)));
		setError(limitType, MEASURE_ERROR_TYPE_REDUCE,			std::abs(((nominal(limitType)-measure(limitType)) / (highLimit(limitType) - lowLimit(limitType))) * 100.0));

		setErrorLimit(limitType, MEASURE_ERROR_TYPE_ABSOLUTE,	std::abs((highLimit(limitType) - lowLimit(limitType)) * errorLimit / 100.0));
		setErrorLimit(limitType, MEASURE_ERROR_TYPE_REDUCE,		errorLimit);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcAdditionalParam(int limitType)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		return;
	}

	// calc additional parameters
	//
	double maxDeviation = 0;
	int maxDeviationIndex = 0;

	for(int index = 0; index < measureCount(); index++)
	{
		if (maxDeviation < std::abs(measure(limitType) - measureItemArray(limitType, index)))
		{
			maxDeviation = std::abs(measure(limitType) - measureItemArray(limitType, index));
			maxDeviationIndex = index;
		}
	}

	setAdditionalParam(MEASURE_ADDITIONAL_PARAM_MAX_VALUE, measureItemArray(limitType, maxDeviationIndex));

		// according to GOST 8.508-84 paragraph 3.4.1 formula 42
		//
	double systemError = std::abs(measure(limitType) - nominal(limitType));

	setAdditionalParam(MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR, systemError);

		// according to GOST 8.736-2011 paragraph 5.3 formula 3
		//
	double sumDeviation = 0;

	for(int index = 0; index < measureCount(); index++)
	{
		sumDeviation += pow(measure(limitType) - measureItemArray(limitType, index), 2);		// 1. sum of deviations
	}

	sumDeviation /= (double) measureCount() - 1;												// 2. divide on (count of measure - 1)
	double sco = sqrt(sumDeviation);															// 3. sqrt

	setAdditionalParam(MEASURE_ADDITIONAL_PARAM_MSE, sco);

		// according to GOST 8.207-76 paragraph 2.4
		//
	double estimateSCO = sco / sqrt((double) measureCount());

		// Student's rate according to GOST 27.202 on P = 0.95
		// or GOST 8.207-76 application 2 (last page)
		//
	double k_student = studentK(measureCount(), CT_PROPABILITY_95);

		// according to GOST 8.207-76 paragraph 3.2
		//
	double border = k_student * estimateSCO;

	setAdditionalParam(MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER, border);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::signalID(int type) const
{
	QString strID;

	switch (type)
	{
		case SIGNAL_ID_TYPE_APP:		strID = m_appSignalID;				break;
		case SIGNAL_ID_TYPE_CUSTOM:		strID = m_customAppSignalID;		break;
		case SIGNAL_ID_TYPE_EQUIPMENT:	strID = m_location.equipmentID();	break;
		default:						assert(0);
	}

	return strID;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::nominal(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_nominal[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::nominalStr(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (m_hasLimit[limitType] == false)
	{
		return QString("N/A");
	}

	return QString("%1 %2").arg(QString::number(m_nominal[limitType], 10, m_limitPrecision[limitType])).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setNominal(int limitType, double value)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_nominal[limitType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::measure(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_measure[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureStr(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (m_hasLimit[limitType] == false)
	{
		return QString("N/A");
	}

	return QString("%1 %2").arg(QString::number(m_measure[limitType], 10, m_limitPrecision[limitType])).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setMeasure(int limitType, double value)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_measure[limitType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

bool LinearityMeasurement::hasLimit(int limitType)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return false;
	}

	return m_hasLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setHasLimit(int limitType, bool hasLimit)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_hasLimit[limitType] = hasLimit;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::lowLimit(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_lowLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setLowLimit(int limitType, double lowLimit)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_lowLimit[limitType] = lowLimit;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::highLimit(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_highLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setHighLimit(int limitType, double highLimit)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_highLimit[limitType] = highLimit;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::unit(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return m_unit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setUnit(int limitType, QString unit)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_unit[limitType] = unit;
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityMeasurement::limitPrecision(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_limitPrecision[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setLimitPrecision(int limitType, int precision)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_limitPrecision[limitType] = precision;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::limitStr(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (m_hasLimit[limitType] == false)
	{
		return QString("N/A");
	}

	QString low = QString::number(m_lowLimit[limitType], 10, m_limitPrecision[limitType]);
	QString high = QString::number(m_highLimit[limitType], 10, m_limitPrecision[limitType]);

	return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[limitType]);;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::error(int limitType, int errotType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	if (errotType < 0 || errotType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_error[limitType][errotType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::errorStr(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (m_hasLimit[limitType] == false)
	{
		return QString("N/A");
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	QString str;

	if (errorType == MEASURE_ERROR_TYPE_ABSOLUTE)
	{
		switch (limitType)
		{
			case VALUE_TYPE_IN_ELECTRIC:
			case VALUE_TYPE_PHYSICAL:

				switch(theOptions.linearity().showInputErrorType())
				{
					case LO_SHOW_INPUT_ERROR_ELECTRIC:	str = QString::number(m_error[limitType][errorType], 10, m_limitPrecision[VALUE_TYPE_IN_ELECTRIC]) + " " + m_unit[VALUE_TYPE_IN_ELECTRIC];		break;
					case LO_SHOW_INPUT_ERROR_PHYSICAL:	str = QString::number(m_error[limitType][errorType], 10, m_limitPrecision[VALUE_TYPE_PHYSICAL]) + " " + m_unit[VALUE_TYPE_PHYSICAL];			break;
					default:							assert(0);
				}

				break;

			case VALUE_TYPE_OUT_ELECTRIC:				str = QString::number(m_error[limitType][errorType], 10, m_limitPrecision[VALUE_TYPE_OUT_ELECTRIC]) + " " + m_unit[VALUE_TYPE_OUT_ELECTRIC];	break;
			default:									assert(0);
		}
	}
	else
	{
		str = QString::number(m_error[limitType][errorType], 10, 2) + " %" ;
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setError(int limitType, int errotType, double value)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	if (errotType < 0 || errotType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_error[limitType][errotType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::errorLimit(int limitType, int errotType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	if (errotType < 0 || errotType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_errorLimit[limitType][errotType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::errorLimitStr(int limitType) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (m_hasLimit[limitType] == false)
	{
		return QString("N/A");
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	QString str;

	if (errorType == MEASURE_ERROR_TYPE_ABSOLUTE)
	{
		switch (limitType)
		{
			case VALUE_TYPE_IN_ELECTRIC:
			case VALUE_TYPE_PHYSICAL:

				switch(theOptions.linearity().showInputErrorType())
				{
					case LO_SHOW_INPUT_ERROR_ELECTRIC:	str = QString::number(m_errorLimit[limitType][errorType], 10, m_limitPrecision[VALUE_TYPE_IN_ELECTRIC]) + " " + m_unit[VALUE_TYPE_IN_ELECTRIC];		break;
					case LO_SHOW_INPUT_ERROR_PHYSICAL:	str = QString::number(m_errorLimit[limitType][errorType], 10, m_limitPrecision[VALUE_TYPE_PHYSICAL]) + " " + m_unit[VALUE_TYPE_PHYSICAL];			break;
					default:							assert(0);
				}

				break;

			case VALUE_TYPE_OUT_ELECTRIC:				str = QString::number(m_errorLimit[limitType][errorType], 10, m_limitPrecision[VALUE_TYPE_OUT_ELECTRIC]) + " " + m_unit[VALUE_TYPE_OUT_ELECTRIC];	break;
			default:									assert(0);
		}
	}
	else
	{
		str = QString::number(m_errorLimit[limitType][errorType], 10, 2) + " %";
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setErrorLimit(int limitType, int errotType, double value)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	if (errotType < 0 || errotType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_errorLimit[limitType][errotType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::measureItemArray(int limitType, int index) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT)
	{
		assert(0);
		return 0;
	}

	return m_measureArray[limitType][index];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureItemStr(int limitType, int index) const
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT)
	{
		assert(0);
		return QString();
	}

	return QString::number(m_measureArray[limitType][index], 10, m_limitPrecision[limitType]);
}


// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setMeasureItemArray(int limitType, int index, double value)
{
	if (limitType < 0 || limitType >= VALUE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT)
	{
		assert(0);
		return;
	}

	m_measureArray[limitType][index] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::additionalParam(int paramType) const
{
	if (paramType < 0 || paramType >= MEASURE_ADDITIONAL_PARAM_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_additionalParam[paramType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setAdditionalParam(int paramType, double value)
{
	if (paramType < 0 || paramType >= MEASURE_ADDITIONAL_PARAM_COUNT)
	{
		assert(0);
		return;
	}

	m_additionalParam[paramType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::updateMeasureArray(int limitType, Measurement* pMeasurement)
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
		m_measureArray[limitType][m] = pLinearityMeasureItem->measureItemArray(limitType, m);
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

	m_location = from.m_location;
	m_percent = from.m_percent;

	for(int t = 0; t < VALUE_TYPE_COUNT; t++)
	{
		m_nominal[t] = from.m_nominal[t];
		m_measure[t] = from.m_measure[t];

		m_hasLimit[t] = from.m_hasLimit[t];
		m_lowLimit[t] = from.m_lowLimit[t];;
		m_highLimit[t] = from.m_highLimit[t];;
		m_unit[t] = from.m_unit[t];;
		m_limitPrecision[t] = from.m_limitPrecision[t];

		for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
		{
			m_error[t][e] = from.m_error[t][e];;
			m_errorLimit[t][e] = from.m_errorLimit[t][e];;
		}

		for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
		{
			m_measureArray[t][m] = from.m_measureArray[t][m];
		}
	}

	m_adjustment = from.m_adjustment;

	m_measureCount = from.m_measureCount;

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
