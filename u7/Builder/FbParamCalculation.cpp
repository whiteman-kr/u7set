#include "ApplicationLogicCompiler.h"

namespace Builder
{

#define CHECK_REQUIRED_PARAMETERS(paramList)	if (checkRequiredParameters(paramList) == false) { return false; }
#define CHECK_UNSIGNED_INT(param)				if (checkUnsignedInt(param) == false) { return false; }
#define CHECK_UNSIGNED_INT16(param)				if (checkUnsignedInt16(param) == false) { return false; }
#define CHECK_UNSIGNED_INT32(param)				if (checkUnsignedInt32(param) == false) { return false; }
#define CHECK_SIGNED_INT32(param)				if (checkSignedInt32(param) == false) { return false; }
#define CHECK_FLOAT32(param)					if (checkFloat32(param) == false) { return false; }


	bool AppFb::calculateFbParamValues(ModuleLogicCompiler* compiler)
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

		if (isFb() == false)
		{
			assert(false);
			return true;
		}

		bool result = true;

		switch(afb().type().toOpCode())
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

/*		case Afb::AfbType::SCALE_P:			// opcode 15
			result = calculate_SCALE_paramValues();
			break; */

		case Afb::AfbType::FUNC:			// opcode 16
			result = calculate_FUNC_paramValues();
			break;



