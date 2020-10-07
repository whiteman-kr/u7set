#include "MeasureBase.h"

#include <QThread>

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

void Measurement::clear()
{
	m_appSignalID.clear();
	m_customAppSignalID.clear();
	m_equipmentID.clear();
	m_caption.clear();

	m_location.clear();

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
	}

	m_adjustment = 0;
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::measureTimeStr() const
{
	QString timeStr;

	timeStr = QString::asprintf("%02d-%02d-%04d %02d:%02d:%02d",

								m_measureTime.date().day(),
								m_measureTime.date().month(),
								m_measureTime.date().year(),

								m_measureTime.time().hour(),
								m_measureTime.time().minute(),
								m_measureTime.time().second());

	return timeStr;
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setLimits(const Metrology::SignalParam& param)
{
	setLowLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricLowLimit());
	setHighLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricHighLimit());
	setUnit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricUnitStr());
	setLimitPrecision(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricPrecision());

	setLowLimit(MEASURE_LIMIT_TYPE_ENGINEER, param.lowEngineeringUnits());
	setHighLimit(MEASURE_LIMIT_TYPE_ENGINEER, param.highEngineeringUnits());
	setUnit(MEASURE_LIMIT_TYPE_ENGINEER, param.unit());
	setLimitPrecision(MEASURE_LIMIT_TYPE_ENGINEER, param.decimalPlaces());
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::calcError()
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

