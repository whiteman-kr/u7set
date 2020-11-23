#include "Conversion.h"

#include "../lib/UnitsConvertor.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------

double conversion(double val, ConversionType conversionType, const Metrology::SignalParam& param)
{
	double retVal = 0;

	switch(conversionType)
	{
		case ConversionType::PhysicalToElectric:

			switch(param.electricUnitID())
			{
				case E::ElectricUnit::Ohm:

					switch(param.electricSensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:			retVal = (val - param.physicalLowLimit())*(param.electricHighLimit() - param.electricLowLimit())/(param.physicalHighLimit() - param.physicalLowLimit()) + param.electricLowLimit();	break;

						case E::SensorType::Ohm_Pt_a_391:		retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, true);	retVal = retVal * param.electricR0() / 100; break;
						case E::SensorType::Ohm_Pt_a_385:		retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, true);	retVal = retVal * param.electricR0() / 100; break;
						case E::SensorType::Ohm_Cu_a_428:		retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, true);	retVal = retVal * param.electricR0() / 100; break;
						case E::SensorType::Ohm_Cu_a_426:		retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, true);	retVal = retVal * param.electricR0() / 100; break;
						case E::SensorType::Ohm_Ni_a_617:		retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, true);	retVal = retVal * param.electricR0() / 100; break;

						case E::SensorType::Ohm_Pt50_W1391:
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, param.electricR0()); break;
						case E::SensorType::Ohm_Pt50_W1385:
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, param.electricR0()); break;

						case E::SensorType::Ohm_Cu50_W1428:
						case E::SensorType::Ohm_Cu100_W1428:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, param.electricR0()); break;
						case E::SensorType::Ohm_Cu50_W1426:
						case E::SensorType::Ohm_Cu100_W1426:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, param.electricR0()); break;

						case E::SensorType::Ohm_Ni50_W1617:
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, param.electricR0()); break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_21[0][0], PT_21_COUNT, true);	break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_23[0][0], CU_23_COUNT, true);	break;

						default:
							assert(0);
					}

					break;

				case E::ElectricUnit::mV:

					switch(param.electricSensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:		retVal = (val - param.physicalLowLimit())*(param.electricHighLimit() - param.electricLowLimit())/(param.physicalHighLimit() - param.physicalLowLimit()) + param.electricLowLimit(); break;


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

				case E::ElectricUnit::NoUnit:
				case E::ElectricUnit::mA:
				case E::ElectricUnit::V:
				case E::ElectricUnit::uA:

					retVal = (val - param.physicalLowLimit())*(param.electricHighLimit() - param.electricLowLimit())/(param.physicalHighLimit() - param.physicalLowLimit()) + param.electricLowLimit();

					break;

				default:
					assert(0);
			}

			break;

		case ConversionType::ElectricToPhysical:

			switch(param.electricUnitID())
			{
				case E::ElectricUnit::Ohm:

					switch(param.electricSensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::Ohm_Raw:			retVal = (val - param.electricLowLimit())*(param.physicalHighLimit() - param.physicalLowLimit())/(param.electricHighLimit() - param.electricLowLimit()) + param.physicalLowLimit();	break;

						case E::SensorType::Ohm_Pt_a_391:		if (param.electricR0() == 0.0) break; val = val / param.electricR0() * 100; retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, false);	break;
						case E::SensorType::Ohm_Pt_a_385:		if (param.electricR0() == 0.0) break; val = val / param.electricR0() * 100; retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, false);	break;
						case E::SensorType::Ohm_Cu_a_428:		if (param.electricR0() == 0.0) break; val = val / param.electricR0() * 100; retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, false);	break;
						case E::SensorType::Ohm_Cu_a_426:		if (param.electricR0() == 0.0) break; val = val / param.electricR0() * 100; retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, false);	break;
						case E::SensorType::Ohm_Ni_a_617:		if (param.electricR0() == 0.0) break; val = val / param.electricR0() * 100; retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, false);	break;


						case E::SensorType::Ohm_Pt50_W1391:
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, param.electricR0()); break;
						case E::SensorType::Ohm_Pt50_W1385:
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, param.electricR0()); break;

						case E::SensorType::Ohm_Cu50_W1428:
						case E::SensorType::Ohm_Cu100_W1428:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, param.electricR0()); break;
						case E::SensorType::Ohm_Cu50_W1426:
						case E::SensorType::Ohm_Cu100_W1426:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, param.electricR0()); break;

						case E::SensorType::Ohm_Ni50_W1617:
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, param.electricR0()); break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_21[0][0], PT_21_COUNT, false);	break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_23[0][0], CU_23_COUNT, false);	break;

						default:
							assert(0);
					}

					break;

				case E::ElectricUnit::mV:

					switch(param.electricSensorType())
					{
						case E::SensorType::NoSensor:
						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:		retVal = (val - param.electricLowLimit())*(param.physicalHighLimit() - param.physicalLowLimit())/(param.electricHighLimit() - param.electricLowLimit()) + param.physicalLowLimit();	break;

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

				case E::ElectricUnit::NoUnit:
				case E::ElectricUnit::mA:
				case E::ElectricUnit::V:
				case E::ElectricUnit::uA:

					retVal = (val - param.electricLowLimit())*(param.physicalHighLimit() - param.physicalLowLimit())/(param.electricHighLimit() - param.electricLowLimit()) + param.physicalLowLimit();

					break;

				default:
					assert(0);
			}

			break;

		case ConversionType::EnginnerToElectric:
			{
				double phVal = (val - param.lowEngineeringUnits())*(param.physicalHighLimit() - param.physicalLowLimit())/(param.highEngineeringUnits() - param.lowEngineeringUnits()) + param.physicalLowLimit();
				retVal = conversion(phVal, ConversionType::PhysicalToElectric, param);
			}
			break;

		case ConversionType::ElectricToEnginner:
			{
				double phVal = conversion(val, ConversionType::ElectricToPhysical, param);
				retVal = (phVal - param.physicalLowLimit())*(param.highEngineeringUnits() - param.lowEngineeringUnits())/(param.physicalHighLimit() - param.physicalLowLimit()) + param.lowEngineeringUnits();
			}
			break;

		default:
			assert(0);
	}

	return retVal;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------

