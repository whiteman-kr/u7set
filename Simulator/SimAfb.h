#ifndef COMPONENT_H
#define COMPONENT_H
#include <map>
#include <memory>
#include <QObject>
#include "../VFrame30/Afb.h"

struct lua_State;

namespace Sim
{
	class AfbComponent
	{
	public:
		AfbComponent() = delete;
		AfbComponent(const AfbComponent&) = default;
		AfbComponent(std::shared_ptr<Afb::AfbComponent> afbComponent);
		~AfbComponent() = default;

		static void registerLuaClass(lua_State* L);

	public:
		bool isNull() const;

		int opCode() const;
		std::string caption() const;
		int maxInstCount() const;
		std::string simulationFunc() const;

		bool pinExists(int pinOpIndex) const;
		std::string pinCaption(int pinOpIndex) const;

	private:
		std::shared_ptr<Afb::AfbComponent> m_afbComponent;
	};


	class AfbComponentParam
	{
//		Q_PROPERTY(int OpIndex READ opIndex WRITE setOpIndex)
//		Q_PROPERTY(quint16 AsWord READ wordValue WRITE setWordValue)
//		Q_PROPERTY(quint32 AsDword READ dwordValue WRITE setDwordValue)
//		Q_PROPERTY(float AsFloat READ floatValue WRITE setFloatValue)
//		Q_PROPERTY(double AsDouble READ doubleValue WRITE setDoubleValue)
//		Q_PROPERTY(qint32 AsSignedInt READ signedIntValue WRITE setSignedIntValue)

//		Q_PROPERTY(bool MathOverflow READ mathOverflow)
//		Q_PROPERTY(bool MathUnderflow READ mathUnderflow)
//		Q_PROPERTY(bool MathZero READ mathZero)
//		Q_PROPERTY(bool MathNan READ mathNan)
//		Q_PROPERTY(bool MathDivByZero READ mathDivByZero)

	public:
		AfbComponentParam() = default;
		AfbComponentParam(const AfbComponentParam& that) = default;
		AfbComponentParam(AfbComponentParam&&) = default;
		AfbComponentParam(quint16 paramOpIndex);
		AfbComponentParam& operator=(const AfbComponentParam& that) = default;
		AfbComponentParam& operator=(AfbComponentParam&&) = default;

		static void registerLuaClass(lua_State* L);

	public:
		int opIndex() const;
		void setOpIndex(int index);

		quint16 wordValue() const;
		void setWordValue(quint16 value);

		quint32 dwordValue() const;
		void setDwordValue(quint32 value);

		float floatValue() const;
		void setFloatValue(float value);

		double doubleValue() const;
		void setDoubleValue(double value);

		qint32 signedIntValue() const;
		void setSignedIntValue(qint32 value);

		// --
		//
		void addSignedInteger(AfbComponentParam* operand);
		void subSignedInteger(AfbComponentParam* operand);
		void mulSignedInteger(AfbComponentParam* operand);
		void divSignedInteger(AfbComponentParam* operand);

		void addSignedIntegerNumber(qint32 operand);
		void subSignedIntegerNumber(qint32 operand);
		void mulSignedIntegerNumber(qint32 operand);
		void divSignedIntegerNumber(qint32 operand);

		void addFloatingPoint(AfbComponentParam* operand);
		void subFloatingPoint(AfbComponentParam* operand);
		void mulFloatingPoint(AfbComponentParam* operand);
		void divFloatingPoint(AfbComponentParam* operand);

		void convertWordToFloat();
		void convertWordToSignedInt();

	private:
		void resetMathFlags();
		bool mathOverflow() const;
		bool mathUnderflow() const;
		bool mathZero() const;
		bool mathNan() const;
		bool mathDivByZero() const;

	private:
		quint16 m_paramOpIndex = 0;

		union
		{
			quint16 asWord;
			quint32 asDword;
			qint32 asSignedInt;
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
	};


	// AfbComponentInstance, contains a set of params (InstantiatorParam) for this instance
	//
	class AfbComponentInstance
	{
	public:
		AfbComponentInstance(quint16 instanceNo);

		static void registerLuaClass(lua_State* L);

	public:
		bool addParam(const AfbComponentParam& param);
		AfbComponentParam* param(int opIndex);

		bool paramExists(int opIndex) const;

		bool addParamWord(int opIndex, quint16 value);
		bool addParamFloat(int opIndex, float value);
		bool addParamSignedInt(int opIndex, qint32 value);

	private:
		quint16 m_instanceNo = 0;
		std::map<quint16, AfbComponentParam> m_params;		// Key is AfbComponentParam.opIndex()
	};


	// Model Component with a set of Instances
	//
	class ModelComponent : public Afb::AfbComponent
	{
		Q_GADGET

	public:
		ModelComponent() = delete;
		ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp);

	public:
		Q_INVOKABLE bool addParam(int instanceNo, const AfbComponentParam& instParam, QString* errorMessage);
		Q_INVOKABLE AfbComponentInstance* instance(quint16 instance);

	private:
		std::map<quint16, AfbComponentInstance> m_instances;			// Key is instNo
		std::shared_ptr<const Afb::AfbComponent> m_afbComp;
	};


	class AfbComponentSet
	{
	public:
		AfbComponentSet();

	public:
		void clear();
		bool addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, int instanceNo, const AfbComponentParam& instParam, QString* errorMessage);

		AfbComponentInstance* componentInstance(int componentOpCode, int instance);

	private:
		std::map<quint16, std::shared_ptr<ModelComponent>> m_components;		// Key is component opcode
	};
}


#endif // COMPONENT_H
