#include "UnitsConverter.h"
#include "UnitsConverterTable.h"
#include "MetrologyConnection.h"

// -------------------------------------------------------------------------------------------------------------------
//
// UnitsConvertResult class implementation
//
// -------------------------------------------------------------------------------------------------------------------

UnitsConvertResult::UnitsConvertResult()
{
}

UnitsConvertResult::UnitsConvertResult(double result)
{
	m_ok = true;
	m_result = result;
}

UnitsConvertResult::UnitsConvertResult(UnitsConvertResultError errorCode, const QString& errorMessage)
{
	if (errorCode != UnitsConvertResultError::Generic)
	{
		assert(false);
		errorCode = UnitsConvertResultError::Generic;
	}

	m_ok = false;

	m_errorCode = errorCode;

	m_errorMessage = errorMessage;
}

UnitsConvertResult::UnitsConvertResult(UnitsConvertResultError errorCode, double expectedLowValidRange, double expectedHighValidRange)
{
	if (errorCode != UnitsConvertResultError::LowLimitOutOfRange &&
			errorCode != UnitsConvertResultError::HighLimitOutOfRange)
	{
		assert(false);
		errorCode = UnitsConvertResultError::Generic;
		m_errorMessage = QString("Internal error, wrong error code");
	}

	m_ok = false;

	m_errorCode = errorCode;

	m_expectedLowValidRange = expectedLowValidRange;
	m_expectedHighValidRange = expectedHighValidRange;

}

bool UnitsConvertResult::ok() const
{
	return m_ok;
}

bool UnitsConvertResult::isEqual(double value) const
{
	return std::nextafter(m_result, std::numeric_limits<double>::lowest()) <= value && std::nextafter(m_result, std::numeric_limits<double>::max()) >= value;
}

double UnitsConvertResult::toDouble() const
{
	return m_result;
}

int UnitsConvertResult::errorCode() const
{
	return static_cast<int>(m_errorCode);
}

QString UnitsConvertResult::errorMessage() const
{
	return m_errorMessage;
}

double UnitsConvertResult::expectedLowValidRange() const
{
	return m_expectedLowValidRange;
}

double UnitsConvertResult::expectedHighValidRange() const
{
	return m_expectedHighValidRange;
}

// -------------------------------------------------------------------------------------------------------------------
//
// SignalElectricLimit struct implementation
//
// -------------------------------------------------------------------------------------------------------------------

