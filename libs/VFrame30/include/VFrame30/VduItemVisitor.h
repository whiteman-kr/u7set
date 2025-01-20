#pragma once

namespace VFrame30
{
	class SchemaItemVduLine;
	class SchemaItemVduRect;
	class SchemaItemVduValue;

	class VduItemVisitor
	{
	public:
		virtual ~VduItemVisitor() = default;

		virtual void visit(const SchemaItemVduLine& item) = 0;
		virtual void visit(const SchemaItemVduRect& item) = 0;
		virtual void visit(const SchemaItemVduValue& item) = 0;
	};
} // namespace VFrame30