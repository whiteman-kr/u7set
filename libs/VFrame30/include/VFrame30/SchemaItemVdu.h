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
} // namespace VFrame30
