#include "TrendRuler.h"
#include "../Proto/trends.pb.h"

namespace TrendLib
{

	TrendRuler::TrendRuler()
	{
	}

	TrendRuler::TrendRuler(TimeStamp& timeStamp) :
		m_timeStamp(timeStamp)
	{
	}

	bool TrendRuler::save(::Proto::TrendRuler* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_time_stamp(m_timeStamp.timeStamp);
		message->set_show(m_showRuler);
		message->set_show_signal_values(m_showSignalValues);

		return true;
	}

	bool TrendRuler::load(const ::Proto::TrendRuler& message)
	{
		m_timeStamp.timeStamp = message.time_stamp();
		m_showRuler = message.show();
		m_showSignalValues = message.show_signal_values();

		return true;
	}

	const TimeStamp& TrendRuler::timeStamp() const
	{
		return m_timeStamp;
	}

	void TrendRuler::setTimeStamp(const TimeStamp& value)
	{
		m_timeStamp = value.timeStamp;
	}

	bool TrendRuler::isShowRuler() const
	{
		return m_showRuler;
	}

	void TrendRuler::setShowRuler(bool value)
	{
		m_showRuler = value;
	}

	bool TrendRuler::showSignalValues() const
	{
		return m_showSignalValues;
	}

	void TrendRuler::setShowSignalValue(bool value)
	{
		m_showSignalValues = value;
	}

	//
	// TrendRulerSet
	//
	TrendRulerSet::TrendRulerSet()
	{
		m_rulers.reserve(16);
	}


	bool TrendRulerSet::save(::Proto::TrendRulerSet* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		bool ok = true;

		for (const TrendLib::TrendRuler& ruler : m_rulers)
		{
			ok &= ruler.save(message->add_rulers());
		}

		return ok;
	}

	bool TrendRulerSet::load(const ::Proto::TrendRulerSet& message)
	{
		if (message.IsInitialized() == false)
		{
			assert(message.IsInitialized());
			return false;
		}

		bool ok = true;

		m_rulers.clear();
		m_rulers.reserve(message.rulers().size());

		for (int i = 0; i < message.rulers().size(); i++)
		{
			TrendRuler r;
			ok &= r.load(message.rulers(i));

			m_rulers.push_back(r);
		}

		return ok;
	}


	void TrendRulerSet::addRuler(const TrendLib::TrendRuler& ruler)
	{
		TrendLib::TrendRuler r = ruler;
		r.setTimeStamp(ruler.timeStamp().timeStamp - ruler.timeStamp().timeStamp % 5);

		m_rulers.push_back(r);
	}

	void TrendRulerSet::deleteRuler(const TimeStamp& rulerTimeStamp)
	{
		auto it = std::remove_if(m_rulers.begin(), m_rulers.end(),
								[&rulerTimeStamp](TrendRuler& ruler)
								{
									return ruler.timeStamp() == rulerTimeStamp;
								});
		m_rulers.erase(it);
		return;
	}

	std::vector<TrendLib::TrendRuler> TrendRulerSet::getRulers(const TimeStamp& startTime, const TimeStamp& finishTime) const
	{
		std::vector<TrendLib::TrendRuler> result;
		result.reserve(8);

		for (const TrendLib::TrendRuler& r : m_rulers)
		{
			if (r.timeStamp() >= startTime && r.timeStamp() <= finishTime)
			{
				result.push_back(r);
			}
		}

		return result;
	}

	std::vector<TrendLib::TrendRuler>& TrendRulerSet::rulers()
	{
		return m_rulers;
	}

	const std::vector<TrendLib::TrendRuler>& TrendRulerSet::rulers() const
	{
		return m_rulers;
	}

	TrendLib::TrendRuler& TrendRulerSet::at(int index)
	{
		return m_rulers.at(index);
	}

	const TrendLib::TrendRuler& TrendRulerSet::at(int index) const
	{
		return m_rulers.at(index);
	}


}
