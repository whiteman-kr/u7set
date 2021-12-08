#pragma once

#include <map>
#include <QDateTime>
#include "../CommonLib/Hash.h"
#include "../Proto/network.pb.h"

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

	int statesChannelsCount() const;		// Gets number of Channels of states received from TuningSourceState

	const ::Network::TuningSourceState& state(int channel) const;
	const ::Network::TuningSourceState& previousState(int channel) const;

	void setState(const ::Network::TuningSourceState& newState);

	int getErrorsCount(int channel) const;

	bool valid() const;
	void invalidate();

private:
	::Network::DataSourceInfo m_info;

	std::vector<::Network::TuningSourceState> m_states;
	std::vector<::Network::TuningSourceState> m_previousStates;

	std::map<Hash, int> m_controllerToStateMap;	// Key is Ethernet Controller Hash, value is index in m_states

	bool m_valid = true;

	qint64 m_previousStateUpdatePeriod = 5;

	QDateTime m_perviousStateLastUpdateTime;
};