bool SignalElectricLimit::isValid() const
{
	if (unit == E::ElectricUnit::NoUnit)
	{
		return false;
	}

	if (sensorType == E::SensorType::NoSensor)
	{
		return false;
	}

	if (lowLimit == 0.0 && highLimit == 0.0)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
//
// UnitsConvert class implementation
//
// -------------------------------------------------------------------------------------------------------------------

UnitsConverter::UnitsConverter(QObject *parent)
	: QObject(parent)
{
	static bool UnitsConvertResultRegistered = false;
	if (UnitsConvertResultRegistered == false)
	{
		UnitsConvertResultRegistered = true;
		qRegisterMetaType<UnitsConvertResult>();
	}
}

UnitsConverter::~UnitsConverter()
{
}

double UnitsConverter::conversion(double val, UnitsConvertType conversionType, const AppSignal& signal)
{
	double retVal = 0;

	switch(conversionType)
	{
		case UnitsConvertType::PhysicalToElectric:

			switch(signal.electricUnit())
			{
				case E::ElectricUnit::Ohm:

					switch(signal.sensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:

							if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
							{
								break;
							}

							retVal = conversionLinearity(val, conversionType, signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.electricLowLimit(), signal.electricHighLimit());

							break;

						default:

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
							{
								break;
							}

							retVal = conversionDegree(val, conversionType, signal.electricUnit(), signal.sensorType(), r0_from_signal(signal));

							break;
					}

					break;

				case E::ElectricUnit::mV:

					switch(signal.sensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:
						case E::SensorType::mV_Raw_m1200_p1200:

							if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
							{
								break;
							}

							retVal = conversionLinearity(val, conversionType, signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.electricLowLimit(), signal.electricHighLimit());

							break;

						default:

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
							{
								break;
							}

							retVal = conversionDegree(val, conversionType, signal.electricUnit(), signal.sensorType());

							break;
					}

					break;

				case E::ElectricUnit::NoUnit:
				case E::ElectricUnit::mA:
				case E::ElectricUnit::V:
				case E::ElectricUnit::uA:
				case E::ElectricUnit::Hz:

					if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
					{
						break;
					}

					if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
					{
						break;
					}

					retVal = conversionLinearity(val, conversionType, signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.electricLowLimit(), signal.electricHighLimit());

					break;

				default:
					assert(0);
			}

			break;

		case UnitsConvertType::ElectricToPhysical:

			switch(signal.electricUnit())
			{
				case E::ElectricUnit::Ohm:

					switch(signal.sensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:

							if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
							{
								break;
							}

							retVal = conversionLinearity(val, conversionType, signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.electricLowLimit(), signal.electricHighLimit());

							break;

						default:

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
							{
								break;
							}

							retVal = conversionDegree(val, conversionType, signal.electricUnit(), signal.sensorType(), r0_from_signal(signal));

							break;
					}

					break;

				case E::ElectricUnit::mV:

					switch(signal.sensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:
						case E::SensorType::mV_Raw_m1200_p1200:

							if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
							{
								break;
							}

							retVal = conversionLinearity(val, conversionType, signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.electricLowLimit(), signal.electricHighLimit());

							break;

						default:

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
							{
								break;
							}

							retVal = conversionDegree(val, conversionType, signal.electricUnit(), signal.sensorType());

							break;
					}

					break;

				case E::ElectricUnit::NoUnit:
				case E::ElectricUnit::mA:
				case E::ElectricUnit::V:
				case E::ElectricUnit::uA:
				case E::ElectricUnit::Hz:

					if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
					{
						break;
					}

					if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
					{
						break;
					}

					retVal = conversionLinearity(val, conversionType, signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.electricLowLimit(), signal.electricHighLimit());

					break;

				default:
					assert(0);
			}

			break;

		case UnitsConvertType::CelsiusToFahrenheit:

			retVal = conversionDegree(val, UnitsConvertType::CelsiusToFahrenheit);

			break;

		case UnitsConvertType::FahrenheitToCelsius:

			retVal = conversionDegree(val, UnitsConvertType::FahrenheitToCelsius);

			break;

		default:
			assert(0);
	}

	return retVal;
}

double UnitsConverter::conversionLinearity(double val, UnitsConvertType conversionType, double lowEn, double highEn, double lowEl, double highEl)
{
	double retVal = 0;

	switch(conversionType)
	{
		case UnitsConvertType::PhysicalToElectric:

			retVal = (val - lowEn)*(highEl - lowEl)/(highEn - lowEn) + lowEl;

			break;


		case UnitsConvertType::ElectricToPhysical:

			retVal = (val - lowEl)*(highEn - lowEn)/(highEl - lowEl) + lowEn;

			break;

		default:
			assert(0);
	}

	return retVal;
}

double UnitsConverter::conversionDegree(double val, UnitsConvertType conversionType, E::ElectricUnit unitID, E::SensorType sensorType, double r0)
{
	double retVal = 0;

	switch(conversionType)
	{
		case UnitsConvertType::PhysicalToElectric:

			switch(unitID)
			{
				case E::ElectricUnit::Ohm:

					if (r0_OhmIsValid(r0) == false)
					{
						r0 = default_r0(sensorType);
					}

					switch(sensorType)
					{
						//
						//
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:			retVal = val;	break;

						//
						//
						case E::SensorType::Ohm_Pt_a_391:		retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, true);								break;
						case E::SensorType::Ohm_Pt_a_385:		retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, true);								break;
						case E::SensorType::Ohm_Cu_a_428:		retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, true);								break;
						case E::SensorType::Ohm_Cu_a_426:		retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, true);								break;
						case E::SensorType::Ohm_Ni_a_617:		retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, true);								break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_22[0][0], PT_22_COUNT, true);												break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_24[0][0], CU_24_COUNT, true);												break;

						// for non ptaform module
						//
						case E::SensorType::Ohm_Pt50_W1391:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, r0);			break;
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, r0);			break;
						case E::SensorType::Ohm_Pt50_W1385:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, r0);			break;
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, r0);			break;

						case E::SensorType::Ohm_Cu50_W1428:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, r0);			break;
						case E::SensorType::Ohm_Cu100_W1428:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, r0);			break;
						case E::SensorType::Ohm_Cu50_W1426:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, r0);			break;
						case E::SensorType::Ohm_Cu100_W1426:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, r0);			break;

						case E::SensorType::Ohm_Ni50_W1617:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, r0);			break;
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, r0);			break;

						default:
							assert(0);
					}

					if (r0_is_use(sensorType) == true)
					{
						retVal = retVal * r0 / 100;
					}

					break;

				case E::ElectricUnit::mV:

					switch(sensorType)
					{
						//
						//
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:
						case E::SensorType::mV_Raw_m1200_p1200:	retVal = val;																		break;

						//
						//
						case E::SensorType::mV_K_TXA: 			retVal = findConversionVal(val, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, true);			break;
						case E::SensorType::mV_L_TXK:			retVal = findConversionVal(val, &MV_TYPE_L[0][0], MV_TYPE_L_COUNT, true);			break;
						case E::SensorType::mV_N_THH:			retVal = findConversionVal(val, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, true);			break;

						//
						//
						case E::SensorType::mV_Type_B:			retVal = findConversionVal(val, &MV_TYPE_B[0][0], MV_TYPE_B_COUNT, true);			break;
						case E::SensorType::mV_Type_E:			retVal = findConversionVal(val, &MV_TYPE_E[0][0], MV_TYPE_E_COUNT, true);			break;
						case E::SensorType::mV_Type_J:			retVal = findConversionVal(val, &MV_TYPE_J[0][0], MV_TYPE_J_COUNT, true);			break;
						case E::SensorType::mV_Type_K:			retVal = findConversionVal(val, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, true);			break;
						case E::SensorType::mV_Type_N:			retVal = findConversionVal(val, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, true);			break;
						case E::SensorType::mV_Type_R:			retVal = findConversionVal(val, &MV_TYPE_R[0][0], MV_TYPE_R_COUNT, true);			break;
						case E::SensorType::mV_Type_S:			retVal = findConversionVal(val, &MV_TYPE_S[0][0], MV_TYPE_S_COUNT, true);			break;
						case E::SensorType::mV_Type_T:			retVal = findConversionVal(val, &MV_TYPE_T[0][0], MV_TYPE_T_COUNT, true);			break;
						case E::SensorType::mV_Type_L:			retVal = findConversionVal(val, &MV_TYPE_L[0][0], MV_TYPE_L_COUNT, true);			break;
						case E::SensorType::mV_Type_M:			retVal = findConversionVal(val, &MV_TYPE_M[0][0], MV_TYPE_M_COUNT, true);			break;

						default:
							assert(0);
					}

					break;

				default:
					assert(0);
			}
			break;

		case UnitsConvertType::ElectricToPhysical:

			switch(unitID)
			{
				case E::ElectricUnit::Ohm:

					if (r0_OhmIsValid(r0) == false)
					{
						r0 = default_r0(sensorType);
					}

					if (r0_is_use(sensorType) == true)
					{
						val = val / r0 * 100;
					}

					switch(sensorType)
					{
						//
						//
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:			retVal = val;	break;

						//
						//
						case E::SensorType::Ohm_Pt_a_391:		retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, false);						break;
						case E::SensorType::Ohm_Pt_a_385:		retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, false);						break;
						case E::SensorType::Ohm_Cu_a_428:		retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, false);						break;
						case E::SensorType::Ohm_Cu_a_426:		retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, false);						break;
						case E::SensorType::Ohm_Ni_a_617:		retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, false);						break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_22[0][0], PT_22_COUNT, false);										break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_24[0][0], CU_24_COUNT, false);										break;

						// for non ptaform module
						//
						case E::SensorType::Ohm_Pt50_W1391:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, r0);	break;
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, r0);	break;
						case E::SensorType::Ohm_Pt50_W1385:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, r0);	break;
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, r0);	break;

						case E::SensorType::Ohm_Cu50_W1428:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, r0);	break;
						case E::SensorType::Ohm_Cu100_W1428:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, r0);	break;
						case E::SensorType::Ohm_Cu50_W1426:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, r0);	break;
						case E::SensorType::Ohm_Cu100_W1426:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, r0);	break;

						case E::SensorType::Ohm_Ni50_W1617:		retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, r0);	break;
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, r0);	break;

						default:
							assert(0);
					}
					break;

				case E::ElectricUnit::mV:

					switch(sensorType)
					{
						//
						//
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:
						case E::SensorType::mV_Raw_m1200_p1200:	retVal = val;																		break;

						//
						//
						case E::SensorType::mV_K_TXA: 			retVal = findConversionVal(val, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, false);			break;
						case E::SensorType::mV_L_TXK:			retVal = findConversionVal(val, &MV_TYPE_L[0][0], MV_TYPE_L_COUNT, false);			break;
						case E::SensorType::mV_N_THH:			retVal = findConversionVal(val, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, false);			break;

						//
						//
						case E::SensorType::mV_Type_B:			retVal = findConversionVal(val, &MV_TYPE_B[0][0], MV_TYPE_B_COUNT, false);			break;
						case E::SensorType::mV_Type_E:			retVal = findConversionVal(val, &MV_TYPE_E[0][0], MV_TYPE_E_COUNT, false);			break;
						case E::SensorType::mV_Type_J:			retVal = findConversionVal(val, &MV_TYPE_J[0][0], MV_TYPE_J_COUNT, false);			break;
						case E::SensorType::mV_Type_K:			retVal = findConversionVal(val, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, false);			break;
						case E::SensorType::mV_Type_N:			retVal = findConversionVal(val, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, false);			break;
						case E::SensorType::mV_Type_R:			retVal = findConversionVal(val, &MV_TYPE_R[0][0], MV_TYPE_R_COUNT, false);			break;
						case E::SensorType::mV_Type_S:			retVal = findConversionVal(val, &MV_TYPE_S[0][0], MV_TYPE_S_COUNT, false);			break;
						case E::SensorType::mV_Type_T:			retVal = findConversionVal(val, &MV_TYPE_T[0][0], MV_TYPE_T_COUNT, false);			break;
						case E::SensorType::mV_Type_L:			retVal = findConversionVal(val, &MV_TYPE_L[0][0], MV_TYPE_L_COUNT, false);			break;
						case E::SensorType::mV_Type_M:			retVal = findConversionVal(val, &MV_TYPE_M[0][0], MV_TYPE_M_COUNT, false);			break;

						default:
							assert(0);
					}
					break;

				default:
					assert(0);
			}
			break;

		case UnitsConvertType::CelsiusToFahrenheit:

			retVal = conversionDegree(val, UnitsConvertType::CelsiusToFahrenheit);

			break;

		case UnitsConvertType::FahrenheitToCelsius:

			retVal = conversionDegree(val, UnitsConvertType::FahrenheitToCelsius);

			break;

		default:
			assert(0);
	}

	return retVal;
}

