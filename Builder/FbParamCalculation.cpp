#include "AppLogicCompiler.h"

namespace Builder
{

#define CHECK_REQUIRED_PARAMETERS(paramList)	if (checkRequiredParameters(paramList) == false) { return false; }
#define CHECK_REQUIRED_PARAMETER(paramName)		if (checkRequiredParameter(paramName) == false) { return false; }

#define CHECK_AND_GET_REQUIRED_PARAMETER(paramName, paramPtr)		if (checkRequiredParameter(paramName, true) == false) \
																	{ \
																		return false; \
																	} \
																	else \
																	{ \
																		paramPtr = &m_paramValuesArray[paramName]; \
																	}

#define CHECK_UNSIGNED_INT(param)				if (checkUnsignedInt(param) == false) { return false; }
#define CHECK_UNSIGNED_INT16(param)				if (checkUnsignedInt16(param) == false) { return false; }
#define CHECK_UNSIGNED_INT32(param)				if (checkUnsignedInt32(param) == false) { return false; }
#define CHECK_SIGNED_INT32(param)				if (checkSignedInt32(param) == false) { return false; }
#define CHECK_FLOAT32(param)					if (checkFloat32(param) == false) { return false; }


	bool UalAfb::calculateFbParamValues(ModuleLogicCompiler* compiler)
	{
		if (compiler == nullptr)
		{
			assert(false);
			return false;
		}

		m_compiler = compiler;
		m_log = compiler->log();

		if (m_log == nullptr)
		{
			assert(false);
			return false;
		}

		if (isAfb() == false)
		{
			assert(false);
			return true;
		}

		if (isSetFlagsItem() == true ||
			afb().opCode() == Afb::AFB_NOT_ACC_OPCODE ||
			isPackedLogic() == true)
		{
			return true;			// no parameters processing required
		}

		bool result = true;

		switch(static_cast<Afb::AfbType>(afb().opCode()))
		{
		case Afb::AfbType::LOGIC:			// opcode 1
			result = calculate_LOGIC_paramValues();
			break;

		case Afb::AfbType::NOT:				// opcode 2
			result = calculate_NOT_paramValues();
			break;

		case Afb::AfbType::TCT:				// opcode 3
			result = calculate_TCT_paramValues();
			break;

		case Afb::AfbType::FLIP_FLOP:		// opcode 4
			result = calculate_FLIP_FLOP_paramValues();
			break;

		case Afb::AfbType::CTUD:			// opcode 5
			result = calculate_CTUD_paramValues();
			break;

		case Afb::AfbType::MAJ:				// opcode 6
			result = calculate_MAJ_paramValues();
			break;

		case Afb::AfbType::SRSST:			// opcode 7
			result = calculate_SRSST_paramValues();
			break;

		case Afb::AfbType::BCOD:			// opcode 8
			result = calculate_BCOD_paramValues();
			break;

		case Afb::AfbType::BDEC:			// opcode 9
			result = calculate_BDEC_paramValues();
			break;

		case Afb::AfbType::BCOMP:			// opcode 10
			result = calculate_BCOMP_paramValues();
			break;

		case Afb::AfbType::DAMPER:			// opcode 11
			result = calculate_DAMPER_paramValues();
			break;

		case Afb::AfbType::MEM:				// opcode 12
			result = calculate_MEM_paramValues();
			break;

		case Afb::AfbType::MATH:			// opcode 13
			result = calculate_MATH_paramValues();
			break;

		case Afb::AfbType::SCALE:			// opcode 14
			result = calculate_SCALE_paramValues();
			break;

		case Afb::AfbType::SCALE_P:			// opcode 15
			result = calculate_SCALE_P_paramValues();
			break;

		case Afb::AfbType::FUNC:			// opcode 16
			result = calculate_FUNC_paramValues();
			break;

		case Afb::AfbType::INT:				// opcode 17
			result = calculate_INT_paramValues();
			break;

		case Afb::AfbType::DPCOMP:			// opcode 20
			result = calculate_DPCOMP_paramValues();
			break;

		case Afb::AfbType::MUX:				// opcode 21
			result = calculate_MUX_paramValues();
			break;

		case Afb::AfbType::LATCH:			// opcode 22
			result = calculate_LATCH_paramValues();
			break;

		case Afb::AfbType::LIM:				// opcode 23
			result = calculate_LIM_paramValues();
			break;

		case Afb::AfbType::DEAD_ZONE:		// opcode 24
			result = calculate_DEAD_ZONE_paramValues();
			break;

		case Afb::AfbType::POL:				// opcode 25
			result = calculate_POL_paramValues();
			break;

		case Afb::AfbType::DER:				// opcode 26
			result = calculate_DERIV_paramValues();
			break;

		case Afb::AfbType::MISMATCH:		// opcode 27
			result = calculate_MISMATCH_paramValues();
			break;

		case Afb::AfbType::TCONV:			// opcode 28
			result = calculate_TCONV_paramValues();
			break;

		case Afb::AfbType::INDICATION:		// opcode 29
			result = calculate_INDICATION_paramValues();
			break;

		case Afb::AfbType::PULSE_GEN:		// opcode 30
			result = calculate_PULSE_GENERATOR_paramValues();
			break;

		default:
			// Parameter's calculation for AFB '%1' (opcode %2) is not implemented.
			//
			m_log->errALC5044(afb().caption(), afb().opCode(), guid());
			result = false;
		}

		return result;
	}

	bool UalAfb::calculate_LOGIC_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 2;

