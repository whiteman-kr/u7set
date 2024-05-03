#pragma once

#include <SchemaClientLib/IDevTools.h>
#include <SchemaClientLib/SchemaDrawStatistics.h>

namespace Monitor
{
	class DevToolsSchemaStats : public SchemaClientLib::IDevToolsSchemaStats
	{
	public:
		explicit DevToolsSchemaStats(SchemaClientLib::SchemaDrawStatistics& schemaDrawStatistics, QTabWidget* monitorCentralWidget);

		virtual void clear() override;
		virtual void clear(const QString& module) override;
		virtual void clear(const QString& module, const QString& item) override;
		virtual void addRecord(const QString& module, const QString& item, QString action, std::chrono::microseconds us) override;

		[[nodiscard]] virtual std::vector<QString> modules() const override;
		[[nodiscard]] virtual std::vector<QString> items(const QString& module) const override;
		[[nodiscard]] virtual std::vector<VFrame30::TimeStatsRecord> itemRecords(const QString& module, const QString& item) const override;

		virtual void highlightItems(QStringList items) override;

	private:
		SchemaClientLib::SchemaDrawStatistics& m_schemaDrawStatistics;
		QTabWidget* m_monitorCentralWidget = nullptr;
	};
} // namespace Monitor