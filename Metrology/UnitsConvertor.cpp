#include "UnitsConvertor.h"
#include "UnitsConvertorTable.h"
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

bool SignalElectricLimit::isValid()
{
	if (lowLimit == 0.0 && highLimit == 0.0)
	{
		return false;
	}

	if (unit == E::ElectricUnit::NoUnit)
	{
		return false;
	}

	if (sensorType == E::SensorType::NoSensor)
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

UnitsConvertor::UnitsConvertor(QObject *parent)
	: QObject(parent)
{
	static bool UnitsConvertResultRegistered = false;
	if (UnitsConvertResultRegistered == false)
	{
		UnitsConvertResultRegistered = true;
		qRegisterMetaType<UnitsConvertResult>();
	}
}

UnitsConvertor::~UnitsConvertor()
{
}

double UnitsConvertor::conversion(double val, const UnitsConvertType& conversionType, const AppSignal& signal)
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

							retVal = (val - signal.lowEngineeringUnits())*(signal.electricHighLimit() - signal.electricLowLimit())/(signal.highEngineeringUnits() - signal.lowEngineeringUnits()) + signal.electricLowLimit();

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

							if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
							{
								break;
							}

							retVal = (val - signal.lowEngineeringUnits())*(signal.electricHighLimit() - signal.electricLowLimit())/(signal.highEngineeringUnits() - signal.lowEngineeringUnits()) + signal.electricLowLimit();

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

					retVal = (val - signal.lowEngineeringUnits())*(signal.electricHighLimit() - signal.electricLowLimit())/(signal.highEngineeringUnits() - signal.lowEngineeringUnits()) + signal.electricLowLimit();

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

							retVal = (val - signal.electricLowLimit())*(signal.highEngineeringUnits() - signal.lowEngineeringUnits())/(signal.electricHighLimit() - signal.electricLowLimit()) + signal.lowEngineeringUnits();

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

							if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false || signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
							{
								break;
							}

							if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false || signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
							{
								break;
							}

							retVal = (val - signal.electricLowLimit())*(signal.highEngineeringUnits() - signal.lowEngineeringUnits())/(signal.electricHighLimit() - signal.electricLowLimit()) + signal.lowEngineeringUnits();

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

					retVal = (val - signal.electricLowLimit())*(signal.highEngineeringUnits() - signal.lowEngineeringUnits())/(signal.electricHighLimit() - signal.electricLowLimit()) + signal.lowEngineeringUnits();

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

double UnitsConvertor::conversionDegree(double val, const UnitsConvertType& conversionType, const E::ElectricUnit& unitID, const E::SensorType& sensorType, double r0)
{
	double retVal = 0;

	switch(conversionType)
	{
		case UnitsConvertType::PhysicalToElectric:

			switch(unitID)
			{
				case E::ElectricUnit::Ohm:

					switch(sensorType)
					{
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:			retVal = val;																									break;

						case E::SensorType::Ohm_Pt_a_391:		retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, true);	retVal = retVal * r0 / 100;	break;
						case E::SensorType::Ohm_Pt_a_385:		retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, true);	retVal = retVal * r0 / 100;	break;
						case E::SensorType::Ohm_Cu_a_428:		retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, true);	retVal = retVal * r0 / 100;	break;
						case E::SensorType::Ohm_Cu_a_426:		retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, true);	retVal = retVal * r0 / 100;	break;
						case E::SensorType::Ohm_Ni_a_617:		retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, true);	retVal = retVal * r0 / 100;	break;

						case E::SensorType::Ohm_Pt50_W1391:
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, r0);			break;
						case E::SensorType::Ohm_Pt50_W1385:
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, r0);			break;

						case E::SensorType::Ohm_Cu50_W1428:
						case E::SensorType::Ohm_Cu100_W1428:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, r0);			break;
						case E::SensorType::Ohm_Cu50_W1426:
						case E::SensorType::Ohm_Cu100_W1426:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, r0);			break;

						case E::SensorType::Ohm_Ni50_W1617:
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, r0);			break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_21[0][0], PT_21_COUNT, true);												break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_23[0][0], CU_23_COUNT, true);												break;

						default:
							assert(0);
					}

					break;

				case E::ElectricUnit::mV:

					switch(sensorType)
					{
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:		retVal = val;																		break;

						case E::SensorType::mV_K_TXA: 			retVal = findConversionVal(val, &K_TXA[0][0], K_TXA_COUNT, true);					break;
						case E::SensorType::mV_L_TXK:			retVal = findConversionVal(val, &L_TXK[0][0], L_TXK_COUNT, true);					break;
						case E::SensorType::mV_N_THH:			retVal = findConversionVal(val, &N_THH[0][0], N_THH_COUNT, true);					break;

						case E::SensorType::mV_Type_B:			retVal = findConversionVal(val, &MV_TYPE_B[0][0], MV_TYPE_B_COUNT, true);			break;
						case E::SensorType::mV_Type_E:			retVal = findConversionVal(val, &MV_TYPE_E[0][0], MV_TYPE_E_COUNT, true);			break;
						case E::SensorType::mV_Type_J:			retVal = findConversionVal(val, &MV_TYPE_J[0][0], MV_TYPE_J_COUNT, true);			break;
						case E::SensorType::mV_Type_K:			retVal = findConversionVal(val, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, true);			break;
						case E::SensorType::mV_Type_N:			retVal = findConversionVal(val, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, true);			break;
						case E::SensorType::mV_Type_R:			retVal = findConversionVal(val, &MV_TYPE_R[0][0], MV_TYPE_R_COUNT, true);			break;
						case E::SensorType::mV_Type_S:			retVal = findConversionVal(val, &MV_TYPE_S[0][0], MV_TYPE_S_COUNT, true);			break;
						case E::SensorType::mV_Type_T:			retVal = findConversionVal(val, &MV_TYPE_T[0][0], MV_TYPE_T_COUNT, true);			break;

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

					switch(sensorType)
					{
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:			retVal = val;	break;

						case E::SensorType::Ohm_Pt_a_391:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, false);	break;
						case E::SensorType::Ohm_Pt_a_385:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, false);	break;
						case E::SensorType::Ohm_Cu_a_428:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, false);	break;
						case E::SensorType::Ohm_Cu_a_426:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, false);	break;
						case E::SensorType::Ohm_Ni_a_617:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, false);	break;


						case E::SensorType::Ohm_Pt50_W1391:
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, r0); break;
						case E::SensorType::Ohm_Pt50_W1385:
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, r0); break;

						case E::SensorType::Ohm_Cu50_W1428:
						case E::SensorType::Ohm_Cu100_W1428:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, r0); break;
						case E::SensorType::Ohm_Cu50_W1426:
						case E::SensorType::Ohm_Cu100_W1426:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, r0); break;

						case E::SensorType::Ohm_Ni50_W1617:
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversionDegree(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, r0); break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_21[0][0], PT_21_COUNT, false);	break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_23[0][0], CU_23_COUNT, false);	break;

						default:
							assert(0);
					}

					break;

				case E::ElectricUnit::mV:

					switch(sensorType)
					{
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:		retVal = val;																		break;

						case E::SensorType::mV_K_TXA: 			retVal = findConversionVal(val, &K_TXA[0][0], K_TXA_COUNT, false);					break;
						case E::SensorType::mV_L_TXK:			retVal = findConversionVal(val, &L_TXK[0][0], L_TXK_COUNT, false);					break;
						case E::SensorType::mV_N_THH:			retVal = findConversionVal(val, &N_THH[0][0], N_THH_COUNT, false);					break;

						case E::SensorType::mV_Type_B:			retVal = findConversionVal(val, &MV_TYPE_B[0][0], MV_TYPE_B_COUNT, false);			break;
						case E::SensorType::mV_Type_E:			retVal = findConversionVal(val, &MV_TYPE_E[0][0], MV_TYPE_E_COUNT, false);			break;
						case E::SensorType::mV_Type_J:			retVal = findConversionVal(val, &MV_TYPE_J[0][0], MV_TYPE_J_COUNT, false);			break;
						case E::SensorType::mV_Type_K:			retVal = findConversionVal(val, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, false);			break;
						case E::SensorType::mV_Type_N:			retVal = findConversionVal(val, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, false);			break;
						case E::SensorType::mV_Type_R:			retVal = findConversionVal(val, &MV_TYPE_R[0][0], MV_TYPE_R_COUNT, false);			break;
						case E::SensorType::mV_Type_S:			retVal = findConversionVal(val, &MV_TYPE_S[0][0], MV_TYPE_S_COUNT, false);			break;
						case E::SensorType::mV_Type_T:			retVal = findConversionVal(val, &MV_TYPE_T[0][0], MV_TYPE_T_COUNT, false);			break;

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

double UnitsConvertor::conversionDegree(double val, const UnitsConvertType& conversionType)
{
	double retVal = 0;

	switch (conversionType)
	{
		case UnitsConvertType::CelsiusToFahrenheit:	retVal = (val * (9.0 / 5.0)) + 32;		break;
		case UnitsConvertType::FahrenheitToCelsius:	retVal = ((val - 32) * (5.0 / 9.0));	break;
		default:									assert(0);
	}

	return retVal;
}

double UnitsConvertor::conversionByConnection(double val, int connectionType, const AppSignal& sourSignal, const AppSignal& destSignal, ConversionDirection directType)
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
			break;
	}

	return retVal;
}

