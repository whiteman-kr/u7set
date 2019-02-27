#include "UnitsConvertor.h"

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

int UnitsConvertResult::errorCode() const
{
	return static_cast<int>(m_errorCode);
}

double UnitsConvertResult::toDouble() const
{
	return m_result;
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

UnitsConvertResult UnitsConvertor::electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	switch(unitID)
	{
		case E::ElectricUnit::V:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					if (electricLowLimit < V_0_5_LOW_LIMIT || electricLowLimit > V_0_5_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::LowLimitOutOfRange, V_0_5_LOW_LIMIT, V_0_5_HIGH_LIMIT);
					}
					if (electricHighLimit < V_0_5_LOW_LIMIT || electricHighLimit > V_0_5_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::HighLimitOutOfRange, V_0_5_LOW_LIMIT, V_0_5_HIGH_LIMIT);
					}

					return UnitsConvertResult(elVal);

				case E::SensorType::V_m10_p10:

					if (electricLowLimit < V_m10_p10_LOW_LIMIT || electricLowLimit > V_m10_p10_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::LowLimitOutOfRange, V_m10_p10_LOW_LIMIT, V_m10_p10_HIGH_LIMIT);
					}
					if (electricHighLimit < V_m10_p10_LOW_LIMIT || electricHighLimit > V_m10_p10_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::HighLimitOutOfRange, V_m10_p10_LOW_LIMIT, V_m10_p10_HIGH_LIMIT);
					}

					return UnitsConvertResult(elVal);

				default:
					// assert(false); commented by WhiteMan
					return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for V"));
			}

			break;

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					return  UnitsConvertResult(elVal * RESISTOR_V_0_5);

				default:
					// assert(false); commented by WhiteMan
					return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for mA"));
			}

			break;

		default: ;
			//	assert(false); commented by WhiteMan
			//
	}

	return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown unitID"));
}

UnitsConvertResult UnitsConvertor::electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int outputMode)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	double minElectricLowLimit = 0;
	double maxElectricHighLimit = 0;

	switch(outputMode)
	{
		case E::OutputMode::Plus0_Plus5_V:		minElectricLowLimit = 0;		maxElectricHighLimit = 5;	break;
		case E::OutputMode::Plus4_Plus20_mA:	minElectricLowLimit = 4;		maxElectricHighLimit = 20;	break;
		case E::OutputMode::Minus10_Plus10_V:	minElectricLowLimit = -10;		maxElectricHighLimit = 10;	break;
		case E::OutputMode::Plus0_Plus5_mA:		minElectricLowLimit = 0;		maxElectricHighLimit = 5;	break;
		case E::OutputMode::Plus0_Plus20_mA:	minElectricLowLimit = 0;		maxElectricHighLimit = 20;	break;
		case E::OutputMode::Plus0_Plus24_mA:	minElectricLowLimit = 0;		maxElectricHighLimit = 24;	break;

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

	double phVal = (elVal - electricLowLimit) * (OUT_PH_HIGH_LIMIT - OUT_PH_LOW_LIMIT) / (electricHighLimit - electricLowLimit) + OUT_PH_LOW_LIMIT;

	return UnitsConvertResult(phVal);
}

UnitsConvertResult UnitsConvertor::electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	if (unitID != E::ElectricUnit::mV)
	{
		return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect unitID for mV"));
	}

	double phVal = 0;

	switch(sensorType)
	{
		case E::SensorType::NoSensor:
			return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for mv"));

		case E::SensorType::mV_K_TXA: 			phVal = findConversionVal(elVal, &K_TXA[0][0], K_TXA_COUNT, false);					break;
		case E::SensorType::mV_L_TXK:			phVal = findConversionVal(elVal, &L_TXK[0][0], L_TXK_COUNT, false);					break;
		case E::SensorType::mV_N_THH:			phVal = findConversionVal(elVal, &N_THH[0][0], N_THH_COUNT, false);					break;

		case E::SensorType::mV_Type_B:			phVal = findConversionVal(elVal, &MV_TYPE_B[0][0], MV_TYPE_B_COUNT, false);			break;
		case E::SensorType::mV_Type_E:			phVal = findConversionVal(elVal, &MV_TYPE_E[0][0], MV_TYPE_E_COUNT, false);			break;
		case E::SensorType::mV_Type_J:			phVal = findConversionVal(elVal, &MV_TYPE_J[0][0], MV_TYPE_J_COUNT, false);			break;
		case E::SensorType::mV_Type_K:			phVal = findConversionVal(elVal, &MV_TYPE_K[0][0], MV_TYPE_K_COUNT, false);			break;
		case E::SensorType::mV_Type_N:			phVal = findConversionVal(elVal, &MV_TYPE_N[0][0], MV_TYPE_N_COUNT, false);			break;
		case E::SensorType::mV_Type_R:			phVal = findConversionVal(elVal, &MV_TYPE_R[0][0], MV_TYPE_R_COUNT, false);			break;
		case E::SensorType::mV_Type_S:			phVal = findConversionVal(elVal, &MV_TYPE_S[0][0], MV_TYPE_S_COUNT, false);			break;
		case E::SensorType::mV_Type_T:			phVal = findConversionVal(elVal, &MV_TYPE_T[0][0], MV_TYPE_T_COUNT, false);			break;

		case E::SensorType::mV_Raw_Mul_8:
		case E::SensorType::mV_Raw_Mul_32:		phVal = elVal;																		break;

		default:
			assert(false);
			return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for mV"));
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

	if (r0 == 0.0)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Incorrect R0 for Ohm"));
	}

	elVal = elVal / r0 * 100;

	double phVal = 0;

	switch(sensorType)
	{
		case E::SensorType::NoSensor:
			return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for Ohm"));

		case E::SensorType::Ohm_Pt_a_391:	phVal = findConversionVal(elVal, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, false);		break;
		case E::SensorType::Ohm_Pt_a_385:	phVal = findConversionVal(elVal, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, false);		break;
		case E::SensorType::Ohm_Cu_a_428:	phVal = findConversionVal(elVal, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, false);		break;
		case E::SensorType::Ohm_Cu_a_426:	phVal = findConversionVal(elVal, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, false);		break;
		case E::SensorType::Ohm_Ni_a_617:	phVal = findConversionVal(elVal, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, false);		break;

		default:
			assert(false);
			return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for Ohm"));
	}

	return UnitsConvertResult(phVal);
}

