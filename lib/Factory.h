#pragma once

#include <QtGlobal>
#include <unordered_map>
#include <memory>
#include "../lib/CUtils.h"

template<typename BaseClass>
class Factory
{
public:
	template<typename DerivedClass>
	void Register(const std::string& className)
	{
		quint32 classHash = CUtils::GetClassHashCode(className);

		Q_ASSERT(factories.find(classHash) == std::end(factories));
		factories[classHash] = std::make_shared<DerivedType<DerivedClass>>();		// new DerivedType<DerivedClass>();

		return;
	}

	template<typename DerivedClass>
	void Register()
	{
		quint32 classHash = CUtils::GetClassHashCode(DerivedClass::staticMetaObject.className());

		Q_ASSERT(factories.find(classHash) == std::end(factories));
		factories[classHash] = std::make_shared<DerivedType<DerivedClass>>();		// new DerivedType<DerivedClass>();

		return;
	}

	std::shared_ptr<BaseClass> Create(quint32 classHash)
	{
		auto it = factories.find(classHash);
		if (it != factories.end())
		{
			return it->second->Create();
		}
		else
		{
			// Crash? Forger to register class? VFrame30Library.cpp?
			//
			Q_ASSERT(it == factories.end());
			return {};
		}
	}

	class BaseType
	{
	public:
		virtual ~BaseType()
		{
		}
		virtual std::shared_ptr<BaseClass> Create() const = 0;
	};
	
	template<typename DerivedClass>
	class DerivedType : public BaseType
	{
	public:
		virtual std::shared_ptr<BaseClass> Create() const override
		{
			return std::make_shared<DerivedClass>();
		}
	};

private:
	std::unordered_map<quint32, std::shared_ptr<BaseType>> factories;
};


