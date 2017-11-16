#include "Component.h"

namespace LmModel
{

	InstantiatorParam::InstantiatorParam(quint16 implNo, quint16 implParamOpIndex, quint16 dataLow, quint16 dataHigh) :
		implNo(implNo),
		implParamOpIndex(implParamOpIndex),
		dataLow(dataLow),
		dataHigh(dataHigh)
	{
	}

//	InstantiatorSet::InstantiatorSet()
//	{
//	}

//	InstantiatorSet::clear()
//	{
//		components.clear();
//	}

//	bool InstantiatorSet::addInstantiatorParam(quint16 compOpCode, const InstantiatorParam& param, QString* errorMessage)
//	{
//		assert(errorMessage);

//		bool result = true;

//		InstantiatorComponent& comp = m_components[compOpCode];

//		// ToDo, check if compOpCode exists in LmDescription
//		//
//		int To_Do_Check_if_compOpCode_exists_in_lmdescription;
//		int To_Do_Check_if_param_impl_no_is_in_limits;
//		int To_Do_Check_if_param_implParamOpIndex_is_esists_in_lm_description;

//		if (comp.params.count(param.implParamOpIndex) != 0)
//		{
//			// This param already has been initialized
//			//
//			*errorMessage = QString("Parametr %1 of component %2 already has been initialized").arg(param.implParamOpIndex).arg(compOpCode);
//			result = false;
//		}

//		comp.params.insert(param.implParamOpIndex, param)

//		return result;
//	}

	ModelComponent::ModelComponent()
	{
	}

	AfbComponentSet::AfbComponentSet()
	{
	}

}
