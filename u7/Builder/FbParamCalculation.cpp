#include "ApplicationLogicCompiler.h"

namespace Builder
{

#define CHECK_REQUIRED_PARAMTERES(paramList)	if (checkRequiredParameters(paramList) == false) { return false; }
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
		case Afb::AfbType::LOGIC:
			// parameter's values calculation is not required
			break;

		case Afb::AfbType::TCT:
			result = calculate_TCT_paramValues();
			break;

		case Afb::AfbType::BCOMP:
			result = calculate_BCOMP_paramValues();
			break;


		case Afb::AfbType::SCAL:
			result = calculate_SCAL_paramValues();
			break;

		default:
			LOG_ERROR(m_log, QString(tr("Parameter's values calculation for FB %1 (opcode %2) is not implemented")).
					arg(afb().caption()).arg(afb().type().toOpCode()));
			result = false;
		}

		return result;
	}


	bool AppFb::calculate_TCT_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_counter");

		CHECK_REQUIRED_PARAMTERES(requiredParams);

		AppFbParamValue& i_counter = m_paramValuesArray["i_counter"];

		CHECK_UNSIGNED_INT(i_counter)

		i_counter.setUnsignedIntValue((i_counter.unsignedIntValue() * 1000) / m_compiler->lmCycleDuration());

		return true;
	}


	bool AppFb::calculate_BCOMP_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_sp_s");
		requiredParams.append("i_sp_r");
		requiredParams.append("hysteresis");

		CHECK_REQUIRED_PARAMTERES(requiredParams)

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


		if (iConf == BCOMP_32SI_EQU || iConf == BCOMP_32SI_GREAT ||
			iConf == BCOMP_32SI_LESS || iConf == BCOMP_32SI_NOT_EQU)
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

		if (iConf == BCOMP_32FP_EQU || iConf == BCOMP_32FP_GREAT ||
			iConf == BCOMP_32FP_LESS || iConf == BCOMP_32FP_NOT_EQU)
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

		LOG_ERROR(m_log, QString(tr("Value %1 of parameter 'i_config' of FB %2 is incorrect")).
				  arg(iConf).arg(caption()));

		return false;
	}


	bool AppFb::calculate_SCAL_paramValues()
	{
		QStringList requiredParams;

		requiredParams.append("i_conf");
		requiredParams.append("i_scal_k1_coef");
		requiredParams.append("i_scal_k2_coef");
		requiredParams.append("InputLow");
		requiredParams.append("InputHigh");
		requiredParams.append("OutputLow");
		requiredParams.append("OutputHigh");

		CHECK_REQUIRED_PARAMTERES(requiredParams)

		AppFbParamValue& iConfParam = m_paramValuesArray["i_conf"];
		AppFbParamValue& k1Param = m_paramValuesArray["i_scal_k1_coef"];
		AppFbParamValue& k2Param = m_paramValuesArray["i_scal_k2_coef"];
		AppFbParamValue& x1Param = m_paramValuesArray["InputLow"];
		AppFbParamValue& x2Param = m_paramValuesArray["InputHigh"];
		AppFbParamValue& y1Param = m_paramValuesArray["OutputLow"];
		AppFbParamValue& y2Param = m_paramValuesArray["OutputHigh"];

		CHECK_UNSIGNED_INT(iConfParam)

		int iConf = iConfParam.unsignedIntValue();

		const int	SCAL_16UI_16UI = 1,
					SCAL_16UI_32SI = 2,
					SCAL_32SI_16UI = 3,
					SCAL_32SI_32SI = 4,
					SCAL_32SI_32FP = 5,
					SCAL_32FP_32FP = 6,
					SCAL_32FP_16UI = 7,
					SCAL_32FP_32SI = 8,
					SCAL_16UI_32FP = 9;

		if (iConf == SCAL_16UI_16UI || iConf == SCAL_16UI_32SI ||
			iConf == SCAL_32SI_16UI || iConf == SCAL_32SI_32SI)
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
			case SCAL_16UI_16UI:
				CHECK_UNSIGNED_INT16(x1Param)
				CHECK_UNSIGNED_INT16(x2Param)
				CHECK_UNSIGNED_INT16(y1Param)
				CHECK_UNSIGNED_INT16(y2Param)

				x1 = x1Param.unsignedIntValue();
				x2 = x2Param.unsignedIntValue();
				y1 = y1Param.unsignedIntValue();
				y2 = y2Param.unsignedIntValue();
				break;

			case SCAL_16UI_32SI:
				CHECK_UNSIGNED_INT16(x1Param)
				CHECK_UNSIGNED_INT16(x2Param)
				CHECK_SIGNED_INT32(y1Param);
				CHECK_SIGNED_INT32(y2Param);

				x1 = x1Param.unsignedIntValue();
				x2 = x2Param.unsignedIntValue();
				y1 = y1Param.signedIntValue();
				y2 = y2Param.signedIntValue();
				break;

			case SCAL_32SI_16UI:
				CHECK_SIGNED_INT32(x1Param)
				CHECK_SIGNED_INT32(x2Param)
				CHECK_UNSIGNED_INT16(y1Param)
				CHECK_UNSIGNED_INT16(y2Param)

				x1 = x1Param.signedIntValue();
				x2 = x2Param.signedIntValue();
				y1 = y1Param.unsignedIntValue();
				y2 = y2Param.unsignedIntValue();
				break;

			case SCAL_32SI_32SI:
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

			const int MULTIPLIER = 32768 - 1;

			int k1 = ((y2 - y1) * MULTIPLIER) / (x2 - x1);

			k1Param.setSignedIntValue(k1);
			k2Param.setSignedIntValue(y1 - (k1 * x1) / MULTIPLIER);

			return true;
		}

		if (iConf == SCAL_32SI_32FP || iConf == SCAL_32FP_32FP || iConf == SCAL_32FP_16UI ||
			iConf == SCAL_32FP_32SI || iConf == SCAL_16UI_32FP)
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
			case SCAL_32SI_32FP:
				CHECK_SIGNED_INT32(x1Param)
				CHECK_SIGNED_INT32(x2Param)
				CHECK_FLOAT32(y1Param)
				CHECK_FLOAT32(y2Param)

				x1 = x1Param.signedIntValue();
				x2 = x2Param.signedIntValue();
				y1 = y1Param.floatValue();
				y2 = y2Param.floatValue();
				break;

			case SCAL_32FP_32FP:
				CHECK_FLOAT32(x1Param)
				CHECK_FLOAT32(x2Param)
				CHECK_FLOAT32(y1Param)
				CHECK_FLOAT32(y2Param)

				x1 = x1Param.floatValue();
				x2 = x2Param.floatValue();
				y1 = y1Param.floatValue();
				y2 = y2Param.floatValue();
				break;

			case SCAL_32FP_16UI:
				CHECK_FLOAT32(x1Param);
				CHECK_FLOAT32(x2Param);
				CHECK_UNSIGNED_INT16(y1Param);
				CHECK_UNSIGNED_INT16(y2Param);

				x1 = x1Param.floatValue();
				x2 = x2Param.floatValue();
				y1 = y1Param.unsignedIntValue();
				y2 = y2Param.unsignedIntValue();
				break;

			case SCAL_32FP_32SI:
				CHECK_FLOAT32(x1Param)
				CHECK_FLOAT32(x2Param)
				CHECK_SIGNED_INT32(y1Param)
				CHECK_SIGNED_INT32(y2Param)

				x1 = x1Param.floatValue();
				x2 = x2Param.floatValue();
				y1 = y1Param.signedIntValue();
				y2 = y2Param.signedIntValue();
				break;

			case SCAL_16UI_32FP:
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

		LOG_ERROR(m_log, QString(tr("Value %1 of parameter 'i_config' of FB %2 is incorrect")).
				  arg(iConf).arg(caption()));

		return false;
	}
}
