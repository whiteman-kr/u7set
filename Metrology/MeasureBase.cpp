#include "MeasureBase.h"

#include <QThread>
#include <QtConcurrent>
#include <QMessageBox>

#include "../lib/UnitsConvertor.h"

#include "Database.h"
#include "Conversion.h"
#include "Options.h"

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
	m_measureType = MEASURE_TYPE_UNDEFINED;
	m_signalHash = UNDEFINED_HASH;

	m_measureID = -1;
	m_filter = false;

	m_signalValid = true;

	//
	//
	m_connectionAppSignalID.clear();
	m_connectionType = Metrology::ConnectionType::Unknown;

	m_appSignalID.clear();
	m_customAppSignalID.clear();
	m_equipmentID.clear();
	m_caption.clear();

	m_location.clear();

	m_calibratorPrecision = DEFAULT_ECLECTRIC_UNIT_PRECESION;

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

	//
	//
	m_measureTime.setTime_t(0);
	m_calibrator.clear();
	m_reportType = -1;

	m_foundInStatistics = true;
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

QString Measurement::connectionAppSignalID() const
{
	if (m_connectionType == Metrology::ConnectionType::Unsed)
	{
		return QString();
	}

	return m_connectionAppSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

QString Measurement::connectionTypeStr() const
{
	if (m_connectionType < 0 || m_connectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return QString("???");
	}

	if (m_connectionType == Metrology::ConnectionType::Unsed)
	{
		return QString();
	}

	return Metrology::ConnectionTypeCaption(m_connectionType);
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
	int precision = DEFAULT_ECLECTRIC_UNIT_PRECESION;

	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	precision = limitPrecision(limitType);

	if (theOptions.measureView().precesionByCalibrator() == true)
	{
		if (limitType == MEASURE_LIMIT_TYPE_ELECTRIC)
		{
			precision = calibratorPrecision();
		}
	}

	return QString("%1 %2").arg(QString::number(m_nominal[limitType], 'f', precision)).arg(m_unit[limitType]);
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
	if (theOptions.measureView().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
		}
	}

	int precision = DEFAULT_ECLECTRIC_UNIT_PRECESION;

	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	precision = limitPrecision(limitType);

	if (theOptions.measureView().precesionByCalibrator() == true)
	{
		if (limitType == MEASURE_LIMIT_TYPE_ELECTRIC)
		{
			precision = calibratorPrecision();
		}
	}

	return QString("%1 %2").arg(QString::number(m_measure[limitType], 'f', precision)).arg(m_unit[limitType]);
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

void Measurement::setLimits(const IoSignalParam& ioParam)
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	switch (signalConnectionType)
	{
		case Metrology::ConnectionType::Unsed:
			{
			    const Metrology::SignalParam& param = ioParam.param(Metrology::ConnectionIoType::Source);
				if (param.isValid() == false)
				{
					assert(false);
					break;
				}

				setLowLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricLowLimit());
				setHighLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricHighLimit());
				setUnit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricUnitStr());
				setLimitPrecision(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricPrecision());

				setLowLimit(MEASURE_LIMIT_TYPE_ENGINEER, param.lowEngineeringUnits());
				setHighLimit(MEASURE_LIMIT_TYPE_ENGINEER, param.highEngineeringUnits());
				setUnit(MEASURE_LIMIT_TYPE_ENGINEER, param.unit());
				setLimitPrecision(MEASURE_LIMIT_TYPE_ENGINEER, param.decimalPlaces());
			}
			break;

		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			{
			    const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
				if (inParam.isValid() == false)
				{
					assert(false);
					break;
				}

				const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
				if (outParam.isValid() == false)
				{
					assert(false);
					break;
				}

				setLowLimit(MEASURE_LIMIT_TYPE_ELECTRIC, inParam.electricLowLimit());
				setHighLimit(MEASURE_LIMIT_TYPE_ELECTRIC, inParam.electricHighLimit());
				setUnit(MEASURE_LIMIT_TYPE_ELECTRIC, inParam.electricUnitStr());
				setLimitPrecision(MEASURE_LIMIT_TYPE_ELECTRIC, inParam.electricPrecision());

				setLowLimit(MEASURE_LIMIT_TYPE_ENGINEER, outParam.lowEngineeringUnits());
				setHighLimit(MEASURE_LIMIT_TYPE_ENGINEER, outParam.highEngineeringUnits());
				setUnit(MEASURE_LIMIT_TYPE_ENGINEER, outParam.unit());
				setLimitPrecision(MEASURE_LIMIT_TYPE_ENGINEER, outParam.decimalPlaces());
			}
			break;


		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Tuning_Output:
			{
			    const Metrology::SignalParam& param = ioParam.param(Metrology::ConnectionIoType::Destination);
				if (param.isValid() == false)
				{
					assert(false);
					break;
				}

				setLowLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricLowLimit());
				setHighLimit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricHighLimit());
				setUnit(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricUnitStr());
				setLimitPrecision(MEASURE_LIMIT_TYPE_ELECTRIC, param.electricPrecision());

				setLowLimit(MEASURE_LIMIT_TYPE_ENGINEER, param.lowEngineeringUnits());
				setHighLimit(MEASURE_LIMIT_TYPE_ENGINEER, param.highEngineeringUnits());
				setUnit(MEASURE_LIMIT_TYPE_ENGINEER, param.unit());
				setLimitPrecision(MEASURE_LIMIT_TYPE_ENGINEER, param.decimalPlaces());
			}
			break;

		default:
			assert(0);
	}
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