double UnitsConverter::conversionDegree(double val, UnitsConvertType conversionType)
{
	double retVal = 0;

	switch (conversionType)
	{
		case UnitsConvertType::CelsiusToFahrenheit:	retVal = (val * (9.0 / 5.0)) + 32;		break;
		case UnitsConvertType::FahrenheitToCelsius:	retVal = ((val - 32) * (5.0 / 9.0));	break;

		default:
			assert(0);
	}

	return retVal;
}

double UnitsConverter::conversionByConnection(double val, int connectionType, const AppSignal& sourSignal, const AppSignal& destSignal, ConversionDirection directType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return val;
	}

	if (sourSignal.hash() == UNDEFINED_HASH)
	{
		return val;
	}

	if (sourSignal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
		sourSignal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
	{
		return val;
	}

	if (destSignal.hash() == UNDEFINED_HASH)
	{
		return val;
	}

	if (destSignal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
		destSignal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
	{
		return val;
	}

	double retVal = val;

	switch (directType)
	{
		case ConversionDirection::Normal:

			switch (connectionType)
			{
				case Metrology::ConnectionType::Input_DP_Internal_F:
					{
						double K = (destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) /
								sqrt(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits());

						retVal = K * sqrt( val );
					}
					break;

				case Metrology::ConnectionType::Input_DP_Output_F:
				{
					val = (val - destSignal.lowEngineeringUnits())*
							(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits())/
							(destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) +
							sourSignal.lowEngineeringUnits();

					double K = (destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) /
							sqrt(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits());

					retVal = K * sqrt( val );
				}
				break;

				case Metrology::ConnectionType::Input_C_Internal_F:
					{
						retVal = conversionDegree(val, UnitsConvertType::CelsiusToFahrenheit);
					}
					break;

				case Metrology::ConnectionType::Input_C_Output_F:
					{
						val = (val - destSignal.lowEngineeringUnits())*
								(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits())/
								(destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) +
								sourSignal.lowEngineeringUnits();

						retVal = conversionDegree(val, UnitsConvertType::CelsiusToFahrenheit);
					}
					break;

				default:
					{
						retVal = (val - sourSignal.lowEngineeringUnits())*
								(destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits())/
								(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits()) +
								destSignal.lowEngineeringUnits();
					}
					break;
			}

			break;

		case ConversionDirection::Inversion:

			switch (connectionType)
			{
				case Metrology::ConnectionType::Input_DP_Internal_F:
					{
						double K = (destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) /
								sqrt(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits());

						retVal = pow(val / K, 2);
					}
					break;

				case Metrology::ConnectionType::Input_DP_Output_F:
					{
						val = (val - sourSignal.lowEngineeringUnits())*
								(destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits())/
								(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits()) +
								destSignal.lowEngineeringUnits();

						double K = (destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) /
								sqrt(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits());

						retVal = pow(val / K, 2);
					}
					break;

				case Metrology::ConnectionType::Input_C_Internal_F:
					{
						retVal = conversionDegree(val, UnitsConvertType::FahrenheitToCelsius);
					}
					break;

				case Metrology::ConnectionType::Input_C_Output_F:
					{
						val = (val - sourSignal.lowEngineeringUnits())*
								(destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits())/
								(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits()) +
								destSignal.lowEngineeringUnits();

						retVal = conversionDegree(val, UnitsConvertType::FahrenheitToCelsius);
					}
					break;

				default:
					{
						retVal = (val - destSignal.lowEngineeringUnits())*
								(sourSignal.highEngineeringUnits() - sourSignal.lowEngineeringUnits())/
								(destSignal.highEngineeringUnits() - destSignal.lowEngineeringUnits()) +
								sourSignal.lowEngineeringUnits();
					}
					break;
			}

			break;

		default:
			Q_ASSERT(0);		// undefinded ConversionDirection
	}

	return retVal;
}

double UnitsConverter::r0_from_signal(const AppSignal& signal)
{
	double r0 = 0;

	if (signal.isSpecPropExists(AppSignalPropNames::R0_OHM) == true)
	{
		r0 = signal.r0_Ohm();
	}
	else
	{
		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == true)
		{
			r0  = default_r0(signal.sensorType());
		}
	}

	return r0;
}

