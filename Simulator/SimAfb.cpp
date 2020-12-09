#include <type_traits>
#include <cfenv>
#include "SimAfb.h"
#include "SimException.h"


namespace Sim
{

	AfbComponent::AfbComponent(std::shared_ptr<Afb::AfbComponent>& afbComponent) :
		m_afbComponent(afbComponent)
	{
		// assert(m_afbComponent);	Actually m_afbComponent can be nullptr, script should call isNull to detect it
	}

	AfbComponent::AfbComponent(std::shared_ptr<Afb::AfbComponent>&& afbComponent) :
		m_afbComponent(std::move(afbComponent))
	{
		// assert(m_afbComponent);	Actually m_afbComponent can be nullptr, script should call isNull to detect it
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

	QString AfbComponent::caption() const
	{
		if (m_afbComponent == nullptr)
		{
			return {};
		}

		return m_afbComponent->caption();
	}

	int AfbComponent::maxInstCount() const
	{
		if (m_afbComponent == nullptr)
		{
			return -1;
		}

		return m_afbComponent->maxInstCount();
	}

	QString AfbComponent::simulationFunc() const
	{
		if (m_afbComponent == nullptr)
		{
			return {};
		}

		return m_afbComponent->simulationFunc();
	}

	bool AfbComponent::pinExists(int pinOpIndex) const
	{
		if (m_afbComponent == nullptr)
		{
			return false;
		}

		return m_afbComponent->pinExists(pinOpIndex);
	}

	QString AfbComponent::pinCaption(int pinOpIndex) const
	{
		if (m_afbComponent == nullptr)
		{
			return {};
		}

		return m_afbComponent->pinCaption(pinOpIndex);
	}

	void AfbComponentParam::addSignedInteger(const AfbComponentParam& operand)
	{
		// Signed integer overflow in c++ is undefined behavior, so we extend sinå32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand.signedIntValue();
		qint32 result = op1 + op2;

		qint64 wideResult = static_cast<qint64>(op1) + static_cast<qint64>(op2);

		if (wideResult > std::numeric_limits<qint32>::max())
		{
			result = std::numeric_limits<qint32>::max();
		}

		if (wideResult < std::numeric_limits<qint32>::min())
		{
			result = std::numeric_limits<qint32>::min();
		}

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

	void AfbComponentParam::subSignedInteger(const AfbComponentParam& operand)
	{
		// Signed integer overflow in c++ is undefined behavior, so we extend sinå32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand.signedIntValue();
		qint32 result = op1 - op2;

		qint64 wideResult = static_cast<qint64>(op1) - static_cast<qint64>(op2);

		if (wideResult > std::numeric_limits<qint32>::max())
		{
			result = std::numeric_limits<qint32>::max();
		}

		if (wideResult < std::numeric_limits<qint32>::min())
		{
			result = std::numeric_limits<qint32>::min();
		}

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

	void AfbComponentParam::mulSignedInteger(const AfbComponentParam& operand)
	{
		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand.signedIntValue();
		qint32 result = op1 * op2;
		qint64 wideResult = static_cast<qint64>(op1) * static_cast<qint64>(op2);

		if (wideResult > std::numeric_limits<qint32>::max())
		{
			result = std::numeric_limits<qint32>::max();
		}

		if (wideResult < std::numeric_limits<qint32>::min())
		{
			result = std::numeric_limits<qint32>::min();
		}

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

	void AfbComponentParam::divSignedInteger(const AfbComponentParam& operand)
	{
		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		qint32 op1 = this->signedIntValue();
		qint32 op2 = operand.signedIntValue();
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
		return addSignedInteger(cp);
	}

	void AfbComponentParam::subSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return subSignedInteger(cp);
	}

	void AfbComponentParam::mulSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return mulSignedInteger(cp);
	}

	void AfbComponentParam::divSignedIntegerNumber(qint32 operand)
	{
		AfbComponentParam cp(*this);
		cp.setSignedIntValue(operand);
		return divSignedInteger(cp);
	}

	void AfbComponentParam::addFloatingPoint(const AfbComponentParam& operand)
	{
		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand.floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 + op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f) || m_mathFlags.underflow;
		m_mathFlags.nan = std::isnan(result);

		setFloatValue(result);

		return;
	}

	void AfbComponentParam::subFloatingPoint(const AfbComponentParam& operand)
	{
		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand.floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 - op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f) || m_mathFlags.underflow;
		m_mathFlags.nan = std::isnan(result);

		setFloatValue(result);

		return;
	}

