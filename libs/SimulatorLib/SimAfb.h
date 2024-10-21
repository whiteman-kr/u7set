#pragma once

#include <HardwareLib/Afb.h>

class LmDescription;
class SimAfbParamTests;


namespace Sim
{
	class AfbComponent final
	{
	public:
		AfbComponent() = delete;
		AfbComponent(const AfbComponent&) = default;
		AfbComponent(AfbComponent&&) noexcept = default;
		explicit AfbComponent(std::shared_ptr<Afb::AfbComponent>& afbComponent);
		explicit AfbComponent(std::shared_ptr<Afb::AfbComponent>&& afbComponent);

		AfbComponent& operator=(const AfbComponent&) = default;
		AfbComponent& operator=(AfbComponent&&) noexcept = default;

		~AfbComponent() = default;

	public:
		[[nodiscard]] bool isNull() const;

		[[nodiscard]] int opCode() const;
		[[nodiscard]] QString caption() const;
		[[nodiscard]] int maxInstCount() const;
		[[nodiscard]] QString simulationFunc() const;

		[[nodiscard]] bool pinExists(int pinOpIndex) const;
		[[nodiscard]] QString pinCaption(int pinOpIndex) const;

	private:
		std::shared_ptr<Afb::AfbComponent> m_afbComponent;
	};


	class AfbComponentParam final
	{
	public:
		AfbComponentParam();
		AfbComponentParam(const AfbComponentParam& that) noexcept = default;
		explicit AfbComponentParam(quint16 paramOpIndex);
		explicit AfbComponentParam(quint16 paramOpIndex, quint16 word);

		AfbComponentParam& operator=(const AfbComponentParam&) noexcept = default;
		AfbComponentParam& operator=(AfbComponentParam&&) noexcept = default;

	public:
		[[nodiscard]] int opIndex() const noexcept;
		void setOpIndex(int index) noexcept;

		[[nodiscard]] quint16 wordValue() const noexcept;
		void setWordValue(quint16 value) noexcept;

		[[nodiscard]] quint32 dwordValue() const  noexcept;
		void setDwordValue(quint32 value)  noexcept;

		[[nodiscard]] float floatValue() const noexcept;
		void setFloatValue(float value) noexcept;

		[[nodiscard]] double doubleValue() const  noexcept;
		void setDoubleValue(double value) noexcept;

		[[nodiscard]] qint32 signedIntValue() const noexcept;
		void setSignedIntValue(qint32 value) noexcept;

		[[nodiscard]] qint64 signedInt64Value() const noexcept;
		void setSignedInt64Value(qint64 value) noexcept;

		// --
		//
		void addSignedInteger(const AfbComponentParam& operand);
		void subSignedInteger(const AfbComponentParam& operand);
		void mulSignedInteger(const AfbComponentParam& operand);
		void divSignedInteger(const AfbComponentParam& operand);

		void addSignedInteger(qint32 operand);
		void subSignedInteger(qint32 operand);
		void mulSignedInteger(qint32 operand);
		void divSignedInteger(qint32 operand);

		void addFloatingPoint(const AfbComponentParam& operand);
		void subFloatingPoint(const AfbComponentParam& operand);
		void mulFloatingPoint(const AfbComponentParam& operand);
		void divFloatingPoint(const AfbComponentParam& operand);

		void addFloatingPoint(float operand);
		void subFloatingPoint(float operand);
		void mulFloatingPoint(float operand);
		void divFloatingPoint(float operand);

		void absFloatingPoint();
		void absSignedInt();

		void sinFloatingPoint();
		void cosFloatingPoint();
		void logFloatingPoint();
		void expFloatingPoint();

		void convertSInt32ToSInt64();
		void convertSInt64ToSInt32();
		void convertSignedIntToFloat();
		void convertWordToFloat();
		void convertWordToSignedInt();

		// --
		//
		void resetMathFlags() noexcept
		{
			m_mathFlags = 0;
		}

		[[nodiscard]] quint16 mathOverflow() const noexcept
		{
			return getMathFlag<FLAG_OVERFLOW>();
		}

		void setMathOverflow(std::integral auto value) noexcept
		{
			setMathFlag<FLAG_OVERFLOW>(value);
		}

		[[nodiscard]] quint16 mathUnderflow() const noexcept
		{
			return getMathFlag<FLAG_UNDERFLOW>();
		}

		void setMathUnderflow(std::integral auto value) noexcept
		{
			setMathFlag<FLAG_UNDERFLOW>(value);
		}

		[[nodiscard]] quint16 mathZero() const noexcept
		{
			return getMathFlag<FLAG_ZERO>();
		}

