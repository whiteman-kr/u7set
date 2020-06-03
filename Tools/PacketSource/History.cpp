#include "History.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalForLog::SignalForLog()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog::SignalForLog(const SignalForLog& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog::SignalForLog(PS::Signal* pSignal, double prevState, double state) :
	m_pSignal(pSignal),
	m_prevState(prevState),
	m_state(state)
{
	QDateTime cdt = QDateTime::currentDateTime();

	m_time = QString::asprintf("%02d-%02d-%04d %02d:%02d:%02d:%03d",

					cdt.date().day(),
					cdt.date().month(),
					cdt.date().year(),

					cdt.time().hour(),
					cdt.time().minute(),
					cdt.time().second(),
					cdt.time().msec());
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog::~SignalForLog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalForLog::clear()
{
	m_time.clear();

	m_pSignal = nullptr;

	m_prevState = 0.0;
	m_state = 0.0;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalForLog::stateStr(double state) const
{
	if (m_pSignal == nullptr)
	{
		return QString();
	}

	QString str, formatStr;

	switch (m_pSignal->signalType())
	{
		case E::SignalType::Analog:

			switch (m_pSignal->analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:		formatStr = QString::asprintf("%%.%df", 0);								break;
				case E::AnalogAppSignalFormat::Float32:			formatStr = QString::asprintf("%%.%df", m_pSignal->decimalPlaces());	break;
				default:										assert(0);													break;
			}

			str = QString::asprintf(formatStr.toLocal8Bit(), state);

			if (m_pSignal->unit().isEmpty() == false)
			{
				str.append(" " + m_pSignal->unit());
			}

			break;

		case E::SignalType::Discrete:

			str = state == 0.0 ? "No (0)" : "Yes (1)";

			break;
	}

	return str;
}


// -------------------------------------------------------------------------------------------------------------------

SignalForLog& SignalForLog::operator=(const SignalForLog& from)
{
	m_signalMutex.lock();

		m_time = from.m_time;

		m_pSignal = from.m_pSignal;

		m_prevState = from.m_prevState;
		m_state = from.m_state;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalHistory::SignalHistory(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

SignalHistory::~SignalHistory()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistory::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();

	emit signalCountChanged();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistory::count() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistory::append(const SignalForLog& signalLog)
{
	PS::Signal* pSignal = signalLog.signalPtr();
	if (pSignal == nullptr)
	{
		assert(false);
		return -1;
	}

	if (pSignal->hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

		m_signalList.append(signalLog);
		index = m_signalList.count() - 1;

	m_signalMutex.unlock();

	emit signalCountChanged();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog* SignalHistory::signalPtr(int index) const
{
	SignalForLog* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = (SignalForLog*) &m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog SignalHistory::signal(int index) const
{
	SignalForLog signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

SignalHistory& SignalHistory::operator=(const SignalHistory& from)
{
	m_signalMutex.lock();

		m_signalList = from.m_signalList;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
