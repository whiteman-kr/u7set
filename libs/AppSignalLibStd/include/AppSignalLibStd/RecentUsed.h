#pragma once

// #include <chrono> // Include chrono via precompiled header! It speeds up compilation significantly.
#include <cstdint>
#include <map>
#include <span>
#include <vector>

namespace AppSignalLib
{
	class RecentUsed
	{
		class ElapsedTimer
		{
		public:
			void start();
			void restart();
			bool hasExpired(std::chrono::milliseconds timeout) const;

		private:
			std::chrono::steady_clock::time_point m_startTime;
		};

	public:
		explicit RecentUsed(size_t maxSize = 500);

	public:
		using time_point = std::chrono::system_clock::time_point;

		void add(Hash hash, time_point now = std::chrono::system_clock::now());
		void add(std::span<const Hash> hashes);

		bool remove(Hash hash);
		bool remove(std::span<const Hash> hashes);

		bool removeOutdated();

		[[nodiscard]] std::vector<Hash> hashes() const;

	private:
		const size_t m_maxSize{};
		std::map<Hash, time_point> m_signalToTime;      // key - signal hash, value - time of last update.
		std::multimap<time_point, Hash> m_timeToSignal; // key - time of last update, value - signal hash.

		mutable ElapsedTimer m_lastTimeDataFetched;     // If data not fetched regularly, then ignore any add(...).

		static constexpr auto ExpirationInterval =
			std::chrono::milliseconds{3000};            // If not fetch for this time, all cache is expired and cleared.
	};

} // namespace AppSignalLib