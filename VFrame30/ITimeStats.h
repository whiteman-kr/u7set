#pragma once
#include <QString>
#include <chrono>
#include <vector>

namespace VFrame30
{
	struct TimeStatsRecord
	{
		QString action;
		std::chrono::microseconds time;
	};

	class ITimeStats
	{
	public:
		virtual ~ITimeStats() = default;

		virtual void clear() = 0;
		virtual void clear(const QString& module) = 0;
		virtual void clear(const QString& module, const QString& item) = 0;
		virtual void addRecord(const QString& module, const QString& item, QString action, std::chrono::microseconds us) = 0;

		[[nodiscard]] virtual std::vector<QString> modules() const = 0;
		[[nodiscard]] virtual std::vector<QString> items(const QString& module) const = 0;
		[[nodiscard]] virtual std::vector<TimeStatsRecord> itemRecords(const QString& module, const QString& item) const = 0;
	};
} // namespace VFrame30
