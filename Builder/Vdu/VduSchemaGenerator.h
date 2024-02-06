#pragma once

namespace VFrame30
{
	class Schema;
	class VduSchema;
	class SchemaItem;
} // namespace VFrame30

namespace Builder
{
	class VduSchemaGenerator
	{
	public:
		VduSchemaGenerator() = delete;

	public:
		static bool generateVduSchema(const VFrame30::VduSchema& schema, QByteArray& out, QStringList& outErrorMessages);
		static bool generateVduBackgroundBitmap(std::shared_ptr<VFrame30::Schema> schema, QImage& out);

	private:
		static bool saveSchemaItem1(const VFrame30::SchemaItem& schemaItem, QByteArray& out, std::list<std::pair<QString, size_t>>& addedStringReferences);
	};
} // namespace vdu