bool UnitsConverter::r0_is_use(E::SensorType sensorType)
{
	bool result = false;

	switch(sensorType)
	{
		case E::SensorType::Ohm_Pt_a_391:
		case E::SensorType::Ohm_Pt_a_385:
		case E::SensorType::Ohm_Cu_a_428:
		case E::SensorType::Ohm_Cu_a_426:
		case E::SensorType::Ohm_Ni_a_617:

		case E::SensorType::Ohm_Pt21:
		case E::SensorType::Ohm_Cu23:

			result = true;

			break;

		default:

			result = false;
	}

	return result;
}

double UnitsConverter::default_r0(E::SensorType sensorType)
{
	double r0 = 0;

	switch(sensorType)
	{
		//
		//
		case E::SensorType::NoSensor:
		case E::SensorType::Ohm_Raw:			r0 = 0;		break;

		//
		//
		case E::SensorType::Ohm_Pt_a_391:
		case E::SensorType::Ohm_Pt_a_385:
		case E::SensorType::Ohm_Cu_a_428:
		case E::SensorType::Ohm_Cu_a_426:
		case E::SensorType::Ohm_Ni_a_617:		r0 = 100;	break;

		case E::SensorType::Ohm_Pt21:			r0 = 46;	break;
		case E::SensorType::Ohm_Cu23:			r0 = 53;	break;

		// for non ptaform module
		//
		case E::SensorType::Ohm_Pt50_W1391:		r0 = 50;	break;
		case E::SensorType::Ohm_Pt100_W1391:	r0 = 100;	break;
		case E::SensorType::Ohm_Pt50_W1385:		r0 = 50;	break;
		case E::SensorType::Ohm_Pt100_W1385:	r0 = 100;	break;

		case E::SensorType::Ohm_Cu50_W1428:		r0 = 50;	break;
		case E::SensorType::Ohm_Cu100_W1428:	r0 = 100;	break;
		case E::SensorType::Ohm_Cu50_W1426:		r0 = 50;	break;
		case E::SensorType::Ohm_Cu100_W1426:	r0 = 100;	break;

		case E::SensorType::Ohm_Ni50_W1617:		r0 = 50;	break;
		case E::SensorType::Ohm_Ni100_W1617:	r0 = 100;	break;

		default:
			assert(0);
	}

	return r0;
}

