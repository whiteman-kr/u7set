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

		Q_PROPERTY(int OpIndex READ opIndex)
		Q_PROPERTY(quint16 AsWord READ wordValue)
		Q_PROPERTY(double AsFloat READ floatValue)
		Q_PROPERTY(qint32 AsSignedInt READ signedIntValue)

		Q_PROPERTY(bool MathOverflow READ mathOverflow)
		Q_PROPERTY(bool MathUnderflow READ mathUnderflow)
		Q_PROPERTY(bool MathZero READ mathZero)
		Q_PROPERTY(bool MathNan READ mathNan)
		Q_PROPERTY(bool MathDivByZero READ mathDivByZero)

	public:
		ComponentParam() = default;
		ComponentParam(const ComponentParam& that);
		ComponentParam(quint16 paramOpIndex, quint32 data);
		ComponentParam& operator=(const ComponentParam& that);

	public:
		int opIndex() const;

		quint16 wordValue() const;
		void setWordValue(quint16 value);

		float floatValue() const;
		void setFloatValue(float value);

		qint32 signedIntValue() const;
		void setSignedIntValue(qint32 value);

	public slots:
		void addSignedInteger(ComponentParam* operand);
		void subSignedInteger(ComponentParam* operand);
		void mulSignedInteger(ComponentParam* operand);
		void divSignedInteger(ComponentParam* operand);

		void addFloatingPoint(ComponentParam* operand);
		void subFloatingPoint(ComponentParam* operand);
		void mulFloatingPoint(ComponentParam* operand);
		void divFloatingPoint(ComponentParam* operand);

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
		quint32 m_data = 0;

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
		Q_INVOKABLE bool addOutputParam(int opIndex, ComponentParam* param);
		Q_INVOKABLE bool addOutputParamWord(int opIndex, quint16 value);
		Q_INVOKABLE bool addOutputParamFloat(int opIndex, float value);
		Q_INVOKABLE bool addOutputParamSignedInt(int opIndex, qint32 value);

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

		ComponentInstance* componentInstance(quint16 componentOpCode, quint16 instance);

	private:
		std::map<quint16, std::shared_ptr<ModelComponent>> m_components;		// Key is component opcode
	};
}


#endif // COMPONENT_H
