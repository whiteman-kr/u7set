#include "RecentUsed.h"

#include <QDateTime>

using namespace std::chrono_literals;

namespace AppSignalLib
{
	RecentUsed::RecentUsed(size_t maxSize /*= 750*/) :
		m_maxSize(maxSize)
	{
		m_lastTimeDataFetched.start();
	}

	void RecentUsed::add(Hash hash)
	{
		if (m_lastTimeDataFetched.hasExpired(ExpiredTimeMs) == true)
		{
			// Nobody is fetching data from the recents, most likely there is no such thread.
			//
			m_signalToTime.clear();
			m_timeToSignal.clear();
			return;
		}

		qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

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
			qint64 itemTime = it->second;
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
			Q_ASSERT(updated == true);

			it->second = now;
		}

		Q_ASSERT(m_signalToTime.size() == m_timeToSignal.size());
		return;
	}

	void RecentUsed::add(const std::vector<Hash>& hashes)
	{
		if (m_lastTimeDataFetched.hasExpired(ExpiredTimeMs) == true)
		{
			// Nobody is fetching data from the recents, most likely there is no such thread.
			//
			m_signalToTime.clear();
			m_timeToSignal.clear();
			return;
		}

		for (Hash hash : hashes)
		{
			add(hash);
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

		qint64 itemTime = it->second;
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
		Q_ASSERT(removedFromTimeMap == true);

		m_signalToTime.erase(it);

		Q_ASSERT(m_signalToTime.size() == m_timeToSignal.size());
		return true;
	}

	bool RecentUsed::remove(const std::vector<Hash>& hashes)
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
		qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		std::vector<Hash> hashesToRemove;
		hashesToRemove.reserve(m_signalToTime.size());

		for (const auto&[hash, lastAccessTime] : m_signalToTime)
		{
			if (now - lastAccessTime > ExpiredTimeMs)
			{
				hashesToRemove.push_back(hash);
			}
		}

		return remove(hashesToRemove);
	}

	std::vector<Hash> RecentUsed::hashes() const
	{
		// Restart timer, it indicates that sometheng is fetching data, so this recent thing shoud work.
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
}