		switch(i_conf.unsignedIntValue())
		{
		case 1:		//	and
		case 2:		//	or
		case 3:		//	xor
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_NOT_paramValues()
	{
		m_runTime = 3 + 2;
		return true;
	}

	bool UalAfb::calculate_TCT_paramValues()
	{
		m_runTime = 4 + 32;

		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf);

		switch(i_conf.unsignedIntValue())
		{
		case 1:			// tct_on,			tctc_on
		case 2:			// tct_off,			tctc_off
		case 3:			// tct_vibr,		tctc_vibr
		case 4:			// tct_filter,		tctc_filter
		case 5:			// tct_rsv,			tctc_rsv
		case 6:			// tct_rcfilter,	tctc_rcfilter
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_FLIP_FLOP_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 22;

		switch(i_conf.unsignedIntValue())
		{
		case 1:		// ff_sr
		case 2:		// ff_rs
		case 3:		// ff_d_front
		case 4:		// ff_t_front
		case 5:		// ff_d_decay
		case 6:		// ff_t_decay
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_CTUD_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 32;

		switch(i_conf.unsignedIntValue())
		{
		case 1:		// cnt_up
		case 2:		// cnt_dn
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MAJ_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf_y");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf_y = m_paramValuesArray["i_conf_y"];

		CHECK_UNSIGNED_INT(i_conf_y)

		m_runTime = 3 + 2;

		if (i_conf_y.unsignedIntValue() > 3)
		{
			m_runTime += i_conf_y.unsignedIntValue() + 1;
		}

		return true;
	}

	bool UalAfb::calculate_SRSST_paramValues()
	{
		m_runTime = 3 + 2;
		return true;
	}

	bool UalAfb::calculate_BCOD_paramValues()
	{
		m_runTime = 3 + 2;
		return true;
	}

	bool UalAfb::calculate_BDEC_paramValues()
	{
		m_runTime = 3 + 2;
		return true;
	}

	bool UalAfb::calculate_BCOMP_paramValues()
	{
		m_runTime = 3 + 12;

		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);
		requiredParams.append("i_sp_s");
		requiredParams.append("i_sp_r");
		requiredParams.append("hysteresis");

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];
		AfbParamValue& sSettingParam = m_paramValuesArray["i_sp_s"];
		AfbParamValue& rSettingParam = m_paramValuesArray["i_sp_r"];
		AfbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		const int	BCOMP_32SI_EQU = 1,
					BCOMP_32SI_GREAT = 2,
					BCOMP_32SI_LESS = 3,
					BCOMP_32SI_NOT_EQU = 4,

					BCOMP_32FP_EQU = 5,
					BCOMP_32FP_GREAT = 6,
					BCOMP_32FP_LESS = 7,
					BCOMP_32FP_NOT_EQU = 8,

					// from LM8-SR10
					//
					BCOMP_32SI_GREAT_EQU = 9,
					BCOMP_32SI_LESS_EQU = 10,

					BCOMP_32FP_GREAT_EQU = 11,
					BCOMP_32FP_LESS_EQU = 12;

		if (iConf == BCOMP_32SI_EQU ||
			iConf == BCOMP_32SI_GREAT ||
			iConf == BCOMP_32SI_LESS ||
			iConf == BCOMP_32SI_NOT_EQU ||
				(m_lmsWithLessGreateEqMode.contains(lmDescriptionName()) == true &&
				(iConf == BCOMP_32SI_GREAT_EQU || iConf == BCOMP_32SI_LESS_EQU))
			)
		{
			// comparison of signed int values
			//
			CHECK_SIGNED_INT32(sSettingParam)
			CHECK_SIGNED_INT32(rSettingParam)
			CHECK_SIGNED_INT32(hysteresisParam)

			qint64 sSetting = sSettingParam.signedIntValue();
			qint64 hysteresis = hysteresisParam.signedIntValue();

			if (hysteresis < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), hysteresisParam.caption(), guid());
				return false;
			}

			switch(iConf)
			{
			case BCOMP_32SI_EQU:
				{
					qint64 sValue = sSetting + hysteresis / 2;
					qint64 rValue = sSetting - hysteresis / 2;

					if (checkInt32Range(sValue) == false ||
						checkInt32Range(rValue) == false)
					{
						m_log->errALC5199(caption(), guid(), schemaID());
						return false;
					}

					sSettingParam.setSignedIntValue(sValue);
					rSettingParam.setSignedIntValue(rValue);
				}
				break;

			case BCOMP_32SI_GREAT:
			case BCOMP_32SI_GREAT_EQU:
				{
					qint64 rValue = sSetting - hysteresis;

					if (checkInt32Range(rValue) == false)
					{
						m_log->errALC5199(caption(), guid(), schemaID());
						return false;
					}

					rSettingParam.setSignedIntValue(rValue);
				}
				break;

			case BCOMP_32SI_LESS:
			case BCOMP_32SI_LESS_EQU:
				{
					qint64 rValue = sSetting + hysteresis;

					if (checkInt32Range(rValue) == false)
					{
						m_log->errALC5199(caption(), guid(), schemaID());
						return false;
					}

					rSettingParam.setSignedIntValue(rValue);
				}
				break;

			case BCOMP_32SI_NOT_EQU:
				{
					qint64 sValue = sSetting + hysteresis / 2;
					qint64 rValue = sSetting - hysteresis / 2;

					if (checkInt32Range(sValue) == false ||
						checkInt32Range(rValue) == false)
					{
						m_log->errALC5199(caption(), guid(), schemaID());
						return false;
					}

					sSettingParam.setSignedIntValue(sValue);
					rSettingParam.setSignedIntValue(rValue);
				}
				break;

			default:
				assert(false);
			}

			return true;
		}

