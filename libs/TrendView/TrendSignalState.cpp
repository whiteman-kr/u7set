#include <TrendView/TrendSignalState.h>

namespace
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

		void clear() { *this = TrendStateItem_v1{}; }

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
} // namespace

namespace TrendLib
{
	bool TrendStateRecord::save(::Proto::TrendStateRecord* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		// Saving TrendStateItem_v2
		//
		static_assert(std::is_same<std::vector<TrendStateItem>::value_type, TrendStateItem_v2>::value, "Expected TrendStateItem_v2");
		message->set_states_raw_buffer_v2(reinterpret_cast<const char*>(states.data()), states.size() * sizeof(TrendStateItem_v2));

		return ok;
	}

	bool TrendStateRecord::load(const ::Proto::TrendStateRecord& message)
	{
		bool dataLoaded = false;

		// Loading TrendStateItem_v2, which is the latest version
		//
		if (message.has_states_raw_buffer_v2() == true)
		{
			Q_ASSERT(message.states_raw_buffer_v2().size() % sizeof(TrendStateItem_v2) == 0);
			size_t stateCount = message.states_raw_buffer_v2().size() / sizeof(TrendStateItem_v2);

			std::vector<TrendStateItem_v2> loadedStates;
			loadedStates.resize(stateCount);

			memcpy(loadedStates.data(), message.states_raw_buffer_v2().data(), message.states_raw_buffer_v2().size());

			states = std::move(loadedStates);

			dataLoaded = true;
		}

		// Loading TrendStateItem_v1
		//
		if (dataLoaded == false && message.has_states_raw_buffer_v1() == true)
		{
			Q_ASSERT(message.states_raw_buffer_v1().size() % sizeof(TrendStateItem_v1) == 0);
			size_t stateCount = message.states_raw_buffer_v1().size() / sizeof(TrendStateItem_v1);

			std::vector<TrendStateItem_v1> v1_states;
			v1_states.resize(stateCount);

			memcpy(v1_states.data(), message.states_raw_buffer_v1().data(), message.states_raw_buffer_v1().size());

			// Fill TrendStateItem_v2 from TrendStateItem_v1
			//
			states.clear();
			states.reserve(stateCount);

			for (const auto& state : v1_states)
			{
				TrendStateItem_v2 state_v2;

				state_v2.system = state.system;
				state_v2.local = state.local;
				state_v2.plant = state.plant;
				state_v2.flags = state.flags;
				state_v2.value = state.value;
				state_v2.padding = 0;

				states.push_back(state_v2);
			}

			dataLoaded = true;
		}

		return dataLoaded;
	}

	bool OneHourData::save(const TimeStamp& timeStamp, Proto::TrendArchiveHour* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		message->set_time_stamp(timeStamp.timeStamp);
		message->set_state(static_cast<int>(state));

		message->mutable_records()->Reserve(static_cast<int>(data.size()));
		for (const TrendStateRecord& record : data)
		{
			ok &= record.save(message->add_records());
		}

		return ok;
	}

	bool OneHourData::load(const Proto::TrendArchiveHour& message)
	{
		bool ok = true;

		// message.time_stamp() -- is not read jere, it is required on one level lower, in TrendArchive as a key to map
		state = static_cast<OneHourData::State>(message.state());

		data.clear();
		data.reserve(message.records_size());

		for (int i = 0; i < message.records_size(); i++)
		{
			TrendStateRecord& record = data.emplace_back();
			ok &= record.load(message.records(i));
		}

		return ok;
	}
} // namespace TrendLib
