#pragma once

#include <VFrame30/VduItemVisitor.h>

namespace VFrame30
{
	class SchemaItemVdu
	{
	public:
		virtual ~SchemaItemVdu() = default;

		virtual bool accept(VduItemVisitor& visitor) const = 0;
	};

	template<typename T>
	class SchemaItemVduVisitable : public SchemaItemVdu
	{
	public:
		bool accept(VduItemVisitor& visitor) const override { return visitor.visit(static_cast<const T&>(*this)); }
	};
} // namespace VFrame30
