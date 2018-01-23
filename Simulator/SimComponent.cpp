#include "SimComponent.h"
#include <type_traits>
#include <cfenv>
#include <QQmlEngine>

namespace Sim
{

	ComponentParam::ComponentParam(const ComponentParam& that)
	{
		*this = that;
	}

	ComponentParam::ComponentParam(quint16 paramOpIndex) :
		m_paramOpIndex(paramOpIndex)
	{
	}

	ComponentParam& ComponentParam::operator=(const ComponentParam& that)
	{
		m_paramOpIndex = that.m_paramOpIndex;
		m_data = that.m_data;
		m_mathFlags.data = that.m_mathFlags.data;

		return *this;
	}

	int ComponentParam::opIndex() const
	{
		return m_paramOpIndex;
	}

	void ComponentParam::setOpIndex(int index)
	{
		m_paramOpIndex = index;
	}

	quint16 ComponentParam::wordValue() const
	{
		return m_data.asWord;
	}

	void ComponentParam::setWordValue(quint16 value)
	{
		m_data.data = 0;
		m_data.asWord = value;
	}

	float ComponentParam::floatValue() const
	{
		return m_data.asFloat;
	}

	void ComponentParam::setFloatValue(float value)
	{
		m_data.data = 0;
		m_data.asFloat = value;
	}

	double ComponentParam::doubleValue() const
	{
		return m_data.asDouble;
	}

	void ComponentParam::setDoubleValue(double value)
	{
		m_data.data = 0;
		m_data.asDouble = value;
	}

	qint32 ComponentParam::signedIntValue() const
	{
		return m_data.asSignedInt;
	}

	void ComponentParam::setSignedIntValue(qint32 value)
	{
		m_data.data = 0;
		m_data.asSignedInt = value;
	}

	void ComponentParam::addSignedInteger(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand->signedIntValue();
		qint32 result = op1 + op2;
		qint64 wideResult = static_cast<qint64>(op1) + static_cast<qint64>(op2);

		setSignedIntValue(result);

		// Setting math flags, matters only:
		// overflow
		// zero
		//
		resetMathFlags();

		m_mathFlags.overflow = wideResult > std::numeric_limits<qint32>::max() ||
							   wideResult < std::numeric_limits<qint32>::min();

		m_mathFlags.zero = (result == 0);

		return;
	}

	void ComponentParam::subSignedInteger(ComponentParam* operand)
	{
		ComponentParam negativeParam = *operand;
		negativeParam.setSignedIntValue(negativeParam.signedIntValue() * (-1));

		return addSignedInteger(&negativeParam);
	}

	void ComponentParam::mulSignedInteger(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand->signedIntValue();
		qint32 result = op1 * op2;
		qint64 wideResult = static_cast<qint64>(op1) * static_cast<qint64>(op2);

		setSignedIntValue(result);

		// Setting math flags, matters only:
		// overflow
		// zero
		//
		resetMathFlags();

		m_mathFlags.overflow = wideResult > std::numeric_limits<qint32>::max() ||
							   wideResult < std::numeric_limits<qint32>::min();

		m_mathFlags.zero = (result == 0);

		return;
	}

	void ComponentParam::divSignedInteger(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand->signedIntValue();
		qint32 result = 0;

		if (op2 != 0)
		{
			result = op1 / op2;
		}
		else
		{
			//  X / 0 = -1
			// -X / 0 =  1
			//  0 / 0 = -1
			//
			if (op1 == 0)
			{
				result = -1;
			}
			else
			{
				if (op1 < 0)
				{
					result = 1;
				}
				else
				{
					result = -1;
				}
			}
		}

		setSignedIntValue(result);

		// Setting math flags, matters only:
		// zero
		// divByZero
		//
		resetMathFlags();

		if (op2 == 0)
		{
			m_mathFlags.divByZero = true;
		}
		else
		{
			m_mathFlags.zero = (result == 0);
		}

		return;
	}

	void ComponentParam::addFloatingPoint(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand->floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 + op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f);
		m_mathFlags.nan = (result != result);		// According to the IEEE standard, NaN values have the odd property that comparisons involving
													// them are always false. That is, for a float f, f != f will be true only if f is NaN.
													// Some compilers can optimize it!

		setFloatValue(result);

		return;
	}

	void ComponentParam::subFloatingPoint(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand->floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 - op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f);
		m_mathFlags.nan = (result != result);		// According to the IEEE standard, NaN values have the odd property that comparisons involving
													// them are always false. That is, for a float f, f != f will be true only if f is NaN.
													// Some compilers can optimize it!

		setFloatValue(result);

		return;
	}

	void ComponentParam::mulFloatingPoint(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand->floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 * op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f);
		m_mathFlags.nan = (result != result);		// According to the IEEE standard, NaN values have the odd property that comparisons involving
													// them are always false. That is, for a float f, f != f will be true only if f is NaN.
													// Some compilers can optimize it!

		setFloatValue(result);

		return;
	}

	void ComponentParam::divFloatingPoint(ComponentParam* operand)
	{
		if (operand == nullptr)
		{
			assert(operand);
			return;
		}

		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand->floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 / op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f);
		m_mathFlags.nan = (result != result);		// According to the IEEE standard, NaN values have the odd property that comparisons involving
													// them are always false. That is, for a float f, f != f will be true only if f is NaN.
													// Some compilers can optimize it!

		setFloatValue(result);

		return;
	}

	void ComponentParam::resetMathFlags()
	{
		m_mathFlags.data = 0;
	}

	bool ComponentParam::mathOverflow() const
	{
		return m_mathFlags.overflow;
	}

	bool ComponentParam::mathUnderflow() const
	{
		return m_mathFlags.underflow;
	}

	bool ComponentParam::mathZero() const
	{
		return m_mathFlags.zero;
	}

	bool ComponentParam::mathNan() const
	{
		return m_mathFlags.nan;
	}

	bool ComponentParam::mathDivByZero() const
	{
		return m_mathFlags.divByZero;
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

	const ComponentParam* ComponentInstance::param(int opIndex) const
	{
		auto it = m_params.find(opIndex);
		if (it == m_params.end())
		{
			return nullptr;
		}

		const ComponentParam* componentParam = &it->second;
		return componentParam;
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

	bool ComponentInstance::addParam(int opIndex, ComponentParam* param)
	{
		if (param == nullptr)
		{
			assert(param);
			return false;
		}

		m_params[opIndex] = *param;
		return true;
	}

	bool ComponentInstance::addParamWord(int opIndex, quint16 value)
	{
		ComponentParam param(opIndex);
		param.setWordValue(value);

		m_params[opIndex] = param;
		return true;
	}

	bool ComponentInstance::addParamFloat(int opIndex, float value)
	{
		ComponentParam param(opIndex);
		param.setFloatValue(value);

		m_params[opIndex] = param;
		return true;
	}

	bool ComponentInstance::addParamSignedInt(int opIndex, qint32 value)
	{
		ComponentParam param(opIndex);
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

	ComponentInstance* AfbComponentSet::componentInstance(int componentOpCode, int instance)
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
