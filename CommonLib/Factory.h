#pragma once

#include <QtGlobal>
#include <unordered_map>
#include <memory>
#include "../CommonLib/Hash.h"

template<typename BaseClass>
class Factory
{
public:
	void clear()
	{
		factories.clear();
	}

	template<typename DerivedClass>
	void Register(const std::string& className)
	{
		quint32 classHash = ::ClassNameHashCode(className);

		Q_ASSERT(factories.find(classHash) == std::end(factories));
		factories[classHash] = std::make_shared<DerivedType<DerivedClass>>();		// new DerivedType<DerivedClass>();

		return;
	}

	template<typename DerivedClass>
	void Register()
	{
		quint32 classHash = ::ClassNameHashCode(DerivedClass::staticMetaObject.className());

		Q_ASSERT(factories.find(classHash) == std::end(factories));
		factories[classHash] = std::make_shared<DerivedType<DerivedClass>>();		// new DerivedType<DerivedClass>();

		return;
	}

	template<typename DerivedClass>
	void isRegistered()
	{
		return isRegistered(::ClassNameHashCode(DerivedClass::staticMetaObject.className()));
	}

	[[nodiscard]] bool isRegistered(quint32 classHash) const
	{
		return factories.find(classHash) != factories.end();
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


