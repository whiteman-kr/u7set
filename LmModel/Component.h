#ifndef COMPONENT_H
#define COMPONENT_H
#include <map>
#include <memory>
#include <QObject>
#include "../VFrame30/Afb.h"

namespace LmModel
{

	struct InstantiatorParam
	{
		InstantiatorParam() = default;
		InstantiatorParam(const InstantiatorParam&) = default;
		InstantiatorParam(quint16 instNo, quint16 instParamOpIndex, quint32 data);

		quint16 instNo = 0;
		quint16 instParamOpIndex = 0;

		quint32 data = 0;
	};

	// AfbComponentInstance, contains a set of params (InstantiatorParam) for this instance
	//
	class ComponentInst
	{
	public:
		ComponentInst();

	public:
		bool addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const InstantiatorParam& instParam, QString* errorMessage);

	private:
		std::map<quint16, InstantiatorParam> m_params;		// Key is InstantiatorParam.instParamOpIndex
	};

	// Model Component with a set of Instances
	//
	class ModelComponent : public Afb::AfbComponent
	{
	public:
		ModelComponent() = delete;
		ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp);

	public:
		bool addInstantiatorParam(const InstantiatorParam& instParam, QString* errorMessage);

	private:
		std::map<quint16, ComponentInst> m_instances;			// Key is instNo
		std::shared_ptr<const Afb::AfbComponent> m_afbComp;
	};


	class AfbComponentSet
	{
	public:
		AfbComponentSet();

	public:
		void clear();
		bool addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const InstantiatorParam& instParam, QString* errorMessage);

	private:
		std::map<quint16, std::shared_ptr<ModelComponent>> m_components;		// Key is component opcode
	};
}


#endif // COMPONENT_H
