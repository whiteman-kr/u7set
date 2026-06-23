#pragma once

struct TrendItemSignal;

namespace VFrame30
{
	class Schema;
	class VduSchema;
	class SchemaItem;
} // namespace VFrame30

namespace Builder
{
	class Context;
	class IssueLogger;

	class VduSchemaGenerator
	{
	public:
		VduSchemaGenerator() = delete;

	public:
		static bool generateVduSchemas(const std::vector<VFrame30::VduSchema*>& schemas, Builder::Context& context);

		static bool generateVduSchema(QString vduEquipmentId,
									  QString subsystemId,
									  const VFrame30::VduSchema& schema,
									  const std::map<Hash, int>& appSignalHashToSignalIndex,
									  const std::set<TrendItemSignal>& vduTrendSignals,
									  QByteArray& out,
									  Builder::Context& context);

		static bool generateVduBackgroundBitmap(std::shared_ptr<VFrame30::Schema> schema, QImage& out);
	};
} // namespace Builder