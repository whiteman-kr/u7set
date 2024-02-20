#include "TuningSignalState.h"

//
// TuningSignalState
//
TuningSignalState::TuningSignalState(const ::Network::TuningSignalState& message)
{
	setState(message);
}

TuningSignalState::TuningSignalState(Hash hash, TuningSignalStateFlags flags, TuningValue value) :
	m_hash(hash),
	m_flags(flags),
	m_value(value)
{
}

Hash TuningSignalState::hash() const
{
	return m_hash;
}

TuningValue TuningSignalState::value() const
{
	return m_value;
}

QVariant TuningSignalState::toVariant() const
{
	return m_value.toVariant();
}

double TuningSignalState::toDouble() const
{
	return m_value.toDouble();
}

TuningValue TuningSignalState::lowBound() const
{
	return m_lowBound;
}

QVariant TuningSignalState::lowBoundToVariant() const
{
	return m_lowBound.toVariant();
}

TuningValue TuningSignalState::highBound() const
{
	return m_highBound;
}

QVariant TuningSignalState::highBoundToVariant() const
{
	return m_highBound.toVariant();
}

bool TuningSignalState::valid() const
{
	return m_flags.valid;
}

bool TuningSignalState::outOfRange() const
{
	return m_flags.outOfRange;
}

bool TuningSignalState::writeInProgress() const
{
	return m_flags.writeInProgress;
}

bool TuningSignalState::controlIsEnabled() const
{
    return m_flags.controlIsEnabled;
}

bool TuningSignalState::writingIsEnabled() const
{
	return m_flags.writingIsEnabled;
}

bool TuningSignalState::isTuningDefault() const
{
	return m_flags.tuningDefault;
}

int TuningSignalState::writeErrorCode() const
{
	return m_writeErrorCode;
}

Hash TuningSignalState::writeClient() const
{
	return m_writeClient;
}

QDateTime TuningSignalState::successfulReadTime() const
{
	return QDateTime::fromMSecsSinceEpoch(m_successfulReadTime);
}

QDateTime TuningSignalState::writeRequestTime() const
{
	return QDateTime::fromMSecsSinceEpoch(m_writeRequestTime);
}

QDateTime TuningSignalState::successfulWriteTime() const
{
	return QDateTime::fromMSecsSinceEpoch(m_successfulWriteTime);
}

QDateTime TuningSignalState::unsuccessfulWriteTime() const
{
	return QDateTime::fromMSecsSinceEpoch(m_unsuccessfulWriteTime);
}

QDateTime TuningSignalState::lmTime() const
{
	return QDateTime::fromMSecsSinceEpoch(m_lmTime);
}

bool TuningSignalState::setState(const ::Network::TuningSignalState& message)
{
	m_hash = message.signalhash();
	m_flags.valid = message.valid();

	m_value.load(message.value());
	m_lowBound.load(message.readlowbound());
	m_highBound.load(message.readhighbound());

	m_flags.outOfRange = m_value < m_lowBound || m_value > m_highBound;

	m_flags.writeInProgress = message.writeinprogress();
	m_writeErrorCode = message.writeerrorcode();

	m_flags.writingIsEnabled = message.valid() == true && message.writingdisabled() == false;

	m_flags.tuningDefault = message.tuningdefault();

	m_writeClient = message.writeclient();

	m_successfulReadTime = message.successfulreadtime();
	m_writeRequestTime = message.writerequesttime();
	m_successfulWriteTime = message.successfulwritetime();
	m_unsuccessfulWriteTime = message.unsuccessfulwritetime();
	m_lmTime = message.lmtime();

	return true;
}

void TuningSignalState::invalidate()
{
	m_flags.valid = false;
}

bool TuningSignalState::limitsUnbalance(const AppSignalParam& asp) const
{
	if (valid() == true && asp.isAnalog() == true)
	{
		if (lowBound() != asp.tuningLowBound() || highBound() != asp.tuningHighBound())
		{
			return true;
		}
	}
	return false;
}

