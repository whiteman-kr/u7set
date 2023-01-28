#include "SchemaDrawStatistics.h"
#include <algorithm>
#include <set>

void SchemaDrawStatistics::clear(const QString& module)
{
	m_modules[module].clear();
}

void SchemaDrawStatistics::clear(const QString& module, const QString& item)
{
	m_modules[module].clear();
	m_modules[module].operator[](item).clear();
}

void SchemaDrawStatistics::addRecord(const QString& module, const QString& item, QString action, std::chrono::microseconds us)
{
	m_modules[module].at(item).emplace_back(std::move(action), us);
}

void SchemaDrawStatistics::addRecord(const QString& module, const QString& item, QString action, std::chrono::milliseconds ms)
{
	m_modules[module].at(item).emplace_back(std::move(action), ms);
}

std::vector<QString> SchemaDrawStatistics::modules() const
{
	std::vector<QString> result;
	result.reserve(m_modules.size());

	for (const auto&[module, items] : m_modules)
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

std::vector<TimeStatsRecord> SchemaDrawStatistics::itemRecords(const QString& module, const QString& item) const
{
	std::vector<TimeStatsRecord> result;

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

	const std::vector<TimeStatsRecord>& itemDatata = iit->second;
	result = itemDatata;

	return result;
}
