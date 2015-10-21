#include "ApplicationLogicCompiler.h"

namespace Builder
{

	bool ModuleLogicCompiler::calculateFbParamsValues(const AppFb& appFb, AfbParamValuesArray& paramValuesArray, bool /* instantiatorOnly */ )
	{
		bool result = true;

		for(const Afb::AfbParam& afbParam : appFb.afb().params())
		{
			AfbParamValue afbParamValue(afbParam);

			paramValuesArray.insert(afbParam.opName(), afbParamValue);
		}

		switch(appFb.opcode())
		{
		case Afb::AfbType::LOGIC:
			// parameter's values calculation are not required
			break;

		case Afb::AfbType::TCT:
			result = calculate_TCT_paramsValues(appFb, paramValuesArray);
			break;

		case Afb::AfbType::BCOMP:
			result = calculate_BCOMP_paramsValues(appFb, paramValuesArray);
			break;


		case Afb::AfbType::SCAL:
			//result = calculate_SCAL_paramsValues(appFb, paramValuesArray);
			break;

		default:
			LOG_ERROR(m_log, QString(tr("Parameter's values calculation for FB %1 (opcode %2) is not implemented")).
					arg(appFb.caption()).arg(appFb.opcode()));
			result = false;
		}

		/*
			for(Afb::AfbParam afbParam : afb.params())
			{
				if (afbParam.operandIndex() == NOT_FB_OPERAND_INDEX)
				{
					continue;
				}

				if (instantiatorOnly && !afbParam.instantiator())
				{
					continue;
				}

				AfbParamValue afbParamValue(afbParam);

				if (afbParam.isAnalog())
				{
					result &= calculateFbAnalogParamValue(*appFb, afbParam, &afbParamValue);
				}

				result &= generateWriteAfbParamCode(*appFb, afbParam, afbParamValue);
			}
		*/

		return result;
	}


	bool ModuleLogicCompiler::checkRequiredParameters(const AppFb& appFb, AfbParamValuesArray& paramValuesArray, const QStringList& requiredParams)
	{
		bool result = true;

		for(const QString& param : requiredParams)
		{
			if (paramValuesArray.contains(param) == false)
			{
				LOG_ERROR(m_log, QString(tr("Required parameter '%1' of FB %2 (%3) is missing")).
						  arg(param).arg(appFb.caption()).arg(appFb.typeCaption()));
				result = false;
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::calculate_TCT_paramsValues(const AppFb& appFb, AfbParamValuesArray& paramValuesArray)
	{
		QStringList requiredParams;

		requiredParams.append("i_counter");

		if (checkRequiredParameters(appFb, paramValuesArray, requiredParams) == false)
		{
			return false;
		}

		AfbParamValue& afbParamValue = paramValuesArray["i_counter"];

		assert(afbParamValue.isUnsignedInt32());

		afbParamValue.unsignedIntValue = (afbParamValue.unsignedIntValue * 1000) / m_lmCycleDuration;

		return true;
	}


	bool ModuleLogicCompiler::calculate_BCOMP_paramsValues(const AppFb& appFb, AfbParamValuesArray& paramValuesArray)
	{
		QStringList requiredParams;

		requiredParams.append("i_sp_s");
		requiredParams.append("i_sp_r");
		requiredParams.append("hysteresis");
		requiredParams.append("i_conf");

		if (checkRequiredParameters(appFb, paramValuesArray, requiredParams) == false)
		{
			return false;
		}

		AfbParamValue& iConfParam = paramValuesArray["i_conf"];
		AfbParamValue& sSettingParam = paramValuesArray["i_sp_s"];
		AfbParamValue& rSettingParam = paramValuesArray["i_sp_r"];
		AfbParamValue& hysteresisRParam = paramValuesArray["hysteresis"];

		assert(iConfParam.isUnsignedInt());

		int iConf = iConfParam.unsignedIntValue;

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
			assert(sSettingParam.isSignedInt32());
			assert(rSettingParam.isSignedInt32());
			assert(hysteresisRParam.isSignedInt32());

			int sSetting = sSettingParam.signedIntValue;
			int hysteresis = abs(hysteresisRParam.signedIntValue);

			switch(iConf)
			{
			case BCOMP_32SI_EQU:
				sSettingParam.signedIntValue = sSetting + hysteresis / 2;
				rSettingParam.signedIntValue = sSetting - hysteresis / 2;
				break;

			case BCOMP_32SI_GREAT:
				rSettingParam.signedIntValue = sSetting - hysteresis;
				break;

			case BCOMP_32SI_LESS:
				rSettingParam.signedIntValue = sSetting + hysteresis;
				break;

			case BCOMP_32SI_NOT_EQU:
				sSettingParam.signedIntValue = sSetting - hysteresis / 2;
				rSettingParam.signedIntValue = sSetting + hysteresis / 2;
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
			assert(sSettingParam.isFloat32());
			assert(rSettingParam.isFloat32());
			assert(hysteresisRParam.isFloat32());

			double sSetting = sSettingParam.floatValue;
			double hysteresis = abs(hysteresisRParam.floatValue);

			switch(iConf)
			{
			case BCOMP_32FP_EQU:
				sSettingParam.floatValue = sSetting + hysteresis / 2;
				rSettingParam.floatValue = sSetting - hysteresis / 2;
				break;

			case BCOMP_32FP_GREAT:
				rSettingParam.floatValue = sSetting - hysteresis;
				break;

			case BCOMP_32FP_LESS:
				rSettingParam.floatValue = sSetting + hysteresis;
				break;

			case BCOMP_32FP_NOT_EQU:
				sSettingParam.floatValue = sSetting - hysteresis / 2;
				rSettingParam.floatValue = sSetting + hysteresis / 2;
				break;

			default:
				assert(false);
			}

			return true;
		}

		LOG_ERROR(m_log, QString(tr("Value %1 of parameter 'i_confog' of FB %2 is incorrect")).
				  arg(iConf).arg(appFb.caption()));

		return false;
	}


	bool ModuleLogicCompiler::calculate_SCAL_paramsValues(const AppFb&, AfbParamValuesArray& paramValuesArray)
	{
		return true;
	}
}
