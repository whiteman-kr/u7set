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

	QString lanEquipmentId(int lanIndex) const;

	int getErrorsCount(int channel) const;
	int getErrorsCount(Hash controllerHash) const;

	int channelsCount() const;	// Gets number of received channels (LANs count in info and received states)

	const ::Network::DataSourceInfo& info() const;

	const ::Network::TuningSourceState& state(int channel) const;
	const ::Network::TuningSourceState& state(Hash controllerHash) const;

	const ::Network::TuningSourceState& previousState(int channel) const;
	const ::Network::TuningSourceState& previousState(Hash controllerHash) const;

	void setState(const ::Network::TuningSourceState& newState);

	void invalidate();

private:
	::Network::DataSourceInfo m_info;

	std::vector<::Network::TuningSourceState> m_states;
	std::vector<::Network::TuningSourceState> m_previousStates;

	std::map<Hash, int> m_controllerToStateMap;	// Key is Ethernet Controller Hash, value is index in m_states

	qint64 m_previousStateUpdatePeriod = 5;

	QDateTime m_perviousStateLastUpdateTime;
};