double Measurement::nominal(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_nominal[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::nominalStr(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return QString("%1 %2").arg(QString::number(m_nominal[limitType], 'f', m_limitPrecision[limitType])).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setNominal(int limitType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_nominal[limitType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double Measurement::measure(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_measure[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::measureStr(int limitType) const
{
	if (theOptions.module().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return SignalNoValidStr;
		}
	}

	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return QString("%1 %2").arg(QString::number(m_measure[limitType], 'f', m_limitPrecision[limitType])).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setMeasure(int limitType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_measure[limitType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

double Measurement::lowLimit(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_lowLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setLowLimit(int limitType, double lowLimit)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_lowLimit[limitType] = lowLimit;
}

// -------------------------------------------------------------------------------------------------------------------

double Measurement::highLimit(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_highLimit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setHighLimit(int limitType, double highLimit)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_highLimit[limitType] = highLimit;
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::unit(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	return m_unit[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setUnit(int limitType, QString unit)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_unit[limitType] = unit;
}

// -------------------------------------------------------------------------------------------------------------------

int Measurement::limitPrecision(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_limitPrecision[limitType];
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setLimitPrecision(int limitType, int precision)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_limitPrecision[limitType] = precision;
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::limitStr(int limitType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	QString low = QString::number(m_lowLimit[limitType], 'f', m_limitPrecision[limitType]);
	QString high = QString::number(m_highLimit[limitType], 'f', m_limitPrecision[limitType]);

	return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[limitType]);
}

// -------------------------------------------------------------------------------------------------------------------

double Measurement::error(int limitType, int errotType) const
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

QString Measurement::errorStr() const
{
	if (theOptions.module().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return SignalNoValidStr;
		}
	}

	int limitType = theOptions.linearity().limitType();
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

	QString str;

	switch(errorType)
	{
		case MEASURE_ERROR_TYPE_ABSOLUTE:	str = QString::number(m_error[limitType][errorType], 'f', m_limitPrecision[limitType]) + " " + m_unit[limitType];	break;
		case MEASURE_ERROR_TYPE_REDUCE:		str = QString::number(m_error[limitType][errorType], 'f', 3) + " %" ;												break;
		default:							assert(0);
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setError(int limitType, int errotType, double value)
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

double Measurement::errorLimit(int limitType, int errotType) const
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

QString Measurement::errorLimitStr() const
{
	int limitType = theOptions.linearity().limitType();
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

	QString str;

	switch(errorType)
	{
		case MEASURE_ERROR_TYPE_ABSOLUTE:	str = QString::number(m_errorLimit[limitType][errorType], 'f', m_limitPrecision[limitType]) + " " + m_unit[limitType];	break;
		case MEASURE_ERROR_TYPE_REDUCE:		str = QString::number(m_errorLimit[limitType][errorType], 'f', 3) + " %";												break;
		default:							assert(0);
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setErrorLimit(int limitType, int errotType, double value)
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

int Measurement::errorResult() const
{
	int limitType = theOptions.linearity().limitType();
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return MEASURE_ERROR_RESULT_UNKNOWN;
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return MEASURE_ERROR_RESULT_UNKNOWN;
	}

	if (m_error[limitType][errorType] > m_errorLimit[limitType][errorType])
	{
		return MEASURE_ERROR_RESULT_FAILED;
	}

	return MEASURE_ERROR_RESULT_OK;
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::errorResultStr() const
{
	if (theOptions.module().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return SignalNoValidStr;
		}
	}

	int errResult = errorResult();
	if (errResult < 0 || errResult > MEASURE_ERROR_RESULT_COUNT)
	{
		return QString();
	}

	return ErrorResult[errResult];
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

	m_signalValid = from.m_signalValid;

	m_measureTime = from.m_measureTime;
	m_reportType = from.m_reportType;

	m_appSignalID = from.m_appSignalID;
	m_customAppSignalID = from.m_customAppSignalID;
	m_equipmentID = from.m_equipmentID;
	m_caption = from.m_caption;

	m_location = from.m_location;

	for(int t = 0; t < MEASURE_LIMIT_TYPE_COUNT; t++)
	{
		m_nominal[t] = from.m_nominal[t];
		m_measure[t] = from.m_measure[t];

		m_lowLimit[t] = from.m_lowLimit[t];
		m_highLimit[t] = from.m_highLimit[t];
		m_unit[t] = from.m_unit[t];
		m_limitPrecision[t] = from.m_limitPrecision[t];

		for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
		{
			m_error[t][e] = from.m_error[t][e];
			m_errorLimit[t][e] = from.m_errorLimit[t][e];
		}
	}

	m_adjustment = from.m_adjustment;

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

LinearityMeasurement::LinearityMeasurement() : Measurement(MEASURE_TYPE_LINEARITY)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement::LinearityMeasurement(const IoSignalParam &ioParam) : Measurement(MEASURE_TYPE_LINEARITY)
{
	clear();

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	if (ioParam.isValid() == false)
	{
		return;
	}

	int signalConnectionType = ioParam.signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	switch (signalConnectionType)
	{
		case SIGNAL_CONNECTION_TYPE_UNUSED:					fill_measure_input(ioParam);	break;
		case SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL:
		case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:	fill_measure_internal(ioParam);	break;
		case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_OUTPUT:
		case SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT:			fill_measure_output(ioParam);	break;
		default:											assert(0);
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

	m_percent = 0;

	for(int t = 0; t < MEASURE_LIMIT_TYPE_COUNT; t++)
	{
		for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
		{
			m_measureArray[t][m] = 0;
		}
	}

	m_measureCount = 0;

	m_additionalParamCount = 0;

	for(int l = 0; l < MEASURE_LIMIT_TYPE_COUNT; l++)
	{
		for(int a = 0; a < MEASURE_ADDITIONAL_PARAM_COUNT; a++)
		{
			m_additionalParam[l][a] = 0;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::fill_measure_input(const IoSignalParam &ioParam)
{
	if (ioParam.isValid() == false)
	{
		assert(false);
		return;
	}

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		assert(false);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
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
	setEquipmentID(inParam.equipmentID());
	setCaption(inParam.caption());

	if (inParam.location().moduleSerialNoID().isEmpty() == false)
	{
		Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
		const Metrology::SignalState& signalState = theSignalBase.signalState(serialNumberModuleHash);
		if (signalState.valid() == true)
		{
			inParam.location().setModuleSerialNo(static_cast<int>(signalState.value()));
		}
	}

	setLocation(inParam.location());

	// nominal
	//

	double electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
	double engineering = conversion(electric, CT_ELECTRIC_TO_ENGINEER, inParam);

	setPercent(((engineering - inParam.lowEngineeringUnits()) * 100)/(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()));

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// measure
	//

	setSignalValid(theSignalBase.signalState(inParam.hash()).valid());

	double averageElVal = 0;
	double averageEnVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		double enVal = theSignalBase.signalState(inParam.hash()).value();
		double elVal = conversion(enVal, CT_ENGINEER_TO_ELECTRIC, inParam);

		setMeasureItemArray(MEASURE_LIMIT_TYPE_ELECTRIC, index, elVal);
		setMeasureItemArray(MEASURE_LIMIT_TYPE_ENGINEER, index, enVal);

		averageElVal += elVal;
		averageEnVal += enVal;

		QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount));
	}

	averageElVal /= measureCount;
	averageEnVal /= measureCount;

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, averageElVal);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, averageEnVal);

	// limits
	//
	setLimits(inParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(pCalibrator, inParam.electricSensorType(), MEASURE_LIMIT_TYPE_ELECTRIC);
	calcAdditionalParam(pCalibrator, inParam.electricSensorType(), MEASURE_LIMIT_TYPE_ENGINEER);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::fill_measure_internal(const IoSignalParam &ioParam)
{
	if (ioParam.isValid() == false)
	{
		assert(false);
		return;
	}

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		assert(false);
		return;
	}

	int signalConnectionType = ioParam.signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	Metrology::SignalParam outParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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
	setEquipmentID(outParam.equipmentID());
	setCaption(outParam.caption());

	if (inParam.location().moduleSerialNoID().isEmpty() == false)
	{
		Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
		Metrology::SignalState signalState = theSignalBase.signalState(serialNumberModuleHash);
		if (signalState.valid() == true)
		{
			inParam.location().setModuleSerialNo(static_cast<int>(signalState.value()));
		}
	}

	setLocation(inParam.location());

	// nominal
	//

	double engineering = (ioParam.percent() * (inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) / 100) + inParam.lowEngineeringUnits();
	double electric = conversion(engineering, CT_ENGINEER_TO_ELECTRIC, inParam);
	double engineeringCalc = conversionCalcVal(engineering, CT_CALC_VAL_NORMAL, ioParam.signalConnectionType(), ioParam);

	setPercent(ioParam.percent());

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineeringCalc);

	// measure
	//

	setSignalValid(theSignalBase.signalState(outParam.hash()).valid());

	double averageElVal = 0;
	double averagePhVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		double enVal = theSignalBase.signalState(outParam.hash()).value();
		double enCalcVal = conversionCalcVal(enVal, CT_CALC_VAL_INVERSION, ioParam.signalConnectionType(), ioParam);
		double elVal = conversion(enCalcVal, CT_ENGINEER_TO_ELECTRIC, inParam);

		setMeasureItemArray(MEASURE_LIMIT_TYPE_ELECTRIC, index, elVal);
		setMeasureItemArray(MEASURE_LIMIT_TYPE_ENGINEER, index, enVal);

		averageElVal += elVal;
		averagePhVal += enVal;

		QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount));
	}

	averageElVal /= measureCount;
	averagePhVal /= measureCount;

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, averageElVal);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, averagePhVal);

	// limits
	//
	outParam.setElectricLowLimit(inParam.electricLowLimit());
	outParam.setElectricHighLimit(inParam.electricHighLimit());
	outParam.setElectricUnitID(inParam.electricUnitID());
	outParam.setElectricPrecision(inParam.electricPrecision());

	setLimits(outParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(pCalibrator, inParam.electricSensorType(), MEASURE_LIMIT_TYPE_ELECTRIC);
	calcAdditionalParam(pCalibrator, inParam.electricSensorType(), MEASURE_LIMIT_TYPE_ENGINEER);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::fill_measure_output(const IoSignalParam &ioParam)
{
	if (ioParam.isValid() == false)
	{
		assert(false);
		return;
	}

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		assert(false);
		return;
	}

	int signalConnectionType = ioParam.signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& outParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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
	setEquipmentID(outParam.equipmentID());
	setCaption(outParam.caption());

	if (outParam.location().moduleSerialNoID().isEmpty() == false)
	{
		Hash serialNumberModuleHash = calcHash(outParam.location().moduleSerialNoID());
		Metrology::SignalState signalState = theSignalBase.signalState(serialNumberModuleHash);
		if (signalState.valid() == true)
		{
			outParam.location().setModuleSerialNo(static_cast<int>(signalState.value()));
		}
	}

	setLocation(outParam.location());

	// nominal
	//

	double engineering = (ioParam.percent() * (outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) / 100) + outParam.lowEngineeringUnits();
	double engineeringCalc = conversionCalcVal(engineering, CT_CALC_VAL_NORMAL, ioParam.signalConnectionType(), ioParam);
	double electric = conversion(engineeringCalc, CT_ENGINEER_TO_ELECTRIC, outParam);

	setPercent(ioParam.percent());

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineeringCalc);

	// measure
	//

	setSignalValid(theSignalBase.signalState(outParam.hash()).valid());

	double averageElVal = 0;
	double averagePhVal = 0;

	int measureCount = theOptions.linearity().measureCountInPoint();

	setMeasureCount(measureCount);

	for(int index = 0; index < measureCount; index++)
	{
		double elVal = 0;
		double enVal = theSignalBase.signalState(outParam.hash()).value();

		if (outParam.isOutput() == true)
		{
			ioParam.calibratorManager()->value();
			ioParam.calibratorManager()->waitReadyForManage();

			elVal = pCalibrator->measureValue();
		}

		setMeasureItemArray(MEASURE_LIMIT_TYPE_ELECTRIC, index, elVal);
		setMeasureItemArray(MEASURE_LIMIT_TYPE_ENGINEER, index, enVal);

		averageElVal += elVal;
		averagePhVal += enVal;

		QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount));
	}

	averageElVal /= measureCount;
	averagePhVal /= measureCount;

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, averageElVal);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, averagePhVal);

	// limits
	//
	setLimits(outParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(pCalibrator, outParam.electricSensorType(), MEASURE_LIMIT_TYPE_ELECTRIC);
	calcAdditionalParam(pCalibrator, outParam.electricSensorType(), MEASURE_LIMIT_TYPE_ENGINEER);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcAdditionalParam(Calibrator* pCalibrator, E::SensorType sensorType, int limitType)
{
	if (pCalibrator == nullptr)
	{
		return;
	}

	int сalibratorType = pCalibrator->type();
	if (сalibratorType < 0 || сalibratorType >= CALIBRATOR_TYPE_COUNT)
	{
		return;
	}

	int сalibratorSiurceUnit = pCalibrator->sourceUnit();
	if (сalibratorSiurceUnit < 0 || сalibratorSiurceUnit >= CALIBRATOR_UNIT_COUNT)
	{
		return;
	}

	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		return;
	}

	// calc additional parameters
	//

	setAdditionalParamCount(MEASURE_ADDITIONAL_PARAM_COUNT);

		// max deviation
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

	setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_MAX_VALUE, measureItemArray(limitType, maxDeviationIndex));


		// according to GOST 8.508-84 paragraph 3.4.1 formula 42
		//
	double systemError = std::abs(measure(limitType) - nominal(limitType));

	setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR, systemError);


		// according to GOST 8.736-2011 paragraph 5.3 formula 3
		//
	double sumDeviation = 0;

	for(int index = 0; index < measureCount(); index++)
	{
		qDebug() << measure(limitType);
		qDebug() << measureItemArray(limitType, index);
		qDebug() << pow(measure(limitType) - measureItemArray(limitType, index), 2);

		sumDeviation += pow(measure(limitType) - measureItemArray(limitType, index), 2);		// 1. sum of deviations

		qDebug() << sumDeviation;
	}

	sumDeviation /= static_cast<double>(measureCount() - 1);									// 2. divide on (count of measure - 1)
	double sco = sqrt(sumDeviation);															// 3. sqrt

	setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_SD, sco);


		// according to GOST 8.207-76 paragraph 2.4
		//
	double estimateSCO = sco / sqrt(static_cast<double>(measureCount()));

		// Student's rate according to GOST 27.202 on P = 0.95
		// or GOST 8.207-76 application 2 (last page)
		//
	double k_student = studentK(measureCount(), CT_PROPABILITY_95);


		// according to GOST 8.207-76 paragraph 3.2
		//
	double border = k_student * estimateSCO;

	setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER, border);


		// Uncertainty of measurement to Document: EA-04/02 M:2013
		//
	double uncertainty = 0;

	switch (limitType)
	{
		case MEASURE_LIMIT_TYPE_ELECTRIC:
			{
				double Kox = 2;

				double Kx = 0;

				if ( (сalibratorSiurceUnit == CALIBRATOR_UNIT_MV  &&  (sensorType == E::SensorType::mV_Raw_Mul_8 || sensorType == E::SensorType::mV_Raw_Mul_32)) ||
					((сalibratorSiurceUnit == CALIBRATOR_UNIT_LOW_OHM || сalibratorSiurceUnit == CALIBRATOR_UNIT_HIGH_OHM)  &&  sensorType == E::SensorType::Ohm_Raw) )
				{
					// for non-linear electrical ranges (mV and ohms) Kx is calculated differently
					//
					Kx = nominal(MEASURE_LIMIT_TYPE_ENGINEER) / nominal(MEASURE_LIMIT_TYPE_ELECTRIC);
				}
				else
				{
					// for linear electrical ranges (mA and V) Kx is calculated differently
					//
					Kx = (highLimit(MEASURE_LIMIT_TYPE_ENGINEER) - lowLimit(MEASURE_LIMIT_TYPE_ENGINEER)) / (highLimit(MEASURE_LIMIT_TYPE_ELECTRIC) - lowLimit(MEASURE_LIMIT_TYPE_ELECTRIC));
				}

				double dI = (	CalibratorTS[сalibratorType][сalibratorSiurceUnit][CALIBRATION_TS_AC0] * nominal(MEASURE_LIMIT_TYPE_ELECTRIC) +
								CalibratorTS[сalibratorType][сalibratorSiurceUnit][CALIBRATION_TS_AC1] *
								CalibratorTS[сalibratorType][сalibratorSiurceUnit][CALIBRATION_TS_RANGE]) / 100.0;

				double MPx = 1 / pow(10.0, limitPrecision(MEASURE_LIMIT_TYPE_ENGINEER));

				//
				//
				uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kx,2) * pow(dI,2) / 3) + (pow(MPx,2) / 3) );
			}
			break;

		case MEASURE_LIMIT_TYPE_ENGINEER:
			{
				double Kox = 2;

				double Kx = 0;

				if ( (сalibratorSiurceUnit == CALIBRATOR_UNIT_MV  &&  (sensorType == E::SensorType::mV_Raw_Mul_8 || sensorType == E::SensorType::mV_Raw_Mul_32)) ||
					((сalibratorSiurceUnit == CALIBRATOR_UNIT_LOW_OHM || сalibratorSiurceUnit == CALIBRATOR_UNIT_HIGH_OHM)  &&  sensorType == E::SensorType::Ohm_Raw) )
				{
					// for non-linear electrical ranges (mV and ohms) Kx is calculated differently
					//
					Kx = nominal(MEASURE_LIMIT_TYPE_ENGINEER) / nominal(MEASURE_LIMIT_TYPE_ELECTRIC);
				}
				else
				{
					// for linear electrical ranges (mA and V) Kx is calculated differently
					//
					Kx = (highLimit(MEASURE_LIMIT_TYPE_ENGINEER) - lowLimit(MEASURE_LIMIT_TYPE_ENGINEER)) / (highLimit(MEASURE_LIMIT_TYPE_ELECTRIC) - lowLimit(MEASURE_LIMIT_TYPE_ELECTRIC));
				}

				double dI = (	CalibratorTS[сalibratorType][сalibratorSiurceUnit][CALIBRATION_TS_AC0] * nominal(MEASURE_LIMIT_TYPE_ELECTRIC) +
								CalibratorTS[сalibratorType][сalibratorSiurceUnit][CALIBRATION_TS_AC1] *
								CalibratorTS[сalibratorType][сalibratorSiurceUnit][CALIBRATION_TS_RANGE]) / 100.0;

				double MPx = 1 / pow(10.0, limitPrecision(MEASURE_LIMIT_TYPE_ENGINEER));

				//
				//
				uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kx,2) * pow(dI,2) / 3) + (pow(MPx,2) / 3) );
			}

			break;

		default:
			assert(0);
			break;
	}

	setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_UNCERTAINTY, uncertainty);
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
	if (theOptions.module().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return SignalNoValidStr;
		}
	}

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

	return QString::number(m_measureArray[limitType][index], 'f', limitPrecision(limitType));
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