double UnitsConvertor::conversion(double val, const UnitsConvertType& conversionType, const E::ElectricUnit& unitID, const E::SensorType& sensorType, double r0)
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
						case E::SensorType::NoSensor:			retVal = 0;	break;

						case E::SensorType::Ohm_Pt_a_391:		retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, true);	retVal = retVal * r0 / 100; break;
						case E::SensorType::Ohm_Pt_a_385:		retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, true);	retVal = retVal * r0 / 100; break;
						case E::SensorType::Ohm_Cu_a_428:		retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, true);	retVal = retVal * r0 / 100; break;
						case E::SensorType::Ohm_Cu_a_426:		retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, true);	retVal = retVal * r0 / 100; break;
						case E::SensorType::Ohm_Ni_a_617:		retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, true);	retVal = retVal * r0 / 100; break;

						case E::SensorType::Ohm_Pt50_W1391:		retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 50); break;
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 100); break;
						case E::SensorType::Ohm_Pt50_W1385:		retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 50); break;
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 100); break;

						case E::SensorType::Ohm_Cu_50_W1428:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 50); break;
						case E::SensorType::Ohm_Cu_100_W1428:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 100); break;
						case E::SensorType::Ohm_Cu_50_W1426:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 50); break;
						case E::SensorType::Ohm_Cu_100_W1426:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 100); break;

						case E::SensorType::Ohm_Ni50_W1617:		retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 50); break;
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 100); break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_21[0][0], PT_21_COUNT, true);	break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_23[0][0], CU_23_COUNT, true);	break;


						default:
							assert(0);
					}

					break;

				case E::ElectricUnit::mV:

					switch(sensorType)
					{
						case E::SensorType::NoSensor:			retVal = 0;																			break;

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

						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:		retVal = 0;																			break;


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
						case E::SensorType::NoSensor:			retVal = 0;	break;

						case E::SensorType::Ohm_Pt_a_391:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &PT_100_W_1391[0][0], PT_100_W_1391_COUNT, false);	break;
						case E::SensorType::Ohm_Pt_a_385:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &PT_100_W_1385[0][0], PT_100_W_1385_COUNT, false);	break;
						case E::SensorType::Ohm_Cu_a_428:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &CU_100_W_1428[0][0], CU_100_W_1428_COUNT, false);	break;
						case E::SensorType::Ohm_Cu_a_426:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &CU_100_W_1426[0][0], CU_100_W_1426_COUNT, false);	break;
						case E::SensorType::Ohm_Ni_a_617:		if (r0 == 0.0) break; val = val / r0 * 100; retVal = findConversionVal(val, &NI_100_W_1617[0][0], NI_100_W_1617_COUNT, false);	break;

						case E::SensorType::Ohm_Pt50_W1391:		retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 50); break;
						case E::SensorType::Ohm_Pt100_W1391:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 100); break;
						case E::SensorType::Ohm_Pt50_W1385:		retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 50); break;
						case E::SensorType::Ohm_Pt100_W1385:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 100); break;

						case E::SensorType::Ohm_Cu_50_W1428:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 50); break;
						case E::SensorType::Ohm_Cu_100_W1428:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 100); break;
						case E::SensorType::Ohm_Cu_50_W1426:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 50); break;
						case E::SensorType::Ohm_Cu_100_W1426:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 100); break;

						case E::SensorType::Ohm_Ni50_W1617:		retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 50); break;
						case E::SensorType::Ohm_Ni100_W1617:	retVal = conversion(val, conversionType, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 100); break;

						case E::SensorType::Ohm_Pt21:			retVal = findConversionVal(val, &PT_21[0][0], PT_21_COUNT, false);	break;
						case E::SensorType::Ohm_Cu23:			retVal = findConversionVal(val, &CU_23[0][0], CU_23_COUNT, false);	break;


						default:
							assert(0);
					}

					break;

				case E::ElectricUnit::mV:

					switch(sensorType)
					{
						case E::SensorType::NoSensor:			retVal = 0;																			break;

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

						case E::SensorType::mV_Raw_Mul_8:
						case E::SensorType::mV_Raw_Mul_32:		retVal = 0;																			break;

						default:
							assert(0);
					}

					break;

				default:
					assert(0);
			}
			break;

		default:

			assert(0);
	}

	return retVal;
}
