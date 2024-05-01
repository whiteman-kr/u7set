#pragma once

namespace VFrame30
{
	class Schema;
	class VduSchema;
	class SchemaItem;
} // namespace VFrame30

namespace Builder
{
	class IssueLogger;

	class VduSchemaGenerator
	{
	public:
		VduSchemaGenerator() = delete;

	public:
		static bool generateVduSchema(QString vduEquipmentId,
									  const VFrame30::VduSchema& schema,
									  const std::map<Hash, int>& appSignalHashToSignalIndex,
									  QByteArray& out,
									  IssueLogger& log);

		static bool generateVduBackgroundBitmap(std::shared_ptr<VFrame30::Schema> schema, QImage& out);

	private:
		static bool saveSchemaItem1(QString vduEquipmentId,
									const VFrame30::SchemaItem& schemaItem,
									const std::map<Hash, int>& appSignalHashToSignalIndex,
									QByteArray& out,
									std::list<std::pair<QString, size_t>>& addedStringReferences,
									IssueLogger& log);
	};
} // namespace Builder