double LinearityMeasurement::additionalParam(int limitType, int paramType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return 0;
	}

	if (paramType < 0 || paramType >= MEASURE_ADDITIONAL_PARAM_COUNT)
	{
		assert(0);
		return 0;
	}

	return m_additionalParam[limitType][paramType];
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityMeasurement::additionalParamStr(int limitType, int paramType) const
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	if (theOptions.module().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return SignalNoValidStr;
		}
	}

	if (paramType < 0 || paramType >= MEASURE_ADDITIONAL_PARAM_COUNT)
	{
		assert(0);
		return QString();
	}

	return QString::number(m_additionalParam[limitType][paramType], 'f', 2);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::setAdditionalParam(int limitType, int paramType, double value)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	if (paramType < 0 || paramType >= MEASURE_ADDITIONAL_PARAM_COUNT)
	{
		assert(0);
		return;
	}

	m_additionalParam[limitType][paramType] = value;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::updateMeasureArray(int limitType, Measurement* pMeasurement)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		return;
	}

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

void LinearityMeasurement::updateAdditionalParam(int limitType, Measurement* pMeasurement)
{
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		return;
	}

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
		m_additionalParam[limitType][a] = pLinearityMeasureItem->additionalParam(limitType, a);
	}
}

// -------------------------------------------------------------------------------------------------------------------

