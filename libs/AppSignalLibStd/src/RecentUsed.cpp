#include "../include/AppSignalLibStd/RecentUsed.h"

using namespace std::chrono_literals;

namespace AppSignalLib
{

	void RecentUsed::ElapsedTimer::start()
	{
		m_startTime = std::chrono::steady_clock::now();
	}

	void RecentUsed::ElapsedTimer::restart()
	{
		m_startTime = std::chrono::steady_clock::now();
	}

	bool RecentUsed::ElapsedTimer::hasExpired(std::chrono::milliseconds timeout) const
	{
		auto now = std::chrono::steady_clock::now();
		return (now - m_startTime) > timeout;
	}


	RecentUsed::RecentUsed(size_t maxSize /*= 750*/) :
		m_maxSize(maxSize)
	{
		m_lastTimeDataFetched.start();
	}

	void RecentUsed::add(Hash hash, time_point now)
	{
		if (m_lastTimeDataFetched.hasExpired(ExpirationInterval) == true)
		{
			// Nobody is fetching data from the recents, most likely there is no such thread.
			//
			m_signalToTime.clear();
			m_timeToSignal.clear();
			return;
		}

		auto it = m_signalToTime.find(hash);
		if (it == m_signalToTime.end())
		{
			if (m_signalToTime.size() >= m_maxSize)
			{
				auto lastTimeIt = m_timeToSignal.begin();

				m_signalToTime.erase(lastTimeIt->second);
				m_timeToSignal.erase(lastTimeIt);
			}

			m_signalToTime.insert({hash, now});
			m_timeToSignal.insert({now, hash});
		}
		else
		{
			// Update add time.
			//
			auto itemTime = it->second;
			bool updated = false;

			auto range = m_timeToSignal.equal_range(itemTime);
			for (auto th = range.first; th != range.second; ++th)
			{
				if (th->second == hash)
				{
					m_timeToSignal.erase(th);
					m_timeToSignal.insert({now, hash});
					updated = true;
					break;
				}
			}
			assert(updated == true);

			it->second = now;
		}

		assert(m_signalToTime.size() == m_timeToSignal.size());
		return;
	}

	void RecentUsed::add(std::span<const Hash> hashes)
	{
		if (m_lastTimeDataFetched.hasExpired(ExpirationInterval) == true)
		{
			// Nobody is fetching data from the recents, most likely there is no such thread.
			//
			m_signalToTime.clear();
			m_timeToSignal.clear();
			return;
		}

		auto now = std::chrono::system_clock::now();

		for (Hash hash : hashes)
		{
			add(hash, now);
		}

		return;
	}

	bool RecentUsed::remove(Hash hash)
	{
		auto it = m_signalToTime.find(hash);
		if (it == m_signalToTime.end())
		{
			return false;
		}

		auto itemTime = it->second;
		bool removedFromTimeMap = false;

		auto range = m_timeToSignal.equal_range(itemTime);
		for (auto th = range.first; th != range.second; ++th)
		{
			if (th->second == hash)
			{
				m_timeToSignal.erase(th);
				removedFromTimeMap = true;
				break;
			}
		}
		assert(removedFromTimeMap == true);

		m_signalToTime.erase(it);

		assert(m_signalToTime.size() == m_timeToSignal.size());
		return true;
	}

	bool RecentUsed::remove(std::span<const Hash> hashes)
	{
		bool ok = true;
		for (Hash hash : hashes)
		{
			ok &= remove(hash);
		}

		return ok;
	}

	bool RecentUsed::removeOutdated()
	{
		auto now = std::chrono::system_clock::now();

		std::vector<Hash> hashesToRemove;
		hashesToRemove.reserve(m_signalToTime.size());

		for (const auto& [hash, lastAccessTime] : m_signalToTime)
		{
			if (now - lastAccessTime > ExpirationInterval)
			{
				hashesToRemove.push_back(hash);
			}
		}

		return remove(hashesToRemove);
	}

	std::vector<Hash> RecentUsed::hashes() const
	{
		// Restart timer, it indicates that something is fetching data, so this recent thing should work.
		//
		m_lastTimeDataFetched.restart();

		std::vector<Hash> result;
		result.reserve(m_signalToTime.size());

		for (auto p : m_signalToTime)
		{
			result.push_back(p.first);
		}

		return result;
	}
} // namespace AppSignalLib
