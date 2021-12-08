#include "../lib/Tuning/TuningSourceState.h"

//
//TuningSource
//

TuningSource::TuningSource()
{
	m_perviousStateLastUpdateTime = QDateTime::currentDateTime();
}

TuningSource::TuningSource(const ::Network::DataSourceInfo& info):
	TuningSource()
{
	m_info = info;
}

quint64 TuningSource::id() const
{
	return m_info.id();
}

QString TuningSource::equipmentId() const
{
	return QString::fromStdString(m_info.moduleequipmentid());
}

const ::Network::DataSourceInfo& TuningSource::info() const
{
	return m_info;
}

int TuningSource::controllersCount() const
{
	return m_info.lancontrollerinfo_size();
}

QString TuningSource::controllerEquipmentId(int index) const
{
	if (index < 0 || index >= m_info.lancontrollerinfo_size())
	{
		Q_ASSERT(false);
		return {};
	}

	return QString::fromStdString(m_info.lancontrollerinfo(index).equipmentid());
}

int TuningSource::statesChannelsCount() const
{
	return static_cast<int>(m_states.size());
}

const ::Network::TuningSourceState& TuningSource::state(int channel) const
{
	if (channel < 0 || channel >= m_states.size())
	{
		static ::Network::TuningSourceState emptyState;

		Q_ASSERT(false);
		return emptyState;
	}

	return m_states[channel];
}

const ::Network::TuningSourceState& TuningSource::previousState(int channel) const
{
	if (channel < 0 || channel >= m_previousStates.size())
	{
		static ::Network::TuningSourceState emptyState;

		Q_ASSERT(false);
		return emptyState;
	}

	return m_previousStates[channel];
}

void TuningSource::setState(const ::Network::TuningSourceState& newState)
{
	Hash controllerHash = ::calcHash(QString::fromStdString(newState.lanequipmentid()));

	int channel = -1;

	auto it = m_controllerToStateMap.find(controllerHash);
	if (it == m_controllerToStateMap.end())
	{
		// Insert a new state to array of states
		//
		m_states.push_back(newState);
		m_previousStates.push_back(newState);

		channel = static_cast<int>(m_states.size() - 1);
		m_controllerToStateMap[controllerHash] = channel;

		Q_ASSERT(m_states.size() == m_previousStates.size());
	}
	else
	{
		Q_ASSERT(m_states.size() == m_previousStates.size());

		// Modify existing state in array of states
		//
		channel = it->second;
		m_states[channel] = newState;

		// Every 5 seconds modify previous state
		//
		QDateTime ct = QDateTime::currentDateTime();

		qint64 secsTo = m_perviousStateLastUpdateTime.secsTo(ct);

		if (secsTo > m_previousStateUpdatePeriod)
		{
			m_previousStates[channel] = newState;
			m_perviousStateLastUpdateTime = ct;
		}
	}

	return;
}

int TuningSource::getErrorsCount(int channel) const
{
	if (channel < 0 || channel >= m_states.size())
	{
		Q_ASSERT(false);
		return 0;
	}

	const ::Network::TuningSourceState& currentState = m_states[channel];
	const ::Network::TuningSourceState& previousState = m_previousStates[channel];

	int result = 0;

	// Errors counter

	// errors in reply RupFrameHeader
	//

	if (currentState.errrupprotocolversion() > previousState.errrupprotocolversion())
	{
		result++;
	}

	if (currentState.errrupframesize() > previousState.errrupframesize())
	{
		result++;
	}

	if (currentState.errrupnontuningdata() > previousState.errrupnontuningdata())
	{
		result++;
	}

	if (currentState.errrupmoduletype() > previousState.errrupmoduletype())
	{
		result++;
	}

	if (currentState.errrupframesquantity() > previousState.errrupframesquantity())
	{
		result++;
	}

	if (currentState.errrupframenumber() > previousState.errrupframenumber())
	{
		result++;
	}

	if (currentState.errrupcrc() > previousState.errrupcrc())
	{
		result++;
	}

	// errors in reply FotipHeader
	//

	if (currentState.errfotipprotocolversion() > previousState.errfotipprotocolversion())
	{
		result++;
	}

	if (currentState.errfotipuniqueid() > previousState.errfotipuniqueid())
	{
		result++;
	}

	if (currentState.errfotiplmnumber() > previousState.errfotiplmnumber())
	{
		result++;
	}

	if (currentState.errfotipsubsystemcode() > previousState.errfotipsubsystemcode())
	{
		result++;
	}

	if (currentState.errfotipoperationcode() > previousState.errfotipoperationcode())
	{
		result++;
	}

	if (currentState.errfotipframesize() > previousState.errfotipframesize())
	{
		result++;
	}

	if (currentState.errfotipromsize() > previousState.errfotipromsize())
	{
		result++;
	}

	if (currentState.errfotipromframesize() > previousState.errfotipromframesize())
	{
		result++;
	}

	// errors reported by LM in reply FotipHeader.flags
	//

	//if (state.fotipflagboundschecksuccess() > m_previousState.fotipflagboundschecksuccess())
	//{
		//result++;
	//}

	//if (state.fotipflagwritesuccess() > m_previousState.fotipflagwritesuccess())
	//{
	//	result++;
	//}

	if (currentState.fotipflagdatatypeerr() > previousState.fotipflagdatatypeerr())
	{
		result++;
	}

	if (currentState.fotipflagopcodeerr() > previousState.fotipflagopcodeerr())
	{
		result++;
	}

	if (currentState.fotipflagstartaddrerr() > previousState.fotipflagstartaddrerr())
	{
		result++;
	}

	if (currentState.fotipflagromsizeerr() > previousState.fotipflagromsizeerr())
	{
		result++;
	}

	if (currentState.fotipflagromframesizeerr() > previousState.fotipflagromframesizeerr())
	{
		result++;
	}

	if (currentState.fotipflagframesizeerr() > previousState.fotipflagframesizeerr())
	{
		result++;
	}

	if (currentState.fotipflagprotocolversionerr() > previousState.fotipflagprotocolversionerr())
	{
		result++;
	}

	if (currentState.fotipflagsubsystemkeyerr() > previousState.fotipflagsubsystemkeyerr())
	{
		result++;
	}

	if (currentState.fotipflaguniueiderr() > previousState.fotipflaguniueiderr())
	{
		result++;
	}

	if (currentState.fotipflagoffseterr() > previousState.fotipflagoffseterr())
	{
		result++;
	}

	//if (state.fotipflagapplysuccess() > m_previousState.fotipflagapplysuccess())
	//{
		//result++;
	//}


	// General errors
	//

	if (currentState.erranaloglowboundcheck() > previousState.erranaloglowboundcheck())
	{
		result++;
	}

	if (currentState.erranaloghighboundcheck() > previousState.erranaloghighboundcheck())
	{
		result++;
	}

	return result;
}

bool TuningSource::valid() const
{
	return m_valid;
}

void TuningSource::invalidate()
{
	m_valid = false;
	for (int i = 0; i < static_cast<int>(m_states.size()); i++)
	{
		m_states[i].set_setsor(false);
	}
}
