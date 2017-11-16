#ifndef COMPONENT_H
#define COMPONENT_H
#include <map>
#include <memory>
#include <QObject>

namespace LmModel
{

	struct InstantiatorParam
	{
		InstantiatorParam(quint16 implNo, quint16 implParamOpIndex, quint16 dataLow, quint16 dataHigh);

		quint16 implNo = 0;
		quint16 implParamOpIndex = 0;

		quint16 dataLow = 0;
		quint16 dataHigh = 0;
	};

	struct InstantiatorComponent
	{
		quint16 opCode = 0;
		std::map<quint16, InstantiatorParam> params;			// Key InstantiatorParam::implParamOpIndex
	};

	class InstantiatorSet
	{
	public:
		InstantiatorSet();

	public:
		clear();
		bool addInstantiatorParam(quint16 compOpCode, const InstantiatorParam& param, QString* errorMessage);

	private:
		std::map<quint16, InstantiatorComponent> m_components;		// Key InstantiatorComponent::opCode
	};

	class ComponentImpl
	{

	};

	class Component
	{
	public:
		Component();

	private:
		std::map<quint16, ComponentImpl> m_implements;	// Key is ImplementationNo
	};

	class ComponentSet
	{
	public:
		ComponentSet();

	private:
		std::map<quint16, std::shared_ptr<Component>> m_components;		// Key is component opcode
	};
}


#endif // COMPONENT_H
