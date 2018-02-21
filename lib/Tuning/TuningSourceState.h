#pragma once

#include <QDateTime>
#include "../Proto/network.pb.h"

class TuningSource
{
public:
	TuningSource();

	quint64 id() const;
	QString equipmentId() const;

	void setNewState(const ::Network::TuningSourceState& newState);

	int getErrorsCount() const;

	bool valid() const;
	void invalidate();

public:
	::Network::DataSourceInfo info;
	::Network::TuningSourceState state;

private:
	int m_previousStateUpdatePeriod = 5;

	bool m_valid = true;

	::Network::TuningSourceState m_previousState;	// Previous state is updated every 5 seconds

	QDateTime m_perviousStateLastUpdateTime;
};
