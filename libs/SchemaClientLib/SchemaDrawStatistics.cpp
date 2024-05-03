#include <SchemaClientLib/SchemaDrawStatistics.h>

namespace SchemaClientLib
{
	void SchemaDrawStatistics::clear()
	{
		m_modules.clear();
	}

	void SchemaDrawStatistics::clear(const QString& module)
	{
		m_modules[module].clear();
	}

	void SchemaDrawStatistics::clear(const QString& module, const QString& item)
	{
		ItemsMap& itemMap = m_modules[module];
		itemMap[item].clear();
	}

	void SchemaDrawStatistics::addRecord(const QString& module, const QString& item, QString action, std::chrono::microseconds us)
	{
		ItemsMap& itemMap = m_modules[module];
		itemMap[item].emplace_back(std::move(action), us);
	}

	std::vector<QString> SchemaDrawStatistics::modules() const
	{
		std::vector<QString> result;
		result.reserve(m_modules.size());

		for (const auto& [module, items] : m_modules)
		{
			Q_UNUSED(items);
			result.push_back(module);
		}

		return result;
	}

	std::vector<QString> SchemaDrawStatistics::items(const QString& module) const
	{
		std::set<QString> set;

		auto mit = m_modules.find(module);
		if (mit != m_modules.end())
		{
			const auto& items = mit->second;

			for (const auto& [item, record] : items)
			{
				Q_UNUSED(record);
				set.insert(item);
			}
		}

		std::vector<QString> result;
		result.reserve(set.size());
		result.assign(set.begin(), set.end());

		return result;
	}

	std::vector<VFrame30::TimeStatsRecord> SchemaDrawStatistics::itemRecords(const QString& module, const QString& item) const
	{
		std::vector<VFrame30::TimeStatsRecord> result;

		auto mit = m_modules.find(module);
		if (mit == m_modules.end())
		{
			return result;
		}

		const auto& items = mit->second;
		auto iit = items.find(item);
		if (iit == items.end())
		{
			return result;
		}

		const std::vector<VFrame30::TimeStatsRecord>& itemDatata = iit->second;
		result = itemDatata;

		return result;
	}
} // namespace SchemaClientLib