SignalElectricLimit UnitsConverter::getElectricLimit(E::ElectricUnit unit, E::SensorType sensorType)
{
	auto it = m_electricLimits.find({ unit, sensorType });

	if (it == m_electricLimits.end())
	{
		return SignalElectricLimit();
	}

	return it->second;
}

UnitsConvertResult UnitsConverter::electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit,
														E::ElectricUnit unitID, E::SensorType sensorType, double r0)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	SignalElectricLimit el = getElectricLimit(unitID, sensorType);

	if(el.isValid() == false)
	{
		assert(false);
		QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for %1").arg(meu.key(unitID)));
	}

	double lowLimit = el.lowLimit;
	double highLimit = el.highLimit;

	if (unitID == E::ElectricUnit::Ohm)
	{
		if (sensorType != E::SensorType::NoSensor && sensorType != E::SensorType::Ohm_Raw)
		{
			if (r0_OhmIsValid(r0) == false)
			{
				return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect R0 for Ohm"));
			}

			lowLimit = lowLimit * r0 / 100;
			highLimit = highLimit * r0 / 100;
		}
	}

	if (electricLowLimit < lowLimit || electricLowLimit > highLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::LowLimitOutOfRange, lowLimit, highLimit);
	}

	if (electricHighLimit < lowLimit || electricHighLimit > highLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::HighLimitOutOfRange, lowLimit, highLimit);
	}

	return UnitsConvertResult(elVal);
}

