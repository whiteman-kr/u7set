#include "TrendSignalState.h"


namespace TrendLib
{
	bool TrendStateRecord::save(Proto::TrendStateRecord* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		// Saving TrendStateItem_v1
		//
		static_assert(std::is_same<std::vector<TrendStateItem>::value_type, TrendStateItem_v1>::value, "Expepcted TrendStateItem_v1");
		message->set_states_raw_buffer_v1(reinterpret_cast<const char*>(states.data()), states.size() * sizeof(TrendStateItem_v1));

		return ok;
	}

	bool TrendStateRecord::load(const Proto::TrendStateRecord& message)
	{
		bool ok = true;

		// Loading TrendStateItem_v1
		//
		Q_ASSERT(message.states_raw_buffer_v1().size() % sizeof(TrendStateItem_v1) == 0);
		size_t stateCount = message.states_raw_buffer_v1().size() / sizeof(TrendStateItem_v1);

		states.clear();
		states.resize(stateCount);

		memcpy(states.data(), message.states_raw_buffer_v1().data(), message.states_raw_buffer_v1().size());

		return ok;
	}
}
