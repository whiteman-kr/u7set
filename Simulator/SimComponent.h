#ifndef COMPONENT_H
#define COMPONENT_H
#include <map>
#include <memory>
#include <QObject>
#include "../VFrame30/Afb.h"

namespace Sim
{

	class ComponentParam : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(int OpIndex READ opIndex WRITE setOpIndex)
		Q_PROPERTY(quint16 AsWord READ wordValue WRITE setWordValue)
		Q_PROPERTY(quint32 AsDword READ dwordValue WRITE setDwordValue)
		Q_PROPERTY(float AsFloat READ floatValue WRITE setFloatValue)
		Q_PROPERTY(double AsDouble READ doubleValue WRITE setDoubleValue)
		Q_PROPERTY(qint32 AsSignedInt READ signedIntValue WRITE setSignedIntValue)

		Q_PROPERTY(bool MathOverflow READ mathOverflow)
		Q_PROPERTY(bool MathUnderflow READ mathUnderflow)
		Q_PROPERTY(bool MathZero READ mathZero)
		Q_PROPERTY(bool MathNan READ mathNan)
		Q_PROPERTY(bool MathDivByZero READ mathDivByZero)

	public:
		ComponentParam() = default;
		ComponentParam(const ComponentParam& that);
		ComponentParam(quint16 paramOpIndex);
		ComponentParam& operator=(const ComponentParam& that);

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

	public slots:
		void addSignedInteger(ComponentParam* operand);
		void subSignedInteger(ComponentParam* operand);
		void mulSignedInteger(ComponentParam* operand);
		void divSignedInteger(ComponentParam* operand);

		void addSignedIntegerNumber(qint32 operand);
		void subSignedIntegerNumber(qint32 operand);
		void mulSignedIntegerNumber(qint32 operand);
		void divSignedIntegerNumber(qint32 operand);

		void addFloatingPoint(ComponentParam* operand);
		void subFloatingPoint(ComponentParam* operand);
		void mulFloatingPoint(ComponentParam* operand);
		void divFloatingPoint(ComponentParam* operand);

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
		// Warning, class has operator =
		//
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
		// Warning, class has operator =
		//
	};


	// AfbComponentInstance, contains a set of params (InstantiatorParam) for this instance
	//
	class ComponentInstance : public QObject
	{
		Q_OBJECT

	public:
		ComponentInstance(quint16 instanceNo);

	public:
		bool addParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const ComponentParam& param, QString* errorMessage);
		const ComponentParam* param(int opIndex) const;

	public:	// For access from JavaScript
		Q_INVOKABLE bool paramExists(int opIndex) const;
		Q_INVOKABLE QObject* param(int opIndex);
		Q_INVOKABLE bool addParam(int opIndex, ComponentParam* param);
		Q_INVOKABLE bool addParamWord(int opIndex, quint16 value);
		Q_INVOKABLE bool addParamFloat(int opIndex, float value);
		Q_INVOKABLE bool addParamSignedInt(int opIndex, qint32 value);

	private:
		quint16 m_instanceNo = 0;
		std::map<quint16, ComponentParam> m_params;		// Key is ComponentParam.opIndex()
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
		Q_INVOKABLE bool addParam(int instanceNo, const ComponentParam& instParam, QString* errorMessage);
		Q_INVOKABLE ComponentInstance* instance(quint16 instance);

	private:
		std::map<quint16, ComponentInstance> m_instances;			// Key is instNo
		std::shared_ptr<const Afb::AfbComponent> m_afbComp;
	};


	class AfbComponentSet
	{
	public:
		AfbComponentSet();

	public:
		void clear();
		bool addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, int instanceNo, const ComponentParam& instParam, QString* errorMessage);

		ComponentInstance* componentInstance(int componentOpCode, int instance);

	private:
		std::map<quint16, std::shared_ptr<ModelComponent>> m_components;		// Key is component opcode
	};
}


#endif // COMPONENT_H
