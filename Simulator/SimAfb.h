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
	class AfbComponent
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


	class AfbComponentParam
	{
	public:
		AfbComponentParam() = default;
		AfbComponentParam(const AfbComponentParam& that) = default;
		explicit AfbComponentParam(quint16 paramOpIndex) :
			m_paramOpIndex(paramOpIndex)
		{
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
			return m_data.asWord;
		}
		void setWordValue(quint16 value) noexcept
		{
			m_data.data = 0;
			m_data.asWord = value;
		}

		[[nodiscard]] quint32 dwordValue() const  noexcept
		{
			return m_data.asDword;
		}
		void setDwordValue(quint32 value)  noexcept
		{
			m_data.data = 0;
			m_data.asDword = value;
		}

		[[nodiscard]] float floatValue() const  noexcept
		{
			return m_data.asFloat;
		}
		void setFloatValue(float value) noexcept
		{
			m_data.data = 0;
			m_data.asFloat = value;
		}

		[[nodiscard]] double doubleValue() const  noexcept
		{
			return m_data.asDouble;
		}
		void setDoubleValue(double value) noexcept
		{
			m_data.data = 0;
			m_data.asDouble = value;
		}

		[[nodiscard]] qint32 signedIntValue() const noexcept
		{
			return m_data.asSignedInt;
		}
		void setSignedIntValue(qint32 value) noexcept
		{
			m_data.data = 0;
			m_data.asSignedInt = value;
		}

		[[nodiscard]] qint64 signedInt64Value() const noexcept
		{
			return m_data.asSignedInt64;
		}
		void setSignedInt64Value(qint64 value) noexcept
		{
			m_data.data = 0;
			m_data.asSignedInt64 = value;
		}

		// --
		//
		void addSignedInteger(const AfbComponentParam& operand);
		void subSignedInteger(const AfbComponentParam& operand);
		void mulSignedInteger(const AfbComponentParam& operand);
		void divSignedInteger(const AfbComponentParam& operand);

		void addSignedIntegerNumber(qint32 operand);
		void subSignedIntegerNumber(qint32 operand);
		void mulSignedIntegerNumber(qint32 operand);
		void divSignedIntegerNumber(qint32 operand);

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

		void convertSignedIntToFloat();
		void convertWordToFloat();
		void convertWordToSignedInt();

		// --
		//
		void resetMathFlags() noexcept
		{
			m_mathFlags.data = 0;
		}

		[[nodiscard]] quint16 mathOverflow() const noexcept
		{
			return m_mathFlags.overflow ? 0x0001 : 0x0000;
		}
		void setMathOverflow(quint16 value) noexcept
		{
			m_mathFlags.overflow = (value != 0);
		}

		[[nodiscard]] quint16 mathUnderflow() const noexcept
		{
			return m_mathFlags.underflow ? 0x0001 : 0x0000;
		}
		void setMathUnderflow(quint16 value) noexcept
		{
			m_mathFlags.underflow = (value != 0);
		}

		[[nodiscard]] quint16 mathZero() const noexcept
		{
			return m_mathFlags.zero ? 0x0001 : 0x0000;
		}
		void setMathZero(quint16 value) noexcept
		{
			m_mathFlags.zero = (value != 0);
		}

		[[nodiscard]] quint16 mathNan() const noexcept
		{
			return m_mathFlags.nan ? 0x0001 : 0x0000;
		}
		void setMathNan(quint16 value) noexcept
		{
			m_mathFlags.nan = (value != 0);
		}

		[[nodiscard]] quint16 mathDivByZero() const noexcept
		{
			return m_mathFlags.divByZero ? 0x0001 : 0x0000;
		}
		void setMathDivByZero(quint16 value) noexcept
		{
			m_mathFlags.divByZero = (value != 0);
		}

	private:
		union
		{
			quint16 asWord;
			quint32 asDword;
			qint32 asSignedInt;
			quint64 asSignedInt64;
			float asFloat;
			double asDouble;
			quint64 data = 0;
		} m_data;

		// Math operations flags
		//
		union
		{
			struct
			{
				bool overflow : 1;
				bool underflow  : 1;
				bool zero : 1;
				bool nan : 1;
				bool divByZero : 1;
			};
			quint32 data = 0;
		} m_mathFlags;

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
