#include "ApplicationLogicCompiler.h"

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
			result = calculate_DER_paramValues();
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

		case Afb::AfbType::PULSE_GEN:	// opcode 30
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

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 4;

		switch(i_conf.unsignedIntValue())
		{
		case 1:		//	and
		case 2:		//	or
		case 3:		//	xor
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_NOT_paramValues()
	{
		m_runTime = 3 + 4;
		return true;
	}

	bool UalAfb::calculate_TCT_paramValues()
	{
		m_runTime = 4 + 34;

		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf);

		switch(i_conf.unsignedIntValue())
		{
		case 1:			// tct_on,		tctc_on
		case 2:			// tct_off,		tctc_off
		case 3:			// tct_vibr,	tctc_vibr
		case 4:			// tct_filter,	tctc_filter
		case 5:			// tct_rsv,		tctc_rsv
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_FLIP_FLOP_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 24;

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
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_CTUD_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 34;

		switch(i_conf.unsignedIntValue())
		{
		case 1:		// cnt_up
		case 2:		// cnt_dn
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MAJ_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf_y");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf_y = m_paramValuesArray["i_conf_y"];

		CHECK_UNSIGNED_INT(i_conf_y)

		m_runTime = 3;

		if (i_conf_y.unsignedIntValue() > 3)
		{
			m_runTime += i_conf_y.unsignedIntValue() + 1;
		}

		return true;
	}

	bool UalAfb::calculate_SRSST_paramValues()
	{
		m_runTime = 3 + 4;
		return true;
	}

	bool UalAfb::calculate_BCOD_paramValues()
	{
		m_runTime = 3 + 4;
		return true;
	}

	bool UalAfb::calculate_BDEC_paramValues()
	{
		m_runTime = 3 + 4;
		return true;
	}

	bool UalAfb::calculate_BCOMP_paramValues()
	{
		m_runTime = 3 + 14;

		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_sp_s");
		requiredParams.append("i_sp_r");
		requiredParams.append("hysteresis");

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];
		AppFbParamValue& sSettingParam = m_paramValuesArray["i_sp_s"];
		AppFbParamValue& rSettingParam = m_paramValuesArray["i_sp_r"];
		AppFbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		const int	BCOMP_32SI_EQU = 1,
					BCOMP_32SI_GREAT = 2,
					BCOMP_32SI_LESS = 3,
					BCOMP_32SI_NOT_EQU = 4,
					BCOMP_32FP_EQU = 5,
					BCOMP_32FP_GREAT = 6,
					BCOMP_32FP_LESS = 7,
					BCOMP_32FP_NOT_EQU = 8;

		if (iConf == BCOMP_32SI_EQU ||
			iConf == BCOMP_32SI_GREAT ||
			iConf == BCOMP_32SI_LESS ||
			iConf == BCOMP_32SI_NOT_EQU)
		{
			// comparison of signed int values
			//
			CHECK_SIGNED_INT32(sSettingParam)
			CHECK_SIGNED_INT32(rSettingParam)
			CHECK_SIGNED_INT32(hysteresisParam)

			int sSetting = sSettingParam.signedIntValue();
			int hysteresis = hysteresisParam.signedIntValue();

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
				sSettingParam.setSignedIntValue(sSetting + hysteresis / 2);
				rSettingParam.setSignedIntValue(sSetting - hysteresis / 2);
				break;

			case BCOMP_32SI_GREAT:
				rSettingParam.setSignedIntValue(sSetting - hysteresis);
				break;

			case BCOMP_32SI_LESS:
				rSettingParam.setSignedIntValue(sSetting + hysteresis);
				break;

			case BCOMP_32SI_NOT_EQU:
				sSettingParam.setSignedIntValue(sSetting + hysteresis / 2);
				rSettingParam.setSignedIntValue(sSetting - hysteresis / 2);
				break;

			default:
				assert(false);
			}

			return true;
		}

		if (iConf == BCOMP_32FP_EQU ||
			iConf == BCOMP_32FP_GREAT ||
			iConf == BCOMP_32FP_LESS ||
			iConf == BCOMP_32FP_NOT_EQU)
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

			switch(iConf)
			{
			case BCOMP_32FP_EQU:
				sSettingParam.setFloatValue(sSetting + hysteresis / 2);
				rSettingParam.setFloatValue(sSetting - hysteresis / 2);
				break;

			case BCOMP_32FP_GREAT:
				rSettingParam.setFloatValue(sSetting - hysteresis);
				break;

			case BCOMP_32FP_LESS:
				rSettingParam.setFloatValue(sSetting + hysteresis);
				break;

			case BCOMP_32FP_NOT_EQU:
				sSettingParam.setFloatValue(sSetting + hysteresis / 2);
				rSettingParam.setFloatValue(sSetting - hysteresis / 2);
				break;

			default:
				assert(false);
			}

			return true;
		}

		// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		//
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

		return false;
	}

	bool UalAfb::calculate_DAMPER_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		bool isConstDamper = caption() == "dampc_si" ||	caption() == "dampc_fp";

		if (isConstDamper == true)
		{
			requiredParams.append("i_del");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		if (isConstDamper == true)
		{
			AppFbParamValue& i_del = m_paramValuesArray["i_del"];
			CHECK_SIGNED_INT32(i_del)
		}

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
			m_runTime = 6 + 34;		// for signed int input
			break;

		case 2:
			m_runTime = 24 + 34;	// for float input
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MEM_paramValues()
	{
		m_runTime = 0;

		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_count");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];
		AppFbParamValue& i_count = m_paramValuesArray["i_count"];

		CHECK_UNSIGNED_INT(i_conf)
		CHECK_UNSIGNED_INT(i_count)

		if (i_count.unsignedIntValue() < 3 || i_count.unsignedIntValue() > 8)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_count.unsignedIntValue(), i_count.caption(), caption(), guid());
			return false;
		}

		int index = i_count.unsignedIntValue() - 3;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
			{
				int siTiming[] = { 4, 17, 23, 30, 38, 47 };		// exec time for signed int inputs

				if (index < 0 || index >= sizeof(siTiming) / sizeof(int) )
				{
					assert(false);
				}
				else
				{
					m_runTime = siTiming[index] + 4;
				}
			}
			break;

		case 2:
			{
				int fpTiming[] = { 21, 36, 44, 49, 57, 66 };	// exec time for float inputs

				if (index < 0 || index >= sizeof(fpTiming) / sizeof(int) )
				{
					assert(false);
				}
				else
				{
					m_runTime = fpTiming[index] + 4;
				}
			}
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MATH_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:			// add_si
		case 2:			// sub_si
		case 3:			// mul_si
		case 4:			// div_si
			m_runTime = 3 + 4;
			break;

		case 5:			// add_fp
		case 6:			// sub_fp
		case 7:			// mul_fp
		case 8:			// div_fb
			m_runTime = 9 + 4;
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_SCALE_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_scal_k1_coef");
		requiredParams.append("i_scal_k2_coef");
		requiredParams.append("x1");
		requiredParams.append("x2");
		requiredParams.append("y1");
		requiredParams.append("y2");

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];
		AppFbParamValue& k1Param = m_paramValuesArray["i_scal_k1_coef"];
		AppFbParamValue& k2Param = m_paramValuesArray["i_scal_k2_coef"];
		AppFbParamValue& x1Param = m_paramValuesArray["x1"];
		AppFbParamValue& x2Param = m_paramValuesArray["x2"];
		AppFbParamValue& y1Param = m_paramValuesArray["y1"];
		AppFbParamValue& y2Param = m_paramValuesArray["y2"];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		m_runTime = 0;

		switch(iConf)
		{
		case 1:
		case 2:
		case 3:
		case 4:
			m_runTime = 4 + 4;
			break;

		case 5:
		case 7:
		case 8:
		case 9:
			m_runTime = 21 + 4;
			break;

		case 6:
			m_runTime = 14 + 4;
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
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
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

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

		requiredParams.append("i_conf");

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

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		m_runTime = 0;

		if (iConf == 1)
		{
			// signed int scale
			//
			m_runTime = 4 + 4;

			// get parameters that defined by user
			//
			AppFbParamValue* x[XY_POINT_COUNT];
			AppFbParamValue* y[XY_POINT_COUNT];

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
			AppFbParamValue* x_data[CHECK_POINT_COUNT];

			for(int i = 0; i < CHECK_POINT_COUNT; i++)
			{
				x_data[i] = &m_paramValuesArray[x_data_str.arg(i + 1)];

				CHECK_SIGNED_INT32(*x_data[i]);
			}

			AppFbParamValue* scal_k1[RANGE_COUNT];
			AppFbParamValue* scal_k2[RANGE_COUNT];

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
			m_runTime = 20 + 4;

			// get parameters that defined by user
			//
			AppFbParamValue* x[XY_POINT_COUNT];
			AppFbParamValue* y[XY_POINT_COUNT];

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
			AppFbParamValue* x_data[CHECK_POINT_COUNT];

			for(int i = 0; i < CHECK_POINT_COUNT; i++)
			{
				x_data[i] = &m_paramValuesArray[x_data_str.arg(i + 1)];

				CHECK_FLOAT32(*x_data[i]);
			}

			AppFbParamValue* scal_k1[RANGE_COUNT];
			AppFbParamValue* scal_k2[RANGE_COUNT];

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
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

		return false;
	}

	bool UalAfb::calculate_FUNC_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
			m_runTime = 20 + 4;		// sqrt
			break;

		case 2:
			m_runTime = 4 + 4;		// abs fp
			break;

		case 3:
			m_runTime = 40 + 4;		// sin
			break;

		case 4:
			m_runTime = 40 + 4;		// cos
			break;

		case 5:
			m_runTime = 25 + 4;		// log
			break;

		case 6:
			m_runTime = 21 + 4;		// exp
			break;

		case 7:
			m_runTime = 25 + 4;		// inv
			break;

		case 8:
			m_runTime = 4 + 4;		// abs si
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_INT_paramValues()
	{
		bool isConstIntegrator = caption() == "integratorc";

		if (isConstIntegrator == true)
		{
			AppFbParamValue* i_ti = nullptr;
			AppFbParamValue* i_ki = nullptr;
			AppFbParamValue* i_max = nullptr;
			AppFbParamValue* i_min = nullptr;

			CHECK_AND_GET_REQUIRED_PARAMETER("i_ti", i_ti);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_ki", i_ki);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_max", i_max);
			CHECK_AND_GET_REQUIRED_PARAMETER("i_min", i_min);

			CHECK_SIGNED_INT32(*i_ti);
			CHECK_FLOAT32(*i_ki);
			CHECK_FLOAT32(*i_max);
			CHECK_FLOAT32(*i_min);

			if (i_ti->signedIntValue() < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_ti->caption(), guid());

				return false;
			}

			if (i_max->floatValue() <= i_min->floatValue())
			{
				// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
				//
				m_log->errALC5052(caption(), i_max->caption(), i_min->caption(), guid(), schemaID(), label());

				return false;
			}

		}

		m_runTime = 27 + 24;

		return true;
	}

	bool UalAfb::calculate_DPCOMP_paramValues()
	{
		QStringList requiredParams;

		bool hasHysteresisParam = true;

		if (caption() == "cmp_dh_fp_eq" ||
			caption() == "cmp_dh_fp_ne" ||
			caption() == "cmp_dh_fp_gr" ||
			caption() == "cmp_dh_fp_ls")
		{
			hasHysteresisParam = false;
		}

		requiredParams.append("i_conf");

		if (hasHysteresisParam == true)
		{
			requiredParams.append("hysteresis");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		int iConf = i_conf.unsignedIntValue();

		const int	CMP_32SI_EQU = 1,
					CMP_32SI_GREAT = 2,
					CMP_32SI_LESS = 3,
					CMP_32SI_NOT_EQU = 4,
					CMP_32FP_EQU = 5,
					CMP_32FP_GREAT = 6,
					CMP_32FP_LESS = 7,
					CMP_32FP_NOT_EQU = 8;

		m_runTime = 0;

		if (iConf == CMP_32SI_EQU ||
			iConf == CMP_32SI_GREAT ||
			iConf == CMP_32SI_LESS ||
			iConf == CMP_32SI_NOT_EQU)
		{
			m_runTime = 5 + 14;

			if (hasHysteresisParam == true)
			{
				AppFbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

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
			iConf == CMP_32FP_NOT_EQU)
		{
			m_runTime = 16 + 14;

			if (hasHysteresisParam == true)
			{
				AppFbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

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
			}

			return true;
		}

		// Value %1 of parameter '%2' of AFB '%3' is incorrect.
		//
		m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

		return false;
	}

	bool UalAfb::calculate_MUX_paramValues()
	{
		m_runTime = 3 + 4;

		return true;
	}

	bool UalAfb::calculate_LATCH_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 3 + 34;

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
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_LIM_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		bool isConstLimiter = caption() == "limc_fp" || caption() == "limc_si";

		if (isConstLimiter == true)
		{
			requiredParams.append("i_lim_max");
			requiredParams.append("i_lim_min");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf);

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:								// signed int limiter
			m_runTime = 3 + 4;

			if (isConstLimiter == true)
			{
				AppFbParamValue& i_lim_max = m_paramValuesArray["i_lim_max"];
				AppFbParamValue& i_lim_min = m_paramValuesArray["i_lim_min"];

				CHECK_SIGNED_INT32(i_lim_max);
				CHECK_SIGNED_INT32(i_lim_min);

				if (i_lim_min.signedIntValue() > i_lim_max.signedIntValue())
				{
					// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
					//
					m_log->errALC5052(caption(), i_lim_max.caption(), i_lim_min.caption(), guid(), schemaID(), label());

					return false;
				}

				if (i_lim_min.signedIntValue() == i_lim_max.signedIntValue())
				{
					// Values of parameters %1.%2 and %1.%3 are equal.
					//
					m_log->wrnALC5139(caption(), i_lim_max.caption(), i_lim_min.caption(), guid(), schemaID(), label());
				}
			}

			break;

		case 2:								// float limiter
			m_runTime = 4 + 4;

			if (isConstLimiter == true)
			{
				AppFbParamValue& i_lim_max = m_paramValuesArray["i_lim_max"];
				AppFbParamValue& i_lim_min = m_paramValuesArray["i_lim_min"];

				CHECK_FLOAT32(i_lim_max);
				CHECK_FLOAT32(i_lim_min);

				if (i_lim_min.floatValue() > i_lim_max.floatValue())
				{
					// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
					//
					m_log->errALC5052(caption(), i_lim_max.caption(), i_lim_min.caption(), guid(), schemaID(), label());

					return false;
				}

				if (static_cast<float>(i_lim_min.floatValue()) == static_cast<float>(i_lim_max.floatValue()))
				{
					// Values of parameters %1.%2 and %1.%3 are equal.
					//
					m_log->wrnALC5139(caption(), i_lim_max.caption(), i_lim_min.caption(), guid(), schemaID(), label());
				}
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_DEAD_ZONE_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_data_x");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];
		AppFbParamValue& i_data_x = m_paramValuesArray["i_data_x"];

		CHECK_UNSIGNED_INT(i_conf);

		m_runTime = 5 + 4;

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
			m_log->errALC5051(i_conf.unsignedIntValue(), i_conf.caption(), caption(), guid());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_POL_paramValues()
	{
		m_runTime = 24 + 4;

		const quint32 COEF_MAX_NUM = 10;

		QStringList requiredParams;

		requiredParams.append("i_conf");

		for(quint32 n = 1; n <= COEF_MAX_NUM; n++)
		{
			requiredParams.append(QString("i_%1_oprd").arg(n));
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(i_conf);

		quint32 coefCount = i_conf.unsignedIntValue();

		for(quint32 n = 1; n <= COEF_MAX_NUM; n++)
		{
			AppFbParamValue& i_N_oprd = m_paramValuesArray[QString("i_%1_oprd").arg(n)];

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

	bool UalAfb::calculate_DER_paramValues()
	{
		m_runTime = 35 + 44;

		QStringList requiredParams;

		requiredParams.append("i_max");
		requiredParams.append("i_min");

		bool isConstDerivative = caption() == "derivc";

		if (isConstDerivative == true)
		{
			requiredParams.append("i_kd");
			requiredParams.append("i_td");
		}

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		if (isConstDerivative == true)
		{
			AppFbParamValue& i_kd = m_paramValuesArray["i_kd"];
			AppFbParamValue& i_td = m_paramValuesArray["i_td"];

			CHECK_FLOAT32(i_kd);
			CHECK_SIGNED_INT32(i_td);

			int i_td_value = i_td.signedIntValue();

			if (i_td_value < 0)
			{
				// Value of parameter '%1.%2' must be greater or equal to 0.
				//
				m_log->errALC5043(caption(), i_td.caption(), guid());

				return false;
			}
		}

		AppFbParamValue& i_max = m_paramValuesArray["i_max"];
		AppFbParamValue& i_min = m_paramValuesArray["i_min"];

		CHECK_FLOAT32(i_max);
		CHECK_FLOAT32(i_min);

		float i_max_value = i_max.floatValue();
		float i_min_value = i_min.floatValue();

		if (i_max_value <= i_min_value)
		{
			// Value of parameter '%1.%2' must be greate then the value of '%1.%3'.
			//
			m_log->errALC5052(caption(), i_max.caption(), i_min.caption(), guid(), schemaID(), label());

			return false;
		}

		return true;
	}

	bool UalAfb::calculate_MISMATCH_paramValues()
	{
		AppFbParamValue* i_conf = nullptr;
		AppFbParamValue* i_conf_n = nullptr;

		CHECK_AND_GET_REQUIRED_PARAMETER("i_conf", i_conf);
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
		AppFbParamValue* i_lowlim = nullptr;
		AppFbParamValue* i_highlim = nullptr;
		AppFbParamValue* i_relvalue = nullptr;

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
		AppFbParamValue* i_ust = nullptr;

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
			m_runTime = 5 + 4;

			if (mismatchWithRange == true)
			{
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

				if (value == 0)
				{
					// Parameter '%1' of AFB '%2' can't be 0.
					//
					m_log->errALC5058(i_ust->caption(), caption(), guid());
					return false;
				}

				i_ust->setSignedIntValue(static_cast<qint32>(value));
			}
			else
			{
				if (mismatchDynamic == false)
				{
					CHECK_SIGNED_INT32(*i_ust);

					if (i_ust->signedIntValue() <= 0)
					{
						// Value of parameter '%1.%2' must be greater then 0.
						//
						m_log->errALC5088(i_ust->caption(), caption(), guid());
						return false;
					}
				}
			}
			break;

		case 2:				// FP
			m_runTime = 14 + 4;

			if (mismatchWithRange == true)
			{
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

				if (value == 0)
				{
					// Parameter '%1' of AFB '%2' can't be 0.
					//
					m_log->errALC5058(i_ust->caption(), caption(), guid());
					return false;
				}

				i_ust->setFloatValue(value);
			}
			else
			{
				if (mismatchDynamic == false)
				{
					CHECK_FLOAT32(*i_ust);

					if (i_ust->floatValue() <= 0)
					{
						// Value of parameter '%1.%2' must be greater then 0.
						//
						m_log->errALC5088(i_ust->caption(), caption(), guid());
						return false;
					}
				}
			}

			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf->unsignedIntValue(), i_conf->caption(), caption(), guid());
			return false;
		}

		// i_conf_n must have value from 2 to 4
		//
		if (i_conf_n->unsignedIntValue() < 2 || i_conf_n->unsignedIntValue() > 4)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_n->unsignedIntValue(), i_conf_n->caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_TCONV_paramValues()
	{
		m_runTime = 0;

		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_conf"];

		// i_conf must have value from 1 to 4
		//

		quint32 i_conf_value = i_conf.unsignedIntValue();

		switch(i_conf_value)
		{
		case 1:
		case 2:
			m_runTime = 3;
			break;

		case 3:
		case 4:
			m_runTime = 7;
			break;

		default:
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_value, i_conf.caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_INDICATION_paramValues()
	{
		m_runTime = 3 + 34;

		AppFbParamValue* i_conf = nullptr;

		CHECK_AND_GET_REQUIRED_PARAMETER("i_conf", i_conf);

		CHECK_UNSIGNED_INT(*i_conf);

		// i_conf must have value 1 or 2
		//
		quint32 i_conf_value = i_conf->unsignedIntValue();

		if (i_conf_value != 1 && i_conf_value != 2)
		{
			// Value %1 of parameter '%2' of AFB '%3' is incorrect.
			//
			m_log->errALC5051(i_conf_value, i_conf->caption(), caption(), guid());
			return false;
		}

		return true;
	}

	bool UalAfb::calculate_PULSE_GENERATOR_paramValues()
	{
		bool result = true;

		m_runTime = 10 + 34;

		AppFbParamValue* i_conf = nullptr;
		AppFbParamValue* i_t_high = nullptr;
		AppFbParamValue* i_t_low = nullptr;

		CHECK_AND_GET_REQUIRED_PARAMETER("i_conf", i_conf);
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
			m_log->errALC5051(i_conf_value, i_conf->caption(), caption(), guid());
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