	void AfbComponentParam::mulFloatingPoint(const AfbComponentParam& operand)
	{
		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand.floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 * op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f) || m_mathFlags.underflow;
		m_mathFlags.nan = std::isnan(result);

		setFloatValue(result);

		return;
	}

	void AfbComponentParam::divFloatingPoint(const AfbComponentParam& operand)
	{
		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float op1 = this->floatValue();
		float op2 = operand.floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = op1 / op2;

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f) || m_mathFlags.underflow;
		m_mathFlags.nan = std::isnan(result);

		setFloatValue(result);

		return;
	}

	void AfbComponentParam::addFloatingPoint(float operand)
	{
		AfbComponentParam cp(*this);
		cp.setFloatValue(operand);
		return addFloatingPoint(cp);
	}

	void AfbComponentParam::subFloatingPoint(float operand)
	{
		AfbComponentParam cp(*this);
		cp.setFloatValue(operand);
		return subFloatingPoint(cp);
	}

	void AfbComponentParam::mulFloatingPoint(float operand)
	{
		AfbComponentParam cp(*this);
		cp.setFloatValue(operand);
		return mulFloatingPoint(cp);
	}

	void AfbComponentParam::divFloatingPoint(float operand)
	{
		AfbComponentParam cp(*this);
		cp.setFloatValue(operand);
		return divFloatingPoint(cp);
	}

	void AfbComponentParam::absFloatingPoint()
	{
		resetMathFlags();

		// signed integer overflow in c++ is undefined behavior, so we extend sion32 to sint64
		//
		float fp = this->floatValue();

		std::feclearexcept(FE_ALL_EXCEPT);
		float result = std::abs(fp);

		// Setting math flags
		//
		m_mathFlags.overflow = std::fetestexcept(FE_OVERFLOW);
		m_mathFlags.underflow = std::fetestexcept(FE_UNDERFLOW);
		m_mathFlags.divByZero = std::fetestexcept(FE_DIVBYZERO);
		m_mathFlags.zero = (result == .0f) || m_mathFlags.underflow;
		m_mathFlags.nan = (result != result);		// According to the IEEE standard, NaN values have the odd property that comparisons involving
													// them are always false. That is, for a float f, f != f will be true only if f is NaN.
													// Some compilers can optimize it!

		setFloatValue(result);

	}

	void AfbComponentParam::absSignedInt()
	{
		resetMathFlags();

		qint32 result = this->signedIntValue();

		if (result == std::numeric_limits<qint32>::min())
		{
			result = std::numeric_limits<qint32>::max();
			m_mathFlags.overflow = true;
		}
		else
		{
			result = std::abs(result);
			m_mathFlags.zero = (result == 0);
		}

		setSignedIntValue(result);

		return;
	}

	void AfbComponentParam::convertSignedIntToFloat()
	{
		resetMathFlags();

		float data = static_cast<float>(m_data.asSignedInt);
		setFloatValue(data);

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

	AfbComponentInstance::AfbComponentInstance(const std::shared_ptr<const Afb::AfbComponent>& afbComp,
											   quint16 instanceNo) :
		m_afbComp(afbComp),
		m_instanceNo(instanceNo)
	{
		Q_ASSERT(m_afbComp);
	}

	void AfbComponentInstance::resetState()
	{
		for (AfbComponentParam& p : m_params_a)
		{
			p = AfbComponentParam{};
		}

		return;
	}

	bool AfbComponentInstance::addParam(const AfbComponentParam& param)
	{
		if (param.opIndex() >= m_params_a.size())				// NOLINT
		{
			Q_ASSERT(param.opIndex() < m_params_a.size());		// NOLINT
			return false;
		}

		m_params_a[param.opIndex()] = param;
		return true;
	}

	bool AfbComponentInstance::addParam(AfbComponentParam&& param)
	{
		if (param.opIndex() >= m_params_a.size())					// NOLINT
		{
			Q_ASSERT(param.opIndex() < m_params_a.size());			// NOLINT
			return false;
		}

		m_params_a[param.opIndex()] = std::move(param);
		return true;
	}

	const AfbComponentParam* AfbComponentInstance::param(quint16 opIndex)
	{
		if (opIndex == m_afbComp->versionOpIndex())
		{
			// This is o_version output, which is constant always
			// if it is not present, then create it in m_params_v with version value from m_afbComp
			// and return it to caller
			//
			if (paramExists(opIndex) == false)
			{
				addParamWord(m_afbComp->versionOpIndex(), m_afbComp->impVersion());
			}
		}

		if (opIndex >= m_params_a.size() ||
			m_params_a[opIndex].opIndex() == 0xFFFF)
		{
			SimException::raise(QString("Param %1 is not found in AFB %2.").arg(opIndex).arg(m_afbComp->caption()));
		}

		return &m_params_a[opIndex];
	}

	bool AfbComponentInstance::paramExists(quint16 opIndex) const
	{
		if (opIndex > m_params_a.size())
		{
			return false;
		}

		return m_params_a[opIndex].opIndex() != 0xFFFF;
	}

	bool AfbComponentInstance::addParamWord(quint16 opIndex, quint16 value)
	{
		return addParam(AfbComponentParam{opIndex, value});
	}

	bool AfbComponentInstance::addParamDword(quint16 opIndex, quint32 value)
	{
		AfbComponentParam param(opIndex);
		param.setDwordValue(value);

		return addParam(std::move(param));
	}

	bool AfbComponentInstance::addParamFloat(quint16 opIndex, float value)
	{
		AfbComponentParam param(opIndex);
		param.setFloatValue(value);

		return addParam(std::move(param));
	}

	bool AfbComponentInstance::addParamDouble(quint16 opIndex, double value)
	{
		AfbComponentParam param(opIndex);
		param.setDoubleValue(value);

		return addParam(std::move(param));
	}

	bool AfbComponentInstance::addParamSignedInt(quint16 opIndex, qint32 value)
	{
		AfbComponentParam param(opIndex);
		param.setSignedIntValue(value);

		return addParam(std::move(param));
	}

	bool AfbComponentInstance::addParamSignedInt64(quint16 opIndex, qint64 value)
	{
		AfbComponentParam param(opIndex);
		param.setSignedInt64Value(value);

		return addParam(std::move(param));
	}

	//
	// ModelComponent
	//
	ModelComponent::ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp) :
		m_afbComp(afbComp)
	{
		assert(m_afbComp);
		return;
	}

	bool ModelComponent::init()
	{
		if (m_afbComp == nullptr)
		{
			Q_ASSERT(m_afbComp);
			return false;
		}

		if (m_afbComp->maxInstCount() > 2048)
		{
			Q_ASSERT(m_afbComp->maxInstCount() <= 2048);	// It seems something wrong here, the nyumber is too big?
			return false;
		}

		m_instances.clear();
		m_instances.reserve(std::max(0, m_afbComp->maxInstCount()));		// Can be negative. to avoid exception max is used here

		for (int i = 0; i < m_afbComp->maxInstCount(); i++)
		{
			m_instances.emplace_back(m_afbComp, i);		// AfbComponentInstance::AfbComponentInstance(quint16 instanceNo);
		}

		return true;
	}

	void ModelComponent::resetState()
	{
		for (AfbComponentInstance& inst : m_instances)
		{
			inst.resetState();
		}

		return;
	}

	bool ModelComponent::isNull() const
	{
		return m_afbComp == nullptr;
	}

	bool ModelComponent::addParam(int instanceNo, const AfbComponentParam& instParam, QString* errorMessage)
	{
		if (instanceNo >= m_afbComp->maxInstCount() ||
			instanceNo >= static_cast<int>(m_instances.size()))
		{
			// Maximum of instatiator is reached
			//
			*errorMessage = QString("InstanceNo (%1) is higher then maximum (%2), Component %3")
								.arg(instanceNo)
								.arg(m_afbComp->maxInstCount())
								.arg(m_afbComp->caption());
			return false;
		}

		// !!!The next condidion is commented fro perfomance reason
		// We check pinExists in all commands on parse stage, so it is nop need to checkit again
		// This check took up to 8% of programm runtime
		// !!!
		//
//		// Check if instParam.implParamOpIndex really exists in AfbComponent
//		//
//		if (m_afbComp->pinExists(instParam.opIndex()) == false)
//		{
//			// Can't find such pin in AfbComponent
//			//
//			*errorMessage = QString("Can't fint pin with OpIndex %1, Component %2")
//								.arg(instParam.opIndex())
//								.arg(m_afbComp->caption());
//			return false;
//		}

		// Get or add instance and set new param
		//
		bool ok = m_instances[instanceNo].addParam(instParam);

		return ok;
	}

	bool ModelComponent::addParam(int instanceNo, AfbComponentParam&& instParam, QString* errorMessage)
	{
		if (instanceNo >= m_afbComp->maxInstCount() ||
			instanceNo >= m_instances.size())					// NOLINT
		{
			// Maximum of instatiator is reached
			//
			*errorMessage = QString("InstanceNo (%1) is higher then maximum (%2), Component %3")
							.arg(instanceNo)
							.arg(m_afbComp->maxInstCount())
							.arg(m_afbComp->caption());
			return false;
		}

		// !!!The next condidion is commented fro perfomance reason
		// We check pinExists in all commands on parse stage, so it is nop need to checkit again
		// This check took up to 8% of programm runtime
		// !!!
		//
		//		// Check if instParam.implParamOpIndex really exists in AfbComponent
		//		//
		//		if (m_afbComp->pinExists(instParam.opIndex()) == false)
		//		{
		//			// Can't find such pin in AfbComponent
		//			//
		//			*errorMessage = QString("Can't fint pin with OpIndex %1, Component %2")
		//								.arg(instParam.opIndex())
		//								.arg(m_afbComp->caption());
		//			return false;
		//		}

		// Get or add instance and set new param
		//
		bool ok = m_instances[instanceNo].addParam(std::move(instParam));

		return ok;
	}

	AfbComponentSet::AfbComponentSet()
	{
		m_components.reserve(32);
	}

	void AfbComponentSet::clear()
	{
		m_components.clear();
	}

	void AfbComponentSet::resetState()
	{
		for (ModelComponent& mc : m_components)
		{
			mc.resetState();
		}

		return;
	}

	bool AfbComponentSet::init(const LmDescription& lmDescription)
	{
		m_components.clear();

		const auto& afbs = lmDescription.afbComponents();
		m_components.resize(256);		// opcode is quint16, and there is opcode 255 for set_flags (((

		for (const auto&[keyAfbOpCode, afbComp] : afbs)
		{
			Q_ASSERT(keyAfbOpCode == afbComp->opCode());

			if (keyAfbOpCode >= m_components.size())					// NOLINT
			{
				m_components.resize(keyAfbOpCode + 1);
			}

			ModelComponent mc{afbComp};

			if (bool ok = mc.init();
				ok == false)
			{
				return false;
			}

			m_components[keyAfbOpCode] = std::move(mc);
		}

		return true;
	}

	bool AfbComponentSet::addInstantiatorParam(int afbOpCode, int instanceNo, const AfbComponentParam& instParam, QString* errorMessage)
	{
		Q_ASSERT(errorMessage);

		if (afbOpCode >= m_components.size())
		{
			Q_ASSERT(afbOpCode < m_components.size());
			*errorMessage = QString("AFB with opcode %1 is not forund").arg(afbOpCode);
			return false;
		}

		ModelComponent& modelComponent = m_components[afbOpCode];
		if (modelComponent.isNull() == true)
		{
			// Component must be created in init();
			//
			*errorMessage = QString("AFB with opcode %1 is not found in AfbComponentSet").arg(afbOpCode);
			return false;
		}

		return modelComponent.addParam(instanceNo, instParam, errorMessage);
	}

	bool AfbComponentSet::addInstantiatorParam(int afbOpCode, int instanceNo, AfbComponentParam&& instParam, QString* errorMessage)
	{
		Q_ASSERT(errorMessage);

		if (afbOpCode >= m_components.size())
		{
			Q_ASSERT(afbOpCode < m_components.size());
			*errorMessage = QString("AFB with opcode %1 is not forund").arg(afbOpCode);
			return false;
		}

		ModelComponent& modelComponent = m_components[afbOpCode];
		if (modelComponent.isNull() == true)
		{
			// Component must be created in init();
			//
			*errorMessage = QString("AFB with opcode %1 is not found in AfbComponentSet").arg(afbOpCode);
			return false;
		}

		return modelComponent.addParam(instanceNo, std::move(instParam), errorMessage);
	}

	AfbComponentInstance* AfbComponentSet::componentInstance(int componentOpCode, int instance) noexcept
	{
		if (componentOpCode < 0 || componentOpCode >= m_components.size())
		{
			Q_ASSERT(componentOpCode >= 0 && componentOpCode < m_components.size());
			return nullptr;
		}

		ModelComponent& component = m_components[componentOpCode];
		if (component.isNull() == true)
		{
			return nullptr;
		}

		return component.instance(instance);
	}

}
