#ifndef COMPONENT_H
#define COMPONENT_H
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
		bool isNull() const;

		int opCode() const;
		QString caption() const;
		int maxInstCount() const;
		QString simulationFunc() const;

		bool pinExists(int pinOpIndex) const;
		QString pinCaption(int pinOpIndex) const;

	private:
		std::shared_ptr<Afb::AfbComponent> m_afbComponent;
	};


	class AfbComponentParam
	{
	public:
		AfbComponentParam() = default;
		AfbComponentParam(const AfbComponentParam& that) = default;
		AfbComponentParam(AfbComponentParam&&) = default;
		explicit AfbComponentParam(quint16 paramOpIndex) :
			m_paramOpIndex(paramOpIndex)
		{
		}

		explicit AfbComponentParam(quint16 paramOpIndex, quint16 word) :
			m_paramOpIndex(paramOpIndex)
		{
			setWordValue(word);
		}

		AfbComponentParam& operator=(const AfbComponentParam& that) = default;
		AfbComponentParam& operator=(AfbComponentParam&&) = default;

	public:
		int opIndex() const noexcept
		{
			return m_paramOpIndex;
		}
		void setOpIndex(int index) noexcept
		{
			m_paramOpIndex = static_cast<quint16>(index);
		}

		quint16 wordValue() const noexcept
		{
			return m_data.asWord;
		}
		void setWordValue(quint16 value) noexcept
		{
			m_data.data = 0;
			m_data.asWord = value;
		}

		quint32 dwordValue() const  noexcept
		{
			return m_data.asDword;
		}
		void setDwordValue(quint32 value)  noexcept
		{
			m_data.data = 0;
			m_data.asDword = value;
		}

		float floatValue() const  noexcept
		{
			return m_data.asFloat;
		}
		void setFloatValue(float value) noexcept
		{
			m_data.data = 0;
			m_data.asFloat = value;
		}

		double doubleValue() const  noexcept
		{
			return m_data.asDouble;
		}
		void setDoubleValue(double value) noexcept
		{
			m_data.data = 0;
			m_data.asDouble = value;
		}

		qint32 signedIntValue() const noexcept
		{
			return m_data.asSignedInt;
		}
		void setSignedIntValue(qint32 value) noexcept
		{
			m_data.data = 0;
			m_data.asSignedInt = value;
		}

		qint64 signedInt64Value() const noexcept
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

		quint16 mathOverflow() const noexcept
		{
			return m_mathFlags.overflow ? 0x0001 : 0x0000;
		}
		void setMathOverflow(quint16 value) noexcept
		{
			m_mathFlags.overflow = value ? 0x0001 : 0x0000;
		}

		quint16 mathUnderflow() const noexcept
		{
			return m_mathFlags.underflow ? 0x0001 : 0x0000;
		}
		void setMathUnderflow(quint16 value) noexcept
		{
			m_mathFlags.underflow = value ? 0x0001 : 0x0000;
		}

		quint16 mathZero() const noexcept
		{
			return m_mathFlags.zero ? 0x0001 : 0x0000;
		}
		void setMathZero(quint16 value) noexcept
		{
			m_mathFlags.zero = value ? 0x0001 : 0x0000;
		}

		quint16 mathNan() const noexcept
		{
			return m_mathFlags.nan ? 0x0001 : 0x0000;
		}
		void setMathNan(quint16 value) noexcept
		{
			m_mathFlags.nan = value ? 0x0001 : 0x0000;
		}

		quint16 mathDivByZero() const noexcept
		{
			return m_mathFlags.divByZero ? 0x0001 : 0x0000;
		}
		void setMathDivByZero(quint16 value) noexcept
		{
			m_mathFlags.divByZero = value ? 0x0001 : 0x0000;
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
		AfbComponentInstance(const std::shared_ptr<const Afb::AfbComponent>& afbComp,
							 quint16 instanceNo);

	public:
		bool addParam(const AfbComponentParam& param) noexcept;
		bool addParam(AfbComponentParam&& param) noexcept;

		const AfbComponentParam* param(quint16 opIndex) noexcept;

		bool paramExists(quint16 opIndex) const noexcept;

		bool addParamWord(quint16 opIndex, quint16 value) noexcept;
		bool addParamDword(quint16 opIndex, quint32 value) noexcept;
		bool addParamFloat(quint16 opIndex, float value) noexcept;
		bool addParamDouble(quint16 opIndex, double value) noexcept;
		bool addParamSignedInt(quint16 opIndex, qint32 value) noexcept;
		bool addParamSignedInt64(quint16 opIndex, qint64 value) noexcept;

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
		bool isNull() const;

		bool addParam(int instanceNo, const AfbComponentParam& instParam, QString* errorMessage) noexcept;
		bool addParam(int instanceNo, AfbComponentParam&& instParam, QString* errorMessage) noexcept;

		AfbComponentInstance* instance(quint16 instance) noexcept
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
		bool init(const LmDescription& lmDescription);
		bool addInstantiatorParam(int afbOpCode, int instanceNo, const AfbComponentParam& instParam, QString* errorMessage) noexcept;
		bool addInstantiatorParam(int afbOpCode, int instanceNo, AfbComponentParam&& instParam, QString* errorMessage) noexcept;

		AfbComponentInstance* componentInstance(int componentOpCode, int instance) noexcept;

	private:
		std::vector<ModelComponent> m_components;		// Index is opcode of AFB
	};
}


#endif // COMPONENT_H