double UnitsConvertor::r0_from_signal(const AppSignal& signal)
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
			switch(signal.sensorType())
			{
				case E::SensorType::Ohm_Pt50_W1391:		r0 = 50; break;
				case E::SensorType::Ohm_Pt100_W1391:	r0 = 100; break;
				case E::SensorType::Ohm_Pt50_W1385:		r0 = 50; break;
				case E::SensorType::Ohm_Pt100_W1385:	r0 = 100; break;

				case E::SensorType::Ohm_Cu50_W1428:		r0 = 50; break;
				case E::SensorType::Ohm_Cu100_W1428:	r0 = 100; break;
				case E::SensorType::Ohm_Cu50_W1426:		r0 = 50; break;
				case E::SensorType::Ohm_Cu100_W1426:	r0 = 100; break;

				case E::SensorType::Ohm_Ni50_W1617:		r0 = 50; break;
				case E::SensorType::Ohm_Ni100_W1617:	r0 = 100; break;
			}
		}
	}

	return r0;
}

bool UnitsConvertor::r0_is_use(int sensorType)
{
	if (	sensorType == E::SensorType::NoSensor || sensorType == E::SensorType::Ohm_Raw ||
			sensorType == E::SensorType::Ohm_Pt21 || sensorType == E::SensorType::Ohm_Cu23)
	{
		return false;
	}

	return true;
}