UnitsConvertResult UnitsConverter::electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit,
										int unitID, int sensorType, double r0)
{
	return electricLimitIsValid(elVal, electricLowLimit, electricHighLimit,
								static_cast<E::ElectricUnit>(unitID),
								static_cast<E::SensorType>(sensorType), r0);
}

bool UnitsConverter::rloadIsValid(double rload)
{
	return rload >= RLOAD_OHM_LOW_LIMIT && rload <= RLOAD_OHM_HIGH_LIMIT;
}

bool UnitsConverter::r0_OhmIsValid(double r0_Ohm)
{
	return r0_Ohm >= R0_OHM_LOW_LIMIT && r0_Ohm <= R0_OHM_HIGH_LIMIT;
}

bool UnitsConverter::isSensorValid(E::ElectricUnit electricUnit, E::SensorType sensorType)
{
	auto it1 = electricUnitSensors.find(electricUnit);

	if (it1 == electricUnitSensors.end())
	{
		return false;
	}

	const std::set<E::SensorType> sensors = it1->second;

	return sensors.contains(sensorType);
}

QString UnitsConverter::electricUnitName(E::ElectricUnit unit)
{
	QString unitName = E::valueToString(unit);

	if (unitName.isEmpty())
	{
		unitName = QStringLiteral("Unknown");
	}

	return unitName	;
}