LinearityMeasurement& LinearityMeasurement::operator=(const LinearityMeasurement& from)
{
	m_percent = from.m_percent;

	for(int t = 0; t < MEASURE_LIMIT_TYPE_COUNT; t++)
	{
		for(int m = 0; m < MAX_MEASUREMENT_IN_POINT; m++)
		{
			m_measureArray[t][m] = from.m_measureArray[t][m];
		}
	}

	m_measureCount = from.m_measureCount;

	m_additionalParamCount = from.m_additionalParamCount;

	for(int l = 0; l < MEASURE_LIMIT_TYPE_COUNT; l++)
	{
		for(int a = 0; a < MEASURE_ADDITIONAL_PARAM_COUNT; a++)
		{
			m_additionalParam[l][a] = from.m_additionalParam[l][a];
		}
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement::ComparatorMeasurement() : Measurement(MEASURE_TYPE_COMPARATOR)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement::ComparatorMeasurement(const IoSignalParam& ioParam) : Measurement(MEASURE_TYPE_COMPARATOR)
{
	clear();

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	if (ioParam.isValid() == false)
	{
		return;
	}

	int signalConnectionType = ioParam.signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	switch (signalConnectionType)
	{
		case SIGNAL_CONNECTION_TYPE_UNUSED:					fill_measure_input(ioParam);	break;
		case SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL:
		case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:	fill_measure_internal(ioParam);	break;
		default:											assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement::~ComparatorMeasurement()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasurement::clear()
{
	setMeasureType(MEASURE_TYPE_COMPARATOR);

	m_compareAppSignalID.clear();
	m_outputAppSignalID.clear();

	m_cmpValueType = Metrology::CmpValueTypeSetPoint;
	m_cmpType = E::CmpType::Greate;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasurement::fill_measure_input(const IoSignalParam &ioParam)
{
	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		assert(false);
		return;
	}

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	if (ioParam.isValid() == false)
	{
		assert(false);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	int comparatorIndex = ioParam.comparatorIndex();
	if (comparatorIndex < 0 || comparatorIndex >= inParam.comparatorCount())
	{
		assert(false);
		return;
	}

	int cmpValueType = ioParam.comparatorValueType();
	if (cmpValueType < 0 || cmpValueType >= Metrology::CmpValueTypeCount)
	{
		assert(false);
		return;
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = inParam.comparator(comparatorIndex);
	if (comparatorEx == nullptr)
	{
		assert(false);
		return;
	}

	if (comparatorEx->signalsIsValid() == false)
	{
		assert(false);
		return;
	}

	//
	//

	setMeasureType(MEASURE_TYPE_COMPARATOR);
	setSignalHash(inParam.hash());

	// features
	//

	setAppSignalID(inParam.appSignalID());
	setCustomAppSignalID(inParam.customAppSignalID());
	setEquipmentID(inParam.equipmentID());
	setCaption(inParam.caption());

	if (inParam.location().moduleSerialNoID().isEmpty() == false)
	{
		Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
		Metrology::SignalState signalState = theSignalBase.signalState(serialNumberModuleHash);
		if (signalState.valid() == true)
		{
			inParam.location().setModuleSerialNo(static_cast<int>(signalState.value()));
		}
	}

	setLocation(inParam.location());

	if (comparatorEx->compare().isConst() == false)
	{
		setCompareAppSignalID(comparatorEx->compare().appSignalID());
	}

	setOutputAppSignalID(comparatorEx->output().appSignalID());

	// nominal
	//

	setCmpValueType(cmpValueType);
	setCmpType(cmpValueType, comparatorEx->cmpType());

	double engineering = comparatorEx->compareOnlineValue(cmpValueType);
	double electric = conversion(engineering, CT_ENGINEER_TO_ELECTRIC, inParam);

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// measure
	//

	setSignalValid(theSignalBase.signalState(inParam.hash()).valid());
	setSignalValid(true);

	electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
	engineering = conversion(electric, CT_ELECTRIC_TO_ENGINEER, inParam);

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// limits
	//
	setLimits(inParam);

	// calc errors
	//
	calcError();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasurement::fill_measure_internal(const IoSignalParam &ioParam)
{
	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		assert(false);
		return;
	}

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return;
	}

	if (ioParam.isValid() == false)
	{
		assert(false);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	Metrology::SignalParam outParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
	if (outParam.isValid() == false)
	{
		assert(false);
		return;
	}

	int comparatorIndex = ioParam.comparatorIndex();
	if (comparatorIndex < 0 || comparatorIndex >= outParam.comparatorCount())
	{
		assert(false);
		return;
	}

	int cmpValueType = ioParam.comparatorValueType();
	if (cmpValueType < 0 || cmpValueType >= Metrology::CmpValueTypeCount)
	{
		assert(false);
		return;
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = outParam.comparator(comparatorIndex);
	if (comparatorEx == nullptr)
	{
		assert(false);
		return;
	}

	if (comparatorEx->signalsIsValid() == false)
	{
		assert(false);
		return;
	}

	//
	//

	setMeasureType(MEASURE_TYPE_COMPARATOR);
	setSignalHash(outParam.hash());

	// features
	//

	setAppSignalID(outParam.appSignalID());
	setCustomAppSignalID(outParam.customAppSignalID());
	setEquipmentID(outParam.equipmentID());
	setCaption(outParam.caption());

	if (inParam.location().moduleSerialNoID().isEmpty() == false)
	{
		Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
		Metrology::SignalState signalState = theSignalBase.signalState(serialNumberModuleHash);
		if (signalState.valid() == true)
		{
			inParam.location().setModuleSerialNo(static_cast<int>(signalState.value()));
		}
	}

	setLocation(inParam.location());

	if (comparatorEx->compare().isConst() == false)
	{
		setCompareAppSignalID(comparatorEx->compare().appSignalID());
	}

	setOutputAppSignalID(comparatorEx->output().appSignalID());

	// nominal
	//

	setCmpValueType(cmpValueType);
	setCmpType(cmpValueType, comparatorEx->cmpType());

	double engineering = comparatorEx->compareOnlineValue(cmpValueType);
	double engineeringCalc = conversionCalcVal(engineering, CT_CALC_VAL_INVERSION, ioParam.signalConnectionType(), ioParam);
	double electric = conversion(engineeringCalc, CT_ENGINEER_TO_ELECTRIC, inParam);

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// measure
	//

	setSignalValid(theSignalBase.signalState(outParam.hash()).valid());
	setSignalValid(true);

	electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
	engineering = conversion(electric, CT_ELECTRIC_TO_ENGINEER, inParam);
	engineeringCalc = conversionCalcVal(engineering, CT_CALC_VAL_NORMAL, ioParam.signalConnectionType(), ioParam);

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, engineeringCalc);

	// limits
	//
	outParam.setElectricLowLimit(inParam.electricLowLimit());
	outParam.setElectricHighLimit(inParam.electricHighLimit());
	outParam.setElectricUnitID(inParam.electricUnitID());
	outParam.setElectricPrecision(inParam.electricPrecision());

	setLimits(outParam);

	// calc errors
	//
	calcError();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorMeasurement::cmpValueTypeStr() const
{
	if (m_cmpValueType < 0 || m_cmpValueType >= Metrology::CmpValueTypeCount)
	{
		assert(0);
		return QString("N/A");
	}

	return Metrology::CmpValueType[m_cmpValueType];
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorMeasurement::cmpTypeStr() const
{
	QString typeStr;

	switch (m_cmpType)
	{
		case E::CmpType::Greate:	typeStr = ">";	break;
		case E::CmpType::Less:		typeStr = "<";	break;
	}

	return typeStr;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasurement::setCmpType(int cmpValueType, E::CmpType cmpType)
{
	if (cmpValueType < 0 || cmpValueType >= Metrology::CmpValueTypeCount)
	{
		assert(0);
		return;
	}

	switch (cmpValueType)
	{
		case Metrology::CmpValueTypeSetPoint:

			m_cmpType = cmpType;

			break;

		case Metrology::CmpValueTypeHysteresis:

			switch (cmpType)
			{
				case E::CmpType::Less:		m_cmpType = E::CmpType::Greate;	break;
				case E::CmpType::Greate:	m_cmpType = E::CmpType::Less;	break;
			}

			break;

		default:
			assert(0);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasurement& ComparatorMeasurement::operator=(const ComparatorMeasurement& from)
{
	m_compareAppSignalID = from.m_compareAppSignalID;
	m_outputAppSignalID = from.m_outputAppSignalID;

	m_cmpValueType = from.m_cmpValueType;
	m_cmpType = from.m_cmpType;

	return *this;
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
	QMutexLocker locker(&m_measureMutex);

	return m_measureList.count();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::clear(bool removeData)
{
	QMutexLocker locker(&m_measureMutex);

	if (removeData == true)
	{
		for(Measurement* pMeasurement: m_measureList)
		{
			if (pMeasurement == nullptr)
			{
				continue;
			}

			delete pMeasurement;
		}
	}

	m_measureList.clear();
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

	m_measureType = measureType;

	QElapsedTimer responseTime;
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
			SqlTable* table = theDatabase.openTable(tableType);
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
					case MEASURE_TYPE_LINEARITY:			data.pMeasurement = new LinearityMeasurement[static_cast<quint64>(data.recordCount)];	break;
					case MEASURE_TYPE_COMPARATOR:			data.pMeasurement = new ComparatorMeasurement[static_cast<quint64>(data.recordCount)];	break;
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
						case SQL_TABLE_LINEARITY_20_EL:			static_cast<LinearityMeasurement*>(pMainMeasure)->updateMeasureArray(MEASURE_LIMIT_TYPE_ELECTRIC, pSubMeasure);		break;
						case SQL_TABLE_LINEARITY_20_EN:			static_cast<LinearityMeasurement*>(pMainMeasure)->updateMeasureArray(MEASURE_LIMIT_TYPE_ENGINEER, pSubMeasure);		break;
						case SQL_TABLE_LINEARITY_ADD_VAL_EL:	static_cast<LinearityMeasurement*>(pMainMeasure)->updateAdditionalParam(MEASURE_LIMIT_TYPE_ELECTRIC, pSubMeasure);	break;
						case SQL_TABLE_LINEARITY_ADD_VAL_EN:	static_cast<LinearityMeasurement*>(pMainMeasure)->updateAdditionalParam(MEASURE_LIMIT_TYPE_ENGINEER, pSubMeasure);	break;
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
		SqlTable* table = theDatabase.openTable(subTable.tableType);
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

	emit updatedMeasureBase(pMeasurement->signalHash());

	return index;
}

// ----------------------------------------------------------------------------------------------

Measurement* MeasureBase::measurement(int index) const
{
	QMutexLocker locker(&m_measureMutex);

	if (index < 0 || index >= m_measureList.count())
	{
		return nullptr;
	}

	return m_measureList[index];
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureBase::remove(int index, bool removeData)
{
	if (index < 0 || index >= count())
	{
		return false;
	}

	Hash signalHash = UNDEFINED_HASH;

	m_measureMutex.lock();

		Measurement* pMeasurement = m_measureList[index];
		if (pMeasurement != nullptr)
		{
			signalHash = pMeasurement->signalHash();

			if (removeData == true)
			{
				delete pMeasurement;
			}
		}

		m_measureList.remove(index);

	m_measureMutex.unlock();

	emit updatedMeasureBase(signalHash);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::remove(int measureType, const QVector<int>& keyList)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	int keyCount = keyList.count();
	if (keyCount == 0)
	{
		return;
	}

	int measureCount = count();
	if (measureCount == 0)
	{
		return;
	}

	for(int k = 0; k < keyCount; k++)
	{
		for(int i = measureCount - 1; i >= 0; i--)
		{
			Measurement* pMeasurement = measurement(i);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			if (pMeasurement->measureType() != measureType)
			{
				continue;
			}

			if (pMeasurement->measureID() != keyList[k])
			{
				continue;
			}

			remove(i);

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::updateStatistics(int measureType, StatisticItem& si)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	Metrology::Signal* pSignal = si.signal();
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	Hash signalHash = si.signal()->param().hash();
	if (signalHash == UNDEFINED_HASH)
	{
		assert(signalHash != UNDEFINED_HASH);
		return;
	}

	int limitType = theOptions.linearity().limitType();
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	QMutexLocker l(&m_measureMutex);

	si.setMeasureCount(0);
	si.setState(StatisticItem::State::Success);

	int measureCount = m_measureList.count();
	for(int i = 0; i < measureCount; i ++)
	{
		Measurement* pMeasurement = m_measureList[i];
		if (pMeasurement == nullptr)
		{
			continue;
		}

		if (pMeasurement->measureType() != measureType)
		{
			continue;
		}

		if (pMeasurement->signalHash() != signalHash)
		{
			continue;
		}

		switch(measureType)
		{
			case MEASURE_TYPE_LINEARITY:
			{
				LinearityMeasurement* pLinearityMeasurement = dynamic_cast<LinearityMeasurement*>(pMeasurement);
				if (pLinearityMeasurement == nullptr)
				{
					break;
				}

				si.setMeasureCount(si.measureCount() + 1);

				if (pLinearityMeasurement->error(limitType, errorType) > pLinearityMeasurement->errorLimit(limitType, errorType))
				{
					si.setState(StatisticItem::State::Failed);
				}
			}

			break;

			case MEASURE_TYPE_COMPARATOR:
			{
				ComparatorMeasurement* pComparatorMeasurement = dynamic_cast<ComparatorMeasurement*>(pMeasurement);
				if (pComparatorMeasurement == nullptr)
				{
					break;
				}

				if (pComparatorMeasurement->cmpValueType() == Metrology::CmpValueTypeHysteresis)
				{
					break;
				}

				std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
				if (comparatorEx == nullptr)
				{
					break;
				}

				if (comparatorEx->cmpType() != pComparatorMeasurement->cmpType())
				{
					break;
				}

				if (comparatorEx->compare().isConst() == true)
				{
					if (compareDouble(comparatorEx->compareConstValue(), pComparatorMeasurement->nominal(MEASURE_LIMIT_TYPE_ENGINEER)) == false)
					{
						break;
					}
				}
				else
				{
					if (comparatorEx->output().appSignalID() != pComparatorMeasurement->outputAppSignalID())
					{
						break;
					}
				}

				si.setMeasureCount(si.measureCount() + 1);

				if (pComparatorMeasurement->error(limitType, errorType) > pComparatorMeasurement->errorLimit(limitType, errorType))
				{
					si.setState(StatisticItem::State::Failed);
				}
			}

			break;

			default:
				assert(0);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

