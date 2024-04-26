#include <ClientLib/AppDataSourceState.h>

namespace ClientLib
{
	//
	// TuningSource
	//
	AppDataSourceState::AppDataSourceState()
	{
		m_perviousStateLastUpdateTime = QDateTime::currentDateTime();
	}

	quint64 AppDataSourceState::id() const
	{
		return info.id();
	}

	QString AppDataSourceState::equipmentId() const
	{
		return QString::fromStdString(info.moduleequipmentid());
	}

	void AppDataSourceState::setNewState(const ::Network::AppDataSourceState& newState)
	{
		QDateTime ct = QDateTime::currentDateTime();

		qint64 secsTo = m_perviousStateLastUpdateTime.secsTo(ct);

		if (secsTo > m_previousStateUpdatePeriod)
		{
			m_previousState = state;
			m_perviousStateLastUpdateTime = ct;
		}

		state = newState;
	}

	int AppDataSourceState::getErrorsCount() const
	{
		int result = 0;

		// Errors counter
		//
		if (state.errorprotocolversion() > m_previousState.errorprotocolversion())
		{
			result++;
		}

		if (state.errorframesquantity() > m_previousState.errorframesquantity())
		{
			result++;
		}

		if (state.errorframeno() > m_previousState.errorframeno())
		{
			result++;
		}

		if (state.errordataid() > m_previousState.errordataid())
		{
			result++;
		}

		if (state.errorframecrc() > m_previousState.errorframecrc())
		{
			result++;
		}

		if (state.errorduplicateplanttime() > m_previousState.errorduplicateplanttime())
		{
			result++;
		}

		if (state.errornonmonotonicplanttime() > m_previousState.errornonmonotonicplanttime())
		{
			result++;
		}

		if (state.errordataid() > m_previousState.errordataid())
		{
			result++;
		}

		return result;
	}

	bool AppDataSourceState::valid() const
	{
		return m_valid;
	}

	void AppDataSourceState::invalidate()
	{
		m_valid = false;
	}

	const ::Network::AppDataSourceState& AppDataSourceState::previousState() const
	{
		return m_previousState;
	}
}