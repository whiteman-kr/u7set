#include "DevToolsSchemaStats.h"

#include <VFrame30/ClientSchemaWidget.h>

namespace Monitor
{
	DevToolsSchemaStats::DevToolsSchemaStats(SchemaClientLib::SchemaDrawStatistics& schemaDrawStatistics,
											 QTabWidget* monitorCentralWidget) :
		m_monitorCentralWidget(monitorCentralWidget),
		m_schemaDrawStatistics(schemaDrawStatistics)
	{
		Q_ASSERT(m_monitorCentralWidget);
	}

	void DevToolsSchemaStats::clear()
	{
		// Not implemented for dev tools.
		//
		Q_ASSERT(false);
		return;
	}

	void DevToolsSchemaStats::clear([[maybe_unused]] const QString& module)
	{
		// Not implemented for dev tools.
		//
		Q_ASSERT(false);
		return;
	}

	void DevToolsSchemaStats::clear([[maybe_unused]] const QString& module, [[maybe_unused]] const QString& item)
	{
		// Not implemented for dev tools.
		//
		Q_ASSERT(false);
		return;
	}

	void DevToolsSchemaStats::addRecord([[maybe_unused]] const QString& module,
										[[maybe_unused]] const QString& item,
										[[maybe_unused]] QString action,
										[[maybe_unused]] std::chrono::microseconds us)
	{
		// Not implemented for dev tools.
		//
		Q_ASSERT(false);
		return;
	}

	std::vector<QString> DevToolsSchemaStats::modules() const
	{
		return m_schemaDrawStatistics.modules();
	}

	std::vector<QString> DevToolsSchemaStats::items(const QString& module) const
	{
		return m_schemaDrawStatistics.items(module);
	}

	std::vector<VFrame30::TimeStatsRecord> DevToolsSchemaStats::itemRecords(const QString& module, const QString& item) const
	{
		return m_schemaDrawStatistics.itemRecords(module, item);
	}

	void DevToolsSchemaStats::highlightItems(QStringList items)
	{
		auto clientWidget = dynamic_cast<VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			clientWidget->clientSchemaView()->setHighlightIds(std::move(items));
		}
	}
} // namespace Monitor