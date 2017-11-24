#include "Component.h"

namespace LmModel
{

	ComponentParam::ComponentParam(quint16 instNo, quint16 paramOpIndex, quint32 data) :
		m_instNo(instNo),
		m_paramOpIndex(paramOpIndex),
		m_data(data)
	{
	}

	int ComponentParam::instanceNo() const
	{
		return m_instNo;
	}

	int ComponentParam::opIndex() const
	{
		return m_paramOpIndex;
	}

	quint16 ComponentParam::wordValue() const
	{
		return m_data & 0xFFFF;
	}

	double ComponentParam::floatValue() const
	{
		float fp = *reinterpret_cast<const float*>(&m_data);
		return static_cast<double>(fp);
	}

	qint32 ComponentParam::signedIntValue() const
	{
		return static_cast<qint32>(m_data);
	}

	ComponentInstance::ComponentInstance()
	{
	}

	bool ComponentInstance::addInputParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const ComponentParam& instParam, QString* errorMessage)
	{
		assert(errorMessage);
		assert(afbComp);

		if (m_params.count(instParam.opIndex()) != 0)
		{
			// This parameter already has been initialized
			//
			*errorMessage = QString("Pin with opIndex %1 already has been initialized, InstanceNo %2, AfbComponent %2")
								.arg(instParam.opIndex())
								.arg(instParam.instanceNo())
								.arg(afbComp->caption());
			return false;
		}

		m_params[instParam.opIndex()] = instParam;

		return true;
	}

	bool ComponentInstance::paramExists(int opIndex) const
	{
		return m_params.count(opIndex) > 0;
	}

	ComponentParam ComponentInstance::param(int opIndex) const
	{
		auto it = m_params.find(opIndex);
		if (it == m_params.end())
		{
			return ComponentParam();
		}

		const ComponentParam& componentParam = it->second;
		return componentParam;
	}

	ModelComponent::ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp) :
		m_afbComp(afbComp)
	{
		assert(m_afbComp);

		return;
	}

	bool ModelComponent::addParam(const ComponentParam& instParam, QString* errorMessage)
	{
		if (static_cast<int>(instParam.instanceNo()) >= m_afbComp->maxInstCount())
		{
			// To Do - ¬ы€снить, у ёрика номер реализации с 1, а ¬ити?
			// “огда и условие другое надо дл €ошибки
			//
			int To_Do;

			// Maximum of instatiator is reached
			//
			*errorMessage = QString("InstanceNo (%1) is higher then maximum (%2), Component %3")
								.arg(instParam.instanceNo())
								.arg(m_afbComp->maxInstCount())
								.arg(m_afbComp->caption());
			return false;
		}

		// Check if instParam.implParamOpIndex really exists in AfbComponent
		//
		const auto& compIns = m_afbComp->pins();

		if (compIns.count(instParam.opIndex()) != 1)
		{
			// Can't find such pin in AfbComponent
			//
			*errorMessage = QString("Can't fint pin with OpIndex %1, Component %2")
								.arg(instParam.opIndex())
								.arg(m_afbComp->caption());
			return false;
		}

		// Get or add instance and set new param
		//
		ComponentInstance& compInst = m_instances[instParam.instanceNo()];

		bool ok = compInst.addInputParam(m_afbComp, instParam, errorMessage);

		return ok;
	}

	ComponentInstance* ModelComponent::instance(quint16 instance)
	{
		auto isntanceIt = m_instances.find(instance);
		if (isntanceIt == m_instances.end())
		{
			return nullptr;
		}

		ComponentInstance* result = &isntanceIt->second;
		return result;
	}

	AfbComponentSet::AfbComponentSet()
	{
	}

	void AfbComponentSet::clear()
	{
		m_components.clear();
		return;
	}

	bool AfbComponentSet::addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const ComponentParam& instParam, QString* errorMessage)
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
		bool ok = modelComponent->addParam(instParam, errorMessage);

		return ok;
	}

	ComponentInstance* AfbComponentSet::componentInstance(quint16 componentOpCode, quint16 instance)
	{
		auto componentIt = m_components.find(componentOpCode);
		if (componentIt == m_components.end())
		{
			return nullptr;
		}

		std::shared_ptr<ModelComponent>	component = componentIt->second;

		ComponentInstance* result = component->instance(instance);
		return result;
	}

}
