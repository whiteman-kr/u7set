#pragma once

#include <vector>
#include <list>
#include <memory>

namespace Proto
{
	class TrendArchiveHour;
	class TrendStateRecord;
}

namespace TrendLib
{
#pragma pack(push, 1)
	struct TrendStateItem_v2
	{
		qint64 system;
		qint64 local;
		qint64 plant;
		double value;
		qint32 flags;
		qint32 padding;

		TrendStateItem_v2() = default;
		TrendStateItem_v2(const AppSignalState& state) :
			system(state.m_time.system.timeStamp),
			local(state.m_time.local.timeStamp),
			plant(state.m_time.plant.timeStamp),
			value(state.m_value),
			flags(state.m_flags.all),
			padding(0)
		{
		}

		void clear() { *this = TrendStateItem_v2{}; }

		[[nodiscard]] bool isValid() const
		{
			return (flags & 0x00000002); // AppSignalStateFlags::stateAvailable, it is a real non valid
			// validity bit 0 is a combination of stateAvailable and coupled validity signal
		}

		void setValid(bool valid)
		{
			if (valid == true)
			{
				flags |= 0x00000003;  // AppSignalStateFlags::stateAvailable && AppSignalStateFlags::valid
			}
			else
			{
				flags &= ~0x00000003; // AppSignalStateFlags::stateAvailable && AppSignalStateFlags::valid
			}
		}

		[[nodiscard]] bool isRealtimePoint() const { return (flags & 0x80000000) ? true : false; }

		void setRealtimePointFlag() { flags |= 0x80000000; }

		void resetRealtimePointFlag() { flags &= ~0x80000000; }

		[[nodiscard]] TimeStamp getTime(E::TimeType timeType) const
		{
			switch (timeType)
			{
			case E::TimeType::Local:
				return TimeStamp{this->local};
			case E::TimeType::System:
				return TimeStamp{this->system};
			case E::TimeType::Plant:
				return TimeStamp{this->plant};
			default:
				assert(false);
				return TimeStamp{this->local};
			}
		}
	};
#pragma pack(pop)

	using TrendStateItem = TrendStateItem_v2;

	struct TrendStateRecord
	{
		std::vector<TrendStateItem> states;
		static const size_t RecomendedSize = 3200;			// TrendStateItem is about 40 bytes, 1600 is about 64KB, 3200 is about 128KB

		// Serialization
		//
		bool save(::Proto::TrendStateRecord* message) const;
		bool load(const ::Proto::TrendStateRecord& message);
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

	struct OneHourData
	{
		enum class State
		{
			NoData,
			Requested,
			Received
		};

		State state = State::NoData;
		std::vector<TrendStateRecord> data;

		// Serialization
		//
		bool save(const TimeStamp& timeStamp, Proto::TrendArchiveHour* message) const;
		bool load(const Proto::TrendArchiveHour& message);
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendStateItem)
Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::RealtimeData>)
