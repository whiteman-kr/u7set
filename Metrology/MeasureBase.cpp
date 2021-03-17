#include "MeasureBase.h"

#include <QThread>
#include <QtConcurrent>
#include <QMessageBox>

#include "../lib/UnitsConvertor.h"

#include "Database.h"
#include "Conversion.h"
#include "Options.h"

namespace Measure
{
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	Item::Item(Measure::Type measureType)
	{
		Item::clear();

		m_measureType = measureType;
	}

	// -------------------------------------------------------------------------------------------------------------------

	Item::~Item()
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::clear()
	{
		m_measureType = Measure::Type::NoMeasureType;
		m_signalHash = UNDEFINED_HASH;

		m_measureID = -1;
		m_filter = false;

		m_signalValid = true;

		//
		//
		m_connectionAppSignalID.clear();
		m_connectionType = Metrology::ConnectionType::NoConnectionType;

		m_appSignalID.clear();
		m_customAppSignalID.clear();
		m_equipmentID.clear();
		m_caption.clear();

		m_location.clear();

		m_calibratorPrecision = DefaultElectricUnitPrecesion;

		for(int t = 0; t < Measure::LimitTypeCount; t++)
		{
			m_nominal[t] = 0;
			m_measure[t] = 0;

			m_lowLimit[t] = 0;
			m_highLimit[t] = 0;
			m_unit[t].clear();
			m_limitPrecision[t] = 0;

			for(int e = 0; e < Measure::ErrorTypeCount; e++)
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

	QString Item::measureTimeStr() const
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

	QString Item::connectionAppSignalID() const
	{
		if (m_connectionType == Metrology::ConnectionType::Unused)
		{
			return QString();
		}

		return m_connectionAppSignalID;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::connectionTypeStr() const
	{
		if (ERR_METROLOGY_CONNECTION_TYPE(m_connectionType) == true)
		{
			assert(0);
			return QString("???");
		}

		if (m_connectionType == Metrology::ConnectionType::Unused)
		{
			return QString();
		}

		return qApp->translate("MetrologyConnection", Metrology::ConnectionTypeCaption(static_cast<Metrology::ConnectionType>(m_connectionType)).toUtf8());
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::nominal(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_nominal[limitType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::nominalStr(LimitType limitType) const
	{
		int precision = DefaultElectricUnitPrecesion;

		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		precision = limitPrecision(limitType);

		if (theOptions.measureView().precesionByCalibrator() == true)
		{
			if (limitType == Measure::LimitType::Electric)
			{
				precision = calibratorPrecision();
			}
		}

		return QString("%1 %2").arg(QString::number(m_nominal[limitType], 'f', precision), m_unit[limitType]);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setNominal(LimitType limitType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_nominal[limitType] = value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::measure(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_measure[limitType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::measureStr(LimitType limitType) const
	{
		if (theOptions.measureView().showNoValid() == false)
		{
			if (isSignalValid() == false)
			{
				return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
			}
		}

		int precision = DefaultElectricUnitPrecesion;

		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		precision = limitPrecision(limitType);

		if (theOptions.measureView().precesionByCalibrator() == true)
		{
			if (limitType == Measure::LimitType::Electric)
			{
				precision = calibratorPrecision();
			}
		}

		return QString("%1 %2").arg(QString::number(m_measure[limitType], 'f', precision), m_unit[limitType]);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setMeasure(LimitType limitType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_measure[limitType] = value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setLimits(const IoSignalParam& ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			assert(0);
			return;
		}

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:
				{
					const Metrology::SignalParam& param = ioParam.param(Metrology::ConnectionIoType::Source);
					if (param.isValid() == false)
					{
						assert(false);
						break;
					}

					setLowLimit(Measure::LimitType::Electric, param.electricLowLimit());
					setHighLimit(Measure::LimitType::Electric, param.electricHighLimit());
					setUnit(Measure::LimitType::Electric, param.electricUnitStr());
					setLimitPrecision(Measure::LimitType::Electric, param.electricPrecision());

					setLowLimit(Measure::LimitType::Engineering, param.lowEngineeringUnits());
					setHighLimit(Measure::LimitType::Engineering, param.highEngineeringUnits());
					setUnit(Measure::LimitType::Engineering, param.unit());
					setLimitPrecision(Measure::LimitType::Engineering, param.decimalPlaces());
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

					setLowLimit(Measure::LimitType::Electric, inParam.electricLowLimit());
					setHighLimit(Measure::LimitType::Electric, inParam.electricHighLimit());
					setUnit(Measure::LimitType::Electric, inParam.electricUnitStr());
					setLimitPrecision(Measure::LimitType::Electric, inParam.electricPrecision());

					setLowLimit(Measure::LimitType::Engineering, outParam.lowEngineeringUnits());
					setHighLimit(Measure::LimitType::Engineering, outParam.highEngineeringUnits());
					setUnit(Measure::LimitType::Engineering, outParam.unit());
					setLimitPrecision(Measure::LimitType::Engineering, outParam.decimalPlaces());
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

					setLowLimit(Measure::LimitType::Electric, param.electricLowLimit());
					setHighLimit(Measure::LimitType::Electric, param.electricHighLimit());
					setUnit(Measure::LimitType::Electric, param.electricUnitStr());
					setLimitPrecision(Measure::LimitType::Electric, param.electricPrecision());

					setLowLimit(Measure::LimitType::Engineering, param.lowEngineeringUnits());
					setHighLimit(Measure::LimitType::Engineering, param.highEngineeringUnits());
					setUnit(Measure::LimitType::Engineering, param.unit());
					setLimitPrecision(Measure::LimitType::Engineering, param.decimalPlaces());
				}
				break;

			default:
				assert(0);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::lowLimit(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_lowLimit[limitType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setLowLimit(LimitType limitType, double lowLimit)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_lowLimit[limitType] = lowLimit;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::highLimit(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_highLimit[limitType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setHighLimit(LimitType limitType, double highLimit)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_highLimit[limitType] = highLimit;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::unit(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		return m_unit[limitType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setUnit(LimitType limitType, QString unit)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_unit[limitType] = unit;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int Item::limitPrecision(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_limitPrecision[limitType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setLimitPrecision(LimitType limitType, int precision)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_limitPrecision[limitType] = precision;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::limitStr(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		QString low = QString::number(m_lowLimit[limitType], 'f', m_limitPrecision[limitType]);
		QString high = QString::number(m_highLimit[limitType], 'f', m_limitPrecision[limitType]);

		return QString("%1 .. %2 %3").arg(low, high, m_unit[limitType]);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::calcError()
	{
		double errorLimit = theOptions.linearity().errorLimit();

		for(int type = 0; type < Measure::LimitTypeCount; type++)
		{
			LimitType limitType = static_cast<LimitType>(type);

			setError(limitType, Measure::ErrorType::Absolute,		std::abs(nominal(limitType)-measure(limitType)));
			setError(limitType, Measure::ErrorType::Reduce,			std::abs(((nominal(limitType)-measure(limitType)) / (highLimit(limitType) - lowLimit(limitType))) * 100.0));
			setError(limitType, Measure::ErrorType::Relative,		std::abs(((nominal(limitType)-measure(limitType)) / nominal(limitType)) * 100.0));

			setErrorLimit(limitType, Measure::ErrorType::Absolute,	std::abs((highLimit(limitType) - lowLimit(limitType)) * errorLimit / 100.0));
			setErrorLimit(limitType, Measure::ErrorType::Reduce,	errorLimit);
			setErrorLimit(limitType, Measure::ErrorType::Relative,	errorLimit);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::error(LimitType limitType, ErrorType errorType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return 0;
		}

		return m_error[limitType][errorType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::errorStr() const
	{
		if (theOptions.measureView().showNoValid() == false)
		{
			if (isSignalValid() == false)
			{
				return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
			}
		}

		int precision = DefaultElectricUnitPrecesion;

		LimitType limitType = static_cast<LimitType>(theOptions.linearity().limitType());
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		precision = limitPrecision(limitType);

		if (theOptions.measureView().precesionByCalibrator() == true)
		{
			if (limitType == Measure::LimitType::Electric)
			{
				precision = calibratorPrecision();
			}
		}

		int errorType = theOptions.linearity().errorType();
		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return QString();
		}

		QString str;

		switch(errorType)
		{
			case Measure::ErrorType::Absolute:	str = QString::number(m_error[limitType][errorType], 'f', precision) + " " + m_unit[limitType];	break;
			case Measure::ErrorType::Reduce:
			case Measure::ErrorType::Relative:	str = QString::number(m_error[limitType][errorType], 'f', 3) + " %" ;							break;
			default:							assert(0);
		}

		return str;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setError(LimitType limitType, ErrorType errorType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return;
		}

		m_error[limitType][errorType] = value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::errorLimit(LimitType limitType, ErrorType errorType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return 0;
		}

		return m_errorLimit[limitType][errorType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::errorLimitStr() const
	{
		int limitType = theOptions.linearity().limitType();
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		int errorType = theOptions.linearity().errorType();
		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return QString();
		}

		QString str;

		switch(errorType)
		{
			case Measure::ErrorType::Absolute:	str = QString::number(m_errorLimit[limitType][errorType], 'f', m_limitPrecision[limitType]) + " " + m_unit[limitType];	break;
			case Measure::ErrorType::Reduce:
			case Measure::ErrorType::Relative:	str = QString::number(m_errorLimit[limitType][errorType], 'f', 3) + " %";												break;
			default:							assert(0);
		}

		return str;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setErrorLimit(LimitType limitType, ErrorType errorType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return;
		}

		m_errorLimit[limitType][errorType] = value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int Item::errorResult() const
	{
		int limitType = theOptions.linearity().limitType();
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return Measure::ErrorResult::NoErrorResult;
		}

		int errorType = theOptions.linearity().errorType();
		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return Measure::ErrorResult::NoErrorResult;
		}

		if (m_error[limitType][errorType] > m_errorLimit[limitType][errorType])
		{
			return Measure::ErrorResult::Failed;
		}

		return Measure::ErrorResult::Ok;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::errorResultStr() const
	{
		if (theOptions.measureView().showNoValid() == false)
		{
			if (isSignalValid() == false)
			{
				return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
			}
		}

		int errResult = errorResult();
		if (ERR_MEASURE_ERROR_RESULT(errResult) == true)
		{
			return QString();
		}

		return qApp->translate("MeasureBase", Measure::ErrorResultCaption(errResult).toUtf8());
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setCalibratorData(const IoSignalParam &ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			assert(0);
			return;
		}

		int precision = DefaultElectricUnitPrecesion;

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:
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

	Item* Item::at(int index)
	{
		Item* pMeasurement = nullptr;

		switch(m_measureType)
		{
			case Measure::Type::Linearity:		pMeasurement = static_cast<LinearityItem*> (this) + index;	break;
			case Measure::Type::Comparators:	pMeasurement = static_cast<ComparatorItem*> (this) + index;	break;
			default:							assert(0);
		}

		return pMeasurement;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Item::findInStatisticsItem(const StatisticsItem& si)
	{
		Metrology::Signal* pSignal = si.signal();
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			return false;
		}

		if (appSignalID() != pSignal->param().appSignalID())
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::updateStatisticsItem(LimitType limitType, ErrorType errorType, StatisticsItem& si)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			return;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			return;
		}

		si.setMeasureCount(si.measureCount() + 1);

		if (error(limitType, errorType) > errorLimit(limitType, errorType))
		{
			si.setState(StatisticsItem::State::Failed);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	Item& Item::operator=(Item& from)
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

		for(int t = 0; t < Measure::LimitTypeCount; t++)
		{
			m_nominal[t] = from.m_nominal[t];
			m_measure[t] = from.m_measure[t];

			m_lowLimit[t] = from.m_lowLimit[t];
			m_highLimit[t] = from.m_highLimit[t];
			m_unit[t] = from.m_unit[t];
			m_limitPrecision[t] = from.m_limitPrecision[t];

			for(int e = 0; e < Measure::ErrorTypeCount; e++)
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
			case Measure::Type::Linearity:		*static_cast<LinearityItem*> (this) = *static_cast <LinearityItem*> (&from);		break;
			case Measure::Type::Comparators:	*static_cast<ComparatorItem*> (this) = *static_cast <ComparatorItem*> (&from);	break;
			default:							assert(0);
		}

		return *this;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	LinearityItem::LinearityItem() : Item(Measure::Type::Linearity)
	{
		LinearityItem::clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	LinearityItem::LinearityItem(const IoSignalParam &ioParam) : Item(Measure::Type::Linearity)
	{
		LinearityItem::clear();

		if (ioParam.calibratorManager() == nullptr)
		{
			assert(0);
			return;
		}

		if (ioParam.isValid() == false)
		{
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			assert(0);
			return;
		}

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:					fill_measure_input(ioParam);	break;
			case Metrology::ConnectionType::Input_Internal:
			case Metrology::ConnectionType::Input_DP_Internal_F:
			case Metrology::ConnectionType::Input_C_Internal_F:		fill_measure_internal(ioParam);	break;
			case Metrology::ConnectionType::Input_DP_Output_F:
			case Metrology::ConnectionType::Input_C_Output_F:
			case Metrology::ConnectionType::Input_Output:
			case Metrology::ConnectionType::Tuning_Output:			fill_measure_output(ioParam);	break;
			default:												assert(0);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	LinearityItem::~LinearityItem()
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::clear()
	{
		Measure::Item::clear();

		setMeasureType(Measure::Type::Linearity);

		m_percent = 0;

		for(int t = 0; t < Measure::LimitTypeCount; t++)
		{
			for(int m = 0; m < Measure::MaxMeasurementInPoint; m++)
			{
				m_measureArray[t][m] = 0;
			}
		}

		m_measureCount = 0;

		m_additionalParamCount = 0;

		for(int l = 0; l < Measure::LimitTypeCount; l++)
		{
			for(int a = 0; a < Measure::AdditionalParamCount; a++)
			{
				m_additionalParam[l][a] = 0;
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::fill_measure_input(const IoSignalParam &ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
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

		setMeasureType(Measure::Type::Linearity);

		// features
		//

		setConnectionAppSignalID(inParam.appSignalID());
		setConnectionType(connectionType);

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

		setNominal(Measure::LimitType::Electric, electric);
		setNominal(Measure::LimitType::Engineering, engineering);

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

			setMeasureItemArray(Measure::LimitType::Electric, index, elVal);
			setMeasureItemArray(Measure::LimitType::Engineering, index, enVal);

			averageElVal += elVal;
			averageEnVal += enVal;

			QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount));
		}

		averageElVal /= measureCount;
		averageEnVal /= measureCount;

		setMeasure(Measure::LimitType::Electric, averageElVal);
		setMeasure(Measure::LimitType::Engineering, averageEnVal);

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

	void LinearityItem::fill_measure_internal(const IoSignalParam &ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
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

		setMeasureType(Measure::Type::Linearity);

		// features
		//

		setConnectionAppSignalID(inParam.appSignalID());
		setConnectionType(connectionType);

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
		double engineeringCalc = conversionByConnection(engineering, ioParam, ConversionDirection::Normal);

		setPercent(ioParam.percent());

		setNominal(Measure::LimitType::Electric, electric);
		setNominal(Measure::LimitType::Engineering, engineeringCalc);

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
			double enCalcVal = conversionByConnection(enVal, ioParam, ConversionDirection::Inversion);
			double elVal = uc.conversion(enCalcVal, UnitsConvertType::PhysicalToElectric, inParam);

			setMeasureItemArray(Measure::LimitType::Electric, index, elVal);
			setMeasureItemArray(Measure::LimitType::Engineering, index, enVal);

			averageElVal += elVal;
			averagePhVal += enVal;

			QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount));
		}

		averageElVal /= measureCount;
		averagePhVal /= measureCount;

		setMeasure(Measure::LimitType::Electric, averageElVal);
		setMeasure(Measure::LimitType::Engineering, averagePhVal);

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

	void LinearityItem::fill_measure_output(const IoSignalParam &ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
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

		setMeasureType(Measure::Type::Linearity);

		// features
		//

		setConnectionAppSignalID(inParam.appSignalID());
		setConnectionType(connectionType);

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
		double engineeringCalc = conversionByConnection(engineering, ioParam, ConversionDirection::Normal);
		double electric = uc.conversion(engineeringCalc, UnitsConvertType::PhysicalToElectric, outParam);

		setPercent(ioParam.percent());

		setNominal(Measure::LimitType::Electric, electric);
		setNominal(Measure::LimitType::Engineering, engineeringCalc);

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

			setMeasureItemArray(Measure::LimitType::Electric, index, elVal);
			setMeasureItemArray(Measure::LimitType::Engineering, index, enVal);

			averageElVal += elVal;
			averagePhVal += enVal;

			QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureCount));
		}

		averageElVal /= measureCount;
		averagePhVal /= measureCount;

		setMeasure(Measure::LimitType::Electric, averageElVal);
		setMeasure(Measure::LimitType::Engineering, averagePhVal);

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

	void LinearityItem::calcAdditionalParam(const IoSignalParam &ioParam)
	{
		if (ioParam.isValid() == false)
		{
			assert(false);
			return;
		}

		//
		//

		for(int type = 0; type < Measure::LimitTypeCount; type++)
		{
			LimitType limitType = static_cast<LimitType>(type);

			// calc additional parameters
			//

			setAdditionalParamCount(Measure::AdditionalParamCount);

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

			setAdditionalParam(limitType, Measure::AdditionalParam::MaxValue, measureItemArray(limitType, maxDeviationIndex));


				// according to GOST 8.508-84 paragraph 3.4.1 formula 42
				//
			double systemError = std::abs(measure(limitType) - nominal(limitType));

			setAdditionalParam(limitType, Measure::AdditionalParam::SystemDeviation, systemError);


				// according to GOST 8.736-2011 paragraph 5.3 formula 3
				//
			double sumDeviation = 0;

			for(int index = 0; index < measureCount(); index++)
			{
				sumDeviation += pow(measure(limitType) - measureItemArray(limitType, index), 2);		// 1. sum of deviations
			}

			sumDeviation /= static_cast<double>(measureCount() - 1);									// 2. divide on (count of measure - 1)
			double sco = sqrt(sumDeviation);															// 3. sqrt

			setAdditionalParam(limitType, Measure::AdditionalParam::StandardDeviation, sco);


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

			setAdditionalParam(limitType, Measure::AdditionalParam::LowHighBorder, border);


				// Uncertainty of measurement to Document: EA-04/02 M:2013
				//
			double uncertainty = calcUcertainty(ioParam, limitType);

			setAdditionalParam(limitType, Measure::AdditionalParam::Uncertainty, uncertainty);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	double LinearityItem::calcUcertainty(const IoSignalParam &ioParam, LimitType limitType) const
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return 0;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			assert(0);
			return 0;
		}

		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(false);
			return 0;
		}

		// sco
		//
		double sco = additionalParam(limitType, Measure::AdditionalParam::StandardDeviation);

		// Uncertainty of measurement to Document: EA-04/02 M:2013
		//
		double uncertainty = 0;

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:
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
						case Measure::LimitType::Electric:		// input electric
							{
								double MPe = 1 / pow(10.0, sourceLimit.precesion);

								// this is formula for case 2 from documet about uncertainty
								//
								uncertainty = Kox * sqrt( pow(sco, 2) + (pow(dEj,2) / 3) + (pow(MPe,2) / 3) );
							}
							break;

						case Measure::LimitType::Engineering:
							{
								double Kxj = 0;

								if (inParam.isLinearRange() == true)
								{
									// for linear electrical ranges (mA and V) Kxj is calculated differently
									//
									Kxj = (highLimit(Measure::LimitType::Engineering) - lowLimit(Measure::LimitType::Engineering)) / (inParam.electricHighLimit() - inParam.electricLowLimit());
								}
								else
								{
									// for non-linear electrical ranges (mV and Ohms) Kxj is calculated differently
									//
									Kxj = measure(Measure::LimitType::Engineering) / pCalibrator->sourceValue();
								}

								double MPx = 1 / pow(10.0, limitPrecision(Measure::LimitType::Engineering));

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

					double dIj = (measureLimit.ac0 * measure(Measure::LimitType::Electric) + measureLimit.ac1 * measureLimit.highLimit) / 100.0;

					switch (limitType)
					{
						case Measure::LimitType::Electric:		// output electric
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
									Kij = measure(Measure::LimitType::Electric) / pCalibrator->sourceValue();

								}

								double MPi = 1 / pow(10.0, measureLimit.precesion);

								// this is formula for case 3 from documet about uncertainty
								//
								uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kij,2) * pow(dEj,2) / 3) + (pow(dIj,2) / 3) + (pow(MPi,2) / 12) );
							}
							break;

						case Measure::LimitType::Engineering:
							{
								double Kxj = 0;

								if (inParam.isLinearRange() == true)
								{
									// for linear electrical ranges (mA and V) Kxj is calculated differently
									//
									Kxj = (highLimit(Measure::LimitType::Engineering) - lowLimit(Measure::LimitType::Engineering)) / (inParam.electricHighLimit() - inParam.electricLowLimit());
								}
								else
								{
									// for non-linear electrical ranges (mV and Ohms) Kxj is calculated differently
									// Output measure avg ENGINEER / Input source mV or Ohms
									//
									Kxj = measure(Measure::LimitType::Engineering) / pCalibrator->sourceValue();
								}

								double MPx = 1 / pow(10.0, limitPrecision(Measure::LimitType::Engineering));

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

					double dIj = (measureLimit.ac0 * measure(Measure::LimitType::Electric) + measureLimit.ac1 * measureLimit.highLimit) / 100.0;

					switch (limitType)
					{
						case Measure::LimitType::Electric:		// output electric
						case Measure::LimitType::Engineering:		// because we have not input therefore uncertainty we will be calc by electric
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

	double LinearityItem::measureItemArray(LimitType limitType, int index) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		if (index < 0 || index >= Measure::MaxMeasurementInPoint)
		{
			assert(0);
			return 0;
		}

		return m_measureArray[limitType][index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString LinearityItem::measureItemStr(LimitType limitType, int index) const
	{
		if (theOptions.measureView().showNoValid() == false)
		{
			if (isSignalValid() == false)
			{
				return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
			}
		}

		int precision = DefaultElectricUnitPrecesion;

		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		precision = limitPrecision(limitType);

		if (theOptions.measureView().precesionByCalibrator() == true)
		{
			if (limitType == Measure::LimitType::Electric)
			{
				precision = calibratorPrecision();
			}
		}

		if (index < 0 || index >= Measure::MaxMeasurementInPoint)
		{
			assert(0);
			return QString();
		}

		return QString::number(m_measureArray[limitType][index], 'f', precision);
	}


	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::setMeasureItemArray(LimitType limitType, int index, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		if (index < 0 || index >= Measure::MaxMeasurementInPoint)
		{
			assert(0);
			return;
		}

		m_measureArray[limitType][index] = value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double LinearityItem::additionalParam(LimitType limitType, int paramType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		if (ERR_MEASURE_ADDITIONAL_PARAM(paramType) == true)
		{
			assert(0);
			return 0;
		}

		return m_additionalParam[limitType][paramType];
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString LinearityItem::additionalParamStr(LimitType limitType, int paramType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		if (theOptions.measureView().showNoValid() == false)
		{
			if (isSignalValid() == false)
			{
				return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
			}
		}

		if (ERR_MEASURE_ADDITIONAL_PARAM(paramType) == true)
		{
			assert(0);
			return QString();
		}

		QString valueStr = QString::number(m_additionalParam[limitType][paramType], 'f', 4);

		if (paramType == Measure::AdditionalParam::LowHighBorder)
		{
			valueStr.insert(0, "± ");
		}

		return valueStr;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::setAdditionalParam(LimitType limitType, int paramType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		if (ERR_MEASURE_ADDITIONAL_PARAM(paramType) == true)
		{
			assert(0);
			return;
		}

		m_additionalParam[limitType][paramType] = value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::updateMeasureArray(LimitType limitType, Item* pMeasurement)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			return;
		}

		if (pMeasurement == nullptr)
		{
			return;
		}

		if (ERR_MEASURE_TYPE(pMeasurement->measureType()) == true)
		{
			return;
		}

		LinearityItem* pLinearityMeasureItem = dynamic_cast <LinearityItem*> (pMeasurement);
		if (pLinearityMeasureItem == nullptr)
		{
			return;
		}

		m_measureCount = pLinearityMeasureItem->measureCount();

		for(int m = 0; m < Measure::MaxMeasurementInPoint; m++)
		{
			m_measureArray[limitType][m] = pLinearityMeasureItem->measureItemArray(limitType, m);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::updateAdditionalParam(LimitType limitType, Item* pMeasurement)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			return;
		}

		if (pMeasurement == nullptr)
		{
			return;
		}

		if (ERR_MEASURE_TYPE(pMeasurement->measureType()) == true)
		{
			return;
		}

		LinearityItem* pLinearityMeasureItem = dynamic_cast <LinearityItem*> (pMeasurement);
		if (pLinearityMeasureItem == nullptr)
		{
			return;
		}

		for(int a = 0; a < Measure::AdditionalParamCount; a++)
		{
			m_additionalParam[limitType][a] = pLinearityMeasureItem->additionalParam(limitType, a);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool LinearityItem::findInStatisticsItem(const StatisticsItem& si)
	{
		return Item::findInStatisticsItem(si);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::updateStatisticsItem(LimitType limitType, ErrorType errorType, StatisticsItem& si)
	{
		Item::updateStatisticsItem(limitType, errorType, si);
	}

	// -------------------------------------------------------------------------------------------------------------------

	LinearityItem& LinearityItem::operator=(const LinearityItem& from)
	{
		m_percent = from.m_percent;

		for(int t = 0; t < Measure::LimitTypeCount; t++)
		{
			for(int m = 0; m < Measure::MaxMeasurementInPoint; m++)
			{
				m_measureArray[t][m] = from.m_measureArray[t][m];
			}
		}

		m_measureCount = from.m_measureCount;

		m_additionalParamCount = from.m_additionalParamCount;

		for(int l = 0; l < Measure::LimitTypeCount; l++)
		{
			for(int a = 0; a < Measure::AdditionalParamCount; a++)
			{
				m_additionalParam[l][a] = from.m_additionalParam[l][a];
			}
		}

		return *this;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	ComparatorItem::ComparatorItem() : Item(Measure::Type::Comparators)
	{
		ComparatorItem::clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	ComparatorItem::ComparatorItem(const IoSignalParam& ioParam) : Item(Measure::Type::Comparators)
	{
		ComparatorItem::clear();

		if (ioParam.calibratorManager() == nullptr)
		{
			assert(0);
			return;
		}

		if (ioParam.isValid() == false)
		{
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
		{
			assert(0);
			return;
		}

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:					fill_measure_input(ioParam);	break;
			case Metrology::ConnectionType::Input_Internal:
			case Metrology::ConnectionType::Input_DP_Internal_F:
			case Metrology::ConnectionType::Input_C_Internal_F:		fill_measure_internal(ioParam);	break;
			default:												assert(0);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	ComparatorItem::~ComparatorItem()
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ComparatorItem::clear()
	{
		Measure::Item::clear();

		setMeasureType(Measure::Type::Comparators);

		m_compareAppSignalID.clear();
		m_outputAppSignalID.clear();

		m_cmpValueType = Metrology::CmpValueType::NoCmpValueType;
		m_cmpType = E::CmpType::Greate;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ComparatorItem::fill_measure_input(const IoSignalParam &ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
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

		Metrology::CmpValueType cmpValueType = ioParam.comparatorValueType();
		if (ERR_METROLOGY_CMP_VALUE_TYPE(cmpValueType) == true)
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

		setMeasureType(Measure::Type::Comparators);

		// features
		//

		setConnectionAppSignalID(inParam.appSignalID());
		setConnectionType(connectionType);

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

		setNominal(Measure::LimitType::Electric, electric);
		setNominal(Measure::LimitType::Engineering, engineering);

		// measure
		//

		setSignalValid(theSignalBase.signalState(inParam.hash()).valid());
		setSignalValid(true);

		electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
		engineering = uc.conversion(electric, UnitsConvertType::ElectricToPhysical, inParam);

		setMeasure(Measure::LimitType::Electric, electric);
		setMeasure(Measure::LimitType::Engineering, engineering);

		// limits
		//
		setLimits(ioParam);

		// calc errors
		//
		calcError();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ComparatorItem::fill_measure_internal(const IoSignalParam &ioParam)
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

		std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
		if (pCalibrator == nullptr)
		{
			assert(false);
			return;
		}

		Metrology::ConnectionType connectionType = ioParam.connectionType();
		if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
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

		Metrology::CmpValueType cmpValueType = ioParam.comparatorValueType();
		if (ERR_METROLOGY_CMP_VALUE_TYPE(cmpValueType) == true)
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

		setMeasureType(Measure::Type::Comparators);

		// features
		//

		setConnectionAppSignalID(inParam.appSignalID());
		setConnectionType(connectionType);

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
		double engineeringCalc = conversionByConnection(engineering, ioParam, ConversionDirection::Inversion);
		double electric = uc.conversion(engineeringCalc, UnitsConvertType::PhysicalToElectric, inParam);

		setNominal(Measure::LimitType::Electric, electric);
		setNominal(Measure::LimitType::Engineering, engineering);

		// measure
		//

		setSignalValid(theSignalBase.signalState(outParam.hash()).valid());
		setSignalValid(true);

		electric = ioParam.isNegativeRange() ? -pCalibrator->sourceValue() : pCalibrator->sourceValue();
		engineering = uc.conversion(electric, UnitsConvertType::ElectricToPhysical, inParam);
		engineeringCalc = conversionByConnection(engineering, ioParam, ConversionDirection::Normal);

		setMeasure(Measure::LimitType::Electric, electric);
		setMeasure(Measure::LimitType::Engineering, engineeringCalc);

		// limits
		//
		setLimits(ioParam);

		// calc errors
		//
		calcError();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorItem::cmpValueTypeStr() const
	{
		if (ERR_METROLOGY_CMP_VALUE_TYPE(m_cmpValueType) == true)
		{
			assert(0);
			return QString("Unknown");
		}

		return qApp->translate("MetrologySignal.h", Metrology::CmpValueTypeCpation(m_cmpValueType).toUtf8());
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorItem::cmpTypeStr() const
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

	void ComparatorItem::setCmpType(Metrology::CmpValueType cmpValueType, E::CmpType cmpType)
	{
		if (ERR_METROLOGY_CMP_VALUE_TYPE(cmpValueType) == true)
		{
			assert(0);
			return;
		}

		switch (cmpValueType)
		{
			case Metrology::CmpValueType::SetPoint:

				m_cmpType = cmpType;

				break;

			case Metrology::CmpValueType::Hysteresis:

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

	bool ComparatorItem::findInStatisticsItem(const StatisticsItem& si)
	{
		if (Item::findInStatisticsItem(si) == false)
		{
			return false;
		}

		std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
		if (comparatorEx == nullptr)
		{
			return false;
		}

		if (outputAppSignalID() != comparatorEx->output().appSignalID())
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ComparatorItem::updateStatisticsItem(LimitType limitType, ErrorType errorType, StatisticsItem& si)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			return;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			return;
		}

		if (m_cmpValueType == Metrology::CmpValueType::Hysteresis)
		{
			return;
		}

		std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
		if (comparatorEx == nullptr)
		{
			return;
		}

		if (comparatorEx->cmpType() != m_cmpType)
		{
			return;
		}

		if (comparatorEx->compare().isConst() == true)
		{
			if (compareDouble(comparatorEx->compareConstValue(), nominal(Measure::LimitType::Engineering)) == false)
			{
				return;
			}
		}
		else
		{
			if (comparatorEx->output().appSignalID() != outputAppSignalID())
			{
				return;
			}
		}

		Item::updateStatisticsItem(limitType, errorType, si);
	}

	// -------------------------------------------------------------------------------------------------------------------

	ComparatorItem& ComparatorItem::operator=(const ComparatorItem& from)
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

	Base::Base(QObject* parent) :
		QObject(parent)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	 Base::~Base()
	 {
	 }

	// -------------------------------------------------------------------------------------------------------------------

	int Base::count() const
	{
		QMutexLocker locker(&m_measureMutex);

		return m_measureList.count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Base::clear()
	{
		QMutexLocker locker(&m_measureMutex);

		for(auto pMeasurement: m_measureList)
		{
			if (pMeasurement == nullptr)
			{
				continue;
			}

			delete pMeasurement;
		}

		m_measureList.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------
	// each measurement is located in several tables,
	// firstly read data from the main table, and additional sub tables in memory
	// later update the data in the main table from sub tables
	//
	int Base::load(Measure::Type measureType)
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
		{
			return 0;
		}

		m_measureType = measureType;

		QElapsedTimer responseTime;
		responseTime.start();

		struct rawTableData
		{
			int		tableType;
			Item*	pMeasurement;
			int		recordCount;
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
						case Measure::Type::Linearity:		data.pMeasurement = new LinearityItem[static_cast<quint64>(data.recordCount)];	break;
						case Measure::Type::Comparators:	data.pMeasurement = new ComparatorItem[static_cast<quint64>(data.recordCount)];	break;
						default:							assert(0);
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
							case Measure::Type::Linearity:		delete [] static_cast<LinearityItem*> (data.pMeasurement);	break;
							case Measure::Type::Comparators:	delete [] static_cast<ComparatorItem*> (data.pMeasurement);	break;
							default:							assert(0);
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
			Item* pMainMeasure = mainTable.pMeasurement->at(mainIndex);
			if (pMainMeasure == nullptr)
			{
				continue;
			}

			for(int tableInMemory = SQL_TABLE_IS_SUB; tableInMemory < tableInMemoryCount; tableInMemory++)
			{
				rawTableData subTable = loadedTablesInMemory[tableInMemory];

				for(int subIndex = 0; subIndex < subTable.recordCount; subIndex++)
				{
					Item* pSubMeasure = subTable.pMeasurement->at(subIndex);
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
						case Measure::Type::Linearity:
							{
								LinearityItem* pSupMeasurement = dynamic_cast<LinearityItem*>(pMainMeasure);
								if (pSupMeasurement == nullptr)
								{
									continue;
								}

								switch(subTable.tableType)
								{
									case SQL_TABLE_LINEARITY_ADD_VAL_EL:	pSupMeasurement->updateAdditionalParam(Measure::LimitType::Electric, pSubMeasure);		break;
									case SQL_TABLE_LINEARITY_ADD_VAL_EN:	pSupMeasurement->updateAdditionalParam(Measure::LimitType::Engineering, pSubMeasure);	break;
									case SQL_TABLE_LINEARITY_20_EL:			pSupMeasurement->updateMeasureArray(Measure::LimitType::Electric, pSubMeasure);			break;
									case SQL_TABLE_LINEARITY_20_EN:			pSupMeasurement->updateMeasureArray(Measure::LimitType::Engineering, pSubMeasure);		break;
								}
							}
							break;

						case Measure::Type::Comparators:
							{
								ComparatorItem* pSupMeasurement = dynamic_cast<ComparatorItem*>(pMainMeasure);
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
			Item* pMeasureTable = mainTable.pMeasurement->at(index);
			if (pMeasureTable == nullptr)
			{
				continue;
			}

			Item* pMeasureAppend = nullptr;

			switch(measureType)
			{
				case Measure::Type::Linearity:		pMeasureAppend = new LinearityItem;		break;
				case Measure::Type::Comparators:	pMeasureAppend = new ComparatorItem;	break;
				default:							assert(0);								break;
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
				Item* pSubMeasure = subTable.pMeasurement->at(subIndex);
				if (pSubMeasure == nullptr)
				{
					continue;
				}

				bool foundMeasure = false;

				for(int mainIndex = 0; mainIndex < mainTable.recordCount; mainIndex++)
				{
					Item* pMainMeasure = mainTable.pMeasurement->at(mainIndex);
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
				case Measure::Type::Linearity:		delete [] static_cast<LinearityItem*> (table.pMeasurement);		break;
				case Measure::Type::Comparators:	delete [] static_cast<ComparatorItem*> (table.pMeasurement);	break;
				default:							assert(0);
			}
		}

		qDebug() << __FUNCTION__ << ": Measure::Type: " << measureType <<
					", Loaded MeasureItem: " << count() <<
					", Time for load: " << responseTime.elapsed() << " ms";

		return count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Base::signalBaseLoaded()
	{
		QtConcurrent::run(Base::markNotExistMeasuremetsFromStatistics, this);
	}

	// -------------------------------------------------------------------------------------------------------------------

	int Base::append(Measure::Item* pMeasurement)
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

	void Base::appendToBase(Measure::Item* pMeasurement)
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

	Measure::Item* Base::measurement(int index) const
	{
		QMutexLocker locker(&m_measureMutex);

		if (index < 0 || index >= m_measureList.count())
		{
			return nullptr;
		}

		return m_measureList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Base::remove(int index)
	{
		QMutexLocker locker(&m_measureMutex);

		if (index < 0 || index >= m_measureList.count())
		{
			return false;
		}

		Hash signalHash = UNDEFINED_HASH;

		auto pMeasurement = m_measureList[index];
		if (pMeasurement != nullptr)
		{
			signalHash = pMeasurement->signalHash();

			delete pMeasurement;
		}

		m_measureList.remove(index);

		if (signalHash != UNDEFINED_HASH)
		{
			emit updatedMeasureBase(signalHash);
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Base::remove(Measure::Type measureType, const QVector<int>& keyList)
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
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
				Item* pMeasurement = measurement(i);
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

	void Base::removeFromBase(Measure::Type measureType, const QVector<int>& keyList)
	{
		bool result = remove(static_cast<Measure::Type>(measureType), keyList);
		if (result == false)
		{
			QMessageBox::critical(nullptr, tr("Delete measurements"), tr("Error remove measurements from memory"));
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Base::updateStatisticsItem(Measure::Type measureType, StatisticsItem& si)
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
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
			assert(signalHash);
			return;
		}

		LimitType limitType = static_cast<LimitType>(theOptions.linearity().limitType());
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		ErrorType errorType = static_cast<ErrorType>(theOptions.linearity().errorType());
		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
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
			Item* pMeasurement = m_measureList[i];
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

			pMeasurement->updateStatisticsItem(limitType, errorType, si);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Base::updateStatisticsBase(Measure::Type measureType)
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
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

	void Base::updateStatisticsBase(Measure::Type measureType, Hash signalHash)
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
		{
			return;
		}

		if (signalHash == UNDEFINED_HASH)
		{
			return;
		}

		QElapsedTimer responseTime;
		responseTime.start();

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

	void Base::markNotExistMeasuremetsFromStatistics(Measure::Base* pThis)
	{
		if (pThis == nullptr)
		{
			return;
		}

		QElapsedTimer responseTime;
		responseTime.start();

		QMutexLocker l(&pThis->m_measureMutex);

		for (auto pMeasurement : pThis->m_measureList)
		{
			if (pMeasurement == nullptr)
			{
				continue;
			}

			Measure::Type measureType = pMeasurement->measureType();
			if (ERR_MEASURE_TYPE(measureType) == true)
			{
				continue;
			}

			pMeasurement->setFoundInStatistics(false);

			int count = theSignalBase.statistics().count(measureType);
			for(int s = 0; s < count; s++)
			{
				const StatisticsItem& si = theSignalBase.statistics().item(measureType, s);

				if (pMeasurement->findInStatisticsItem(si) == true)
				{
					pMeasurement->setFoundInStatistics(true);
				}
			}
		}

		emit pThis->updateMeasureView();

		qDebug() << __FUNCTION__ << "- Signals were marked, " << " Time for marked: " << responseTime.elapsed() << " ms";
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	QString TypeCaption(int measureType)
	{
		QString caption;

		switch (measureType)
		{
			case Measure::Type::Linearity:		caption = QT_TRANSLATE_NOOP("MeasureBase", "Measurements of linearity");	break;
			case Measure::Type::Comparators:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Measurements of comparators");	break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};

	QString KindCaption(int measureKind)
	{
		QString caption;

		switch (measureKind)
		{

			case Measure::Kind::OneRack:	caption = QT_TRANSLATE_NOOP("MeasureBase", " Single channel");	break;
			case Measure::Kind::OneModule:	caption = QT_TRANSLATE_NOOP("MeasureBase", " Single module");	break;
			case Measure::Kind::MultiRack:	caption = QT_TRANSLATE_NOOP("MeasureBase", " Multi channel");	break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};

	QString LimitTypeCaption(int measureLimitType)
	{
		QString caption;

		switch (measureLimitType)
		{
			case Measure::LimitType::Electric:		caption = QT_TRANSLATE_NOOP("MeasureBase", "Electric");		break;
			case Measure::LimitType::Engineering:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Engineering");	break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};

	QString ErrorTypeCaption(int errorType)
	{
		QString caption;

		switch (errorType)
		{
			case Measure::ErrorType::Absolute:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Absolute");	break;
			case Measure::ErrorType::Reduce:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Reduce");	break;
			case Measure::ErrorType::Relative:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Relative");	break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};

	QString ErrorResultCaption(int errorResult)
	{
		QString caption;

		switch (errorResult)
		{
			case Measure::ErrorResult::Ok:		caption = QT_TRANSLATE_NOOP("MeasureBase", "Ok");		break;
			case Measure::ErrorResult::Failed:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Failed");	break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};

	QString MeasureAdditionalParamCaption(int param)
	{
		QString caption;

		switch (param)
		{
			case Measure::AdditionalParam::MaxValue:			caption = QT_TRANSLATE_NOOP("MeasureBase", "Measure value max");	break;
			case Measure::AdditionalParam::SystemDeviation:		caption = QT_TRANSLATE_NOOP("MeasureBase", "System deviation");		break;
			case Measure::AdditionalParam::StandardDeviation:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Standard deviation");	break;
			case Measure::AdditionalParam::LowHighBorder:		caption = QT_TRANSLATE_NOOP("MeasureBase", "Low High border");		break;
			case Measure::AdditionalParam::Uncertainty:			caption = QT_TRANSLATE_NOOP("MeasureBase", "Uncertainty");			break;
			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

