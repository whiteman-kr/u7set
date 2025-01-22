#pragma once

namespace VFrame30
{
	class SchemaItemVduLine;
	class SchemaItemVduRect;
	class SchemaItemVduImage;
	class SchemaItemVduValue;

	class VduItemVisitor
	{
	public:
		virtual ~VduItemVisitor() = default;

		virtual bool visit(const SchemaItemVduLine& item) = 0;
		virtual bool visit(const SchemaItemVduRect& item) = 0;
		virtual bool visit(const SchemaItemVduImage& item) = 0;
		virtual bool visit(const SchemaItemVduValue& item) = 0;
	};
} // namespace VFrame30