		default:
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Parameter's values calculation for FB %1 (opcode %2) is not implemented")).
					  arg(afb().caption()).arg(afb().type().toOpCode()));
			result = false;
		}

		return result;
	}


	bool AppFb::calculate_LOGIC_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_NOT_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_FLIP_FLOP_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_CTUD_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_MAJ_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_SRSST_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_BCOD_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_BDEC_paramValues()
	{
		m_runTime = 2;
		return true;
	}


	bool AppFb::calculate_MATH_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_conf = m_paramValuesArray["i_counf"];

		CHECK_UNSIGNED_INT(i_conf)

		m_runTime = 0;

		switch(i_conf.unsignedIntValue())
		{
		case 1:
		case 2:
		case 3:
		case 4:
			m_runTime = 2;
			break;

		case 5:
		case 6:
		case 8:
			m_runTime = 8;
			break;

		case 7:
			m_runTime = 6;
			break;

		default:
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Value %1 of parameter 'i_config' of FB %2 is incorrect")).
					  arg(i_conf.unsignedIntValue()).arg(caption()));
			return false;
		}

		return true;
	}


	bool AppFb::calculate_TCT_paramValues()
	{
		m_runTime = 2;

		QStringList requiredParams;

		requiredParams.append("i_counter");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& i_counter = m_paramValuesArray["i_counter"];

		CHECK_UNSIGNED_INT(i_counter)

		i_counter.setUnsignedIntValue((i_counter.unsignedIntValue() * 1000) / m_compiler->lmCycleDuration());

		return true;
	}


	bool AppFb::calculate_BCOMP_paramValues()
	{
		m_runTime = 2;

		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_sp_s");
		requiredParams.append("i_sp_r");
		requiredParams.append("hysteresis");

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AppFbParamValue& iConfParam = m_paramValuesArray["i_conf"];
		AppFbParamValue& sSettingParam = m_paramValuesArray["i_sp_s"];
		AppFbParamValue& rSettingParam = m_paramValuesArray["i_sp_r"];
		AppFbParamValue& hysteresisParam = m_paramValuesArray["hysteresis"];

		CHECK_UNSIGNED_INT(iConfParam)

		int iConf = iConfParam.unsignedIntValue();

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
			int hysteresis = abs(hysteresisParam.signedIntValue());

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
				sSettingParam.setSignedIntValue(sSetting - hysteresis / 2);
				rSettingParam.setSignedIntValue(sSetting + hysteresis / 2);
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
			double hysteresis = abs(hysteresisParam.floatValue());

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
				sSettingParam.setFloatValue(sSetting - hysteresis / 2);
				rSettingParam.setFloatValue(sSetting + hysteresis / 2);
				break;

			default:
				assert(false);
			}

			return true;
		}

		LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
				  QString(tr("Value %1 of parameter 'i_config' of FB %2 is incorrect")).arg(iConf).arg(caption()));

		return false;
	}


	bool AppFb::calculate_SCALE_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_scal_k1_coef");
		requiredParams.append("i_scal_k2_coef");
		requiredParams.append("input_low");
		requiredParams.append("input_high");
		requiredParams.append("output_low");
		requiredParams.append("output_high");

		CHECK_REQUIRED_PARAMETERS(requiredParams)

		AppFbParamValue& iConfParam = m_paramValuesArray["i_conf"];
		AppFbParamValue& k1Param = m_paramValuesArray["i_scal_k1_coef"];
		AppFbParamValue& k2Param = m_paramValuesArray["i_scal_k2_coef"];
		AppFbParamValue& x1Param = m_paramValuesArray["input_low"];
		AppFbParamValue& x2Param = m_paramValuesArray["input_high"];
		AppFbParamValue& y1Param = m_paramValuesArray["output_low"];
		AppFbParamValue& y2Param = m_paramValuesArray["output_high"];

		CHECK_UNSIGNED_INT(iConfParam)

		int iConf = iConfParam.unsignedIntValue();

		m_runTime = 0;

		switch(iConf)
		{
		case 1:
		case 2:
		case 3:
		case 4:
			m_runTime = 2;
			break;

		case 5:
		case 7:
		case 8:
		case 9:
			m_runTime = 18;
			break;

		case 6:
			m_runTime = 11;
			break;

		default:
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Value %1 of parameter 'i_config' of FB %2 is incorrect")).arg(iConf).arg(caption()));
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

		if (iConf == SCALE_16UI_16UI || iConf == SCALE_16UI_SI ||
			iConf == SCALE_SI_16UI || iConf == SCALE_SI_SI)
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

			double k1 = (y2 - y1) / (x2 - x1);

			k1Param.setFloatValue(k1);
			k2Param.setFloatValue(y1 - k1 * x1);

			return true;
		}

		LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
				  QString(tr("Value %1 of parameter 'i_config' of FB %2 is incorrect")).arg(iConf).arg(caption()));

		return false;
	}


	bool AppFb::calculate_DAMPER_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_del");
		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& iDel = m_paramValuesArray["i_del"];
		AppFbParamValue& iConf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT16(iDel)
		CHECK_UNSIGNED_INT(iConf)

		m_runTime = 0;

		switch(iConf.unsignedIntValue())
		{
		case 1:
			m_runTime = 3;		// for signed int input
			break;

		case 2:
			m_runTime = 19;		// for float input
			break;

		default:
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Value %1 of parameter 'i_conf' of FB %2 is incorrect")).arg(iConf.unsignedIntValue()).arg(caption()));

			return false;
		}

		int v = iDel.unsignedIntValue();

		v = (v * 1000) / m_compiler->lmCycleDuration();

		if (v > 65535)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Value %1 of parameter 'i_del' of FB %2 out of range (0..65535). The damper time should be less than %3 ms")).
					  arg(v).arg(caption()).arg((65535 * m_compiler->lmCycleDuration()) / 1000));
			return false;
		}

		iDel.setUnsignedIntValue(v);

		return true;
	}


	bool AppFb::calculate_MEM_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_si_fp");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& iSiFp = m_paramValuesArray["i_si_fp"];

		CHECK_UNSIGNED_INT(iSiFp)

		m_runTime = 0;

		switch(iSiFp.unsignedIntValue())
		{
		case 1:
			m_runTime = 9;		// for signed int input
			break;

		case 2:
			m_runTime = 16;		// for float input
			break;

		default:
			assert(false);
		}

		return true;
	}


	bool AppFb::calculate_FUNC_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");

		CHECK_REQUIRED_PARAMETERS(requiredParams);

		AppFbParamValue& iConf = m_paramValuesArray["i_conf"];

		CHECK_UNSIGNED_INT(iConf)

		m_runTime = 0;

		switch(iConf.unsignedIntValue())
		{
		case 1:
			m_runTime = 17;		// sqrt
			break;

		case 2:					// abs fp
		case 8:					// abs si
			m_runTime = 2;
			break;

		case 3:
			m_runTime = 37;		// sin
			break;

		case 4:
			m_runTime = 36;		// cos
			break;

		case 5:
			m_runTime = 22;		// log
			break;

		case 6:
			m_runTime = 18;		// exp
			break;

		case 7:
			m_runTime = 21;		// inv
			break;

		default:
			assert(false);
		}

		return true;
	}
}
