#pragma once

#include <quuid.h>

#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemBus.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/LogicSchema.h"


namespace Ual
{
	class Item
	{
	public:
		uuid() const { return m_uuid; }

	private:
		QUuid	m_uuid;			// for 'value' items - equal to uuid of AppItem's source pin
								// for other items - equal to uuid of  AppItem
	};

	class Value : public Item
	{
	public:
		enum Type
		{
			Constant,
			Signal,
			Bus
		};

	public:
		virtual Type type() const = 0;
	};

	class Constant : public Value
	{
	public:
		virtual Type type() const { return Type::Constant; }
	};

	class Signal : public Value
	{
	public:
		virtual Type type() const { return Type::Signal; }
	};

	class Bus : public Value
	{
	public:
		virtual Type type() const { return Type::Bus; }
	};

	typedef std::shared_ptr<Value> ValueShared;

	//

	class Link
	{
	public:

	private:
		QUuid m_pinUuid;
		ValueShared m_value = nullptr;
		QString m_valueAlias;
	};

	//

	class Afb : public Item
	{
	private:
	};


}