void Measurement::calcError()
{
	double errorLimit = theOptions.linearity().errorLimit();

	for(int limitType = 0; limitType < MEASURE_LIMIT_TYPE_COUNT; limitType++)
	{
		setError(limitType, MEASURE_ERROR_TYPE_ABSOLUTE,		std::abs(nominal(limitType)-measure(limitType)));
		setError(limitType, MEASURE_ERROR_TYPE_REDUCE,			std::abs(((nominal(limitType)-measure(limitType)) / (highLimit(limitType) - lowLimit(limitType))) * 100.0));
		setError(limitType, MEASURE_ERROR_TYPE_RELATIVE,		std::abs(((nominal(limitType)-measure(limitType)) / nominal(limitType)) * 100.0));

		setErrorLimit(limitType, MEASURE_ERROR_TYPE_ABSOLUTE,	std::abs((highLimit(limitType) - lowLimit(limitType)) * errorLimit / 100.0));
		setErrorLimit(limitType, MEASURE_ERROR_TYPE_REDUCE,		errorLimit);
		setErrorLimit(limitType, MEASURE_ERROR_TYPE_RELATIVE,	errorLimit);
	}
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
	if (theOptions.measureView().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
		}
	}

	int precision = DEFAULT_ECLECTRIC_UNIT_PRECESION;

	int limitType = theOptions.linearity().limitType();
	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	precision = limitPrecision(limitType);

	if (theOptions.measureView().precesionByCalibrator() == true)
	{
		if (limitType == MEASURE_LIMIT_TYPE_ELECTRIC)
		{
			precision = calibratorPrecision();
		}
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
		case MEASURE_ERROR_TYPE_ABSOLUTE:	str = QString::number(m_error[limitType][errorType], 'f', precision) + " " + m_unit[limitType];	break;
		case MEASURE_ERROR_TYPE_REDUCE:
		case MEASURE_ERROR_TYPE_RELATIVE:	str = QString::number(m_error[limitType][errorType], 'f', 3) + " %" ;							break;
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
		case MEASURE_ERROR_TYPE_REDUCE:
		case MEASURE_ERROR_TYPE_RELATIVE:	str = QString::number(m_errorLimit[limitType][errorType], 'f', 3) + " %";												break;
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
		return MEASURE_ERROR_RESULT_UNDEFINED;
	}

	int errorType = theOptions.linearity().errorType();
	if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
	{
		assert(0);
		return MEASURE_ERROR_RESULT_UNDEFINED;
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
	if (theOptions.measureView().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
		}
	}

	int errResult = errorResult();
	if (errResult < 0 || errResult > MEASURE_ERROR_RESULT_COUNT)
	{
		return QString();
	}

	return qApp->translate("MeasureBase.h", ErrorResult[errResult]);
}

// -------------------------------------------------------------------------------------------------------------------