double conversion(double val, ConversionType conversionType, const E::ElectricUnit unitID, const E::SensorType sensorType, double r0)
{
	Metrology::SignalParam param;

	param.setElectricUnitID(unitID);
	param.setElectricSensorType(sensorType);
	param.setElectricR0(r0);

	return conversion(val, conversionType, param);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------

double conversionCalcVal(double val, ConversionCalcType calcType, int connectionType, const IoSignalParam &ioParam)
{
	if (connectionType < 0 || connectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return val;
	}

	const Metrology::SignalParam& inParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		return val;
	}

	if (	inParam.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false ||
			inParam.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
	{
		return val;
	}

	const Metrology::SignalParam& outParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
	if (outParam.isValid() == false)
	{
		return val;
	}

	if (	outParam.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false ||
			outParam.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
	{
		return val;
	}

	UnitsConvertor uc;

	double retVal = val;

	switch (calcType)
	{
		case ConversionCalcType::Normal:

			switch (connectionType)
			{
				case SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL:
					{
						retVal = (val - inParam.lowEngineeringUnits())*
								(outParam.highEngineeringUnits() - outParam.lowEngineeringUnits())/
								(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) +
								outParam.lowEngineeringUnits();
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
					{
						double K = (outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) /
								sqrt(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits());

						retVal = K * sqrt( val );
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
				{
					val = (val - outParam.lowEngineeringUnits())*
							(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits())/
							(outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) +
							inParam.lowEngineeringUnits();

					double K = (outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) /
							sqrt(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits());

					retVal = K * sqrt( val );
				}
				break;

				case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:
					{
						retVal = uc.conversionDegree(val, UnitsConvertType::CelsiusToFahrenheit);
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:
					{
						val = (val - outParam.lowEngineeringUnits())*
								(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits())/
								(outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) +
								inParam.lowEngineeringUnits();

						retVal = uc.conversionDegree(val, UnitsConvertType::CelsiusToFahrenheit);
					}
					break;
			}

			break;

		case ConversionCalcType::Inversion:

			switch (connectionType)
			{
				case SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL:
					{
						retVal = (val - outParam.lowEngineeringUnits())*
								(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits())/
								(outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) +
								inParam.lowEngineeringUnits();
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
					{
						double K = (outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) /
								sqrt(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits());

						retVal = pow(val / K, 2);
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
					{
						val = (val - inParam.lowEngineeringUnits())*
								(outParam.highEngineeringUnits() - outParam.lowEngineeringUnits())/
								(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) +
								outParam.lowEngineeringUnits();

						double K = (outParam.highEngineeringUnits() - outParam.lowEngineeringUnits()) /
								sqrt(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits());

						retVal = pow(val / K, 2);
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:
					{
						retVal = uc.conversionDegree(val, UnitsConvertType::FahrenheitToCelsius);
					}
					break;

				case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:
					{
						val = (val - inParam.lowEngineeringUnits())*
								(outParam.highEngineeringUnits() - outParam.lowEngineeringUnits())/
								(inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) +
								outParam.lowEngineeringUnits();

						retVal = uc.conversionDegree(val, UnitsConvertType::FahrenheitToCelsius);
					}
					break;
			}

			break;
	}

	return retVal;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------


