#include "MeasureBase.h"
#include "UnitsConverter.h"
#include "Database.h"
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
		m_connectionSignalID.clear();
		m_connectionType = Metrology::ConnectionType::NoConnectionType;

		m_appSignalID.clear();
		m_customAppSignalID.clear();
		m_equipmentID.clear();
		m_caption.clear();

		m_location.clear();

		m_calibratorPrecision = DEFAULT_ELECTRIC_UNIT_PRECESION;

		for(int limitType = 0; limitType < Measure::LIMIT_TYPE_COUNT; limitType++)
		{
			m_nominal.setValue(limitType, 0);
			m_measure.setValue(limitType, 0);

			m_lowLimit.setValue(limitType, 0);
			m_highLimit.setValue(limitType, 0);
			m_unit.setValue(limitType, QString());
			m_limitPrecision.setValue(limitType, 0);

			for(int e = 0; e < MT::ERROR_TYPE_COUNT; e++)
			{
				m_error[e].setValue(limitType, 0);
				m_errorLimit[e].setValue(limitType, 0);
			}
		}

		m_adjustment = 0;

		//
		//
		m_measureTime.setDate(QDate());
		m_calibrator.clear();
		m_reportType = -1;

		m_foundInStatistics = true;
		m_hasWrongRange = false;
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

	QString Item::connectionSignalID() const
	{
		if (m_connectionType == Metrology::ConnectionType::Unused)
		{
			return QString();
		}

		return m_connectionSignalID;
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

//	Measure::LimitType Item::limitTypeByRange(int byRange) const
//	{
//		return limitTypeByRange(static_cast<MT::CalcErrorRange>(byRange));
//	}

	// -------------------------------------------------------------------------------------------------------------------

	Measure::LimitType Item::limitTypeByRange(MT::CalcErrorRange byRange) const
	{
		Measure::LimitType limitType = Measure::LimitType::NoLimitType;

		switch (byRange)
		{
			case MT::By_Signal_Type:

				switch (m_connectionType)
				{
					case Metrology::Unused:
					case Metrology::Input_Internal:
					case Metrology::Input_DP_Internal_F:
					case Metrology::Input_C_Internal_F:

						limitType = Measure::LimitType::Engineering;
						break;

					case Metrology::Input_Output:
					case Metrology::Input_DP_Output_F:
					case Metrology::Input_C_Output_F:
					case Metrology::Tuning_Output:

						limitType = Measure::LimitType::Electric;
						break;

					default:
						assert(0);
				}
				break;

			case MT::By_Electric_Range:

				limitType = Measure::LimitType::Electric;
				break;

			case MT::By_Engineering_Range:

				limitType = Measure::LimitType::Engineering;
				break;

			default:
				assert(0);
		}

		return limitType;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::nominal(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_nominal.value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::nominalStr(LimitType limitType) const
	{
		int precision = DEFAULT_ELECTRIC_UNIT_PRECESION;

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

		return QString("%1 %2").arg(QString::number(m_nominal.value(limitType), 'f', precision), m_unit.value(limitType));
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setNominal(LimitType limitType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_nominal.setValue(limitType, value);
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::measure(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_measure.value(limitType);
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

		int precision = DEFAULT_ELECTRIC_UNIT_PRECESION;

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

		return QString("%1 %2").arg(QString::number(m_measure.value(limitType), 'f', precision), m_unit.value(limitType));
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setMeasure(LimitType limitType, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_measure.setValue(limitType, value);
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

		return m_lowLimit.value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setLowLimit(LimitType limitType, double lowLimit)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_lowLimit.setValue(limitType, lowLimit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::highLimit(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_highLimit.value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setHighLimit(LimitType limitType, double highLimit)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_highLimit.setValue(limitType, highLimit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::unit(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		return m_unit.value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setUnit(LimitType limitType, QString unit)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_unit.setValue(limitType, unit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	int Item::limitPrecision(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return 0;
		}

		return m_limitPrecision.value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setLimitPrecision(LimitType limitType, int precision)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		m_limitPrecision.setValue(limitType, precision);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::limitStr(LimitType limitType) const
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		QString low = QString::number(m_lowLimit.value(limitType), 'f', m_limitPrecision.value(limitType));
		QString high = QString::number(m_highLimit.value(limitType), 'f', m_limitPrecision.value(limitType));

		return QString("%1 .. %2 %3").arg(low, high, m_unit.value(limitType));
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::calcError()
	{
		// calc errors value
		//
		for(int lType = 0; lType < Measure::LIMIT_TYPE_COUNT; lType++)
		{
			LimitType limitType = static_cast<LimitType>(lType);

			for(int eType = 0; eType < Measure::MT::ERROR_TYPE_COUNT; eType++)
			{
				MT::ErrorType errorType = static_cast<MT::ErrorType>(eType);

				double errorValue = calcMetrologyError(errorType, nominal(limitType), measure(limitType), lowLimit(limitType), highLimit(limitType));

				setError(limitType, errorType, errorValue);
			}
		}

		// calc error limits value
		//
		if (ERR_MEASURE_TYPE(m_measureType) == true)
		{
			assert(0);
			return;
		}

		double errorLimit = 0;

		switch (m_measureType)
		{
			case Measure::Type::Linearity:		errorLimit = theOptions.linearity().errorLimit();	break;
			case Measure::Type::Comparators:	errorLimit = theOptions.comparator().errorLimit();	break;

			default:
				assert(0);
				return;
		}

		calcErrorLimit(errorLimit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::calcErrorLimit(double errorLimit)
	{
		for(int lType = 0; lType < Measure::LIMIT_TYPE_COUNT; lType++)
		{
			LimitType limitType = static_cast<LimitType>(lType);

			for(int eType = 0; eType < Measure::MT::ERROR_TYPE_COUNT; eType++)
			{
				MT::ErrorType errorType = static_cast<MT::ErrorType>(eType);

				double errorValue = calcMetrologyErrorLimit(errorType, errorLimit, lowLimit(limitType), highLimit(limitType));

				setErrorLimit(limitType, errorType, errorValue);
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::error(LimitType limitType, MT::ErrorType errorType) const
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

		return m_error[errorType].value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::errorStr(Measure::Type measureType) const
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
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

		int precision = DEFAULT_ELECTRIC_UNIT_PRECESION;

		LimitType limitType = LimitType::NoLimitType;

		switch (measureType)
		{
			case Measure::Type::Linearity:		limitType = limitTypeByRange(theOptions.linearity().calcErrorByRange());	break;
			case Measure::Type::Comparators:	limitType = limitTypeByRange(theOptions.comparator().calcErrorByRange());	break;

			default:
				assert(0);
				return QString();
		}

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

		MT::ErrorType errorType = MT::ErrorType::Reduce;

		switch (measureType)
		{
			case Measure::Type::Linearity:		errorType = theOptions.linearity().errorType();		break;
			case Measure::Type::Comparators:	errorType = theOptions.comparator().errorType();	break;

			default:
				assert(0);
				return QString();
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return QString();
		}

		QString str;

		switch(errorType)
		{
			case MT::ErrorType::Absolute:	str = QString::number(m_error[errorType].value(limitType), 'f', precision) + " " + m_unit.value(limitType);	break;
			case MT::ErrorType::Reduce:
			case MT::ErrorType::Relative:	str = QString::number(m_error[errorType].value(limitType), 'f', 3) + " %" ;							break;

			default:
				assert(0);
		}

		return str;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setError(LimitType limitType, MT::ErrorType errorType, double value)
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

		m_error[errorType].setValue(limitType, value);
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Item::errorLimit(LimitType limitType, MT::ErrorType errorType) const
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

		return m_errorLimit[errorType].value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::errorLimitStr(Measure::Type measureType) const
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
		{
			assert(0);
			return QString();
		}

		LimitType limitType = LimitType::NoLimitType;

		switch (measureType)
		{
			case Measure::Type::Linearity:		limitType = limitTypeByRange(theOptions.linearity().calcErrorByRange());	break;
			case Measure::Type::Comparators:	limitType = limitTypeByRange(theOptions.comparator().calcErrorByRange());	break;

			default:
				assert(0);
				return QString();
		}

		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return QString();
		}

		MT::ErrorType errorType = MT::ErrorType::Reduce;

		switch (measureType)
		{
			case Measure::Type::Linearity:		errorType = theOptions.linearity().errorType();		break;
			case Measure::Type::Comparators:	errorType = theOptions.comparator().errorType();	break;

			default:
				assert(0);
				return QString();
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return QString();
		}

		QString str;

		switch(errorType)
		{
			case MT::ErrorType::Absolute:	str = QString::number(m_errorLimit[errorType].value(limitType), 'f', m_limitPrecision.value(limitType)) + " " + m_unit.value(limitType);	break;
			case MT::ErrorType::Reduce:
			case MT::ErrorType::Relative:	str = QString::number(m_errorLimit[errorType].value(limitType), 'f', 3) + " %";												break;

			default:
				assert(0);
		}

		return str;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::setErrorLimit(LimitType limitType, MT::ErrorType errorType, double value)
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

		m_errorLimit[errorType].setValue(limitType, value);
	}

	// -------------------------------------------------------------------------------------------------------------------

	Measure::ErrorResult Item::errorResult(Measure::Type measureType) const
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
		{
			assert(0);
			return Measure::ErrorResult::NoErrorResult;
		}

		LimitType limitType = LimitType::NoLimitType;

		switch (measureType)
		{
			case Measure::Type::Linearity:		limitType = limitTypeByRange(theOptions.linearity().calcErrorByRange());	break;
			case Measure::Type::Comparators:	limitType = limitTypeByRange(theOptions.comparator().calcErrorByRange());	break;

			default:
				assert(0);
				return Measure::ErrorResult::NoErrorResult;
		}

		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return Measure::ErrorResult::NoErrorResult;
		}

		MT::ErrorType errorType = MT::ErrorType::Reduce;

		switch (measureType)
		{
			case Measure::Type::Linearity:		errorType = theOptions.linearity().errorType();		break;
			case Measure::Type::Comparators:	errorType = theOptions.comparator().errorType();	break;

			default:
				assert(0);
				return Measure::ErrorResult::NoErrorResult;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return Measure::ErrorResult::NoErrorResult;
		}

		if (m_error[errorType].value(limitType) > m_errorLimit[errorType].value(limitType))
		{
			return Measure::ErrorResult::Failed;
		}

		return Measure::ErrorResult::Ok;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Item::errorResultStr(Measure::Type measureType) const
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
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

		Measure::ErrorResult errResult = errorResult(measureType);
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

		int precision = DEFAULT_ELECTRIC_UNIT_PRECESION;

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

			default:
				assert(0);
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

	bool Item::rangeIsOkInStatisticsItem(const StatisticsItem& si)
	{
		Metrology::Signal* pSignal = si.signal();
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			return false;
		}

		// Measure::LimitType::Electric
		//

		if (pSignal->param().inOutType() == E::SignalInOutType::Input ||  pSignal->param().inOutType() == E::SignalInOutType::Output)
		{
			if (lowLimit(Measure::LimitType::Electric) != pSignal->param().electricLowLimit())
			{
				return false;
			}

			if (highLimit(Measure::LimitType::Electric) != pSignal->param().electricHighLimit())
			{
				return false;
			}

			if (unit(Measure::LimitType::Electric) != pSignal->param().electricUnitStr())
			{
				return false;
			}
		}

		// Measure::LimitType::Engineering
		//

		if (lowLimit(Measure::LimitType::Engineering) != pSignal->param().lowEngineeringUnits())
		{
			return false;
		}

		if (highLimit(Measure::LimitType::Engineering) != pSignal->param().highEngineeringUnits())
		{
			return false;
		}

		if (unit(Measure::LimitType::Engineering) != pSignal->param().unit())
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Item::updateStatisticsItem(LimitType limitType, MT::ErrorType errorType, StatisticsItem& si)
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
		m_connectionSignalID = from.m_connectionSignalID;
		m_connectionType = from.m_connectionType;

		m_appSignalID = from.m_appSignalID;
		m_customAppSignalID = from.m_customAppSignalID;
		m_equipmentID = from.m_equipmentID;
		m_caption = from.m_caption;

		m_location = from.m_location;

		m_calibratorPrecision = from.m_calibratorPrecision;

		m_nominal = from.m_nominal;
		m_measure = from.m_measure;

		m_lowLimit = from.m_lowLimit;
		m_highLimit = from.m_highLimit;
		m_unit = from.m_unit;
		m_limitPrecision = from.m_limitPrecision;

		for(int e = 0; e < MT::ERROR_TYPE_COUNT; e++)
		{
			m_error[e] = from.m_error[e];
			m_errorLimit[e] = from.m_errorLimit[e];
		}

		m_adjustment = from.m_adjustment;

		//
		//
		m_measureTime = from.m_measureTime;
		m_calibrator = from.m_calibrator;
		m_reportType = from.m_reportType;

		m_foundInStatistics = from.m_foundInStatistics;
		m_hasWrongRange = from.m_hasWrongRange;

		//
		//
		switch(m_measureType)
		{
			case Measure::Type::Linearity:		*static_cast<LinearityItem*> (this) = *static_cast <LinearityItem*> (&from);	break;
			case Measure::Type::Comparators:	*static_cast<ComparatorItem*> (this) = *static_cast <ComparatorItem*> (&from);	break;

			default:
				assert(0);
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

		setMeasureTime(QDateTime::currentDateTime());

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

			default:
				assert(0);
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

		//
		//
		m_measureInPoint = 0;
		m_measureArray.resize(MAX_MEASUREMENT_IN_POINT);

		for(int limitType = 0; limitType < Measure::LIMIT_TYPE_COUNT; limitType++)
		{
			int count = static_cast<int>(m_measureArray.size());
			for(int m = 0; m < count; m++)
			{
				m_measureArray[m].setValue(limitType, 0);
			}
		}

		//
		//
		m_additionalParamCount = 0;
		m_additionalParam.resize(ADDITIONAL_PARAM_COUNT);

		for(int limitType = 0; limitType < Measure::LIMIT_TYPE_COUNT; limitType++)
		{
			int count = static_cast<int>(m_additionalParam.size());
			for(int a = 0; a < count; a++)
			{
				m_additionalParam[a].setValue(limitType, 0);
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

		UnitsConverter uc;

		//
		//

		setMeasureType(Measure::Type::Linearity);

		// features
		//

		setConnectionSignalID(inParam.customAppSignalID());
		setConnectionType(connectionType);

		setAppSignalID(inParam.appSignalID());
		setCustomAppSignalID(inParam.customAppSignalID());
		setEquipmentID(inParam.equipmentID());
		setCaption(inParam.caption());

		Metrology::SignalLocation location = inParam.location();

		if (inParam.location().moduleSerialNoID().isEmpty() == false)
		{
			Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
			const Metrology::SignalState& signalState = theSignalBase.signalState(serialNumberModuleHash);
			if (signalState.flags().valid == true)
			{
				location.setModuleSerialNo(static_cast<int>(signalState.value()));
			}
		}

		setLocation(location);

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

		setSignalValid(theSignalBase.signalState(inParam.hash()).flags().valid);

		double averageElVal = 0;
		double averageEnVal = 0;

		int measureInPoint = theOptions.linearity().measureCountInPoint();

		setMeasureInPoint(measureInPoint);

		for(int index = 0; index < measureInPoint; index++)
		{
			double enVal = theSignalBase.signalState(inParam.hash()).value();
			double elVal = uc.conversion(enVal, UnitsConvertType::PhysicalToElectric, inParam);

			setMeasureItemArray(Measure::LimitType::Electric, index, elVal);
			setMeasureItemArray(Measure::LimitType::Engineering, index, enVal);

			averageElVal += elVal;
			averageEnVal += enVal;

			QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureInPoint));
		}

		averageElVal /= measureInPoint;
		averageEnVal /= measureInPoint;

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

		UnitsConverter uc;

		//
		//

		setMeasureType(Measure::Type::Linearity);

		// features
		//

		setConnectionSignalID(inParam.customAppSignalID());
		setConnectionType(connectionType);

		setAppSignalID(outParam.appSignalID());
		setCustomAppSignalID(outParam.customAppSignalID());
		setEquipmentID(outParam.equipmentID());
		setCaption(outParam.caption());

		Metrology::SignalLocation location = inParam.location();

		if (inParam.location().moduleSerialNoID().isEmpty() == false)
		{
			Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
			const Metrology::SignalState& signalState = theSignalBase.signalState(serialNumberModuleHash);
			if (signalState.flags().valid == true)
			{
				location.setModuleSerialNo(static_cast<int>(signalState.value()));
			}
		}

		setLocation(location);

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

		setSignalValid(theSignalBase.signalState(outParam.hash()).flags().valid);

		double averageElVal = 0;
		double averagePhVal = 0;

		int measureInPoint = theOptions.linearity().measureCountInPoint();

		setMeasureInPoint(measureInPoint);

		for(int index = 0; index < measureInPoint; index++)
		{
			double enVal = theSignalBase.signalState(outParam.hash()).value();
			double enCalcVal = conversionByConnection(enVal, ioParam, ConversionDirection::Inversion);
			double elVal = uc.conversion(enCalcVal, UnitsConvertType::PhysicalToElectric, inParam);

			setMeasureItemArray(Measure::LimitType::Electric, index, elVal);
			setMeasureItemArray(Measure::LimitType::Engineering, index, enVal);

			averageElVal += elVal;
			averagePhVal += enVal;

			QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureInPoint));
		}

		averageElVal /= measureInPoint;
		averagePhVal /= measureInPoint;

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

		UnitsConverter uc;

		//
		//

		setMeasureType(Measure::Type::Linearity);

		// features
		//

		setConnectionSignalID(inParam.customAppSignalID());
		setConnectionType(connectionType);

		setAppSignalID(outParam.appSignalID());
		setCustomAppSignalID(outParam.customAppSignalID());
		setEquipmentID(outParam.equipmentID());
		setCaption(outParam.caption());

		Metrology::SignalLocation location = outParam.location();

		if (outParam.location().moduleSerialNoID().isEmpty() == false)
		{
			Hash serialNumberModuleHash = calcHash(outParam.location().moduleSerialNoID());
			Metrology::SignalState signalState = theSignalBase.signalState(serialNumberModuleHash);
			if (signalState.flags().valid == true)
			{
				location.setModuleSerialNo(static_cast<int>(signalState.value()));
			}
		}

		setLocation(location);

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

		setSignalValid(theSignalBase.signalState(outParam.hash()).flags().valid);

		double averageElVal = 0;
		double averagePhVal = 0;

		int measureInPoint = theOptions.linearity().measureCountInPoint();

		setMeasureInPoint(measureInPoint);

		for(int index = 0; index < measureInPoint; index++)
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

			QThread::msleep(static_cast<unsigned long>((theOptions.linearity().measureTimeInPoint() * 1000) / measureInPoint));
		}

		averageElVal /= measureInPoint;
		averagePhVal /= measureInPoint;

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

		setAdditionalParamCount(Measure::ADDITIONAL_PARAM_COUNT);

		//
		//
		for(int lType = 0; lType < Measure::LIMIT_TYPE_COUNT; lType++)
		{
			LimitType limitType = static_cast<LimitType>(lType);

			// create array for calculate
			//
			std::vector<double> measureValueArray;

			for (int index = 0; index < measureInPoint(); index++)
			{
				measureValueArray.push_back(measureItemArray(limitType, index));
			}

			// calc additional parameters
			//
				//
				//
			double maxDeviation = calcMaxDeviation(measure(limitType), measureValueArray);
			setAdditionalParam(limitType, Measure::AdditionalParam::MaxDeviation, maxDeviation);

				//
				//
			double systemDeviation = calcSystemDeviation(measure(limitType), nominal(limitType));
			setAdditionalParam(limitType, Measure::AdditionalParam::SystemDeviation, systemDeviation);

				//
				//
			double sco = calcSCO(measure(limitType), measureValueArray);
			setAdditionalParam(limitType, Measure::AdditionalParam::StandardDeviation, sco);

				//
				//
			double lowBorder = calcLowBorder(systemDeviation, sco, measureInPoint());
			setAdditionalParam(limitType, Measure::AdditionalParam::LowBorder, lowBorder);

			double highBorder = calcHighBorder(systemDeviation, sco, measureInPoint());
			setAdditionalParam(limitType, Measure::AdditionalParam::HighBorder, highBorder);

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
		// Instruction of Radiy: 460009.034-I19
		//
		double uncertainty = 0;

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:
			case Metrology::ConnectionType::Input_Internal:
			case Metrology::ConnectionType::Input_DP_Internal_F:
			case Metrology::ConnectionType::Input_C_Internal_F:
				{
					// this measurement has only electric input and have not electric output
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
						case Measure::LimitType::Engineering:
							{
								// case 1 of instruction
								//
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

								// Least significant unit (Engineering limit)
								//
								double MPx = 1 / pow(10.0, limitPrecision(Measure::LimitType::Engineering));

								// this is formula 1 for item 2 from documet about uncertainty
								//
								uncertainty = calcUcertainty1(Kox, sco, Kxj, dEj, MPx);
							}
							break;

						case Measure::LimitType::Electric: // input electric
							{
								// case 2 of instruction
								//
								// Least significant unit (Electric limit)
								//
								double MPe = 1 / pow(10.0, sourceLimit.precesion);

								// this is formula 7 for item 3 from documet about uncertainty
								//
								uncertainty = calcUcertainty2(Kox, sco, dEj, MPe);
							}
							break;

						default:
							assert(0);
					}
				}
				break;

			case Metrology::ConnectionType::Input_Output:
			case Metrology::ConnectionType::Input_DP_Output_F:
			case Metrology::ConnectionType::Input_C_Output_F:
				{
					// this measurement has electric input and has electric output
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
						case Measure::LimitType::Engineering:
							{
								// case 1 of instruction
								//
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
									// Output measure avg Engineering / Input source mV or Ohms
									//
									Kxj = measure(Measure::LimitType::Engineering) / pCalibrator->sourceValue();
								}

								// Least significant unit (Engineering limit)
								//
								double MPx = 1 / pow(10.0, limitPrecision(Measure::LimitType::Engineering));

								// this is formula 1 for item 2 from documet about uncertainty
								//
								uncertainty = calcUcertainty1(Kox, sco, Kxj, dEj, MPx);
							}
							break;

						case Measure::LimitType::Electric:	// output electric
							{
								// case 4 of instruction
								//
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

								// Least significant unit (Electric measure limit)
								//
								double MPi = 1 / pow(10.0, measureLimit.precesion);

								// this is formula 9 for item 5 from documet about uncertainty
								//
								uncertainty = calcUcertainty4(Kox, sco, Kij, dEj, dIj, MPi);
							}
							break;

						default:
							assert(0);
					}
				}
				break;

			case Metrology::ConnectionType::Tuning_Output:
				{
					// this measurement dosent have electric input and has only electric output
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
						case Measure::LimitType::Engineering:	// because we dont have electric input, therefore uncertainty we will be calc by output electric
						case Measure::LimitType::Electric:		// output electric
							{
								// case 3 of instruction
								//
								// Least significant unit (Electric measure limit)
								//
								double MPi = 1 / pow(10.0, measureLimit.precesion);

								// this is formula 8 for item 4 from documet about uncertainty
								//
								uncertainty = calcUcertainty3(Kox, sco, dIj, MPi);
							}
							break;

						default:
							assert(0);
					}
				}
				break;

			default:
				assert(0);
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

		if (index < 0 || index >= Measure::MAX_MEASUREMENT_IN_POINT)
		{
			assert(0);
			return 0;
		}

		if (index < 0 || index >= static_cast<int>(m_measureArray.size()))
		{
			assert(0);
			return 0;
		}

		return m_measureArray[index].value(limitType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::setMeasureInPoint(int count)
	{
		if (count > MAX_MEASUREMENT_IN_POINT)
		{
			count = MAX_MEASUREMENT_IN_POINT;
		}

		m_measureInPoint = count;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString LinearityItem::measureItemStr(LimitType limitType, int index) const
	{
		if (index >= measureInPoint())
		{
			return QString();
		}

		if (theOptions.measureView().showNoValid() == false)
		{
			if (isSignalValid() == false)
			{
				return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
			}
		}

		int precision = DEFAULT_ELECTRIC_UNIT_PRECESION;

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

		if (index < 0 || index >= Measure::MAX_MEASUREMENT_IN_POINT)
		{
			assert(0);
			return QString();
		}

		if (index < 0 || index >= static_cast<int>(m_measureArray.size()))
		{
			assert(0);
			return QString();
		}

		return QString::number(m_measureArray[index].value(limitType), 'f', precision);
	}


	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::setMeasureItemArray(LimitType limitType, int index, double value)
	{
		if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
		{
			assert(0);
			return;
		}

		if (index < 0 || index >= Measure::MAX_MEASUREMENT_IN_POINT)
		{
			assert(0);
			return;
		}

		if (index < 0 || index >= static_cast<int>(m_measureArray.size()))
		{
			assert(0);
			return;
		}

		m_measureArray[index].setValue(limitType, value);
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

		return m_additionalParam[paramType].value(limitType);
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

		if (paramType < 0 || paramType >= static_cast<int>(m_additionalParam.size()))
		{
			assert(0);
			return QString();
		}

		QString valueStr = QString::number(m_additionalParam[paramType].value(limitType), 'f', 4);

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

		if (paramType < 0 || paramType >= static_cast<int>(m_additionalParam.size()))
		{
			assert(0);
			return;
		}

		m_additionalParam[paramType].setValue(limitType, value);
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

		m_measureInPoint = pLinearityMeasureItem->measureInPoint();
		if (m_measureInPoint > MAX_MEASUREMENT_IN_POINT)
		{
			m_measureInPoint = MAX_MEASUREMENT_IN_POINT;
		}

		setMeasureInPoint(pLinearityMeasureItem->measureInPoint());

		for(int m = 0; m < Measure::MAX_MEASUREMENT_IN_POINT; m++)
		{
			setMeasureItemArray(limitType, m, pLinearityMeasureItem->measureItemArray(limitType, m));
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

		for(int a = 0; a < Measure::ADDITIONAL_PARAM_COUNT; a++)
		{
			m_additionalParam[a].setValue(limitType, pLinearityMeasureItem->additionalParam(limitType, a));
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool LinearityItem::findInStatisticsItem(const StatisticsItem& si)
	{
		return Item::findInStatisticsItem(si);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void LinearityItem::updateStatisticsItem(LimitType limitType, MT::ErrorType errorType, StatisticsItem& si)
	{
		Item::updateStatisticsItem(limitType, errorType, si);
	}

	// -------------------------------------------------------------------------------------------------------------------

	LinearityItem& LinearityItem::operator=(const LinearityItem& from)
	{
		m_percent = from.m_percent;

		m_measureInPoint = from.m_measureInPoint;
		m_measureArray = from.m_measureArray;

		m_additionalParamCount = from.m_additionalParamCount;
		m_additionalParam = from.m_additionalParam;

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

		setMeasureTime(QDateTime::currentDateTime());

		switch (connectionType)
		{
			case Metrology::ConnectionType::Unused:					fill_measure_input(ioParam);	break;
			case Metrology::ConnectionType::Input_Internal:
			case Metrology::ConnectionType::Input_DP_Internal_F:
			case Metrology::ConnectionType::Input_C_Internal_F:		fill_measure_internal(ioParam);	break;

			default:
				assert(0);
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
		m_cmpType = E::CmpType::Greater;
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

		UnitsConverter uc;

		//
		//

		setMeasureType(Measure::Type::Comparators);

		// features
		//

		setConnectionSignalID(inParam.customAppSignalID());
		setConnectionType(connectionType);

		setAppSignalID(inParam.appSignalID());
		setCustomAppSignalID(inParam.customAppSignalID());
		setEquipmentID(inParam.equipmentID());
		setCaption(inParam.caption());

		Metrology::SignalLocation location = inParam.location();

		if (inParam.location().moduleSerialNoID().isEmpty() == false)
		{
			Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
			const Metrology::SignalState& signalState = theSignalBase.signalState(serialNumberModuleHash);
			if (signalState.flags().valid == true)
			{
				location.setModuleSerialNo(static_cast<int>(signalState.value()));
			}
		}

		setLocation(location);

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

		setSignalValid(theSignalBase.signalState(inParam.hash()).flags().valid);
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

		UnitsConverter uc;

		//
		//

		setMeasureType(Measure::Type::Comparators);

		// features
		//

		setConnectionSignalID(inParam.customAppSignalID());
		setConnectionType(connectionType);

		setAppSignalID(outParam.appSignalID());
		setCustomAppSignalID(outParam.customAppSignalID());
		setEquipmentID(outParam.equipmentID());
		setCaption(outParam.caption());

		Metrology::SignalLocation location = inParam.location();

		if (inParam.location().moduleSerialNoID().isEmpty() == false)
		{
			Hash serialNumberModuleHash = calcHash(inParam.location().moduleSerialNoID());
			const Metrology::SignalState& signalState = theSignalBase.signalState(serialNumberModuleHash);
			if (signalState.flags().valid == true)
			{
				location.setModuleSerialNo(static_cast<int>(signalState.value()));
			}
		}

		setLocation(location);

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

		setSignalValid(theSignalBase.signalState(outParam.hash()).flags().valid);
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

		return qApp->translate("MetrologySignal", Metrology::CmpValueTypeCpation(m_cmpValueType).toUtf8());
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorItem::cmpTypeStr() const
	{
		QString typeStr;

		switch (m_cmpType)
		{
			case E::CmpType::Greater:	typeStr = QChar(0x25B2);	break;
			case E::CmpType::Less:		typeStr = QChar(0x25BC);	break;

			default:
				assert(0);
				typeStr.clear();
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

				m_cmpType = cmpType;	// default

				break;

			case Metrology::CmpValueType::Hysteresis:

				switch (cmpType)		// inversion
				{
					case E::CmpType::Less:		m_cmpType = E::CmpType::Greater;	break;
					case E::CmpType::Greater:	m_cmpType = E::CmpType::Less;	break;

					default:			// for metrology only Great of Less
						break;
				}

				break;

			default:
				assert(0);
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

	void ComparatorItem::updateStatisticsItem(LimitType limitType, MT::ErrorType errorType, StatisticsItem& si)
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

		return TO_INT(m_measureList.size());
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Base::clear()
	{
		QMutexLocker locker(&m_measureMutex);

		qDeleteAll(m_measureList);
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
			int		tableType = 0;
			Item*	pMeasurement = nullptr;
			int		recordCount = 0;
		};

		// step 1
		// -----------------------
		// read all tables for current measureType in memory
		//
		std::map<int, int> measureIDMap; // measureID, Index - for fast search

		std::vector<rawTableData> loadedTablesInMemory;

		for(int tableType = 0; tableType < SQL_TABLE_COUNT; tableType++)
		{
			if (SqlTableByMeasureType[tableType] == measureType)
			{
				SqlTable* table = theDatabase.openTable(tableType);
				TEST_PTR_CONTINUE(table);

				rawTableData data;

				// determine size data to allocate memory
				//
				data.tableType = tableType;
				data.pMeasurement = nullptr;
				data.recordCount = table->recordCount();

				// allocate memory
				//
				switch(measureType)
				{
					case Measure::Type::Linearity:		data.pMeasurement = new LinearityItem[static_cast<quint64>(data.recordCount)];	break;
					case Measure::Type::Comparators:	data.pMeasurement = new ComparatorItem[static_cast<quint64>(data.recordCount)];	break;

					default:
						continue;
				}
				TEST_PTR_CONTINUE(data.pMeasurement);

				// load data to memory
				//
				int readRecord = table->read(data.pMeasurement);

				//
				//
				if (readRecord == data.recordCount)
				{
					loadedTablesInMemory.push_back(data);

					// create map for fast search
					//
					if (SqlTableAppointType[tableType] == SQL_TABLE_IS_MAIN)
					{
						for(int i = 0; i < data.recordCount; i++)
						{
							Item* pMeasureFromMainTable = data.pMeasurement->at(i);
							TEST_PTR_CONTINUE(pMeasureFromMainTable);

							measureIDMap.emplace(pMeasureFromMainTable->measureID(), i);
						}
					}
				}
				else
				{
					//remove data from memory if data was not load correctly
					//
					switch(measureType)
					{
						case Measure::Type::Linearity:		delete [] static_cast<LinearityItem*> (data.pMeasurement);	break;
						case Measure::Type::Comparators:	delete [] static_cast<ComparatorItem*> (data.pMeasurement);	break;

						default:
							continue;
					}
				}

				table->close();
			}
		}

		// if tables for current measureType is not exist, then exit
		//
		quint64 tableInMemoryCount = loadedTablesInMemory.size();
		if (tableInMemoryCount == 0)
		{
			return 0;
		}

		// step 2
		// -----------------------
		// get sub tables and update data in main table
		//
		rawTableData mainTable = loadedTablesInMemory[SQL_TABLE_IS_MAIN];
		if(mainTable.pMeasurement == nullptr)
		{
			return 0;
		}

		for(quint64 tableInMemory = SQL_TABLE_IS_SUB; tableInMemory < tableInMemoryCount; tableInMemory++)
		{
			rawTableData subTable = loadedTablesInMemory[tableInMemory];
			TEST_PTR_CONTINUE(subTable.pMeasurement);

			std::vector<int> removeKeyList; // measureID list for remove

			// find record in sub table by measureID
			//
			for(int subIndex = 0; subIndex < subTable.recordCount; subIndex++)
			{
				Item* pMeasureFromSubTable = subTable.pMeasurement->at(subIndex);
				TEST_PTR_CONTINUE(pMeasureFromSubTable);

				if (measureIDMap.contains(pMeasureFromSubTable->measureID()) == false)
				{
					removeKeyList.push_back(pMeasureFromSubTable->measureID());
					continue;
				}

				int mainIndex = measureIDMap.at(pMeasureFromSubTable->measureID());
				if (mainIndex < 0 || mainIndex > mainTable.recordCount)
				{
					continue;
				}

				Item* pMeasureFromMainTable = mainTable.pMeasurement->at(mainIndex);
				TEST_PTR_CONTINUE(pMeasureFromMainTable);

				// update main measurement from sub measurement
				//
				if (pMeasureFromMainTable->measureID() != pMeasureFromSubTable->measureID())
				{
					Q_ASSERT(false);
					continue;
				}

				switch (pMeasureFromMainTable->measureType())
				{
					case Measure::Type::Linearity:
						{
							LinearityItem* pLinMeasurement = dynamic_cast<LinearityItem*>(pMeasureFromMainTable);
							TEST_PTR_CONTINUE(pLinMeasurement);

							switch(subTable.tableType)
							{
								case SQL_TABLE_LINEARITY_ADD_VAL_EL:	pLinMeasurement->updateAdditionalParam(Measure::LimitType::Electric, pMeasureFromSubTable);		break;
								case SQL_TABLE_LINEARITY_ADD_VAL_EN:	pLinMeasurement->updateAdditionalParam(Measure::LimitType::Engineering, pMeasureFromSubTable);	break;
								case SQL_TABLE_LINEARITY_20_EL:			pLinMeasurement->updateMeasureArray(Measure::LimitType::Electric, pMeasureFromSubTable);			break;
								case SQL_TABLE_LINEARITY_20_EN:			pLinMeasurement->updateMeasureArray(Measure::LimitType::Engineering, pMeasureFromSubTable);		break;
							}
						}
						break;

					case Measure::Type::Comparators:
						{
							ComparatorItem* pCmpMeasurement = dynamic_cast<ComparatorItem*>(pMeasureFromMainTable);
							TEST_PTR_CONTINUE(pCmpMeasurement);
						}

						break;

					default:
						continue;
				}
			}

			// if measureID was not found in main table, but the measurement is exist in sub table,
			// need remove this measurement in sub table
			// remove nonexistent indexes-measurements-ID in sub tables
			//
			if (removeKeyList.size() == 0)
			{
				continue;
			}

			SqlTable* table = theDatabase.openTable(subTable.tableType);
			if (table != nullptr)
			{
				table->remove(removeKeyList.data(), TO_INT(removeKeyList.size()));
				table->close();
			}
		}

		// step 3
		// -----------------------
		// append measuremets to MeasureBase from updated main table
		//
		for(int index = 0; index < mainTable.recordCount; index++)
		{
			Item* pMeasureFromMainTable = mainTable.pMeasurement->at(index);
			TEST_PTR_CONTINUE(pMeasureFromMainTable);

			Item* pMeasureForAppend = nullptr;

			switch(measureType)
			{
				case Measure::Type::Linearity:		pMeasureForAppend = new LinearityItem;	break;
				case Measure::Type::Comparators:	pMeasureForAppend = new ComparatorItem;	break;

				default:
					continue;
			}

			TEST_PTR_CONTINUE(pMeasureForAppend);

			*pMeasureForAppend = *pMeasureFromMainTable;

			append(pMeasureForAppend);
		}

		// step 4
		// -----------------------
		// remove raw table data from memory
		//
		for(quint64 tableInMemory = 0; tableInMemory < tableInMemoryCount; tableInMemory++)
		{
			rawTableData table = loadedTablesInMemory[tableInMemory];
			TEST_PTR_CONTINUE(table.pMeasurement);

			switch(measureType)
			{
				case Measure::Type::Linearity:		delete [] static_cast<LinearityItem*> (table.pMeasurement);		break;
				case Measure::Type::Comparators:	delete [] static_cast<ComparatorItem*> (table.pMeasurement);	break;

				default:
					continue;
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
		QFuture<void> resRun0 = QtConcurrent::run(Base::markNotExistMeasuremetsFromStatistics, this);
		QFuture<void> resRun1 = QtConcurrent::run(Base::markWrongRangeFromStatistics, this);

		//resRun0.waitForFinished();
		//resRun1.waitForFinished();
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

			m_measureList.push_back(pMeasurement);
			index = TO_INT(m_measureList.size() - 1);

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

		if (index < 0 || index >= TO_INT(m_measureList.size()))
		{
			return nullptr;
		}

		return m_measureList[static_cast<quint64>(index)];
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Base::remove(int index)
	{
		QMutexLocker locker(&m_measureMutex);

		if (index < 0 || index >= TO_INT(m_measureList.size()))
		{
			return false;
		}

		auto it = m_measureList.cbegin();

		std::advance(it, index);

		Hash signalHash = UNDEFINED_HASH;

		auto pMeasurement = *it;
		if (pMeasurement != nullptr)
		{
			signalHash = pMeasurement->signalHash();

			delete pMeasurement;
		}

		m_measureList.erase(it);

		if (signalHash != UNDEFINED_HASH)
		{
			emit updatedMeasureBase(signalHash);
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Base::remove(Measure::Type measureType, const std::vector<int>& keyList)
	{
		if (ERR_MEASURE_TYPE(measureType) == true)
		{
			return false;
		}

		int keyCount = TO_INT(keyList.size());
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

				if (pMeasurement->measureID() != keyList[static_cast<quint64>(k)])
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

	void Base::removeFromBase(Measure::Type measureType, const std::vector<int>& keyList)
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

		MT::ErrorType errorType = MT::ErrorType::Reduce;

		switch (measureType)
		{
			case Measure::Type::Linearity:		errorType = theOptions.linearity().errorType();		break;
			case Measure::Type::Comparators:	errorType = theOptions.comparator().errorType();	break;

			default:
				assert(0);
				return;
		}

		if (ERR_MEASURE_ERROR_TYPE(errorType) == true)
		{
			assert(0);
			return;
		}

		QMutexLocker l(&m_measureMutex);

		si.setMeasureCount(0);
		si.setState(StatisticsItem::State::Success);

		int measureCount = TO_INT(m_measureList.size());
		for(int i = 0; i < measureCount; i ++)
		{
			Item* pMeasurement = m_measureList[static_cast<quint64>(i)];
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

			LimitType limitType = LimitType::NoLimitType;

			switch (measureType)
			{
				case Measure::Type::Linearity:		limitType = pMeasurement->limitTypeByRange(theOptions.linearity().calcErrorByRange());	break;
				case Measure::Type::Comparators:	limitType = pMeasurement->limitTypeByRange(theOptions.comparator().calcErrorByRange());	break;

				default:
					assert(0);
					continue;
			}

			if (ERR_MEASURE_LIMIT_TYPE(limitType) == true)
			{
				assert(0);
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
			if (pMeasurement == nullptr || pMeasurement->signalHash() == UNDEFINED_HASH)
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

	void Base::markWrongRangeFromStatistics(Measure::Base* pThis)
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

			pMeasurement->setHasWrongRange(false);

			int count = theSignalBase.statistics().count(measureType);
			for(int s = 0; s < count; s++)
			{
				const StatisticsItem& si = theSignalBase.statistics().item(measureType, s);

				Metrology::Signal* pSignal = si.signal();
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					continue;;
				}

				if (pMeasurement->appSignalID() != pSignal->param().appSignalID())
				{
					continue;
				}

				if (pMeasurement->rangeIsOkInStatisticsItem(si) == false)
				{
					pMeasurement->setHasWrongRange(true);
				}
			}
		}

		emit pThis->updateMeasureView();

		qDebug() << __FUNCTION__ << "- Signals were marked, " << " Time for marked: " << responseTime.elapsed() << " ms";
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	bool ERR_MEASURE_TYPE(Measure::Type measureType)
	{
		return ERR_MEASURE_TYPE(static_cast<int>(measureType));
	}

	bool ERR_MEASURE_TYPE(int measureType)
	{
		if (measureType < 0 || measureType >= Measure::TYPE_COUNT)
		{
			return true;
		}

		return false;
	}

	QString TypeCaption(Measure::Type measureType)
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
	}

	bool ERR_MEASURE_KIND(Measure::Kind measureKind)
	{
		return ERR_MEASURE_KIND(static_cast<int>(measureKind));
	}

	bool ERR_MEASURE_KIND(int measureKind)
	{
		if (measureKind < 0 || measureKind >= Measure::KIND_COUNT)
		{
			return true;
		}

		return false;
	}

	QString KindCaption(Measure::Kind measureKind)
	{
		QString caption;

		switch (measureKind)
		{

			case Measure::Kind::OneRack:		caption = QT_TRANSLATE_NOOP("MeasureBase", " Single channel");		break;
			case Measure::Kind::OneModule:		caption = QT_TRANSLATE_NOOP("MeasureBase", " Single module");		break;
			case Measure::Kind::MultiRack:		caption = QT_TRANSLATE_NOOP("MeasureBase", " Multi channel");		break;
			case Measure::Kind::MultiRack_MC:	caption = QT_TRANSLATE_NOOP("MeasureBase", " Multi channel - MC");	break;

			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	}

	bool ERR_MEASURE_LIMIT_TYPE(Measure::LimitType limitType)
	{
		return ERR_MEASURE_LIMIT_TYPE(static_cast<int>(limitType));
	}

	bool ERR_MEASURE_LIMIT_TYPE(int limitType)
	{
		if (limitType < 0 || limitType >= Measure::LIMIT_TYPE_COUNT)
		{
			return true;
		}

		return false;
	}

	QString LimitTypeCaption(Measure::LimitType limitType)
	{
		QString caption;

		switch (limitType)
		{
			case Measure::LimitType::Electric:		caption = QT_TRANSLATE_NOOP("MeasureBase", "Electric");		break;
			case Measure::LimitType::Engineering:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Engineering");	break;

			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasureBase", "Unknown");
		}

		return caption;
	};

	bool ERR_MEASURE_ERROR_RESULT(Measure::ErrorResult errorResult)
	{
		return ERR_MEASURE_ERROR_RESULT(static_cast<int>(errorResult));
	}

	bool ERR_MEASURE_ERROR_RESULT(int errorResult)
	{
		if (errorResult < 0 || errorResult >= Measure::ERROR_RESULT_COUNT)
		{
			return true;
		}

		return false;
	}

	QString ErrorResultCaption(Measure::ErrorResult errorResult)
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

	bool ERR_MEASURE_ADDITIONAL_PARAM(Measure::AdditionalParam param)
	{
		return ERR_MEASURE_ADDITIONAL_PARAM(static_cast<int>(param));
	}

	bool ERR_MEASURE_ADDITIONAL_PARAM(int param)
	{
		if (param < 0 || param >= Measure::ADDITIONAL_PARAM_COUNT)
		{
			return true;
		}

		return false;
	}

	QString MeasureAdditionalParamCaption(Measure::AdditionalParam param)
	{
		QString caption;

		switch (param)
		{
			case Measure::AdditionalParam::MaxDeviation:		caption = QT_TRANSLATE_NOOP("MeasureBase", "Maximum deviation");	break;
			case Measure::AdditionalParam::SystemDeviation:		caption = QT_TRANSLATE_NOOP("MeasureBase", "System deviation");		break;
			case Measure::AdditionalParam::StandardDeviation:	caption = QT_TRANSLATE_NOOP("MeasureBase", "Standard deviation");	break;
			case Measure::AdditionalParam::LowBorder:			caption = QT_TRANSLATE_NOOP("MeasureBase", "Low border");			break;
			case Measure::AdditionalParam::HighBorder:			caption = QT_TRANSLATE_NOOP("MeasureBase", "High border");			break;
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

double conversionByConnection(double val, const IoSignalParam &ioParam, ConversionDirection directType)
{
	int connectionType = ioParam.connectionType();
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return val;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
	if (inParam.isValid() == false)
	{
		return val;
	}

	const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
	if (outParam.isValid() == false)
	{
		return val;
	}

	UnitsConverter uc;

	double retVal = uc.conversionByConnection(val, connectionType, inParam, outParam, directType);
	return retVal;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
