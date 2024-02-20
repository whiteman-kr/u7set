#pragma once
#include <map>
#include <vector>

#include <QElapsedTimer>

namespace AppSignalLib
{
	class RecentUsed
	{
	public:
		explicit RecentUsed(size_t maxSize = 500);

	public:
		void add(Hash h);
		void add(const std::vector<Hash>& hashes);

		bool remove(Hash hash);
		bool remove(const std::vector<Hash>& hashes);

		bool removeOutdated();

		[[nodiscard]] std::vector<Hash> hashes() const;

	private:
		const size_t m_maxSize{};
		std::map<Hash, qint64> m_signalToTime;				// key - signal hash, value - time of last update.
		std::multimap<qint64, Hash> m_timeToSignal;			// key - time of last update, value - signal hash.

		mutable QElapsedTimer m_lastTimeDataFetched;		// If data not fetched regulary, then ignore any add(...).

		static const int ExpiredTimeMs = 3000;				// If not fetch for this time, all cache is expired and cleared.
	};

}
