#pragma once
#include "../lib/ITimeStats.h"
#include <map>

namespace SchemaClientLib
{
	class SchemaDrawStatistics : public ITimeStats
	{
	public:
		// ITimeStats implementation.
		//
		virtual void clear(const QString& module) override;
		virtual void clear(const QString& module, const QString& item) override;
		virtual void addRecord(const QString& module, const QString& item, QString action, std::chrono::microseconds us) override;

		[[nodiscard]] virtual std::vector<QString> modules() const override;
		[[nodiscard]] virtual std::vector<QString> items(const QString& module) const override;
		[[nodiscard]] virtual std::vector<TimeStatsRecord> itemRecords(const QString& module, const QString& item) const override;

	private:
		using ItemsMap = std::unordered_map<QString, std::vector<TimeStatsRecord>>; // Key is item, value is time statistics

		std::map<QString, ItemsMap> m_modules;                                      // Key is module, value is item
	};
} // namespace SchemaClientLib