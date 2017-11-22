#include "Component.h"

namespace LmModel
{

	InstantiatorParam::InstantiatorParam(quint16 instNo, quint16 implParamOpIndex, quint32 data) :
		instNo(instNo),
		instParamOpIndex(implParamOpIndex),
		data(data)
	{
	}

	ComponentInst::ComponentInst()
	{
	}

	bool ComponentInst::addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const InstantiatorParam& instParam, QString* errorMessage)
	{
		assert(errorMessage);
		assert(afbComp);

		if (m_params.count(instParam.instParamOpIndex) != 0)
		{
			// This parameter already has been initialized
			//
			*errorMessage = QString("Pin with opIndex %1 already has been initialized, InstanceNo %2, AfbComponent %2")
								.arg(instParam.instParamOpIndex)
								.arg(instParam.instNo)
								.arg(afbComp->caption());
			return false;
		}

		m_params[instParam.instParamOpIndex] = instParam;

		return true;
	}

	ModelComponent::ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp) :
		m_afbComp(afbComp)
	{
		assert(m_afbComp);

		return;
	}

	bool ModelComponent::addInstantiatorParam(const InstantiatorParam& instParam, QString* errorMessage)
	{
		if (static_cast<int>(instParam.instNo) >= m_afbComp->maxInstCount())
		{
			// To Do - ¬ы€снить, у ёрика номер реализации с 1, а ¬ити?
			// “огда и условие другое надо дл €ошибки
			//
			int To_Do;

			// Maximum of instatiator is reached
			//
			*errorMessage = QString("InstanceNo (%1) is higher then maximum (%2), Component %3")
								.arg(instParam.instNo)
								.arg(m_afbComp->maxInstCount())
								.arg(m_afbComp->caption());
			return false;
		}

		// Check if instParam.implParamOpIndex really exists in AfbComponent
		//
		const auto& compIns = m_afbComp->pins();

		if (compIns.count(instParam.instParamOpIndex) != 1)
		{
			// Can't find such pin in AfbComponent
			//
			*errorMessage = QString("Can't fint pin with OpIndex %1, Component %2")
								.arg(instParam.instParamOpIndex)
								.arg(m_afbComp->caption());
			return false;
		}

		// Get or add instance and set new param
		//
		ComponentInst& compInst = m_instances[instParam.instNo];

		bool ok = compInst.addInstantiatorParam(m_afbComp, instParam, errorMessage);

		return ok;
	}

	AfbComponentSet::AfbComponentSet()
	{
	}

	void AfbComponentSet::clear()
	{
		m_components.clear();
		return;
	}

	bool AfbComponentSet::addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const InstantiatorParam& instParam, QString* errorMessage)
	{
		assert(errorMessage);

		if (afbComp == nullptr)
		{
			assert(afbComp);
			*errorMessage = "Input param afbComp is null";
			return false;
		}

		std::shared_ptr<ModelComponent> modelComponent;

		auto foundIt = m_components.find(afbComp->opCode());
		if (foundIt == m_components.end())
		{
			modelComponent = std::make_shared<ModelComponent>(afbComp);
			m_components[afbComp->opCode()] = modelComponent;
		}
		else
		{
			modelComponent = foundIt->second;
		}

		assert(modelComponent);

		// --
		//
		bool ok = modelComponent->addInstantiatorParam(instParam, errorMessage);

		return ok;
	}

}