SignalElectricLimit UnitsConvertor::getElectricLimit(int unitID, int sensorType)
{
	SignalElectricLimit limit;

	for(int i = 0; i < SignalElectricLimitCount; i++)
	{
		const SignalElectricLimit& ul = SignalElectricLimits[i];

		if (ul.unit != unitID)
		{
			continue;
		}

		if (ul.sensorType != sensorType)
		{
			continue;
		}

		limit = ul;

		break;
	}

	return limit;
}

UnitsConvertResult UnitsConvertor::electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0)
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

	if (unitID == E::ElectricUnit::Ohm && r0_is_use(sensorType) == true)
	{
		lowLimit = lowLimit * r0 / 100;
		highLimit = highLimit * r0 / 100;
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

UnitsConvertModule UnitsConvertor::getModuleType(int unitID, int sensorType)
{
	UnitsConvertModule moduleType = UnitsConvertModule::NonPlatform;

	switch (unitID)
	{

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:			moduleType = UnitsConvertModule::AIM;	break;
			}

			break;


		case E::ElectricUnit::mV:

			switch (sensorType)
			{
				case E::SensorType::mV_Type_B:
				case E::SensorType::mV_Type_E:
				case E::SensorType::mV_Type_J:
				case E::SensorType::mV_Type_K:
				case E::SensorType::mV_Type_N:
				case E::SensorType::mV_Type_R:
				case E::SensorType::mV_Type_S:
				case E::SensorType::mV_Type_T:

				case E::SensorType::mV_Raw_Mul_8:
				case E::SensorType::mV_Raw_Mul_32:	moduleType = UnitsConvertModule::TIM;	break;
			}

			break;

		case E::ElectricUnit::Ohm:

			switch (sensorType)
			{
				case E::SensorType::Ohm_Pt_a_391:
				case E::SensorType::Ohm_Pt_a_385:
				case E::SensorType::Ohm_Cu_a_428:
				case E::SensorType::Ohm_Cu_a_426:
				case E::SensorType::Ohm_Ni_a_617:

				case E::SensorType::Ohm_Raw:		moduleType = UnitsConvertModule::RIM;	break;
			}

			break;

		case E::ElectricUnit::V:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:			moduleType = UnitsConvertModule::AIM;	break;
				case E::SensorType::V_m10_p10:		moduleType = UnitsConvertModule::WAIM;	break;
			}

			break;


		case E::ElectricUnit::uA:

			switch (sensorType)
			{
				case E::SensorType::uA_m20_p20:		moduleType = UnitsConvertModule::MAIM;	break;
			}

			break;

		case E::ElectricUnit::Hz:

			switch (sensorType)
			{
				case E::SensorType::Hz_005_50000:	moduleType = UnitsConvertModule::FIM;	break;
			}

			break;

		default:
			assert(0);
			break;
	}

	return moduleType;
}

UnitsConvertResult UnitsConvertor::electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double rload)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	switch(unitID)
	{
		case E::ElectricUnit::mA:
			{
				if (sensorType != E::SensorType::V_0_5)
				{
					return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for mA"));
				}

				if (rload < RLOAD_OHM_LOW_LIMIT || rload > RLOAD_OHM_HIGH_LIMIT)
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
				if (sensorType != E::SensorType::Hz_005_50000)
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

UnitsConvertResult UnitsConvertor::electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
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

UnitsConvertResult UnitsConvertor::electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	if (unitID != E::ElectricUnit::Ohm)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect unitID for Ohm"));
	}

	if (r0_is_use(sensorType) == true && r0 == 0.0)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect R0 for Ohm"));
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

UnitsConvertResult UnitsConvertor::electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode)
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
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect electric unit: \"%1\" for mode: \"%2\"").arg(QMetaEnum::fromType<E::ElectricUnit>().key(unitID)).arg(QMetaEnum::fromType<E::OutputMode>().key(outputMode)));
	}

	double phVal = (elVal - electricLowLimit) * (OUT_PH_HIGH_LIMIT - OUT_PH_LOW_LIMIT) / (electricHighLimit - electricLowLimit) + OUT_PH_LOW_LIMIT;

	return UnitsConvertResult(phVal);
}

