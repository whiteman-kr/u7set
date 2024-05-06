#pragma once

#include <Network.pb.h>

namespace ClientLib
{
	class AppDataSourceState
	{
	public:
		AppDataSourceState();

		quint64 id() const;
		QString equipmentId() const;

		void setNewState(const ::Network::AppDataSourceState& newState);

		int getErrorsCount() const;

		bool valid() const;
		void invalidate();

		const ::Network::AppDataSourceState& previousState() const;

	public:
		::Network::DataSourceInfo info;
		::Network::AppDataSourceState state;

	private:
		qint64 m_previousStateUpdatePeriod = 5;

		bool m_valid = true;

		::Network::AppDataSourceState m_previousState; // Previous state is updated every 5 seconds

		QDateTime m_perviousStateLastUpdateTime;
	};
} // namespace ClientLib