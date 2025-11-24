#pragma once
#include "ISignalLogUpdater.h"

#include "../AppSignalLib/DiscretesLogRecord.h"

#include <memory>
#include <span>
#include <utility>
#include <vector>


namespace ClientLib
{
	class SignalLogPrivate;


	class SignalLog final : public ISignalLogUpdater
	{
	public:
		SignalLog();
		~SignalLog();

		SignalLog(const SignalLog&) = delete;
		SignalLog(SignalLog&&) = delete;
		SignalLog& operator=(const SignalLog&) = delete;
		SignalLog& operator=(SignalLog&&) = delete;

	public:
		// -- ISignalLogUpdater
		//
		virtual void clear() override;

		virtual bool enabled() const override;
		void setEnabled(bool enable);

		/// Add records to the log.
		///
		virtual void add(const std::string& adsId, ISignalLogUpdater::RecordIterator begin, ISignalLogUpdater::RecordIterator end) override;

		/// Delete records which are less than recordId.
		///
		virtual void deleteUpTo(const std::string& adsId, int64_t recordId) override;

		virtual std::optional<ISignalLogUpdater::TimeStampType> getNextAckUpTo() override;

		// -- End of ISignalLogUpdater

		/// Send an acknowledgment for all records `up to`, including plantTime.
		///
		bool sendAckUpTo(TimeStamp plantTime);

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