QString UnitsConverter::sensorTypeName(E::SensorType sensorType)
{
	QString sensorName = E::valueToString(sensorType);

	if (sensorName.isEmpty())
	{
		sensorName = QStringLiteral("Unknown");
	}

	return sensorName;
}

UnitsConvertResult UnitsConverter::electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit,
															int unitID, int sensorType, double rload)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	switch(unitID)
	{
		case E::ElectricUnit::mA:
			{
				if (sensorType != E::SensorType::V_0_5 && sensorType != E::SensorType::V_m10_p10)
				{
					return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for mA"));
				}

				if (rloadIsValid(rload) == false)
				{
					return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Rload_Ohm argument is out of range"));
				}

				return  UnitsConvertResult(elVal * (rload / RLOAD_OHM_HIGH_LIMIT));
			}
			break;

		case E::ElectricUnit::V:
			{
				if (sensorType != E::SensorType::V_0_5 && sensorType != E::SensorType::V_m10_p10)
				{
					return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for V"));
				}

				UnitsConvertResult result = electricLimitIsValid(elVal, electricLowLimit, electricHighLimit, unitID, sensorType);

				if (result.ok() == true)
				{
					return UnitsConvertResult(elVal);
				}

				return result;
			}
			break;

		case E::ElectricUnit::uA:
			{
				if (sensorType != E::SensorType::uA_m20_p20)
				{
					return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for uA"));
				}

				UnitsConvertResult result = electricLimitIsValid(elVal, electricLowLimit, electricHighLimit, unitID, sensorType);

				if (result.ok() == true)
				{
					return UnitsConvertResult(elVal);
				}

				return result;
			}
			break;

		case E::ElectricUnit::Hz:
			{
				if (UnitsConverter::isSensorValid(E::ElectricUnit::Hz, static_cast<E::SensorType>(sensorType)) == false)
				{
					return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for Hz"));
				}

				UnitsConvertResult result = electricLimitIsValid(elVal, electricLowLimit, electricHighLimit, unitID, sensorType);

				if (result.ok() == true)
				{
					return UnitsConvertResult(elVal);
				}

				return result;
			}
			break;
	}

	return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown unitID"));
}

