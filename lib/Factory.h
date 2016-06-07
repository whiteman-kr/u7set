#pragma once

#include <QtGlobal>
#include <map>
#include <memory>
#include "../include/CUtils.h"

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

	BaseClass* Create(quint32 classHash)
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
		virtual BaseClass* Create() const = 0;
	};
	
	template<typename DerivedClass>
	class DerivedType : public BaseType
	{
	public:
		virtual BaseClass* Create() const
		{
			return new DerivedClass();
		}
	};

private:
	std::map<quint32, std::shared_ptr<BaseType>> factories;
};


