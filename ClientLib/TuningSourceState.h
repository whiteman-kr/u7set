#pragma once

#include <map>
#include <QDateTime>
#include "../CommonLib/Hash.h"
#include "../Proto/Network.pb.h"

namespace ClientLib
{

	class TuningSource
	{
	public:
		TuningSource();
		TuningSource(const ::Network::DataSourceInfo& info);

		quint64 id() const;
		QString equipmentId() const;

		const ::Network::DataSourceInfo& info() const;

		int controllersCount() const;	// Gets number of LAN controllers received from DataSourceInfo
		QString controllerEquipmentId(int index) const;

		int statesCount() const;		// Gets number of Channels of states received from TuningSourceState

		const ::Network::TuningSourceState& state(int index) const;
		const ::Network::TuningSourceState& state(Hash controllerHash) const;

		const ::Network::TuningSourceState& previousState(int index) const;
		const ::Network::TuningSourceState& previousState(Hash controllerHash) const;

		void setNewState(const ::Network::TuningSourceState& newState);

		int getErrorsCount(int index) const;

		bool valid() const;
		void invalidate();

	private:
		::Network::DataSourceInfo m_info;

		std::vector<::Network::TuningSourceState> m_states;
		std::map<Hash, int> m_controllerToStateMap;						// Key is Ethernet Controller Hash, value is index in m_states

		bool m_valid = true;

		std::map<int, ::Network::TuningSourceState> m_previousStates;	// Used to calculate if errors count in source state is increasing
		std::map<int, ::Network::TuningSourceState> m_temporaryStates;	// Temporary storage of states to compare with
		std::map<int, QDateTime> m_previousStatesUpdateTimes;			// Time when previous state shoud be updated
	};
}