		void setMathZero(std::integral auto value) noexcept
		{
			setMathFlag<FLAG_ZERO>(value);
		}

		[[nodiscard]] quint16 mathNan() const noexcept
		{
			return getMathFlag<FLAG_NAN>();
		}

		void setMathNan(std::integral auto value) noexcept
		{
			setMathFlag<FLAG_NAN>(value);
		}

		[[nodiscard]] quint16 mathDivByZero() const noexcept
		{
			return getMathFlag<FLAG_DIVBYZERO>();
		}

		void setMathDivByZero(std::integral auto value) noexcept
		{
			setMathFlag<FLAG_DIVBYZERO>(value);
		}

	private:
		template<quint16 FLAG_MASK>
		[[nodiscard]] quint16 getMathFlag() const noexcept
		{
			return (m_mathFlags & FLAG_MASK) ? 1 : 0;
		}

		template<quint16 FLAG_MASK, std::integral T>
		void setMathFlag(T value) noexcept
		{
			if (value == static_cast<T>(0))
			{
				m_mathFlags &= ~FLAG_MASK;
			}
			else
			{
				m_mathFlags |= FLAG_MASK;
			}
		}

		template<typename T>
		T dataToType() const
		{
			static_assert(sizeof(T) <= 8);	// 8 is the size of m_data (bytes)
			T value;
			std::memcpy(&value, m_data.data(), sizeof(value));
			return value;
		}

		template<typename T>
		void setDataToType(T value)
		{
			static_assert(sizeof(T) <= 8);	// 8 is the size of m_data (bytes)
			std::fill(m_data.begin(), m_data.end(), '\0');
			std::memcpy(m_data.data(), &value, sizeof(value));
		}

		// Math operations flags
		//
		constexpr static quint16 FLAG_OVERFLOW = 0x0001;
		constexpr static quint16 FLAG_UNDERFLOW = 0x0002;
		constexpr static quint16 FLAG_ZERO = 0x0004;
		constexpr static quint16 FLAG_NAN = 0x0008;
		constexpr static quint16 FLAG_DIVBYZERO = 0x0010;

		std::array<char, 8> m_data{};
		quint16 m_mathFlags = 0;
		quint16 m_paramOpIndex = 0xFFFF;
	};


	// AfbComponentInstance, contains a set of params (InstantiatorParam) for this instance
	//
	class AfbComponentInstance
	{
	public:
		AfbComponentInstance(const std::shared_ptr<const Afb::AfbComponent>& afbComp, quint16 instanceNo);

	public:
		void resetState();

		bool addParam(const AfbComponentParam& param);

		[[nodiscard]] const AfbComponentParam* param(quint16 opIndex);

		[[nodiscard]] bool paramExists(quint16 opIndex) const;

		bool addParamWord(quint16 opIndex, quint16 value);
		bool addParamDword(quint16 opIndex, quint32 value);
		bool addParamFloat(quint16 opIndex, float value);
		bool addParamDouble(quint16 opIndex, double value);
		bool addParamSignedInt(quint16 opIndex, qint32 value);
		bool addParamSignedInt64(quint16 opIndex, qint64 value);

	private:
		std::shared_ptr<const Afb::AfbComponent> m_afbComp;
		quint16 m_instanceNo = 0;

		std::array<AfbComponentParam, 64> m_params_a;		// Index is AfbComponentParam.opIndex()
		quint16 m_versionOpIndex = 0xFFFF;					// Optimisation: cache versionOpIndex so no acces in param(...) for it.
	};


	// Model Component with a set of Instances
	//
	class ModelComponent
	{
	public:
		ModelComponent() = default;
		ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp);

	public:
		bool init();	// Create a number of instances
		void resetState();

		[[nodiscard]] bool isNull() const;

		bool addParam(int instanceNo, const AfbComponentParam& instParam, QString* errorMessage);

		[[nodiscard]] AfbComponentInstance* instance(quint16 instance) noexcept;

	private:
		std::vector<AfbComponentInstance> m_instances;			// Index is instNo
		std::shared_ptr<const Afb::AfbComponent> m_afbComp;
	};


	class AfbComponentSet
	{
	public:
		AfbComponentSet();

	public:
		void clear();
		void resetState();

		bool init(const LmDescription& lmDescription);
		bool addInstantiatorParam(int afbOpCode, int instanceNo, const AfbComponentParam& instParam, QString* errorMessage);

		[[nodiscard]] AfbComponentInstance* componentInstance(int componentOpCode, int instance) noexcept;

	private:
		std::vector<ModelComponent> m_components;		// Index is opcode of AFB

		friend SimAfbParamTests;
	};
}
