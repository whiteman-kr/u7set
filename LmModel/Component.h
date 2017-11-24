#ifndef COMPONENT_H
#define COMPONENT_H
#include <map>
#include <memory>
#include <QObject>
#include "../VFrame30/Afb.h"

namespace LmModel
{

	class ComponentParam
	{
		Q_GADGET

		Q_PROPERTY(int OpIndex READ opIndex)
		Q_PROPERTY(quint16 AsWord READ wordValue)
		Q_PROPERTY(double AsFloat READ floatValue)
		Q_PROPERTY(qint32 AsSignedInt READ signedIntValue)

	public:
		ComponentParam() = default;
		ComponentParam(const ComponentParam&) = default;
		ComponentParam(quint16 instNo, quint16 paramOpIndex, quint32 data);

	public:
		int instanceNo() const;
		int opIndex() const;
		quint16 wordValue() const;
		double floatValue() const;
		qint32 signedIntValue() const;

	private:
		quint16 m_instNo = 0;
		quint16 m_paramOpIndex = 0;
		quint32 m_data = 0;
	};


	// AfbComponentInstance, contains a set of params (InstantiatorParam) for this instance
	//
	class ComponentInstance : public QObject
	{
		Q_OBJECT

	public:
		ComponentInstance();

	public:
		bool addInputParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const ComponentParam& instParam, QString* errorMessage);

	public:	// For access from JavaScript
		Q_INVOKABLE bool paramExists(int opIndex) const;
		Q_INVOKABLE ComponentParam param(int opIndex) const;

	private:
		std::map<quint16, ComponentParam> m_params;		// Key is ComponentParam.opIndex()
	};


	// Model Component with a set of Instances
	//
	class ModelComponent : public Afb::AfbComponent
	{
	public:
		ModelComponent() = delete;
		ModelComponent(std::shared_ptr<const Afb::AfbComponent> afbComp);

	public:
		bool addParam(const ComponentParam& instParam, QString* errorMessage);
		ComponentInstance* instance(quint16 instance);

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
		bool addInstantiatorParam(std::shared_ptr<const Afb::AfbComponent> afbComp, const ComponentParam& instParam, QString* errorMessage);

		ComponentInstance* componentInstance(quint16 componentOpCode, quint16 instance);

	private:
		std::map<quint16, std::shared_ptr<ModelComponent>> m_components;		// Key is component opcode
	};
}


#endif // COMPONENT_H
