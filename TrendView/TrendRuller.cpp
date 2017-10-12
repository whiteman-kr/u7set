#include "TrendRuller.h"
#include "../Proto/trends.pb.h"

namespace TrendLib
{

	TrendRuller::TrendRuller()
	{
	}

	TrendRuller::TrendRuller(TimeStamp& timeStamp) :
		m_timeStamp(timeStamp)
	{
	}

	bool TrendRuller::save(::Proto::TrendRuller* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_time_stamp(m_timeStamp.timeStamp);
		message->set_show(m_showRuller);
		message->set_show_signal_values(m_showSignalValues);

		return true;
	}

	bool TrendRuller::load(const ::Proto::TrendRuller& message)
	{
		m_timeStamp.timeStamp = message.time_stamp();
		m_showRuller = message.show();
		m_showSignalValues = message.show_signal_values();

		return true;
	}

	const TimeStamp& TrendRuller::timeStamp() const
	{
		return m_timeStamp;
	}

	void TrendRuller::setTimeStamp(const TimeStamp& value)
	{
		m_timeStamp = (value.timeStamp / 5) * 5;
	}

	bool TrendRuller::isShowRuller() const
	{
		return m_showRuller;
	}

	void TrendRuller::setShowRuller(bool value)
	{
		m_showRuller = value;
	}

	bool TrendRuller::showSignalValues() const
	{
		return m_showSignalValues;
	}

	void TrendRuller::setShowSignalValue(bool value)
	{
		m_showSignalValues = value;
	}

	//
	// TrendRullerSet
	//
	TrendRullerSet::TrendRullerSet()
	{
		m_rullers.reserve(16);
	}


	bool TrendRullerSet::save(::Proto::TrendRullerSet* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		bool ok = true;

		for (const TrendLib::TrendRuller& ruller : m_rullers)
		{
			ok &= ruller.save(message->add_rullers());
		}

		return ok;
	}

	bool TrendRullerSet::load(const ::Proto::TrendRullerSet& message)
	{
		if (message.IsInitialized() == false)
		{
			assert(message.IsInitialized());
			return false;
		}

		bool ok = true;

		m_rullers.clear();
		m_rullers.reserve(message.rullers().size());

		for (int i = 0; i < message.rullers().size(); i++)
		{
			TrendRuller r;
			ok &= r.load(message.rullers(i));

			m_rullers.push_back(r);
		}

		return ok;
	}


	void TrendRullerSet::addRuller(const TrendLib::TrendRuller& ruller)
	{
		TrendLib::TrendRuller r = ruller;
		r.setTimeStamp(ruller.timeStamp().timeStamp - ruller.timeStamp().timeStamp % 5);

		m_rullers.push_back(r);
	}

	void TrendRullerSet::deleteRuller(const TimeStamp& rullerTimeStamp)
	{
		auto it = std::remove_if(m_rullers.begin(), m_rullers.end(),
								[&rullerTimeStamp](TrendRuller& ruller)
								{
									return ruller.timeStamp() == rullerTimeStamp;
								});
		m_rullers.erase(it);
		return;
	}

	std::vector<TrendLib::TrendRuller> TrendRullerSet::getRullers(const TimeStamp& startTime, const TimeStamp& finishTime) const
	{
		std::vector<TrendLib::TrendRuller> result;
		result.reserve(8);

		for (const TrendLib::TrendRuller& r : m_rullers)
		{
			if (r.timeStamp() >= startTime && r.timeStamp() <= finishTime)
			{
				result.push_back(r);
			}
		}

		return result;
	}

	std::vector<TrendLib::TrendRuller>& TrendRullerSet::rullers()
	{
		return m_rullers;
	}

	const std::vector<TrendLib::TrendRuller>& TrendRullerSet::rullers() const
	{
		return m_rullers;
	}

	TrendLib::TrendRuller& TrendRullerSet::at(int index)
	{
		return m_rullers.at(index);
	}

	const TrendLib::TrendRuller& TrendRullerSet::at(int index) const
	{
		return m_rullers.at(index);
	}


}