		if (iConf == BCOMP_32FP_EQU ||
			iConf == BCOMP_32FP_GREAT ||
			iConf == BCOMP_32FP_LESS ||
			iConf == BCOMP_32FP_NOT_EQU ||
				(m_lmsWithLessGreateEqMode.contains(lmDescriptionName()) == true &&
				(iConf == BCOMP_32FP_GREAT_EQU || iConf == BCOMP_32FP_LESS_EQU))
			)
		{
			// comparison of floating point values
			//
			CHECK_FLOAT32(sSettingParam)
			CHECK_FLOAT32(rSettingParam)
			CHECK_FLOAT32(hysteresisParam)

			double sSetting = sSettingParam.floatValue();
			double hysteresis = hysteresisParam.floatValue();

			if (hysteresis < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), hysteresisParam.caption(), guid());
				return false;
			}

			if ((iConf == BCOMP_32FP_EQU || iConf == BCOMP_32FP_NOT_EQU) && hysteresis == 0)
			{
				// Using value 0.0 for parameter %1.%2 is not recommend.
				//
				m_log->wrnALC5177(caption(), hysteresisParam.caption(), guid(), schemaID());
			}

			switch(iConf)
			{
			case BCOMP_32FP_EQU:
				{
					double sValue = sSetting + hysteresis / 2;
					double rValue = sSetting - hysteresis / 2;

					if (checkFloat32Range(sValue) == false ||
						checkFloat32Range(rValue) == false)
					{
						m_log->errALC5200(caption(), guid(), schemaID());
						return false;
					}

					sSettingParam.setFloatValue(sValue);
					rSettingParam.setFloatValue(rValue);
				}
				break;

			case BCOMP_32FP_GREAT:
			case BCOMP_32FP_GREAT_EQU:
				{
					double rValue = sSetting - hysteresis;

					if (checkFloat32Range(rValue) == false)
					{
						m_log->errALC5200(caption(), guid(), schemaID());
						return false;
					}

					rSettingParam.setFloatValue(rValue);
				}
				break;

			case BCOMP_32FP_LESS:
			case BCOMP_32FP_LESS_EQU:
				{
					double rValue = sSetting + hysteresis;

					if (checkFloat32Range(rValue) == false)
					{
						m_log->errALC5200(caption(), guid(), schemaID());
						return false;
					}

					rSettingParam.setFloatValue(rValue);
				}
				break;

			case BCOMP_32FP_NOT_EQU:
				{
					double sValue = sSetting + hysteresis / 2;
					double rValue = sSetting - hysteresis / 2;

					if (checkFloat32Range(sValue) == false ||
						checkFloat32Range(rValue) == false)
					{
						m_log->errALC5200(caption(), guid(), schemaID());
						return false;
					}

					sSettingParam.setFloatValue(sValue);
					rSettingParam.setFloatValue(rValue);
				}
				break;

			default:
				assert(false);
			}

			return true;
		}

		// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		//
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());

		return false;
	}

	bool UalAfb::calculate_DAMPER_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		bool isConstDamper = caption() == "dampc_si" ||	caption() == "dampc_fp";

		if (isConstDamper == true)
		{
			requiredParams.append("i_del");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		if (isConstDamper == true)
		{
			AfbParamValue& i_del = m_paramValuesArray["i_del"];
			CHECK_SIGNED_INT32(i_del)
		}

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
			m_runTime = 11 + 32;	// for signed int input
			break;

		case 2:
			m_runTime = 25 + 32;	// for float input
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MEM_paramValues()
	{
		m_runTime = 0;

		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);
		requiredParams.append("i_count");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];
		AfbParamValue& i_count = m_paramValuesArray["i_count"];

		CHECK_UNSIGNED_INT(i_conf)
		CHECK_UNSIGNED_INT(i_count)

		if (i_count.unsignedIntValue() < 3 || i_count.unsignedIntValue() > 8)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_count.unsignedIntValue(), i_count.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		int index = i_count.unsignedIntValue() - 3;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
			{
				const int siTiming[] = { 4, 17, 23, 30, 38, 47 };		// exec time for signed int inputs

				if (index < 0 || index >= static_cast<int>(sizeof(siTiming) / sizeof(int)) )
				{
					assert(false);
				}
				else
				{
					m_runTime = siTiming[index] + 2;
				}
			}
			break;

		case 2:
			{
				const int fpTiming[] = { 21, 36, 44, 49, 57, 66 };	// exec time for float inputs

				if (index < 0 || index >= static_cast<int>(sizeof(fpTiming) / sizeof(int)) )
				{
					assert(false);
				}
				else
				{
					m_runTime = fpTiming[index] + 2;
				}
			}
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MATH_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:			// add_si
		case 2:			// sub_si
		case 3:			// mul_si
		case 4:			// div_si
			m_runTime = 3 + 2;
			break;

		case 5:			// add_fp
		case 6:			// sub_fp
		case 7:			// mul_fp
		case 8:			// div_fb
			m_runTime = 9 + 2;
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_SCALE_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);
		requiredParams.append("i_scal_k1_coef");
		requiredParams.append("i_scal_k2_coef");
		requiredParams.append("x1");
		requiredParams.append("x2");
		requiredParams.append("y1");
		requiredParams.append("y2");

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];
		AfbParamValue& k1Param = m_paramValuesArray["i_scal_k1_coef"];
		AfbParamValue& k2Param = m_paramValuesArray["i_scal_k2_coef"];
		AfbParamValue& x1Param = m_paramValuesArray["x1"];
		AfbParamValue& x2Param = m_paramValuesArray["x2"];
		AfbParamValue& y1Param = m_paramValuesArray["y1"];
		AfbParamValue& y2Param = m_paramValuesArray["y2"];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		m_runTime = 0;

		switch(iConf)
		{
		case 1:
		case 2:
		case 3:
		case 4:
			m_runTime = 4 + 2;
			break;

		case 5:
		case 7:
		case 8:
		case 9:
			m_runTime = 24 + 2;
			break;

		case 6:
			m_runTime = 17 + 2;
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		const int	SCALE_16UI_16UI = 1,
					SCALE_16UI_SI = 2,
					SCALE_SI_16UI = 3,
					SCALE_SI_SI = 4,
					SCALE_SI_FP = 5,
					SCALE_FP_FP = 6,
					SCALE_FP_16UI = 7,
					SCALE_FP_SI = 8,
					SCALE_16UI_FP = 9;

		if (iConf == SCALE_16UI_16UI ||
			iConf == SCALE_16UI_SI ||
			iConf == SCALE_SI_16UI ||
			iConf == SCALE_SI_SI)
		{
			// k1 & k2 are Signed Integer
			//
			CHECK_SIGNED_INT32(k1Param)
			CHECK_SIGNED_INT32(k2Param);

			int x1 = 0;
			int x2 = 0;

			int y1 = 0;
			int y2 = 0;

			switch(iConf)
			{
			case SCALE_16UI_16UI:
				CHECK_UNSIGNED_INT16(x1Param)
				CHECK_UNSIGNED_INT16(x2Param)
				CHECK_UNSIGNED_INT16(y1Param)
				CHECK_UNSIGNED_INT16(y2Param)

				x1 = x1Param.unsignedIntValue();
				x2 = x2Param.unsignedIntValue();
				y1 = y1Param.unsignedIntValue();
				y2 = y2Param.unsignedIntValue();
				break;

			case SCALE_16UI_SI:
				CHECK_UNSIGNED_INT16(x1Param)
				CHECK_UNSIGNED_INT16(x2Param)
				CHECK_SIGNED_INT32(y1Param);
				CHECK_SIGNED_INT32(y2Param);

				x1 = x1Param.unsignedIntValue();
				x2 = x2Param.unsignedIntValue();
				y1 = y1Param.signedIntValue();
				y2 = y2Param.signedIntValue();
				break;

			case SCALE_SI_16UI:
				CHECK_SIGNED_INT32(x1Param)
				CHECK_SIGNED_INT32(x2Param)
				CHECK_UNSIGNED_INT16(y1Param)
				CHECK_UNSIGNED_INT16(y2Param)

				x1 = x1Param.signedIntValue();
				x2 = x2Param.signedIntValue();
				y1 = y1Param.unsignedIntValue();
				y2 = y2Param.unsignedIntValue();
				break;

			case SCALE_SI_SI:
				CHECK_SIGNED_INT32(x1Param)
				CHECK_SIGNED_INT32(x2Param)
				CHECK_SIGNED_INT32(y1Param)
				CHECK_SIGNED_INT32(y2Param)

				x1 = x1Param.signedIntValue();
				x2 = x2Param.signedIntValue();
				y1 = y1Param.signedIntValue();
				y2 = y2Param.signedIntValue();
				break;

			default:
				assert(false);
			}

			if (x2 - x1 == 0)
			{
				// Values of parameters '%1.%2' and '%1.%3' should not be equal.
				//
				m_log->errALC5054(caption(), x1Param.caption(), x2Param.caption(), guid());

				return false;
			}

			const int MULTIPLIER = 32768;

			int k1 = ((y2 - y1) * MULTIPLIER) / (x2 - x1);

			k1Param.setSignedIntValue(k1);
			k2Param.setSignedIntValue(y1 - (k1 * x1) / MULTIPLIER);

			return true;
		}

		if (iConf == SCALE_SI_FP || iConf == SCALE_FP_FP || iConf == SCALE_FP_16UI ||
			iConf == SCALE_FP_SI || iConf == SCALE_16UI_FP)
		{
			// k1 & k2 are Floating Point
			//
			CHECK_FLOAT32(k1Param);
			CHECK_FLOAT32(k2Param);

			double x1 = 0;
			double x2 = 0;

			double y1 = 0;
			double y2 = 0;

			switch(iConf)
			{
			case SCALE_SI_FP:
				CHECK_SIGNED_INT32(x1Param)
				CHECK_SIGNED_INT32(x2Param)
				CHECK_FLOAT32(y1Param)
				CHECK_FLOAT32(y2Param)

				x1 = x1Param.signedIntValue();
				x2 = x2Param.signedIntValue();
				y1 = y1Param.floatValue();
				y2 = y2Param.floatValue();
				break;

			case SCALE_FP_FP:
				CHECK_FLOAT32(x1Param)
				CHECK_FLOAT32(x2Param)
				CHECK_FLOAT32(y1Param)
				CHECK_FLOAT32(y2Param)

				x1 = x1Param.floatValue();
				x2 = x2Param.floatValue();
				y1 = y1Param.floatValue();
				y2 = y2Param.floatValue();
				break;

			case SCALE_FP_16UI:
				CHECK_FLOAT32(x1Param);
				CHECK_FLOAT32(x2Param);
				CHECK_UNSIGNED_INT16(y1Param);
				CHECK_UNSIGNED_INT16(y2Param);

				x1 = x1Param.floatValue();
				x2 = x2Param.floatValue();
				y1 = y1Param.unsignedIntValue();
				y2 = y2Param.unsignedIntValue();
				break;

			case SCALE_FP_SI:
				CHECK_FLOAT32(x1Param)
				CHECK_FLOAT32(x2Param)
				CHECK_SIGNED_INT32(y1Param)
				CHECK_SIGNED_INT32(y2Param)

				x1 = x1Param.floatValue();
				x2 = x2Param.floatValue();
				y1 = y1Param.signedIntValue();
				y2 = y2Param.signedIntValue();
				break;

			case SCALE_16UI_FP:
				CHECK_UNSIGNED_INT16(x1Param)
				CHECK_UNSIGNED_INT16(x2Param)
				CHECK_FLOAT32(y1Param)
				CHECK_FLOAT32(y2Param)

				x1 = x1Param.unsignedIntValue();
				x2 = x2Param.unsignedIntValue();
				y1 = y1Param.floatValue();
				y2 = y2Param.floatValue();
				break;

			default:
				assert(false);
			}

			if (x2 - x1 == 0)
			{
				// Values of parameters '%1.%2' and '%1.%3' should not be equal.
				//
				m_log->errALC5054(caption(), x1Param.caption(), x2Param.caption(), guid());

				return false;
			}

			double k1 = (y2 - y1) / (x2 - x1);

			k1Param.setFloatValue(k1);
			k2Param.setFloatValue(y1 - k1 * x1);

			return true;
		}

		// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		//
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());

		return false;
	}

	bool UalAfb::calculate_SCALE_P_paramValues()
	{
		const int XY_POINT_COUNT = 6;
		const int RANGE_COUNT = XY_POINT_COUNT - 1;				// 5
		const int CHECK_POINT_COUNT = XY_POINT_COUNT - 2;		// 4

		// fill requiredParams array
		//
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		QString x_data_str = "i_x%1_data";

		for(int i = 0; i < CHECK_POINT_COUNT; i++)
		{
			requiredParams.append(x_data_str.arg(i + 1));
		}

		QString scal_k1_str = "i_scal_k1_x%1_coef";
		QString scal_k2_str = "i_scal_k2_x%1_coef";

		for(int i = 0; i < RANGE_COUNT; i++)
		{
			requiredParams.append(scal_k1_str.arg(i + 1));
			requiredParams.append(scal_k2_str.arg(i + 1));
		}

		QString x_str = "x%1";		// x0 ... x5
		QString y_str = "y%1";		// y0 ... y5

		for(int i = 0; i < XY_POINT_COUNT; i++)
		{
			requiredParams.append(x_str.arg(i));
			requiredParams.append(y_str.arg(i));
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		m_runTime = 0;

		if (iConf == 1)
		{
			// signed int scale
			//
			m_runTime = 4 + 2;

			// get parameters that defined by user
			//
			AfbParamValue* x[XY_POINT_COUNT];
			AfbParamValue* y[XY_POINT_COUNT];

			for(int i = 0; i < XY_POINT_COUNT; i++)
			{
				x[i] = &m_paramValuesArray[x_str.arg(i)];

				CHECK_SIGNED_INT32(*x[i]);

				y[i] = &m_paramValuesArray[y_str.arg(i)];

				CHECK_SIGNED_INT32(*y[i]);
			}

			// sort XY points by ascending of X
			//
			bool autoSortPerformed = false;

			for(int i = 0; i < XY_POINT_COUNT - 1; i++)
			{
				for(int k = i + 1; k < XY_POINT_COUNT; k++)
				{
					int Xi = x[i]->signedIntValue();
					int Xk = x[k]->signedIntValue();

					if (Xi > Xk)
					{
						int Yi = y[i]->signedIntValue();
						int Yk = y[k]->signedIntValue();

						// swap XiYi <=> XkYk
						//
						x[i]->setSignedIntValue(Xk);
						x[k]->setSignedIntValue(Xi);

						y[i]->setSignedIntValue(Yk);
						y[k]->setSignedIntValue(Yi);

						autoSortPerformed = true;
					}
				}
			}

			if (autoSortPerformed == true)
			{
				// Automatic sorting of XY points of FB '%1' has been performed.
				//
				m_log->wrnALC5053(caption(), guid());
			}

			// get FB's parameters
			//
			AfbParamValue* x_data[CHECK_POINT_COUNT];

			for(int i = 0; i < CHECK_POINT_COUNT; i++)
			{
				x_data[i] = &m_paramValuesArray[x_data_str.arg(i + 1)];

				CHECK_SIGNED_INT32(*x_data[i]);
			}

			AfbParamValue* scal_k1[RANGE_COUNT];
			AfbParamValue* scal_k2[RANGE_COUNT];

			for(int i = 0; i < RANGE_COUNT; i++)
			{
				scal_k1[i] = &m_paramValuesArray[scal_k1_str.arg(i + 1)];

				CHECK_SIGNED_INT32(*scal_k1[i]);

				scal_k2[i] = &m_paramValuesArray[scal_k2_str.arg(i + 1)];

				CHECK_SIGNED_INT32(*scal_k2[i]);
			}

			// calculate FB's parameters based on user-defined paramters
			//
			for(int i = 0; i < CHECK_POINT_COUNT; i++ )
			{
				x_data[i]->setSignedIntValue(x[i + 1]->signedIntValue());
			}

			for(int i = 0; i < RANGE_COUNT; i++)
			{
				int n = i + 1;

				int Xi = x[i]->signedIntValue();
				int Xn = x[n]->signedIntValue();

				int Yi = y[i]->signedIntValue();
				int Yn = y[n]->signedIntValue();

				int k1 = ((Yn - Yi) * 32768) / (Xn - Xi);
				int k2 = Yi - (Xi * k1) / 32768;

				scal_k1[i]->setSignedIntValue(k1);
				scal_k2[i]->setSignedIntValue(k2);
			}

			return true;
		}

		// --------------------------------------------------------------------------

		if (iConf == 2)
		{
			// float scale
			//
			m_runTime = 20 + 2;

			// get parameters that defined by user
			//
			AfbParamValue* x[XY_POINT_COUNT];
			AfbParamValue* y[XY_POINT_COUNT];

			for(int i = 0; i < XY_POINT_COUNT; i++)
			{
				x[i] = &m_paramValuesArray[x_str.arg(i)];

				CHECK_FLOAT32(*x[i]);

				y[i] = &m_paramValuesArray[y_str.arg(i)];

				CHECK_FLOAT32(*y[i]);
			}

			// sort XY points by ascending of X
			//
			bool autoSortPerformed = false;

			for(int i = 0; i < XY_POINT_COUNT - 1; i++)
			{
				for(int k = i + 1; k < XY_POINT_COUNT; k++)
				{
					float Xi = x[i]->floatValue();
					float Xk = x[k]->floatValue();

					if (Xi > Xk)
					{
						float Yi = y[i]->floatValue();
						float Yk = y[k]->floatValue();

						// swap XiYi <=> XkYk
						//
						x[i]->setFloatValue(Xk);
						x[k]->setFloatValue(Xi);

						y[i]->setFloatValue(Yk);
						y[k]->setFloatValue(Yi);

						autoSortPerformed = true;
					}
				}
			}

			if (autoSortPerformed == true)
			{
				// Automatic sorting of XY points of FB '%1' has been performed.
				//
				m_log->wrnALC5053(caption(), guid());
			}

			// get FB's parameters
			//
			AfbParamValue* x_data[CHECK_POINT_COUNT];

			for(int i = 0; i < CHECK_POINT_COUNT; i++)
			{
				x_data[i] = &m_paramValuesArray[x_data_str.arg(i + 1)];

				CHECK_FLOAT32(*x_data[i]);
			}

			AfbParamValue* scal_k1[RANGE_COUNT];
			AfbParamValue* scal_k2[RANGE_COUNT];

			for(int i = 0; i < RANGE_COUNT; i++)
			{
				scal_k1[i] = &m_paramValuesArray[scal_k1_str.arg(i + 1)];

				CHECK_FLOAT32(*scal_k1[i]);

				scal_k2[i] = &m_paramValuesArray[scal_k2_str.arg(i + 1)];

				CHECK_FLOAT32(*scal_k2[i]);
			}

			// calculate FB's parameters based on user-defined paramters
			//
			for(int i = 0; i < CHECK_POINT_COUNT; i++ )
			{
				x_data[i]->setFloatValue(x[i + 1]->floatValue());
			}

			for(int i = 0; i < RANGE_COUNT; i++)
			{
				int n = i + 1;

				float Xi = x[i]->floatValue();
				float Xn = x[n]->floatValue();

				float Yi = y[i]->floatValue();
				float Yn = y[n]->floatValue();

				float k1 = (Yn - Yi) / (Xn - Xi);
				float k2 = Yi - (Xi * k1);

				scal_k1[i]->setFloatValue(k1);
				scal_k2[i]->setFloatValue(k2);
			}

			return true;
		}

		// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		//
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());

		return false;
	}

	bool UalAfb::calculate_FUNC_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
			m_runTime = 20 + 2;		// sqrt
			break;

		case 2:
			m_runTime = 4 + 2;		// abs fp
			break;

		case 3:
			m_runTime = 40 + 2;		// sin
			break;

		case 4:
			m_runTime = 40 + 2;		// cos
			break;

		case 5:
			m_runTime = 25 + 2;		// log
			break;

		case 6:
			m_runTime = 21 + 2;		// exp
			break;

		case 7:
			m_runTime = 25 + 2;		// inv
			break;

		case 8:
			m_runTime = 4 + 2;		// abs si
			break;

		case 9:
			m_runTime = 4 + 2;		// sign inversion fp
			break;

		case 10:
			m_runTime = 4 + 2;		// sign inversion si
			break;

		case 11:
			m_runTime = 4 + 2;		// negate fp
			break;

		case 12:
			m_runTime = 4 + 2;		// negate si
			break;

		case 13:
			m_runTime = 4 + 2;		// get sign fp/si
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_INT_paramValues()
	{
		bool isConstIntegrator = caption() == "integratorc";

		if (isConstIntegrator == true)
		{
			AfbParamValue* i_max = nullptr;
			AfbParamValue* i_min = nullptr;

			CHECK_AND_GET_REQUIRED_PARAMETER("i_max", i_max);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_min", i_min);

			CHECK_FLOAT32(*i_max);
			CHECK_FLOAT32(*i_min);

			if (i_max->floatValue() <= i_min->floatValue())
			{
				// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
				//
				m_log->errALC5052(caption(), i_max->caption(), i_min->caption(), guid(), schemaID(), label());

				return false;
			}

			AfbParamValue* i_ti = nullptr;
			AfbParamValue* i_ki = nullptr;

			CHECK_AND_GET_REQUIRED_PARAMETER("i_ti", i_ti);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_ki", i_ki);

			CHECK_SIGNED_INT32(*i_ti);
			CHECK_FLOAT32(*i_ki);

			if (i_ti->signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_ti->caption(), guid());

				return false;
			}
		}

		m_runTime = 27 + 22;

		return true;
	}

	bool UalAfb::calculate_DPCOMP_paramValues()
	{
		QStringList requiredParams;

		bool hasHysteresisParam = true;

		if (caption() == "cmp_dh_fp_eq" ||
			caption() == "cmp_dh_fp_ne" ||
			caption() == "cmp_dh_fp_gr" ||
			caption() == "cmp_dh_fp_gr_eq" ||
			caption() == "cmp_dh_fp_ls" ||
			caption() == "cmp_dh_fp_ls_eq")
		{
			hasHysteresisParam = false;
		}

		requiredParams.append(Afb::PARAM_I_CONF);

		if (hasHysteresisParam == true)
		{
			requiredParams.append("hysteresis");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		const int	CMP_32SI_EQU = 1,
					CMP_32SI_GREAT = 2,
					CMP_32SI_LESS = 3,
					CMP_32SI_NOT_EQU = 4,
					CMP_32FP_EQU = 5,
					CMP_32FP_GREAT = 6,
					CMP_32FP_LESS = 7,
					CMP_32FP_NOT_EQU = 8,

					// from LM8-SR10
					//
					CMP_32SI_GREAT_EQU = 9,
					CMP_32SI_LESS_EQU = 10,
					CMP_32FP_GREAT_EQU = 11,
					CMP_32FP_LESS_EQU = 12;

		m_runTime = 0;

		std::string lmName = lmDescriptionName().toStdString();

		if (iConf == CMP_32SI_EQU ||
			iConf == CMP_32SI_GREAT ||
			iConf == CMP_32SI_LESS ||
			iConf == CMP_32SI_NOT_EQU ||
				(m_lmsWithLessGreateEqMode.contains(lmDescriptionName()) == true &&
				(iConf == CMP_32SI_GREAT_EQU || iConf == CMP_32SI_LESS_EQU))
			)
		{
			m_runTime = 6 + 12;

			if (hasHysteresisParam == true)
			{
				AfbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

				// comparison of signed int values
				//
				CHECK_SIGNED_INT32(hysteresisParam)

				int hysteresis = hysteresisParam.signedIntValue();

				if (hysteresis < 0)
				{
					// Value of parameter '%1.%2' must be greater or equal to 0.
					//
					m_log->errALC5043(caption(), hysteresisParam.caption(), guid());

					return false;
				}
			}

			return true;
		}

		if (iConf == CMP_32FP_EQU ||
			iConf == CMP_32FP_GREAT ||
			iConf == CMP_32FP_LESS ||
			iConf == CMP_32FP_NOT_EQU ||
				(m_lmsWithLessGreateEqMode.contains(lmDescriptionName()) == true &&
				(iConf == CMP_32FP_GREAT_EQU || iConf == CMP_32FP_LESS_EQU))
			)
		{
			m_runTime = 53 + 12;

			if (hasHysteresisParam == true)
			{
				AfbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

				// comparison of floating point values
				//
				CHECK_FLOAT32(hysteresisParam)

				float hysteresis = hysteresisParam.floatValue();

				if (hysteresis < 0)
				{
					// Value of parameter '%1.%2' must be greater or equal to 0.
					//
					m_log->errALC5043(caption(), hysteresisParam.caption(), guid());

					return false;
				}

				if ((iConf == CMP_32FP_EQU || iConf == CMP_32FP_NOT_EQU) && hysteresis == 0)
				{
					// Using value 0.0 for parameter %1.%2 is not recommend.
					//
					m_log->wrnALC5177(caption(), hysteresisParam.caption(), guid(), schemaID());
				}
			}

			return true;

		}

		// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		//
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());

		return false;
	}

	bool UalAfb::calculate_MUX_paramValues()
	{
		m_runTime = 3 + 2;

		return true;
	}

	bool UalAfb::calculate_LATCH_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 32;

		switch(i_conf.unsignedIntValue())
		{
		case 1:			// latch_front_fp, latch_front_si
		case 2:			// latch_decay_fp, latch_decay_si
		case 3:			// latch_state_fp, latch_state_si
		case 4:			// latch_tm1_fp, latch_tm1_si
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_LIM_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		bool isConstLimiter = caption() == "limc_fp" || caption() == "limc_si";

		if (isConstLimiter == true)
		{
			requiredParams.append("i_lim_max");
			requiredParams.append("i_lim_min");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf);

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:								// signed int limiter
			m_runTime = 3 + 2;

			if (isConstLimiter == true)
			{
				AfbParamValue& i_lim_max = m_paramValuesArray["i_lim_max"];
				AfbParamValue& i_lim_min = m_paramValuesArray["i_lim_min"];

				CHECK_SIGNED_INT32(i_lim_max);
				CHECK_SIGNED_INT32(i_lim_min);

				if (i_lim_min.signedIntValue() >= i_lim_max.signedIntValue())
				{
					// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
					//
					m_log->errALC5052(caption(), i_lim_max.caption(), i_lim_min.caption(), guid(), schemaID(), label());

					return false;
				}
			}

			break;

		case 2:								// float limiter
			m_runTime = 4 + 2;

			if (isConstLimiter == true)
			{
				AfbParamValue& i_lim_max = m_paramValuesArray["i_lim_max"];
				AfbParamValue& i_lim_min = m_paramValuesArray["i_lim_min"];

				CHECK_FLOAT32(i_lim_max);
				CHECK_FLOAT32(i_lim_min);

				if (i_lim_min.floatValue() >= i_lim_max.floatValue())
				{
					// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
					//
					m_log->errALC5052(caption(), i_lim_max.caption(), i_lim_min.caption(), guid(), schemaID(), label());

					return false;
				}
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid(), label(), schemaID());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_DEAD_ZONE_paramValues()
	{
		if (lmDescriptionName() == LmDescriptionName::LM1_SR04)
		{
			return calculate_DEAD_ZONE_paramValues_LM1_SR04();
		}

		if (lmDescriptionName() == LmDescriptionName::LM8_SR10)
		{
			return calculate_DEAD_ZONE_paramValues_LM8_SR10();
		}

		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);
		requiredParams.append("i_data_x");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];
		AfbParamValue& i_data_x = m_paramValuesArray["i_data_x"];

		CHECK_UNSIGNED_INT(i_conf);

		m_runTime = 5 + 2;

		switch(i_conf.unsignedIntValue())
		{
		case 1:								// signed int dead zone
		case 2:
			CHECK_SIGNED_INT32(i_data_x);

			if (i_data_x.signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x.caption(), guid());

				return false;
			}

			break;

		case 3:								// float dead zone
		case 4:
			CHECK_FLOAT32(i_data_x);

			if (i_data_x.floatValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x.caption(), guid());

				return false;
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(static_cast<int>(i_conf.unsignedIntValue()), i_conf.caption(), caption(), guid(), label(), schemaID());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_DEAD_ZONE_paramValues_LM1_SR04()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);
		requiredParams.append(Afb::PARAM_I_DATA_X1);
		requiredParams.append(Afb::PARAM_I_DATA_X2);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];
		AfbParamValue& i_data_x1 = m_paramValuesArray[Afb::PARAM_I_DATA_X1];
		AfbParamValue& i_data_x2 = m_paramValuesArray[Afb::PARAM_I_DATA_X2];

		CHECK_UNSIGNED_INT(i_conf);

		m_runTime = 5 + 2;

		switch(i_conf.unsignedIntValue())
		{
		case 1:								// signed int dead zone
		case 2:
			CHECK_SIGNED_INT32(i_data_x1);

			if (i_data_x1.signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x1.caption(), guid());

				return false;
			}

			CHECK_SIGNED_INT32(i_data_x2);

			if (i_data_x2.signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x2.caption(), guid());

				return false;
			}

			if (i_data_x2.signedIntValue() < i_data_x1.signedIntValue())
			{
				// Value of parameter %1.%2 must be greater or equal then the value of %1.%3.
				//
				m_log->errALC5158(caption(), i_data_x2.caption(), i_data_x1.caption(), guid(), schemaID(), label());

				return false;
			}

			break;

		case 3:								// float dead zone
		case 4:
			CHECK_FLOAT32(i_data_x1);

			if (i_data_x1.floatValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x1.caption(), guid());

				return false;
			}

			CHECK_FLOAT32(i_data_x2);

			if (i_data_x2.floatValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x2.caption(), guid());

				return false;
			}

			if (i_data_x2.floatValue() < i_data_x1.floatValue())
			{
				// Value of parameter %1.%2 must be greater or equal then the value of %1.%3.
				//
				m_log->errALC5158(caption(), i_data_x2.caption(), i_data_x1.caption(), guid(), schemaID(), label());

				return false;
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(static_cast<int>(i_conf.unsignedIntValue()), i_conf.caption(), caption(), guid(), label(), schemaID());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_DEAD_ZONE_paramValues_LM8_SR10()
	{
		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);
		requiredParams.append(Afb::PARAM_I_DATA_X1);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];
		AfbParamValue& i_data_x1 = m_paramValuesArray[Afb::PARAM_I_DATA_X1];

		CHECK_UNSIGNED_INT(i_conf);

		if (i_conf.unsignedIntValue() == 2 ||
			i_conf.unsignedIntValue() == 4)
		{
			requiredParams.append(Afb::PARAM_I_DATA_X2);

			CHECK_REQUIRED_PARAMETERS(requiredParams);
		}

		m_runTime = 5 + 2;

		switch(i_conf.unsignedIntValue())
		{
		case 1:								// signed int dead zone
		case 2:
			CHECK_SIGNED_INT32(i_data_x1);

			if (i_data_x1.signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x1.caption(), guid());

				return false;
			}

			if (i_conf.unsignedIntValue() == 2)
			{
				AfbParamValue& i_data_x2 = m_paramValuesArray[Afb::PARAM_I_DATA_X2];

				CHECK_SIGNED_INT32(i_data_x2);

				if (i_data_x2.signedIntValue() < 0)
				{
					// Value of parameter '%1.%2' must be greater or equal to 0.
					//
					m_log->errALC5043(caption(), i_data_x2.caption(), guid());

					return false;
				}

				if (i_data_x2.signedIntValue() < i_data_x1.signedIntValue())
				{
					// Value of parameter %1.%2 must be greater or equal then the value of %1.%3.
					//
					m_log->errALC5158(caption(), i_data_x2.caption(), i_data_x1.caption(), guid(), schemaID(), label());

					return false;
				}
			}

			break;

		case 3:								// float dead zone
		case 4:
			CHECK_FLOAT32(i_data_x1);

			if (i_data_x1.floatValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_data_x1.caption(), guid());

				return false;
			}

			if (i_conf.unsignedIntValue() == 4)
			{
				AfbParamValue& i_data_x2 = m_paramValuesArray[Afb::PARAM_I_DATA_X2];

				CHECK_FLOAT32(i_data_x2);

				if (i_data_x2.floatValue() < 0)
				{
					// Value of parameter '%1.%2' must be greater or equal to 0.
					//
					m_log->errALC5043(caption(), i_data_x2.caption(), guid());

					return false;
				}

				if (i_data_x2.floatValue() < i_data_x1.floatValue())
				{
					// Value of parameter %1.%2 must be greater or equal then the value of %1.%3.
					//
					m_log->errALC5158(caption(), i_data_x2.caption(), i_data_x1.caption(), guid(), schemaID(), label());

					return false;
				}
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(static_cast<int>(i_conf.unsignedIntValue()), i_conf.caption(), caption(), guid(), label(), schemaID());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_POL_paramValues()
	{
		const quint32 COEF_MAX_NUM = 10;

		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		for(quint32 n = 1; n <= COEF_MAX_NUM; n++)
		{
			requiredParams.append(QString("i_%1_oprd").arg(n));
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		CHECK_UNSIGNED_INT(i_conf);

		quint32 coefCount = i_conf.unsignedIntValue();

		m_runTime = 2 + 21 * coefCount + 2;

		for(quint32 n = 1; n <= COEF_MAX_NUM; n++)
		{
			AfbParamValue& i_N_oprd = m_paramValuesArray[QString("i_%1_oprd").arg(n)];

			CHECK_FLOAT32(i_N_oprd);

			if (n <= coefCount)
			{
				continue;
			}

			// Check: i_N_oprd, where N > coefCount  && N <= COEF_MAX_NUM], NOT equal to 0
			//

			if (i_N_oprd.floatValue() != 0)
			{
				// Possible error. AFB 'Poly' CoefCount = %1, but coefficient '%2' is not equal to 0 (Logic schema %3).
				//
				m_log->wrnALC5072(coefCount, i_N_oprd.caption(), guid(), schemaID());
			}
		}

		return true;
	}

	bool UalAfb::calculate_DERIV_paramValues()
	{
		m_runTime = 35 + 42;

		bool isConstDerivative = caption() == "derivc";

		if (isConstDerivative == true)
		{
			AfbParamValue* i_max = nullptr;
			AfbParamValue* i_min = nullptr;

			CHECK_AND_GET_REQUIRED_PARAMETER("i_max", i_max);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_min", i_min);

			CHECK_FLOAT32(*i_max);
			CHECK_FLOAT32(*i_min);

			if (i_max->floatValue() <= i_min->floatValue())
			{
				// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
				//
				m_log->errALC5052(caption(), i_max->caption(), i_min->caption(), guid(), schemaID(), label());

				return false;
			}

			AfbParamValue* i_kd = nullptr;
			AfbParamValue* i_td = nullptr;

			CHECK_AND_GET_REQUIRED_PARAMETER("i_kd", i_kd);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_td", i_td);

			CHECK_FLOAT32(*i_kd);
			CHECK_SIGNED_INT32(*i_td);

			if (i_td->signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_td->caption(), guid());

				return false;
			}
		}

		return true;
	}

	bool UalAfb::calculate_MISMATCH_paramValues()
	{
		AfbParamValue* i_conf = nullptr;
		AfbParamValue* i_conf_n = nullptr;

		CHECK_AND_GET_REQUIRED_PARAMETER(Afb::PARAM_I_CONF, i_conf);
		CHECK_AND_GET_REQUIRED_PARAMETER("i_conf_n", i_conf_n);

		CHECK_UNSIGNED_INT(*i_conf);
		CHECK_UNSIGNED_INT(*i_conf_n);

		//

		bool mismatchWithRange = false;

		if (caption().startsWith("mismatch_r", Qt::CaseInsensitive) == true)
		{
			mismatchWithRange = true;
		}

		// optional parameters for mismatchWithRange == true
		//
		AfbParamValue* i_lowlim = nullptr;
		AfbParamValue* i_highlim = nullptr;
		AfbParamValue* i_relvalue = nullptr;

		if (mismatchWithRange == true)
		{
			CHECK_AND_GET_REQUIRED_PARAMETER("i_lowlim", i_lowlim);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_highlim", i_highlim);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_relvalue", i_relvalue);
		}

		//

		bool mismatchDynamic = false;

		if (caption().startsWith("mismatch_d", Qt::CaseInsensitive) == true)
		{
			mismatchDynamic = true;
		}

		// optional parameter for mismatchDynamic == false
		//
		AfbParamValue* i_ust = nullptr;

		if (mismatchDynamic == false)
		{
			CHECK_AND_GET_REQUIRED_PARAMETER("i_ust", i_ust);
		}

		m_runTime = 0;

		// i_conf must have value 1 (SI) or 2 (FP)
		//
		switch(i_conf->unsignedIntValue())
		{
		case 1:				// SI
			m_runTime = 5 + 2;

			if (mismatchWithRange == true)
			{
				if (i_lowlim == nullptr ||
					i_highlim == nullptr ||
					i_relvalue == nullptr ||
					i_ust == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				CHECK_SIGNED_INT32(*i_lowlim);
				CHECK_SIGNED_INT32(*i_highlim);
				CHECK_FLOAT32(*i_relvalue);
				CHECK_SIGNED_INT32(*i_ust);

				if (i_lowlim->signedIntValue() == i_highlim->signedIntValue())
				{
					// Parameters '%1' and '%2' of AFB '%3' can't be equal.
					//
					m_log->errALC5054(caption(), i_lowlim->caption(), i_highlim->caption(), guid());
					return false;
				}

				double value = (abs(i_highlim->signedIntValue() - i_lowlim->signedIntValue()) * i_relvalue->floatValue()) / 100.0;

				if (value < 0)
				{
					// Value of parameter %1.%2 must be greater or equal to 0.
					//
					m_log->errALC5043(caption(), i_ust->caption(), guid());
					return false;
				}

				i_ust->setSignedIntValue(static_cast<qint32>(value));
			}
			else
			{
				if (mismatchDynamic == false)
				{
					CHECK_SIGNED_INT32(*i_ust);

					if (i_ust->signedIntValue() < 0)
					{
						// Value of parameter %1.%2 must be greater or equal to 0.
						//
						m_log->errALC5043(caption(), i_ust->caption(), guid());
						return false;
					}
				}
			}
			break;

		case 2:				// FP
			m_runTime = 14 + 2;

			if (mismatchWithRange == true)
			{
				if (i_lowlim == nullptr ||
					i_highlim == nullptr ||
					i_relvalue == nullptr ||
					i_ust == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				CHECK_FLOAT32(*i_lowlim);
				CHECK_FLOAT32(*i_highlim);
				CHECK_FLOAT32(*i_relvalue);
				CHECK_FLOAT32(*i_ust);

				if (i_lowlim->floatValue() == i_highlim->floatValue())
				{
					// Parameters '%1' and '%2' of AFB '%3' can't be equal.
					//
					m_log->errALC5054(caption(), i_lowlim->caption(), i_highlim->caption(), guid());
					return false;
				}

				double value = (std::abs(i_highlim->floatValue() - i_lowlim->floatValue()) * i_relvalue->floatValue()) / 100.0;

				if (value < 0)
				{
					// Value of parameter %1.%2 must be greater or equal to 0.
					//
					m_log->errALC5043(caption(), i_ust->caption(), guid());
					return false;
				}

				i_ust->setFloatValue(value);
			}
			else
			{
				if (mismatchDynamic == false)
				{
					CHECK_FLOAT32(*i_ust);

					if (i_ust->floatValue() < 0)
					{
						// Value of parameter %1.%2 must be greater or equal to 0.
						//
						m_log->errALC5043(caption(), i_ust->caption(), guid());
						return false;
					}
				}
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf->unsignedIntValue(), i_conf->caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		// i_conf_n must have value from 2 to 4
		//
		if (i_conf_n->unsignedIntValue() < 2 || i_conf_n->unsignedIntValue() > 4)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_n->unsignedIntValue(), i_conf_n->caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_TCONV_paramValues()
	{
		m_runTime = 0;

		QStringList requiredParams;

		requiredParams.append(Afb::PARAM_I_CONF);

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AfbParamValue& i_conf = m_paramValuesArray[Afb::PARAM_I_CONF];

		// i_conf must have value from 1 to 4
		//

		quint32 i_conf_value = i_conf.unsignedIntValue();

		switch(i_conf_value)
		{
		case 1:
		case 2:
			m_runTime = 3 + 2;
			break;

		case 3:
		case 4:
			m_runTime = 7 + 2;
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_value, i_conf.caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_INDICATION_paramValues()
	{
		m_runTime = 3 + 32;

		AfbParamValue* i_conf = nullptr;

		CHECK_AND_GET_REQUIRED_PARAMETER(Afb::PARAM_I_CONF, i_conf);

		CHECK_UNSIGNED_INT(*i_conf);

		// i_conf must have value 1 or 2
		//
		quint32 i_conf_value = i_conf->unsignedIntValue();

		if (i_conf_value != 1 && i_conf_value != 2)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_value, i_conf->caption(), caption(), guid(), label(), schemaID());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_PULSE_GENERATOR_paramValues()
	{
		bool result = true;

		m_runTime = 10 + 32;

		AfbParamValue* i_conf = nullptr;
		AfbParamValue* i_t_high = nullptr;
		AfbParamValue* i_t_low = nullptr;

		CHECK_AND_GET_REQUIRED_PARAMETER(Afb::PARAM_I_CONF, i_conf);
		CHECK_AND_GET_REQUIRED_PARAMETER("i_t_high", i_t_high);
		CHECK_AND_GET_REQUIRED_PARAMETER("i_t_low", i_t_low);

		CHECK_UNSIGNED_INT(*i_conf);
		CHECK_UNSIGNED_INT(*i_t_high);
		CHECK_UNSIGNED_INT(*i_t_low);

		// i_conf must have value 1 or 2
		//
		quint32 i_conf_value = i_conf->unsignedIntValue();

		if (i_conf_value != 1 && i_conf_value != 2)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_value, i_conf->caption(), caption(), guid(), label(), schemaID());
			result = false;
		}

		quint32 i_t_high_value = i_t_high->unsignedIntValue();

		if (i_t_high_value < 5 || i_t_high_value > 65535)
		{
			m_log->errALC5141(caption(), i_t_high->caption(), "5..65535", guid(), schemaID());
			result = false;
		}

		quint32 i_t_low_value = i_t_low->unsignedIntValue();

		if (i_t_low_value < 5 || i_t_low_value > 65535)
		{
			m_log->errALC5141(caption(), i_t_low->caption(), "5..65535", guid(), schemaID());
			result = false;
		}

		return result;
	}

}
