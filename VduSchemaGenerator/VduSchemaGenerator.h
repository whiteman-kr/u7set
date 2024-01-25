#pragma once

namespace VFrame30
{
	class VduSchema;
	class SchemaItem;
}

namespace vdu
{
	class VduSchemaGenerator
	{
	public:
		VduSchemaGenerator() = delete;

	public:
		static bool generateVduSchema(const VFrame30::VduSchema& schema, QByteArray& out, QStringList& outErrorMessages);

	private:
		static bool saveSchemaItem1(const VFrame30::SchemaItem& schemaItem, QByteArray& out, std::list<std::pair<QString, size_t>>& addedStringReferences);
	};
}