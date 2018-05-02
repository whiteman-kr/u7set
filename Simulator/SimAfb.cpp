#include "SimAfb.h"
#include <type_traits>
#include <cfenv>
#include <QQmlEngine>

extern "C" {
	#include "../Lua/lua.h"
	#include "../Lua/lauxlib.h"
	#include "../Lua/lualib.h"
}
#include "../LuaIntf/LuaIntf.h"

namespace Sim
{

	AfbComponent::AfbComponent(std::shared_ptr<Afb::AfbComponent> afbComponent) :
		m_afbComponent(afbComponent)
	{
		// assert(m_afbComponent);	Actually m_afbComponent can be nullptr, script should call isNull to detect it
	}

	void AfbComponent::registerLuaClass(lua_State* L)
	{
		using namespace LuaIntf;

		LuaBinding(L).beginClass<Sim::AfbComponent>("AfbComponent")
				.addFunction("isNull", &Sim::AfbComponent::isNull)
				.addPropertyReadOnly("opCode", &Sim::AfbComponent::opCode)
				.addPropertyReadOnly("caption", &Sim::AfbComponent::caption)
				.addPropertyReadOnly("maxInstCount", &Sim::AfbComponent::maxInstCount)
				.addPropertyReadOnly("simulationFunc", &Sim::AfbComponent::simulationFunc)
				.addFunction("pinExists", &Sim::AfbComponent::pinExists, LUA_ARGS(_opt<int>))
				.addFunction("pinCaption", &Sim::AfbComponent::pinCaption, LUA_ARGS(_opt<int>))
				.endClass();

		return;
	}

	bool AfbComponent::isNull() const
	{
		return m_afbComponent.get() == nullptr;
	}

	int AfbComponent::opCode() const
	{
		if (m_afbComponent == nullptr)
		{
			return -1;
		}

		return m_afbComponent->opCode();
	}

	std::string AfbComponent::caption() const
	{
		if (m_afbComponent == nullptr)
		{
			return std::string();
		}

		return m_afbComponent->caption().toStdString();
	}

	int AfbComponent::maxInstCount() const
	{
		if (m_afbComponent == nullptr)
		{
			return -1;
		}

		return m_afbComponent->maxInstCount();
	}

	std::string AfbComponent::simulationFunc() const
	{
		if (m_afbComponent == nullptr)
		{
			return std::string();
		}

		return m_afbComponent->simulationFunc().toStdString();
	}

	bool AfbComponent::pinExists(int pinOpIndex) const
	{
		if (m_afbComponent == nullptr)
		{
			return false;
		}

		return m_afbComponent->pinExists(pinOpIndex);
	}

	std::string AfbComponent::pinCaption(int pinOpIndex) const
	{
		if (m_afbComponent == nullptr)
		{
			return std::string();
		}

		return m_afbComponent->pinCaption(pinOpIndex).toStdString();
	}


	AfbComponentParam::AfbComponentParam(quint16 paramOpIndex) :
		m_paramOpIndex(paramOpIndex)
	{
	}

	void AfbComponentParam::registerLuaClass(lua_State* L)
	{
		using namespace LuaIntf;

		LuaBinding(L).beginClass<Sim::AfbComponentParam>("AfbComponentParam")
				.addConstructor(LUA_ARGS())
				.addConstructor(LUA_ARGS(_opt<quint16>))

				.addProperty("opIndex", &AfbComponentParam::opIndex, &AfbComponentParam::setOpIndex)
				.addProperty("asWord", &AfbComponentParam::wordValue, &AfbComponentParam::setWordValue)
				.addProperty("asDword", &AfbComponentParam::dwordValue, &AfbComponentParam::setDwordValue)
				.addProperty("asFloat", &AfbComponentParam::floatValue, &AfbComponentParam::setFloatValue)
				.addProperty("asDouble", &AfbComponentParam::doubleValue, &AfbComponentParam::setDoubleValue)
				.addProperty("asSignedInt", &AfbComponentParam::signedIntValue, &AfbComponentParam::setSignedIntValue)

//				.addFunction("isNull", &Sim::AfbComponent::isNull)
//				.addFunction("pinCaption", &Sim::AfbComponent::pinCaption, LUA_ARGS(_opt<int>))
				.endClass();
	}

	int AfbComponentParam::opIndex() const
	{
		return m_paramOpIndex;
	}

	void AfbComponentParam::setOpIndex(int index)
	{
		m_paramOpIndex = index;
	}

	quint16 AfbComponentParam::wordValue() const
	{
		return m_data.asWord;
	}

	void AfbComponentParam::setWordValue(quint16 value)
	{
		m_data.data = 0;
		m_data.asWord = value;
	}

	quint32 AfbComponentParam::dwordValue() const
	{
		return m_data.asDword;
	}

	void AfbComponentParam::setDwordValue(quint32 value)
	{
		m_data.data = 0;
		m_data.asDword = value;
	}

	float AfbComponentParam::floatValue() const
	{
		return m_data.asFloat;
	}

	void AfbComponentParam::setFloatValue(float value)
	{
		m_data.data = 0;
		m_data.asFloat = value;
	}

	double AfbComponentParam::doubleValue() const
	{
		return m_data.asDouble;
	}

	void AfbComponentParam::setDoubleValue(double value)
	{
		m_data.data = 0;
		m_data.asDouble = value;
	}

	qint32 AfbComponentParam::signedIntValue() const
	{
		return m_data.asSignedInt;
	}

	void AfbComponentParam::setSignedIntValue(qint32 value)
	{
		m_data.data = 0;
		m_data.asSignedInt = value;
	}

