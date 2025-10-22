#pragma once
#include "../AppSignalLib/DiscretesLogRecord.h"

#include <memory>
#include <span>
#include <utility>
#include <vector>


namespace ClientLib
{
	class SignalLogPrivate;


	class SignalLog final
	{
	public:
		SignalLog();
		~SignalLog();

		SignalLog(const SignalLog&) = delete;
		SignalLog(SignalLog&&) = delete;
		SignalLog& operator=(const SignalLog&) = delete;
		SignalLog& operator=(SignalLog&&) = delete;

	public:
		void clear();

		bool enabled() const;
		void setEnabled(bool enable);

		/// Send an acknowledgment for all records `up to`, including plantTime.
		///
		bool sendAckUpTo(TimeStamp plantTime);

		std::optional<TimeStamp> getNextAckUpTo();

		/// Add records to the log.
		///
		void add(const QString& adsId, std::span<const DiscretesLogRecord> records);

		/// Delete records which are less than recordId.
		///
		void deleteUpTo(const QString& adsId, qint64 recordId);

		/// Get all records in the log.
		/// Returns a pair:
		///		first: vector of DiscretesLogRecord - all records in the log
		///		second: Update counter, changed every time data added or removed, by it we can track if changes were made and update UI.
		///
		std::pair<std::vector<DiscretesLogRecord>, qint64> getRecords() const;

		/// Update counter, changed every time data added or removed, by it we can track if changes were made and update UI.
		///
		qint64 updateCounter() const;

	private:
		std::unique_ptr<SignalLogPrivate> m_impl;
	};
} // namespace ClientLib