#include "Component.h"
#include <QQmlEngine>

namespace LmModel
{

	ComponentParam::ComponentParam(const ComponentParam& that)
	{
		*this = that;
	}

	ComponentParam::ComponentParam(quint16 paramOpIndex, quint32 data) :
		m_paramOpIndex(paramOpIndex),
		m_data(data)
	{
	}

	ComponentParam& ComponentParam::operator=(const ComponentParam& that)
	{
		m_paramOpIndex = that.m_paramOpIndex;
		m_data = that.m_data;

		return *this;
	}

	int ComponentParam::opIndex() const
	{
		return m_paramOpIndex;
	}

	quint16 ComponentParam::wordValue() const
	{
		return m_data & 0xFFFF;
	}

	void ComponentParam::setWordValue(quint16 value)
	{
		m_data = 0;
		m_data = value;
	}

	double ComponentParam::floatValue() const
	{
		float fp = *reinterpret_cast<const float*>(&m_data);
		return static_cast<double>(fp);
	}

	void ComponentParam::setFloatValue(double value)
	{
		float floatVal = static_cast<float>(value);
		*reinterpret_cast<float*>(&m_data) = floatVal;
	}

	qint32 ComponentParam::signedIntValue() const
	{
		return static_cast<qint32>(m_data);
	}

	void ComponentParam::setSignedIntValue(qint32 value)
	{
		m_data = static_cast<quint32>(value);
	}

	ComponentInstance::ComponentInstance(quint16 instanceNo) :
		m_instanceNo(instanceNo)
	{
	}

	bool ComponentInstance::addParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const ComponentParam& param, QString* errorMessage)
	{
		assert(errorMessage);
		assert(afbComp);

		if (m_params.count(param.opIndex()) != 0)
		{
			// This parameter already has been initialized
			//
			*errorMessage = QString("Pin with opIndex %1 already has been initialized, InstanceNo %2, AfbComponent %2")
								.arg(param.opIndex())
								.arg(m_instanceNo)
								.arg(afbComp->caption());
			return false;
		}

		m_params[param.opIndex()] = param;

		return true;
	}

	bool ComponentInstance::paramExists(int opIndex) const
	{
		return m_params.count(opIndex) > 0;
	}

	QObject* ComponentInstance::param(int opIndex)
	{
		auto it = m_params.find(opIndex);
		if (it == m_params.end())
		{
			return nullptr;
		}

		ComponentParam* componentParam = &it->second;
		QQmlEngine::setObjectOwnership(componentParam, QQmlEngine::ObjectOwnership::CppOwnership);

		return componentParam;
	}

	bool ComponentInstance::addOutputParamWord(int opIndex, quint16 value)
	{
		ComponentParam param(opIndex, 0);
		param.setWordValue(value);

		m_params[opIndex] = param;

		return true;
	}

	bool ComponentInstance::addOutputParamFloat(int opIndex, float value)
	{
		ComponentParam param(opIndex, 0);
		param.setFloatValue(value);

		m_params[opIndex] = param;

		return true;
	}

	bool ComponentInstance::addOutputParamSignedInt(int opIndex, qint32 value)
	{
		ComponentParam param(opIndex, 0);
		param.setSignedIntValue(value);

		m_params[opIndex] = param;

		return true;
	}

	ModelComponent::ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp) :
		m_afbComp(afbComp)
	{
		assert(m_afbComp);

		return;
	}

	bool ModelComponent::addParam(int instanceNo, const ComponentParam& instParam, QString* errorMessage)
	{
		if (static_cast<int>(instanceNo) >= m_afbComp->maxInstCount())
		{
			// To Do - ¬ы€снить, у ёрика номер реализации с 1, а ¬ити?
			// “огда и условие другое надо дл €ошибки
			//
			int To_Do;

			// Maximum of instatiator is reached
			//
			*errorMessage = QString("InstanceNo (%1) is higher then maximum (%2), Component %3")
								.arg(instanceNo)
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
		auto compInstIt = m_instances.find(instanceNo);
		if (compInstIt == m_instances.end())
		{
			compInstIt = m_instances.emplace(instanceNo, instanceNo).first;		// Insert new item and update iterator
		}

		ComponentInstance& compInst = compInstIt->second;

		bool ok = compInst.addParam(m_afbComp, instParam, errorMessage);

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
		QQmlEngine::setObjectOwnership(result, QQmlEngine::ObjectOwnership::CppOwnership);

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

	bool AfbComponentSet::addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, int instanceNo, const ComponentParam& instParam, QString* errorMessage)
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
		bool ok = modelComponent->addParam(instanceNo, instParam, errorMessage);

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
		QQmlEngine::setObjectOwnership(result, QQmlEngine::ObjectOwnership::CppOwnership);

		return result;
	}

}
