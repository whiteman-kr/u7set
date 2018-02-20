#include "../lib/Tuning/TuningSourceState.h"

//
//TuningSource
//

TuningSource::TuningSource()
{
	m_perviousStateLastUpdateTime = QDateTime::currentDateTime();
}

quint64 TuningSource::id() const
{
	return info.id();
}

QString TuningSource::equipmentId() const
{
	return info.equipmentid().c_str();
}

void TuningSource::setNewState(const ::Network::TuningSourceState& newState)
{
	QDateTime ct = QDateTime::currentDateTime();

	int secsTo = m_perviousStateLastUpdateTime.secsTo(ct);

	if (secsTo > m_previousStateUpdatePeriod)
	{
		m_previousState = state;
		m_perviousStateLastUpdateTime = ct;
	}

	state = newState;
}

int TuningSource::getErrorsCount()
{
	int result = 0;

	// Errors counter

	// errors in reply RupFrameHeader
	//

	if (state.errrupprotocolversion() > m_previousState.errrupprotocolversion())
	{
		result++;
	}

	if (state.errrupframesize() > m_previousState.errrupframesize())
	{
		result++;
	}

	if (state.errrupnontuningdata() > m_previousState.errrupnontuningdata())
	{
		result++;
	}

	if (state.errrupmoduletype() > m_previousState.errrupmoduletype())
	{
		result++;
	}

	if (state.errrupframesquantity() > m_previousState.errrupframesquantity())
	{
		result++;
	}

	if (state.errrupframenumber() > m_previousState.errrupframenumber())
	{
		result++;
	}

	if (state.errrupcrc() > m_previousState.errrupcrc())
	{
		result++;
	}

	// errors in reply FotipHeader
	//

	if (state.errfotipprotocolversion() > m_previousState.errfotipprotocolversion())
	{
		result++;
	}

	if (state.errfotipuniqueid() > m_previousState.errfotipuniqueid())
	{
		result++;
	}

	if (state.errfotiplmnumber() > m_previousState.errfotiplmnumber())
	{
		result++;
	}

	if (state.errfotipsubsystemcode() > m_previousState.errfotipsubsystemcode())
	{
		result++;
	}

	if (state.errfotipoperationcode() > m_previousState.errfotipoperationcode())
	{
		result++;
	}

	if (state.errfotipframesize() > m_previousState.errfotipframesize())
	{
		result++;
	}

	if (state.errfotipromsize() > m_previousState.errfotipromsize())
	{
		result++;
	}

	if (state.errfotipromframesize() > m_previousState.errfotipromframesize())
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

	if (state.fotipflagdatatypeerr() > m_previousState.fotipflagdatatypeerr())
	{
		result++;
	}

	if (state.fotipflagopcodeerr() > m_previousState.fotipflagopcodeerr())
	{
		result++;
	}

	if (state.fotipflagstartaddrerr() > m_previousState.fotipflagstartaddrerr())
	{
		result++;
	}

	if (state.fotipflagromsizeerr() > m_previousState.fotipflagromsizeerr())
	{
		result++;
	}

	if (state.fotipflagromframesizeerr() > m_previousState.fotipflagromframesizeerr())
	{
		result++;
	}

	if (state.fotipflagframesizeerr() > m_previousState.fotipflagframesizeerr())
	{
		result++;
	}

	if (state.fotipflagprotocolversionerr() > m_previousState.fotipflagprotocolversionerr())
	{
		result++;
	}

	if (state.fotipflagsubsystemkeyerr() > m_previousState.fotipflagsubsystemkeyerr())
	{
		result++;
	}

	if (state.fotipflaguniueiderr() > m_previousState.fotipflaguniueiderr())
	{
		result++;
	}

	if (state.fotipflagoffseterr() > m_previousState.fotipflagoffseterr())
	{
		result++;
	}

	//if (state.fotipflagapplysuccess() > m_previousState.fotipflagapplysuccess())
	//{
		//result++;
	//}


	// General errors
	//

	if (state.erranaloglowboundcheck() > m_previousState.erranaloglowboundcheck())
	{
		result++;
	}

	if (state.erranaloghighboundcheck() > m_previousState.erranaloghighboundcheck())
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
	state.set_setsor(false);
}
