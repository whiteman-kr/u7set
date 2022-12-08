#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <array>
#include <optional>
#include <QObject>
#include "../lib/LmDescription.h"
#include "../VFrame30/Afb.h"


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

		[[nodiscard]]int opCode() const;
		[[nodiscard]]QString caption() const;
		[[nodiscard]]int maxInstCount() const;
		[[nodiscard]]QString simulationFunc() const;

		[[nodiscard]]bool pinExists(int pinOpIndex) const;
		[[nodiscard]]QString pinCaption(int pinOpIndex) const;

	private:
		std::shared_ptr<Afb::AfbComponent> m_afbComponent;
	};


	class AfbComponentParam final
	{
	public:
		AfbComponentParam() = default;
		AfbComponentParam(const AfbComponentParam& that) noexcept = default;
		explicit AfbComponentParam(quint16 paramOpIndex) :
			m_paramOpIndex(paramOpIndex)
		{
			std::fill(m_data.begin(), m_data.end(), 0);
			static_assert(std::is_trivially_copyable_v<AfbComponentParam>);
		}

		explicit AfbComponentParam(quint16 paramOpIndex, quint16 word) :
			m_paramOpIndex(paramOpIndex)
		{
			setWordValue(word);
		}

	public:
		[[nodiscard]] int opIndex() const noexcept
		{
			return m_paramOpIndex;
		}
		void setOpIndex(int index) noexcept
		{
			m_paramOpIndex = static_cast<quint16>(index);
		}

		[[nodiscard]] quint16 wordValue() const noexcept
		{
			return dataToType<quint16>();
		}
		void setWordValue(quint16 value) noexcept
		{
			setDataToType<quint16>(value);
		}

		[[nodiscard]] quint32 dwordValue() const  noexcept
		{
			return dataToType<quint32>();
		}
		void setDwordValue(quint32 value)  noexcept
		{
			setDataToType<quint32>(value);
		}

		[[nodiscard]] float floatValue() const  noexcept
		{
			return dataToType<float>();
		}
		void setFloatValue(float value) noexcept
		{
			setDataToType<float>(value);
		}

		[[nodiscard]] double doubleValue() const  noexcept
		{
			return dataToType<double>();
		}
		void setDoubleValue(double value) noexcept
		{
			setDataToType<double>(value);
		}

		[[nodiscard]] qint32 signedIntValue() const noexcept
		{
			return dataToType<qint32>();
		}
		void setSignedIntValue(qint32 value) noexcept
		{
			setDataToType<qint32>(value);
		}

		[[nodiscard]] qint64 signedInt64Value() const noexcept
		{
			return dataToType<qint64>();
		}
		void setSignedInt64Value(qint64 value) noexcept
		{
			setDataToType<qint64>(value);
		}

		// --
		//
		void addSignedInteger(const AfbComponentParam& operand);	// Tests+
		void subSignedInteger(const AfbComponentParam& operand);	// Tests+
		void mulSignedInteger(const AfbComponentParam& operand);	// Tests+
		void divSignedInteger(const AfbComponentParam& operand);	// Tests+

		void addSignedIntegerNumber(qint32 operand);				// Tests+
		void subSignedIntegerNumber(qint32 operand);				// Tests+
		void mulSignedIntegerNumber(qint32 operand);				// Tests+
		void divSignedIntegerNumber(qint32 operand);				// Tests+

		void addFloatingPoint(const AfbComponentParam& operand);	// Tests+
		void subFloatingPoint(const AfbComponentParam& operand);	// Tests+
		void mulFloatingPoint(const AfbComponentParam& operand);
		void divFloatingPoint(const AfbComponentParam& operand);

		void addFloatingPoint(float operand);						// Tests+
		void subFloatingPoint(float operand);						// Tests+
		void mulFloatingPoint(float operand);
		void divFloatingPoint(float operand);

		void absFloatingPoint();									// Tests+
		void absSignedInt();										// Tests+

		void sinFloatingPoint();
		void cosFloatingPoint();
		void logFloatingPoint();
		void expFloatingPoint();

		void convertSInt32ToSInt64();								// Tests+
		void convertSInt64ToSInt32();								// Tests+
		void convertSignedIntToFloat();								// Tests+
		void convertWordToFloat();									// Tests+
		void convertWordToSignedInt();								// Tests+

		// --
		//
		void resetMathFlags() noexcept
		{
			m_mathFlags = 0;
		}

		[[nodiscard]] quint16 mathOverflow() const noexcept
		{
			return (m_mathFlags & FLAG_OVERFLOW) ? 1 : 0;
		}
		void setMathOverflow(quint16 value) noexcept
		{
			if (value == 0)
			{
				m_mathFlags &= ~FLAG_OVERFLOW;
			}
			else
			{
				m_mathFlags |= FLAG_OVERFLOW;
			}
		}

		[[nodiscard]] quint16 mathUnderflow() const noexcept
		{
			return (m_mathFlags & FLAG_UNDERFLOW) ? 1 : 0;
		}
		void setMathUnderflow(quint16 value) noexcept
		{
			if (value == 0)
			{
				m_mathFlags &= ~FLAG_UNDERFLOW;
			}
			else
			{
				m_mathFlags |= FLAG_UNDERFLOW;
			}
		}

		[[nodiscard]] quint16 mathZero() const noexcept
		{
			return (m_mathFlags & FLAG_ZERO) ? 1 : 0;
		}
		void setMathZero(quint16 value) noexcept
		{
			if (value == 0)
			{
				m_mathFlags &= ~FLAG_ZERO;
			}
			else
			{
				m_mathFlags |= FLAG_ZERO;
			}
		}

		[[nodiscard]] quint16 mathNan() const noexcept
		{
			return (m_mathFlags & FLAG_NAN) ? 1 : 0;
		}
		void setMathNan(quint16 value) noexcept
		{
			if (value == 0)
			{
				m_mathFlags &= ~FLAG_NAN;
			}
			else
			{
				m_mathFlags |= FLAG_NAN;
			}
		}

		[[nodiscard]] quint16 mathDivByZero() const noexcept
		{
			return (m_mathFlags & FLAG_DIVBYZERO) ? 1 : 0;
		}
		void setMathDivByZero(quint16 value) noexcept
		{
			if (value == 0)
			{
				m_mathFlags &= ~FLAG_DIVBYZERO;
			}
			else
			{
				m_mathFlags |= FLAG_DIVBYZERO;
			}
		}

	private:
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
			std::fill(m_data.begin(), m_data.end(), 0);
			std::memcpy(m_data.data(), &value, sizeof(value));
		}

		std::array<char, 8> m_data;

		// Math operations flags
		//
		constexpr static quint16 FLAG_OVERFLOW = 0x0001;
		constexpr static quint16 FLAG_UNDERFLOW = 0x0002;
		constexpr static quint16 FLAG_ZERO = 0x0004;
		constexpr static quint16 FLAG_NAN = 0x0008;
		constexpr static quint16 FLAG_DIVBYZERO = 0x0010;

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
		bool addParam(int instanceNo, AfbComponentParam&& instParam, QString* errorMessage);

		[[nodiscard]] AfbComponentInstance* instance(quint16 instance) noexcept
		{
			return instance > m_instances.size() ? nullptr : &m_instances[instance];
		}

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
		bool addInstantiatorParam(int afbOpCode, int instanceNo, AfbComponentParam&& instParam, QString* errorMessage);

		[[nodiscard]] AfbComponentInstance* componentInstance(int componentOpCode, int instance) noexcept;

	private:
		std::vector<ModelComponent> m_components;		// Index is opcode of AFB
	};
}