UnitsConvertResult UnitsConverter::electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	if (unitID != E::ElectricUnit::mV)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect unitID for mV"));
	}

	double phVal = 0;

	UnitsConvertResult result = electricLimitIsValid(elVal, electricLowLimit, electricHighLimit, unitID, sensorType);
	if (result.ok() == true)
	{
		phVal = conversionDegree(elVal, UnitsConvertType::ElectricToPhysical, static_cast<E::ElectricUnit>(unitID), static_cast<E::SensorType>(sensorType));
	}
	else
	{
		return result;
	}

	return UnitsConvertResult(phVal);
}

UnitsConvertResult UnitsConverter::electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	if (unitID != E::ElectricUnit::Ohm)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect unitID for Ohm"));
	}

	if (sensorType != E::SensorType::NoSensor && sensorType != E::SensorType::Ohm_Raw)
	{
		if (r0_OhmIsValid(r0) == false)
		{
			return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect R0 for Ohm"));
		}
	}

	double phVal = 0;

	UnitsConvertResult result = electricLimitIsValid(elVal, electricLowLimit, electricHighLimit, unitID, sensorType, r0);

	if (result.ok() == true)
	{
		phVal = conversionDegree(elVal, UnitsConvertType::ElectricToPhysical, static_cast<E::ElectricUnit>(unitID), static_cast<E::SensorType>(sensorType), r0);
	}
	else
	{
		return result;
	}

	return UnitsConvertResult(phVal);
}

UnitsConvertResult UnitsConverter::electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	double minElectricLowLimit = 0;
	double maxElectricHighLimit = 0;
	int waitUnitID = E::ElectricUnit::NoUnit;

	switch(outputMode)
	{
		case E::OutputMode::Plus0_Plus5_V:		minElectricLowLimit = 0;		maxElectricHighLimit = 5;	waitUnitID = E::ElectricUnit::V;	break;
		case E::OutputMode::Plus4_Plus20_mA:	minElectricLowLimit = 4;		maxElectricHighLimit = 20;	waitUnitID = E::ElectricUnit::mA;	break;
		case E::OutputMode::Minus10_Plus10_V:	minElectricLowLimit = -10;		maxElectricHighLimit = 10;	waitUnitID = E::ElectricUnit::V;	break;
		case E::OutputMode::Plus0_Plus5_mA:		minElectricLowLimit = 0;		maxElectricHighLimit = 5;	waitUnitID = E::ElectricUnit::mA;	break;
		case E::OutputMode::Plus0_Plus20_mA:	minElectricLowLimit = 0;		maxElectricHighLimit = 20;	waitUnitID = E::ElectricUnit::mA;	break;
		case E::OutputMode::Plus0_Plus24_mA:	minElectricLowLimit = 0;		maxElectricHighLimit = 24;	waitUnitID = E::ElectricUnit::mA;	break;

		default:
			assert(false);
			return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown OutputMode"));
	}

	if (electricLowLimit < minElectricLowLimit || electricLowLimit > maxElectricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::LowLimitOutOfRange, minElectricLowLimit, maxElectricHighLimit);
	}

	if (electricHighLimit < minElectricLowLimit || electricHighLimit > maxElectricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::HighLimitOutOfRange, minElectricLowLimit, maxElectricHighLimit);
	}

	if (waitUnitID != unitID)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect electric unit: \"%1\" for mode: \"%2\"").arg(QMetaEnum::fromType<E::ElectricUnit>().key(unitID), QMetaEnum::fromType<E::OutputMode>().key(outputMode)));
	}

	double phVal = (elVal - electricLowLimit) * (OUT_PH_HIGH_LIMIT - OUT_PH_LOW_LIMIT) / (electricHighLimit - electricLowLimit) + OUT_PH_LOW_LIMIT;

	return UnitsConvertResult(phVal);
}

QString UnitsConverter::electricUnitName(int electricUnit) const
{
	return electricUnitName(static_cast<E::ElectricUnit>(electricUnit));
}

QString UnitsConverter::sensorTypeName(int sensorType) const
{
	return sensorTypeName(static_cast<E::SensorType>(sensorType));
}