	void AfbComponentParam::addSignedInteger(AfbComponentParam* operand)
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

	void AfbComponentParam::subSignedInteger(AfbComponentParam* operand)
	{
		AfbComponentParam negativeParam = *operand;
		negativeParam.setSignedIntValue(negativeParam.signedIntValue() * (-1));

		return addSignedInteger(&negativeParam);
	}

	void AfbComponentParam::mulSignedInteger(AfbComponentParam* operand)
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

	void AfbComponentParam::divSignedInteger(AfbComponentParam* operand)
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

	void AfbComponentParam::addSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return addSignedInteger(&cp);
	}

	void AfbComponentParam::subSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return subSignedInteger(&cp);
	}

	void AfbComponentParam::mulSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return mulSignedInteger(&cp);
	}

	void AfbComponentParam::divSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return divSignedInteger(&cp);
	}

	void AfbComponentParam::addFloatingPoint(AfbComponentParam* operand)
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

	void AfbComponentParam::subFloatingPoint(AfbComponentParam* operand)
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

	void AfbComponentParam::mulFloatingPoint(AfbComponentParam* operand)
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

	void AfbComponentParam::divFloatingPoint(AfbComponentParam* operand)
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

	void AfbComponentParam::convertWordToFloat()
	{
		resetMathFlags();

		float data = static_cast<float>(m_data.asWord);
		setFloatValue(data);

		return;
	}

	void AfbComponentParam::convertWordToSignedInt()
	{
		resetMathFlags();

		qint32 data = static_cast<qint32>(m_data.asWord);
		setSignedIntValue(data);

		return;
	}

	void AfbComponentParam::resetMathFlags()
	{
		m_mathFlags.data = 0;
	}

	bool AfbComponentParam::mathOverflow() const
	{
		return m_mathFlags.overflow;
	}

	bool AfbComponentParam::mathUnderflow() const
	{
		return m_mathFlags.underflow;
	}

	bool AfbComponentParam::mathZero() const
	{
		return m_mathFlags.zero;
	}

	bool AfbComponentParam::mathNan() const
	{
		return m_mathFlags.nan;
	}

	bool AfbComponentParam::mathDivByZero() const
	{
		return m_mathFlags.divByZero;
	}

	AfbComponentInstance::AfbComponentInstance(quint16 instanceNo) :
		m_instanceNo(instanceNo)
	{
	}

	void AfbComponentInstance::registerLuaClass(lua_State* L)
	{
		using namespace LuaIntf;

		LuaBinding(L).beginClass<AfbComponentInstance>("AfbComponentInstance")
			.addFunction("addParam", &Sim::AfbComponentInstance::addParam, LUA_ARGS(_opt<const Sim::AfbComponentParam&>))
			.addFunction("param", &Sim::AfbComponentInstance::param, LUA_ARGS(_opt<int>))
			.addFunction("paramExists", &Sim::AfbComponentInstance::paramExists, LUA_ARGS(_opt<int>))

			.addFunction("addParamWord", &Sim::AfbComponentInstance::addParamWord, LUA_ARGS(_opt<int>, _opt<quint16>))
			.addFunction("addParamFloat", &Sim::AfbComponentInstance::addParamFloat, LUA_ARGS(_opt<int>, _opt<float>))
			.addFunction("addParamSignedInt", &Sim::AfbComponentInstance::addParamSignedInt, LUA_ARGS(_opt<int>, _opt<qint32>))

			.endClass();
	}

	bool AfbComponentInstance::addParam(const AfbComponentParam& param)
	{
		m_params[param.opIndex()] = param;
		return true;
	}

	AfbComponentParam* AfbComponentInstance::param(int opIndex)
	{
		auto it = m_params.find(opIndex);
		if (it == m_params.end())
		{
			return nullptr;
		}

		AfbComponentParam* componentParam = &it->second;
		return componentParam;
	}

	bool AfbComponentInstance::paramExists(int opIndex) const
	{
		return m_params.count(opIndex) > 0;
	}

	bool AfbComponentInstance::addParamWord(int opIndex, quint16 value)
	{
		AfbComponentParam param(opIndex);
		param.setWordValue(value);

		m_params[opIndex] = param;
		return true;
	}

	bool AfbComponentInstance::addParamFloat(int opIndex, float value)
	{
		AfbComponentParam param(opIndex);
		param.setFloatValue(value);

		m_params[opIndex] = param;
		return true;
	}

	bool AfbComponentInstance::addParamSignedInt(int opIndex, qint32 value)
	{
		AfbComponentParam param(opIndex);
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

	bool ModelComponent::addParam(int instanceNo, const AfbComponentParam& instParam, QString* errorMessage)
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

		AfbComponentInstance& compInst = compInstIt->second;

		bool ok = compInst.addParam(instParam);

		return ok;
	}

	AfbComponentInstance* ModelComponent::instance(quint16 instance)
	{
		auto isntanceIt = m_instances.find(instance);
		if (isntanceIt == m_instances.end())
		{
			return nullptr;
		}

		AfbComponentInstance* result = &isntanceIt->second;

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

	bool AfbComponentSet::addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, int instanceNo, const AfbComponentParam& instParam, QString* errorMessage)
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

	AfbComponentInstance* AfbComponentSet::componentInstance(int componentOpCode, int instance)
	{
		auto componentIt = m_components.find(componentOpCode);
		if (componentIt == m_components.end())
		{
			return nullptr;
		}

		std::shared_ptr<ModelComponent>	component = componentIt->second;

		AfbComponentInstance* result = component->instance(instance);

		return result;
	}

}