void Measurement::setCalibratorData(const IoSignalParam &ioParam)
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	int precision = DEFAULT_ECLECTRIC_UNIT_PRECESION;

	switch (signalConnectionType)
	{
		case Metrology::ConnectionType::Unsed:
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			{
				CalibratorLimit sourceLimit = pCalibrator->currentSourceLimit();
				if (sourceLimit.isValid() == false)
				{
					break;
				}

				precision = sourceLimit.precesion;
			}
			break;


		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Tuning_Output:
			{
				CalibratorLimit measureLimit = pCalibrator->currentMeasureLimit();
				if (measureLimit.isValid() == false)
				{
					break;
				}

				precision = measureLimit.precesion;
			}
			break;

		default:
			assert(0);
	}

	setCalibratorPrecision(precision);

	QString calibratorDescription = pCalibrator->typeStr() + ", " + pCalibrator->serialNo();
	setCalibrator(calibratorDescription);
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

	//
	//
	m_connectionAppSignalID = from.m_connectionAppSignalID;
	m_connectionType = from.m_connectionType;

	m_appSignalID = from.m_appSignalID;
	m_customAppSignalID = from.m_customAppSignalID;
	m_equipmentID = from.m_equipmentID;
	m_caption = from.m_caption;

	m_location = from.m_location;

	m_calibratorPrecision = from.m_calibratorPrecision;

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

	//
	//
	m_measureTime = from.m_measureTime;
	m_calibrator = from.m_calibrator;
	m_reportType = from.m_reportType;

	m_foundInStatistics = from.m_foundInStatistics;

	//
	//
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	switch (signalConnectionType)
	{
		case Metrology::ConnectionType::Unsed:					fill_measure_input(ioParam);	break;
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:	fill_measure_internal(ioParam);	break;
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Tuning_Output:			fill_measure_output(ioParam);	break;
		default:												assert(0);
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

	int signalConnectionType = ioParam.signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	UnitsConvertor uc;

	//
	//

	setMeasureType(MEASURE_TYPE_LINEARITY);

	// features
	//

	setConnectionAppSignalID(inParam.appSignalID());
	setConnectionType(signalConnectionType);

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

	setCalibratorData(ioParam);

	// nominal
	//

	double electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
	double engineering = uc.conversion(electric, UnitsConvertType::ElectricToPhysical, inParam);

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
		double elVal = uc.conversion(enVal, UnitsConvertType::PhysicalToElectric, inParam);

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
	setLimits(ioParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(ioParam);
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
	if (outParam.isValid() == false)
	{
		assert(false);
		return;
	}

	UnitsConvertor uc;

	//
	//

	setMeasureType(MEASURE_TYPE_LINEARITY);

	// features
	//

	setConnectionAppSignalID(inParam.appSignalID());
	setConnectionType(signalConnectionType);

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

	setCalibratorData(ioParam);

	// nominal
	//

	double engineering = (ioParam.percent() * (inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) / 100) + inParam.lowEngineeringUnits();
	double electric = uc.conversion(engineering, UnitsConvertType::PhysicalToElectric, inParam);
	double engineeringCalc = conversionCalcVal(engineering, ConversionCalcType::Normal, ioParam.signalConnectionType(), ioParam);

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
		double enCalcVal = conversionCalcVal(enVal, ConversionCalcType::Inversion, ioParam.signalConnectionType(), ioParam);
		double elVal = uc.conversion(enCalcVal, UnitsConvertType::PhysicalToElectric, inParam);

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
	setLimits(ioParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(ioParam);
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
	if (outParam.isValid() == false)
	{
		assert(false);
		return;
	}

	UnitsConvertor uc;

	//
	//

	setMeasureType(MEASURE_TYPE_LINEARITY);

	// features
	//

	setConnectionAppSignalID(inParam.appSignalID());
	setConnectionType(signalConnectionType);

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

	setCalibratorData(ioParam);

	// nominal
	//

	double engineering = (ioParam.percent() * (outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) / 100) + outParam.lowEngineeringUnits();
	double engineeringCalc = conversionCalcVal(engineering, ConversionCalcType::Normal, ioParam.signalConnectionType(), ioParam);
	double electric = uc.conversion(engineeringCalc, UnitsConvertType::PhysicalToElectric, outParam);

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
			ioParam.calibratorManager()->getValue();
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
	setLimits(ioParam);

	// calc errors
	//
	calcError();

	// calc additional parameters
	//
	calcAdditionalParam(ioParam);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityMeasurement::calcAdditionalParam(const IoSignalParam &ioParam)
{
	if (ioParam.isValid() == false)
	{
		assert(false);
		return;
	}

	//
	//

	for(int limitType = 0; limitType < MEASURE_LIMIT_TYPE_COUNT; limitType ++)
	{
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

		setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_SYSTEM_DEVIATION, systemError);


			// according to GOST 8.736-2011 paragraph 5.3 formula 3
			//
		double sumDeviation = 0;

		for(int index = 0; index < measureCount(); index++)
		{
			sumDeviation += pow(measure(limitType) - measureItemArray(limitType, index), 2);		// 1. sum of deviations
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
		double uncertainty = calcUcertainty(ioParam, limitType);

		setAdditionalParam(limitType, MEASURE_ADDITIONAL_PARAM_UNCERTAINTY, uncertainty);
	}
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityMeasurement::calcUcertainty(const IoSignalParam &ioParam, int limitType) const
{
	if (ioParam.isValid() == false)
	{
		assert(false);
		return 0;
	}

	if (ioParam.calibratorManager() == nullptr)
	{
		assert(0);
		return 0;
	}

	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		assert(false);
		return 0;
	}

	int signalConnectionType = ioParam.signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return 0;
	}

	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(false);
		return 0;
	}

	// sco
	//
	double sco = additionalParam(limitType, MEASURE_ADDITIONAL_PARAM_SD);

	// Uncertainty of measurement to Document: EA-04/02 M:2013
	//
	double uncertainty = 0;

	switch (signalConnectionType)
	{
		case Metrology::ConnectionType::Unsed:
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			{
				// this measurement have only electric input and have not electric output
				//
			    const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
				if (inParam.isValid() == false)
				{
					assert(false);
					return 0;
				}

				CalibratorLimit sourceLimit = pCalibrator->currentSourceLimit();
				if (sourceLimit.isValid() == false)
				{
					return 0;
				}

				double Kox = 2;

				double dEj = (sourceLimit.ac0 * pCalibrator->sourceValue() + sourceLimit.ac1 * sourceLimit.highLimit) / 100.0;

				switch (limitType)
				{
					case MEASURE_LIMIT_TYPE_ELECTRIC:		// input electric
						{
							double MPe = 1 / pow(10.0, sourceLimit.precesion);

							// this is formula for case 2 from documet about uncertainty
							//
							uncertainty = Kox * sqrt( pow(sco, 2) + (pow(dEj,2) / 3) + (pow(MPe,2) / 3) );
						}
						break;

					case MEASURE_LIMIT_TYPE_ENGINEER:
						{
							double Kxj = 0;

							if (inParam.isLinearRange() == true)
							{
								// for linear electrical ranges (mA and V) Kxj is calculated differently
								//
								Kxj = (highLimit(MEASURE_LIMIT_TYPE_ENGINEER) - lowLimit(MEASURE_LIMIT_TYPE_ENGINEER)) / (inParam.electricHighLimit() - inParam.electricLowLimit());
							}
							else
							{
								// for non-linear electrical ranges (mV and Ohms) Kxj is calculated differently
								//
								Kxj = measure(MEASURE_LIMIT_TYPE_ENGINEER) / pCalibrator->sourceValue();
							}

							double MPx = 1 / pow(10.0, limitPrecision(MEASURE_LIMIT_TYPE_ENGINEER));

							// this is formula for case 1 from documet about uncertainty
							//
							uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kxj,2) * pow(dEj,2) / 3) + (pow(MPx,2) / 3) );
						}

						break;

					default:
						assert(0);
						break;
				}

			}
			break;

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
			{
				// this measurement have electric input and have electric output
				//
			    const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
				if (inParam.isValid() == false)
				{
					assert(false);
					return 0;
				}

				CalibratorLimit sourceLimit = pCalibrator->currentSourceLimit();
				if (sourceLimit.isValid() == false)
				{
					return 0;
				}

				Metrology::SignalParam outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
				if (outParam.isValid() == false)
				{
					assert(false);
					return 0;
				}

				CalibratorLimit measureLimit = pCalibrator->currentMeasureLimit();
				if (measureLimit.isValid() == false)
				{
					return 0;
				}

				double Kox = 2;

				double dEj = (sourceLimit.ac0 * pCalibrator->sourceValue() + sourceLimit.ac1 * sourceLimit.highLimit) / 100.0;

				double dIj = (measureLimit.ac0 * measure(MEASURE_LIMIT_TYPE_ELECTRIC) + measureLimit.ac1 * measureLimit.highLimit) / 100.0;

				switch (limitType)
				{
					case MEASURE_LIMIT_TYPE_ELECTRIC:		// output electric
						{
							double Kij = 0;

							if (inParam.isLinearRange() == true)
							{
								// for linear electrical ranges (mA and V) Kij is calculated differently
								// Output limit mA or V / Input limit mA or V
								//
								Kij = (outParam.electricHighLimit() - outParam.electricLowLimit()) / (inParam.electricHighLimit() - inParam.electricLowLimit());
							}
							else
							{
								// for non-linear electrical ranges (mV and Ohms) Kij is calculated differently
								// Output measure avg mA or V / Input source mV or Ohms
								//
								Kij = measure(MEASURE_LIMIT_TYPE_ELECTRIC) / pCalibrator->sourceValue();

							}

							double MPi = 1 / pow(10.0, measureLimit.precesion);

							// this is formula for case 3 from documet about uncertainty
							//
							uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kij,2) * pow(dEj,2) / 3) + (pow(dIj,2) / 3) + (pow(MPi,2) / 12) );
						}
						break;

					case MEASURE_LIMIT_TYPE_ENGINEER:
						{
							double Kxj = 0;

							if (inParam.isLinearRange() == true)
							{
								// for linear electrical ranges (mA and V) Kxj is calculated differently
								//
								Kxj = (highLimit(MEASURE_LIMIT_TYPE_ENGINEER) - lowLimit(MEASURE_LIMIT_TYPE_ENGINEER)) / (inParam.electricHighLimit() - inParam.electricLowLimit());
							}
							else
							{
								// for non-linear electrical ranges (mV and Ohms) Kxj is calculated differently
								// Output measure avg ENGINEER / Input source mV or Ohms
								//
								Kxj = measure(MEASURE_LIMIT_TYPE_ENGINEER) / pCalibrator->sourceValue();
							}

							double MPx = 1 / pow(10.0, limitPrecision(MEASURE_LIMIT_TYPE_ENGINEER));

							// this is formula for case 1 from documet about uncertainty
							//
							uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kxj,2) * pow(dEj,2) / 3) + (pow(MPx,2) / 3) );
						}

						break;

					default:
						assert(0);
						break;
				}

			}
			break;

		case Metrology::ConnectionType::Tuning_Output:
			{
				// this measurement have not electric input and have only electric output
				//
				CalibratorLimit measureLimit = pCalibrator->currentMeasureLimit();
				if (measureLimit.isValid() == false)
				{
					return 0;
				}

				double Kox = 2;

				double dIj = (measureLimit.ac0 * measure(MEASURE_LIMIT_TYPE_ELECTRIC) + measureLimit.ac1 * measureLimit.highLimit) / 100.0;

				switch (limitType)
				{
					case MEASURE_LIMIT_TYPE_ELECTRIC:		// output electric
					case MEASURE_LIMIT_TYPE_ENGINEER:		// because we have not input therefore uncertainty we will be calc by electric
						{
							// this is formula for case 4 from documet about uncertainty
							//
							double MPi = 1 / pow(10.0, measureLimit.precesion);

							// this is formula for case 4 from documet about uncertainty
							//
							uncertainty = Kox * sqrt( pow(sco, 2) + (pow(dIj,2) / 3) + (pow(MPi,2) / 3) );
						}

						break;

					default:
						assert(0);
						break;
				}
			}
			break;

		default:
			assert(0);
			break;
	}

	return uncertainty;
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
	if (theOptions.measureView().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
		}
	}

	int precision = DEFAULT_ECLECTRIC_UNIT_PRECESION;

	if (limitType < 0 || limitType >= MEASURE_LIMIT_TYPE_COUNT)
	{
		assert(0);
		return QString();
	}

	precision = limitPrecision(limitType);

	if (theOptions.measureView().precesionByCalibrator() == true)
	{
		if (limitType == MEASURE_LIMIT_TYPE_ELECTRIC)
		{
			precision = calibratorPrecision();
		}
	}

	if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT)
	{
		assert(0);
		return QString();
	}

	return QString::number(m_measureArray[limitType][index], 'f', precision);
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

	if (theOptions.measureView().showNoValid() == false)
	{
		if (isSignalValid() == false)
		{
			return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
		}
	}

	if (paramType < 0 || paramType >= MEASURE_ADDITIONAL_PARAM_COUNT)
	{
		assert(0);
		return QString();
	}

	QString valueStr = QString::number(m_additionalParam[limitType][paramType], 'f', 4);

	if (paramType == MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER)
	{
		valueStr.insert(0, "± ");
	}

	return valueStr;
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	switch (signalConnectionType)
	{
		case Metrology::ConnectionType::Unsed:					fill_measure_input(ioParam);	break;
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:	fill_measure_internal(ioParam);	break;
		default:												assert(0);
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
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

	UnitsConvertor uc;

	//
	//

	setMeasureType(MEASURE_TYPE_COMPARATOR);

	// features
	//

	setConnectionAppSignalID(inParam.appSignalID());
	setConnectionType(signalConnectionType);

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

	setCalibratorData(ioParam);

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
	double electric = uc.conversion(engineering, UnitsConvertType::PhysicalToElectric, inParam);

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// measure
	//

	setSignalValid(theSignalBase.signalState(inParam.hash()).valid());
	setSignalValid(true);

	electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
	engineering = uc.conversion(electric, UnitsConvertType::ElectricToPhysical, inParam);

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// limits
	//
	setLimits(ioParam);

	// calc errors
	//
	calcError();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasurement::fill_measure_internal(const IoSignalParam &ioParam)
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
	if (signalConnectionType < 0 || signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		assert(0);
		return;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
	if (inParam.isValid() == false)
	{
		assert(false);
		return;
	}

	const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
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

	UnitsConvertor uc;

	//
	//

	setMeasureType(MEASURE_TYPE_COMPARATOR);

	// features
	//

	setConnectionAppSignalID(inParam.appSignalID());
	setConnectionType(signalConnectionType);

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

	setCalibratorData(ioParam);

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
	double engineeringCalc = conversionCalcVal(engineering, ConversionCalcType::Inversion, ioParam.signalConnectionType(), ioParam);
	double electric = uc.conversion(engineeringCalc, UnitsConvertType::PhysicalToElectric, inParam);

	setNominal(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setNominal(MEASURE_LIMIT_TYPE_ENGINEER, engineering);

	// measure
	//

	setSignalValid(theSignalBase.signalState(outParam.hash()).valid());
	setSignalValid(true);

	electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
	engineering = uc.conversion(electric, UnitsConvertType::ElectricToPhysical, inParam);
	engineeringCalc = conversionCalcVal(engineering, ConversionCalcType::Normal, ioParam.signalConnectionType(), ioParam);

	setMeasure(MEASURE_LIMIT_TYPE_ELECTRIC, electric);
	setMeasure(MEASURE_LIMIT_TYPE_ENGINEER, engineeringCalc);

	// limits
	//
	setLimits(ioParam);

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

	return qApp->translate("MetrologySignal.h", Metrology::CmpValueType[m_cmpValueType]);
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

MeasureBase::MeasureBase(QObject* parent) :
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

			switch(pMeasurement->measureType())
			{
				case MEASURE_TYPE_LINEARITY:
					{
						LinearityMeasurement* pLinearityMeasurement = dynamic_cast<LinearityMeasurement*>(pMeasurement);
						if (pLinearityMeasurement == nullptr)
						{
							assert(0);
							delete pMeasurement;
							break;
						}

						delete pLinearityMeasurement;
					}
					break;

				case MEASURE_TYPE_COMPARATOR:
					{
						ComparatorMeasurement* pComparatorMeasurement = dynamic_cast<ComparatorMeasurement*>(pMeasurement);
						if (pComparatorMeasurement == nullptr)
						{
							assert(0);
							delete pMeasurement;
							break;
						}

						delete pComparatorMeasurement;
					}

					break;

				default:
					assert(0);
					delete pMeasurement;
					break;
			}
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

				if (data.pMeasurement == nullptr)
				{
					continue;
				}

				// load data to memory
				//

				if (table->read(data.pMeasurement) == data.recordCount)
				{
					loadedTablesInMemory.append(data);
				}
				else
				{
					switch(measureType)
					{
						case MEASURE_TYPE_LINEARITY:	delete [] static_cast<LinearityMeasurement*> (data.pMeasurement);	break;
						case MEASURE_TYPE_COMPARATOR:	delete [] static_cast<ComparatorMeasurement*> (data.pMeasurement);	break;
						default:						assert(0);
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
				if (pMainMeasure->measureID() != pSubMeasure->measureID())
				{
					continue;
				}

				switch (pMainMeasure->measureType())
				{
					case MEASURE_TYPE_LINEARITY:
						{
							LinearityMeasurement* pSupMeasurement = dynamic_cast<LinearityMeasurement*>(pMainMeasure);
							if (pSupMeasurement == nullptr)
							{
								continue;
							}

							switch(subTable.tableType)
							{
								case SQL_TABLE_LINEARITY_ADD_VAL_EL:	pSupMeasurement->updateAdditionalParam(MEASURE_LIMIT_TYPE_ELECTRIC, pSubMeasure);	break;
								case SQL_TABLE_LINEARITY_ADD_VAL_EN:	pSupMeasurement->updateAdditionalParam(MEASURE_LIMIT_TYPE_ENGINEER, pSubMeasure);	break;
								case SQL_TABLE_LINEARITY_20_EL:			pSupMeasurement->updateMeasureArray(MEASURE_LIMIT_TYPE_ELECTRIC, pSubMeasure);		break;
								case SQL_TABLE_LINEARITY_20_EN:			pSupMeasurement->updateMeasureArray(MEASURE_LIMIT_TYPE_ENGINEER, pSubMeasure);		break;
							}
						}
						break;

					case MEASURE_TYPE_COMPARATOR:
						{
							ComparatorMeasurement* pSupMeasurement = dynamic_cast<ComparatorMeasurement*>(pMainMeasure);
							if (pSupMeasurement == nullptr)
							{
								continue;
							}
						}

						break;

					default:
						break;
				}



				break;
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

	qDebug() << __FUNCTION__ << ": MeasureType: " << measureType <<
				", Loaded MeasureItem: " << count() <<
				", Time for load: " << responseTime.elapsed() << " ms";

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::signalBaseLoaded()
{
	QtConcurrent::run(MeasureBase::markNotExistMeasuremetsFromStatistics, this);
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

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::appendToBase(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return;
	}

	int index = append(pMeasurement);
	if (index == -1)
	{
		QMessageBox::critical(nullptr, tr("Save measurements"), tr("Error saving measurements to memory"));
		return;
	}
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

bool MeasureBase::remove(int measureType, const QVector<int>& keyList)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return false;
	}

	int keyCount = keyList.count();
	if (keyCount == 0)
	{
		return false;
	}

	int measureCount = count();
	if (measureCount == 0)
	{
		return false;
	}

	int removed = 0;

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

			if (remove(i) == true)
			{
				removed++;
			}

			break;
		}
	}

	if (removed != keyCount)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::removeFromBase(int measureType, const QVector<int>& keyList)
{
	bool result = remove(measureType, keyList);
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Delete measurements"), tr("Error remove measurements from memory"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::updateStatisticsItem(int measureType, StatisticsItem& si)
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
	si.setState(StatisticsItem::State::Success);

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
					si.setState(StatisticsItem::State::Failed);
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
					si.setState(StatisticsItem::State::Failed);
				}
			}

			break;

			default:
				assert(0);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::updateStatisticsBase(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	QElapsedTimer responseTime;
	responseTime.start();

	int measuredCount = 0;
	int invalidMeasureCount = 0;

	int count = theSignalBase.statistics().count(measureType);
	for(int i = 0; i < count; i++)
	{
		StatisticsItem* pSI = theSignalBase.statistics().itemPtr(measureType, i);
		if (pSI == nullptr)
		{
			continue;
		}

		updateStatisticsItem(measureType, *pSI);

		if (pSI->isMeasured() == true)
		{
			measuredCount++;
		}

		if (pSI->state() == StatisticsItem::State::Failed)
		{
			invalidMeasureCount ++;
		}
	}

	theSignalBase.statistics().setMeasuredCount(measuredCount);
	theSignalBase.statistics().setInvalidMeasureCount(invalidMeasureCount);

	qDebug() << __FUNCTION__ << " Time for update: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::updateStatisticsBase(int measureType, Hash signalHash)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	int count = theSignalBase.statistics().count(measureType);
	for(int i = 0; i < count; i++)
	{
		StatisticsItem* pSI = theSignalBase.statistics().itemPtr(measureType, i);
		if (pSI == nullptr)
		{
			continue;
		}

		Metrology::Signal* pSignal = pSI->signal();
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		if (pSignal->param().hash() == signalHash)
		{
			updateStatisticsItem(measureType, *pSI);
		}
	}

	QElapsedTimer responseTime;
	responseTime.start();

	int measuredCount = 0;
	int invalidMeasureCount = 0;

	for(int i = 0; i < count; i++)
	{
		StatisticsItem* pSI = theSignalBase.statistics().itemPtr(measureType, i);
		if (pSI == nullptr)
		{
			continue;
		}

		if (pSI->isMeasured() == true)
		{
			measuredCount++;
		}

		if (pSI->state() == StatisticsItem::State::Failed)
		{
			invalidMeasureCount ++;
		}
	}

	theSignalBase.statistics().setMeasuredCount(measuredCount);
	theSignalBase.statistics().setInvalidMeasureCount(invalidMeasureCount);

	qDebug() << __FUNCTION__ << " Time for update: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::markNotExistMeasuremetsFromStatistics(MeasureBase* pThis)
{
	if (pThis == nullptr)
	{
		return;
	}

	QElapsedTimer responseTime;
	responseTime.start();

	QMutexLocker l(&pThis->m_measureMutex);

	int measureCount = pThis->m_measureList.count();
	for (int m = 0; m < measureCount; m++)
	{
		Measurement* pMeasurement = pThis->m_measureList[m];
		if (pMeasurement == nullptr)
		{
			continue;
		}

		int measureType = pMeasurement->measureType();
		if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
		{
			continue;
		}

		pMeasurement->setFoundInStatistics(false);

		int count = theSignalBase.statistics().count(measureType);
		for(int s = 0; s < count; s++)
		{
			const StatisticsItem& si = theSignalBase.statistics().item(measureType, s);

			Metrology::Signal* pSignal = si.signal();
			if (pSignal == nullptr || pSignal->param().isValid() == false)
			{
				continue;
			}

			bool measurementIsFound = false;

			switch (measureType)
			{
				case MEASURE_TYPE_LINEARITY:
					{
						LinearityMeasurement* pLinearityMeasurement = dynamic_cast<LinearityMeasurement*>(pMeasurement);
						if (pLinearityMeasurement == nullptr)
						{
							continue;
						}

						if (pLinearityMeasurement->appSignalID() != pSignal->param().appSignalID())
						{
							continue;
						}

						measurementIsFound = true;
					}
					break;

				case MEASURE_TYPE_COMPARATOR:
					{
						std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
						if (comparatorEx == nullptr)
						{
							continue;
						}

						ComparatorMeasurement* pComparatorMeasurement = dynamic_cast<ComparatorMeasurement*>(pMeasurement);
						if (pComparatorMeasurement == nullptr)
						{
							continue;
						}

						if (pComparatorMeasurement->appSignalID() != pSignal->param().appSignalID())
						{
							continue;
						}

						if (pComparatorMeasurement->outputAppSignalID() != comparatorEx->output().appSignalID())
						{
							continue;
						}

						measurementIsFound = true;
					}
					break;

				default:
					assert(0);
					break;
			}

			pMeasurement->setFoundInStatistics(measurementIsFound);
		}
	}

	emit pThis->updateMeasureView();

	qDebug() << __FUNCTION__ << "- Signals were marked, " << " Time for marked: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

