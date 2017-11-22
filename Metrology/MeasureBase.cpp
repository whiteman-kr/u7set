#include "MeasureBase.h"

#include "Database.h"
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

LinearityMeasurement::LinearityMeasurement(const MeasureMultiParam &measureParam)
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
		case OUTPUT_SIGNAL_TYPE_UNUSED:			fill_measure_aim(measureParam);	break;
		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:	fill_measure_aom(measureParam);	break;
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

	for(int t = 0; t < MEASURE_LIMIT_TYPE_COUNT; t++)
	{
		m_nominal[t] = 0;
		m_measure[t] = 0;

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

void LinearityMeasurement::fill_measure_aim(const MeasureMultiParam &measureParam)
{
	if (measureParam.isValid() == false)
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

	Metrology::SignalParam inParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	//
	//
	setMeasureType(MEASURE_TYPE_LINEARITY);
	setSignalHash(inParam.hash());

	// features
	//

	setAppSignalID(inParam.appSignalID());
	setCustomAppSignalID(inParam.customAppSignalID());
	setCaption(inParam.caption());

	setLocation(inParam.location());

	// nominal
	//

	double electric = pCalibrator->sourceValue();
	double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, inParam);

	setPercent(((physical - inParam.physicalLowLimit()) * 100)/(inParam.physicalHighLimit() - inParam.physicalLowLimit()));

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_PHYSICAL, physical);

	// measure
	//

	double averageElVal = 0;
	double averagePhVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		Metrology::SignalState signalState = theSignalBase.signalState(inParam.hash());

		double elVal = conversion(signalState.value(), CT_PHYSICAL_TO_ELECTRIC, inParam);
		double phVal = signalState.value();

		setMeasureItemArray(MEASURE_LIMIT_TYPE_ELECTRIC, index, elVal);
		setMeasureItemArray(MEASURE_LIMIT_TYPE_PHYSICAL, index, phVal);

		averageElVal += elVal;
		averagePhVal += phVal;

		QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
	}

	averageElVal /= measureCount;
	averagePhVal /= measureCount;

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, averageElVal);
	setMeasure(MEASURE_LIMIT_TYPE_PHYSICAL, averagePhVal);

	// limits
	//
	setLimits(inParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(MEASURE_LIMIT_TYPE_PHYSICAL);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::fill_measure_aom(const MeasureMultiParam &measureParam)
{
	if (measureParam.isValid() == false)
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

	int outputSignalType = measureParam.outputSignalType();
	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	Metrology::SignalParam outParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
	if (outParam.isValid() == false)
	{
		assert(false);
		return;
	}

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

	//double physical = theSignalBase.signalState(outParam.hash()).value();
	double physical = (measureParam.percent() * (outParam.physicalHighLimit() - outParam.physicalLowLimit()) / 100) + outParam.physicalLowLimit();
	double electric = conversion(physical, CT_PHYSICAL_TO_ELECTRIC, outParam);

	setPercent(measureParam.percent());

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_PHYSICAL, physical);

	// measure
	//

	double averageElVal = 0;
	double averagePhVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		double elVal = 0;
		double phVal = theSignalBase.signalState(outParam.hash()).value();

		if (outParam.isOutput() == true)
		{
			measureParam.calibratorManager()->value();
			while(measureParam.calibratorManager()->isReadyForManage() != true);

			elVal = pCalibrator->measureValue();
		}

		setMeasureItemArray(MEASURE_LIMIT_TYPE_ELECTRIC, index, elVal);
		setMeasureItemArray(MEASURE_LIMIT_TYPE_PHYSICAL, index, phVal);

		averageElVal += elVal;
		averagePhVal += phVal;

		QThread::msleep((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount);
	}

	averageElVal /= measureCount;
	averagePhVal /= measureCount;

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, averageElVal);
	setMeasure(MEASURE_LIMIT_TYPE_PHYSICAL, averagePhVal);

	// limits
	//
	setLimits(outParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(MEASURE_LIMIT_TYPE_ELECTRIC);
}


// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setLimits(const Metrology::SignalParam& param)
{
	setLowLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricLowLimit());
	setHighLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricHighLimit());
	setUnit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricUnit());
	setLimitPrecision(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricPrecision());

	setLowLimit(MEASURE_LIMIT_TYPE_PHYSICAL, param.physicalLowLimit());
	setHighLimit(MEASURE_LIMIT_TYPE_PHYSICAL, param.physicalHighLimit());
	setUnit(MEASURE_LIMIT_TYPE_PHYSICAL, param.physicalUnit());
	setLimitPrecision(MEASURE_LIMIT_TYPE_PHYSICAL, param.physicalPrecision());
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcError()
{
	double errorLimit = theOptions.linearity().errorLimit();

	for(int limitType = 0; limitType < MEASURE_LIMIT_TYPE_COUNT; limitType++)
	{
		setError(limitType, MEASURE_ERROR_TYPE_ABSOLUTE,		std::abs(nominal(limitType)-measure(limitType)));
		setError(limitType, MEASURE_ERROR_TYPE_REDUCE,			std::abs(((nominal(limitType)-measure(limitType)) / (highLimit(limitType) - lowLimit(limitType))) * 100.0));

		setErrorLimit(limitType, MEASURE_ERROR_TYPE_ABSOLUTE,	std::abs((highLimit(limitType) - lowLimit(limitType)) * errorLimit / 100.0));
		setErrorLimit(limitType, MEASURE_ERROR_TYPE_REDUCE,		errorLimit);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcAdditionalParam(int limitType)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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

	setAdditionalParam(MEASURE_ADDITIONAL_PARAM_SD, sco);

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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_nominal[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::nominalStr(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return QString("%1 %2").arg(QString::number(m_nominal[limitType], 10, m_limitPrecision[limitType])).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setNominal(int limitType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_nominal[limitType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::measure(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_measure[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::measureStr(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return QString("%1 %2").arg(QString::number(m_measure[limitType], 10, m_limitPrecision[limitType])).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setMeasure(int limitType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_measure[limitType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::lowLimit(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_lowLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setLowLimit(int limitType, double lowLimit)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_lowLimit[limitType] = lowLimit;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::highLimit(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_highLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setHighLimit(int limitType, double highLimit)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_highLimit[limitType] = highLimit;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::unit(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return m_unit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setUnit(int limitType, QString unit)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_unit[limitType] = unit;
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityMeasurement::limitPrecision(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_limitPrecision[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setLimitPrecision(int limitType, int precision)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_limitPrecision[limitType] = precision;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::limitStr(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	QString low = QString::number(m_lowLimit[limitType], 10, m_limitPrecision[limitType]);
	QString high = QString::number(m_highLimit[limitType], 10, m_limitPrecision[limitType]);

	return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[limitType]);;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::error(int limitType, int errotType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	int showErrorFromLimit = theOptions.linearity().showErrorFromLimit();
	if (showErrorFromLimit < 0 || showErrorFromLimit >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	QString str;

	switch(errorType)
	{
		case MEASURE_ERROR_TYPE_ABSOLUTE:	str = QString::number(m_error[limitType][errorType], 10, m_limitPrecision[showErrorFromLimit]) + " " + m_unit[showErrorFromLimit];	break;
		case MEASURE_ERROR_TYPE_REDUCE:		str = QString::number(m_error[limitType][errorType], 10, 3) + " %" ;																break;
		default:							assert(0);
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setError(int limitType, int errotType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	int showErrorFromLimit = theOptions.linearity().showErrorFromLimit();
	if (showErrorFromLimit < 0 || showErrorFromLimit >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	QString str;

	switch(errorType)
	{
		case MEASURE_ERROR_TYPE_ABSOLUTE:	str = QString::number(m_errorLimit[limitType][errorType], 10, m_limitPrecision[showErrorFromLimit]) + " " + m_unit[showErrorFromLimit];	break;
		case MEASURE_ERROR_TYPE_REDUCE:		str = QString::number(m_errorLimit[limitType][errorType], 10, 3) + " %";																break;
		default:							assert(0);
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setErrorLimit(int limitType, int errotType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
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

	LinearityMeasurement* pLinearityMeasureItem = dynamic_cast <LinearityMeasurement*> (pMeasurement);
	if (pLinearityMeasureItem == nullptr)
	{
		return;
	}

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

	LinearityMeasurement* pLinearityMeasureItem = dynamic_cast <LinearityMeasurement*> (pMeasurement);
	if (pLinearityMeasureItem == nullptr)
	{
		return;
	}

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

	for(int t = 0; t < MEASURE_LIMIT_TYPE_COUNT; t++)
	{
		m_nominal[t] = from.m_nominal[t];
		m_measure[t] = from.m_measure[t];

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

MeasureBase theMeasureBase;

// -------------------------------------------------------------------------------------------------------------------

MeasureBase::MeasureBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

 MeasureBase::~MeasureBase()
 {
 }

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::count() const
{
	int count = 0;

	m_measureMutex.lock();

		count = m_measureList.count();

	m_measureMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::clear(bool removeData)
{
	m_measureMutex.lock();

		if (removeData == true)
		{
			int count = m_measureList.count();
			for(int i = count - 1; i >= 0; i--)
			{
				Measurement* pMeasurement = m_measureList[i];
				if (pMeasurement == nullptr)
				{
					continue;
				}

				delete pMeasurement;
			}
		}

		m_measureList.clear();

	m_measureMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------
// each measurement is located in several tables,
// firstly read data from the main table, and additional sub tables in memory
// later update the data in the main table from sub tables
//
int MeasureBase::load(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return -1;
	}

	if (thePtrDB == nullptr)
	{
		return -1;
	}

	m_measureType = measureType;

	QTime responseTime;
	responseTime.start();

	struct rawTableData
	{
		int				tableType;
		Measurement*	pMeasurement;
		int				recordCount;
	};

	QVector<rawTableData> loadedTablesInMemory;

	// read all tables for current measureType in memory
	//
	for(int tableType = 0; tableType < SQL_TABLE_COUNT; tableType++)
	{
		if (SqlTableByMeasureType[tableType] == measureType)
		{
			SqlTable* table = thePtrDB->openTable(tableType);
			if (table != nullptr)
			{
				rawTableData data;

				// determine size data to allocate memory

				data.tableType = tableType;
				data.pMeasurement = nullptr;
				data.recordCount = table->recordCount();

				// allocate memory

				switch(measureType)
				{
					case MEASURE_TYPE_LINEARITY:			data.pMeasurement = new LinearityMeasurement[data.recordCount];		break;
					case MEASURE_TYPE_COMPARATOR:			data.pMeasurement = new ComparatorMeasurement[data.recordCount];	break;
					default:								assert(0);
				}

				// load data to memory

				if (data.pMeasurement != nullptr)
				{
					if (table->read(data.pMeasurement) == data.recordCount)
					{
						loadedTablesInMemory.append(data);
					}
				}

				table->close();
			}
		}
	}

	// if tables for current measureType is not exist, then exit
	//
	int tableInMemoryCount = loadedTablesInMemory.count();
	if (tableInMemoryCount == 0)
	{
		return 0;
	}

	// get main table, afterwards from sub tables update data in main table
	// append data-measurement in MeasurementBase
	//

	rawTableData mainTable = loadedTablesInMemory[SQL_TABLE_IS_MAIN];

	for(int mainIndex = 0; mainIndex < mainTable.recordCount; mainIndex++)
	{
		Measurement* pMainMeasure = mainTable.pMeasurement->at(mainIndex);
		if (pMainMeasure == nullptr)
		{
			continue;
		}

		for(int tableInMemory = SQL_TABLE_IS_SUB; tableInMemory < tableInMemoryCount; tableInMemory++)
		{
			rawTableData subTable = loadedTablesInMemory[tableInMemory];

			for(int subIndex = 0; subIndex < subTable.recordCount; subIndex++)
			{
				Measurement* pSubMeasure = subTable.pMeasurement->at(subIndex);
				if (pSubMeasure == nullptr)
				{
					continue;
				}

				// update main measurement from sub measurement
				//
				if (pMainMeasure->measureID() == pSubMeasure->measureID())
				{
					switch(subTable.tableType)
					{
						case SQL_TABLE_LINEARITY_20_EL:			static_cast<LinearityMeasurement*>(pMainMeasure)->updateMeasureArray(MEASURE_LIMIT_TYPE_ELECTRIC, pSubMeasure);	break;
						case SQL_TABLE_LINEARITY_20_PH:			static_cast<LinearityMeasurement*>(pMainMeasure)->updateMeasureArray(MEASURE_LIMIT_TYPE_PHYSICAL, pSubMeasure);	break;
						case SQL_TABLE_LINEARITY_ADD_VAL:		static_cast<LinearityMeasurement*>(pMainMeasure)->updateAdditionalParam(pSubMeasure);							break;
						case SQL_TABLE_COMPARATOR_HYSTERESIS:	static_cast<ComparatorMeasurement*>(pMainMeasure)->updateHysteresis(pSubMeasure);								break;
					}

					break;
				}
			}
		}
	}

	// append measuremets to MeasureBase from updated main table
	//
	for(int index = 0; index < mainTable.recordCount; index++)
	{
		Measurement* pMeasureTable = mainTable.pMeasurement->at(index);
		if (pMeasureTable == nullptr)
		{
			continue;
		}

		Measurement* pMeasureAppend = nullptr;

		switch(measureType)
		{
			case MEASURE_TYPE_LINEARITY:	pMeasureAppend = new LinearityMeasurement;	break;
			case MEASURE_TYPE_COMPARATOR:	pMeasureAppend = new ComparatorMeasurement;	break;
			default:						assert(0);									break;
		}

		if (pMeasureAppend == nullptr)
		{
			continue;
		}

		*pMeasureAppend = *pMeasureTable;

		append(pMeasureAppend);
	}

	// if measurement is nonexistentin in main table, but exist in sub table,
	// need remove this measurement in sub table
	// remove nonexistent indexes-measurements-ID in sub tables
	//
	for(int tableInMemory = SQL_TABLE_IS_SUB; tableInMemory < tableInMemoryCount; tableInMemory++)
	{
		rawTableData subTable = loadedTablesInMemory[tableInMemory];

		QVector<int> removeKeyList;

		for(int subIndex = 0; subIndex < subTable.recordCount; subIndex++)
		{
			Measurement* pSubMeasure = subTable.pMeasurement->at(subIndex);
			if (pSubMeasure == nullptr)
			{
				continue;
			}

			bool foundMeasure = false;

			for(int mainIndex = 0; mainIndex < mainTable.recordCount; mainIndex++)
			{
				Measurement* pMainMeasure = mainTable.pMeasurement->at(mainIndex);
				if (pMainMeasure == nullptr)
				{
					continue;
				}

				if (pMainMeasure->measureID() == pSubMeasure->measureID())
				{
					foundMeasure = true;
					break;
				}
			}

			// if measurement is not found in main table then need remove it in sub table
			//
			if (foundMeasure == false)
			{
				removeKeyList.append(pSubMeasure->measureID());
			}
		}

		// remove unnecessary measurement from sub table
		//
		SqlTable* table = thePtrDB->openTable(subTable.tableType);
		if (table != nullptr)
		{
			table->remove(removeKeyList.data(), removeKeyList.count());
			table->close();
		}
	}

	// remove raw table data from memory
	//
	for(int tableInMemory = 0; tableInMemory < tableInMemoryCount; tableInMemory++)
	{
		rawTableData table = loadedTablesInMemory[tableInMemory];

		if (table.pMeasurement == nullptr)
		{
			continue;
		}

		switch(measureType)
		{
			case MEASURE_TYPE_LINEARITY:	delete [] static_cast<LinearityMeasurement*> (table.pMeasurement);	break;
			case MEASURE_TYPE_COMPARATOR:	delete [] static_cast<ComparatorMeasurement*> (table.pMeasurement);	break;
			default:						assert(0);
		}
	}

	qDebug() << __FUNCTION__ << ": MeasureType: " << measureType << ", Loaded MeasureItem: " << count() << ", Time for load: " << responseTime.elapsed() << " ms";

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::append(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return -1;
	}

	int index = -1;

	m_measureMutex.lock();

		m_measureList.append(pMeasurement);
		index = m_measureList.count() - 1;

	m_measureMutex.unlock();

	return index;
}

// ----------------------------------------------------------------------------------------------

Measurement* MeasureBase::measurement(int index) const
{
	Measurement *pMeasure = nullptr;

	m_measureMutex.lock();

		if (index >= 0 || index < m_measureList.count())
		{
			pMeasure = m_measureList[index];
		}

	m_measureMutex.unlock();

	return pMeasure;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureBase::remove(int index, bool removeData)
{
	if (index < 0 || index >= count())
	{
		return false;
	}

	m_measureMutex.lock();

		Measurement* pMeasurement = m_measureList[index];
		if (pMeasurement != nullptr && removeData == true)
		{
			delete pMeasurement;
		}

		m_measureList.remove(index);

	m_measureMutex.unlock();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalStatistic MeasureBase::statistic(const Hash& signalHash)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return Metrology::SignalStatistic();
	}

	Metrology::SignalStatistic si(signalHash);

	m_measureMutex.lock();

		int measureCount = m_measureList.count();
		for(int i = 0; i < measureCount; i ++)
		{
			Measurement* pMeasurement = m_measureList[i];
			if (pMeasurement == 0)
			{
				continue;
			}

			if (pMeasurement->signalHash() != signalHash)
			{
				continue;
			}

			switch(pMeasurement->measureType())
			{
				case MEASURE_TYPE_LINEARITY:
					{
						LinearityMeasurement* pLinearityMeasurement = dynamic_cast<LinearityMeasurement*>(pMeasurement);
						if (pLinearityMeasurement == nullptr)
						{
							break;
						}

						int errorType = theOptions.linearity().errorType();
						if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
						{
							break;
						}

						si.measureCount()++;

						if (pLinearityMeasurement->error(MEASURE_LIMIT_TYPE_PHYSICAL, errorType) > pLinearityMeasurement->errorLimit(MEASURE_LIMIT_TYPE_PHYSICAL, errorType))
						{
							si.setState(Metrology::StatisticStateInvalid);
						}
					}
					break;

				case MEASURE_TYPE_COMPARATOR:

					// dynamic_cast<ComparatorMeasurement*> (pMeasurement); for future realese

					break;

				default:
					assert(0);
			}
		}

	m_measureMutex.unlock();

	return si;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

