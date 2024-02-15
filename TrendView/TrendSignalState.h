#pragma once

#include <vector>
#include <list>

namespace Proto
{
	class TrendStateRecord;
}

namespace TrendLib
{

#pragma pack(push, 1)
	struct TrendStateItem_v1
	{
		qint64 system;
		qint64 local;
		qint64 plant;
		qint32 flags;
		double value;

		TrendStateItem_v1() = default;
		TrendStateItem_v1(const AppSignalState& state) :
			system(state.m_time.system.timeStamp),
			local(state.m_time.local.timeStamp),
			plant(state.m_time.plant.timeStamp),
			flags(state.m_flags.all),
			value(state.m_value)
		{
		}

		void clear()
		{
			*this = TrendStateItem_v1{};
		}

		[[nodiscard]] bool isValid() const
		{
			return (flags & 0x00000002);	// AppSignalStateFlags::stateAvailable, it is a real non valid
											// validity bit 0 is a combination of stateAvailable and coupled validity signal
		}

		void setValid(bool valid)
		{
			if (valid == true)
			{
				flags |= 0x00000003;		// AppSignalStateFlags::stateAvailable && AppSignalStateFlags::valid
			}
			else
			{
				flags &= ~0x00000003;		// AppSignalStateFlags::stateAvailable && AppSignalStateFlags::valid
			}
		}

		[[nodiscard]] bool isRealtimePoint() const
		{
			return (flags & 0x80000000) ? true : false;
		}

		void setRealtimePointFlag()
		{
			flags |= 0x80000000;
		}

		void resetRealtimePointFlag()
		{
			flags &= ~0x80000000;
		}

		[[nodiscard]] TimeStamp getTime(E::TimeType timeType) const
		{
			switch (timeType)
			{
			case E::TimeType::Local:	return TimeStamp{this->local};
			case E::TimeType::System:	return TimeStamp{this->system};
			case E::TimeType::Plant:	return TimeStamp{this->plant};
			default:
				assert(false);
				return TimeStamp{this->local};
			}
		}
	};
#pragma pack(pop)


	using TrendStateItem = TrendStateItem_v1;

	struct TrendStateRecord
	{
		std::vector<TrendStateItem> states;
		static const size_t RecomendedSize = 3200;			// TrendStateItem is about 36-40 bytes, 1600 is about 64KB, 3200 is about 128KB

		// Serialization
		//
		bool save(Proto::TrendStateRecord* message) const;
		bool load(const Proto::TrendStateRecord& message);
	};


	struct RealtimeDataChunk
	{
		Hash appSignalHash = UNDEFINED_HASH;
		std::vector<TrendStateItem> states;
	};


	struct RealtimeData
	{
		std::list<RealtimeDataChunk> signalData;	// Each item is a signal with the vector of states
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendStateItem)
Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::RealtimeData>)
