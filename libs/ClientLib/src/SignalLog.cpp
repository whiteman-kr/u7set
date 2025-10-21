#include "../include/ClientLib/SignalLog.h"

#include <atomic>
#include <deque>


namespace
{
	constexpr size_t SignalLogMaxSize = 9'999; // Maximum number of records in the log. If exceeded, old records will be removed.
}


namespace ClientLib
{
	class SignalLogPrivate
	{
	public:
		void clear()
		{
			{
				std::lock_guard lock{m_mutex};

				m_recordsByAds.clear();
				m_cachedPlainData.clear();
				m_cacheUpdateCounter++;
			}

			{
				std::lock_guard lock{m_ackMutex};
				m_ackQueue = {};
			}

			return;
		}

		bool enabled() const { return m_enabled.load(); }
		void setEnabled(bool enable)
		{
			m_enabled.store(enable);

			if (enable == false)
			{
				clear();
			}

			return;
		}

		bool sendAckUpTo(TimeStamp plantTime)
		{
			std::lock_guard lock{m_ackMutex};

			if (enabled() == false)
			{
				m_ackQueue = {};
				return false;
			}

			m_ackQueue.push(plantTime);
			return true;
		}

		std::optional<TimeStamp> getNextAckUpTo()
		{
			std::lock_guard lock{m_ackMutex};
			if (m_ackQueue.empty() == true)
			{
				return std::nullopt;
			}

			TimeStamp plantTime = m_ackQueue.front();
			m_ackQueue.pop();

			return plantTime;
		}

		// Add records to the log.
		//
		void add(const QString& adsId, std::span<const DiscretesLogRecord> records)
		{
			std::lock_guard lock{m_mutex};

			auto& recordsByAds = m_recordsByAds[adsId];
			std::copy(records.begin(), records.end(), std::back_inserter(recordsByAds));

			if (records.empty() == false)
			{
				m_cachedPlainData.clear();
				m_cacheUpdateCounter++;
			}

			// If the number of records exceeds the maximum size, remove the oldest records.
			//
			while (recordsByAds.size() > SignalLogMaxSize)
			{
				recordsByAds.pop_front();
			}

			return;
		}

		void deleteUpTo(const QString& adsId, qint64 recordId)
		{
			// Delete records which are less than recordId.
			//
			std::lock_guard lock{m_mutex};
			size_t deletedCount = 0;

			auto& records = m_recordsByAds[adsId];

			while (records.empty() == false && records.front().recordID < recordId)
			{
				deletedCount++;
				records.pop_front();
			}

			if (deletedCount != 0)
			{
				m_cachedPlainData.clear();
				m_cacheUpdateCounter++;
			}

			return;
		}

		std::pair<std::vector<DiscretesLogRecord>, qint64> getRecords() const
		{
			std::lock_guard lock{m_mutex};

			if (m_cachedPlainData.empty() == false)
			{
				// If cached data is not empty, return it.
				//
				return {m_cachedPlainData, m_cacheUpdateCounter};
			}

			for (const auto& [adsId, records] : m_recordsByAds)
			{
				m_cachedPlainData.insert(m_cachedPlainData.end(), records.begin(), records.end());
			}

			std::stable_sort(m_cachedPlainData.begin(),
							 m_cachedPlainData.end(),
							 [](const DiscretesLogRecord& a, const DiscretesLogRecord& b)
							 {
								 std::tuple av = std::make_tuple(a.plantTime, a.signalHash);
								 std::tuple bv = std::make_tuple(b.plantTime, b.signalHash);
								 return av < bv;
							 });

			auto newEndIt = std::unique(m_cachedPlainData.begin(),
										m_cachedPlainData.end(),
										[](const DiscretesLogRecord& a, const DiscretesLogRecord& b)
										{
											std::tuple av = std::make_tuple(a.plantTime, a.signalHash, a.value, a.flags);
											std::tuple bv = std::make_tuple(b.plantTime, b.signalHash, b.value, b.flags);
											return av == bv;
										});

			m_cachedPlainData.erase(newEndIt, m_cachedPlainData.end());

			// Shrink to last 99999 records.
			//
			if (m_cachedPlainData.size() > SignalLogMaxSize)
			{
				m_cachedPlainData.erase(m_cachedPlainData.begin(), m_cachedPlainData.end() - SignalLogMaxSize);
			}

			return {m_cachedPlainData, m_cacheUpdateCounter};
		}

		qint64 updateCounter() const
		{
			std::lock_guard lock{m_mutex};
			return m_cacheUpdateCounter;                                  // Return the last cached time
		}

	private:
		std::atomic<bool> m_enabled{false};                               // Log enabled state

		mutable std::mutex m_mutex;
		std::map<QString, std::deque<DiscretesLogRecord>> m_recordsByAds; // map of records by AppDataService equipmentId

		mutable std::vector<DiscretesLogRecord> m_cachedPlainData;
		mutable qint64 m_cacheUpdateCounter{0};

		// Acknowledge
		//
		mutable std::mutex m_ackMutex;
		std::queue<TimeStamp> m_ackQueue; // Queue of plantTimes to acknowledge
	};

	//
	// SignalLog
	//
	SignalLog::SignalLog() :
		m_impl{std::make_unique<SignalLogPrivate>()}
	{
	}

	SignalLog::~SignalLog() = default;

	void SignalLog::clear()
	{
		return m_impl->clear();
	}

	bool SignalLog::enabled() const
	{
		return m_impl->enabled();
	}

	void SignalLog::setEnabled(bool enable)
	{
		m_impl->setEnabled(enable);
		return;
	}

	bool SignalLog::sendAckUpTo(TimeStamp plantTime)
	{
		return m_impl->sendAckUpTo(plantTime);
	}

	std::optional<TimeStamp> SignalLog::getNextAckUpTo()
	{
		return m_impl->getNextAckUpTo();
	}

	void SignalLog::add(const QString& adsId, std::span<const DiscretesLogRecord> records)
	{
		m_impl->add(adsId, records);
		return;
	}

	void SignalLog::deleteUpTo(const QString& adsId, qint64 recordId)
	{
		return m_impl->deleteUpTo(adsId, recordId);
	}

	std::pair<std::vector<DiscretesLogRecord>, qint64> SignalLog::getRecords() const
	{
		return m_impl->getRecords();
	}

	qint64 SignalLog::updateCounter() const
	{
		return m_impl->updateCounter();
	}
} // namespace ClientLib