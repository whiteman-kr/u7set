#pragma once

#include <QtGlobal>
#include <map>
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
		factories[classHash] = std::make_shared<DerivedType<DerivedClass>>();		// new DerivedType<DerivedClass>();
	}

	template<typename DerivedClass>
	void Register()
	{
		quint32 classHash = CUtils::GetClassHashCode(DerivedClass::staticMetaObject.className());
		factories[classHash] = std::make_shared<DerivedType<DerivedClass>>();		// new DerivedType<DerivedClass>();
	}

	std::shared_ptr<BaseClass> Create(quint32 classHash)
	{
		auto it = factories.find(classHash);
		if (it == factories.end())
		{
			assert(false);
			return nullptr;
		}
		else
		{
			return it->second->Create();
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
		virtual std::shared_ptr<BaseClass> Create() const
		{
			std::shared_ptr<BaseClass> ptr = std::make_shared<DerivedClass>();
			return ptr;
		}
	};

private:
	std::map<quint32, std::shared_ptr<BaseType>> factories;
};


