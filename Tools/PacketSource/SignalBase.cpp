#include "SignalBase.h"

#include <assert.h>
#include <QFile>

#include "../../lib/XmlHelper.h"
#include "../../lib/DataProtocols.h"
#include "../../lib/WUtils.h"
#include "../../lib/ConstStrings.h"

#ifndef Q_CONSOLE_APP
	#include <QMessageBox>
	#include <QProgressDialog>
#endif

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::Signal::Signal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal::Signal(const PS::Signal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal::~Signal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Signal::clear()
{
	m_offset = -1;
	m_frameIndex = -1;
	m_frameOffset = -1;

	m_pValueData = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Signal::calcOffset()
{
	int offset = regValueAddr().offset();
	if (offset == BAD_ADDRESS)
	{
		return;
	}

	// offset - in memory in 16-bit words
	//
	m_offset = offset * 2;

	m_frameIndex = m_offset / Rup::FRAME_DATA_SIZE;

	m_frameOffset = m_offset - Rup::FRAME_DATA_SIZE * m_frameIndex;
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::signalTypeStr() const
{
	return E::valueToString<E::SignalType>(signalType());
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::signalInOutTypeStr() const
{
	return E::valueToString<E::SignalInOutType>(inOutType());
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::engineeringRangeStr() const
{
	if(signalType() != E::SignalType::Analog)
	{
		return QString();
	}

	QString range, formatStr;

	switch (analogSignalFormat())
	{
		case E::AnalogAppSignalFormat::SignedInt32:	formatStr = QString::asprintf("%%.%df", 0);					break;
		case E::AnalogAppSignalFormat::Float32:		formatStr = QString::asprintf("%%.%df", decimalPlaces());	break;
		default:
			assert(0);
	}

	range = QString::asprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), lowEngineeringUnits(), highEngineeringUnits());

	if (unit().isEmpty() == false)
	{
		range.append(" " + unit());
	}

	return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::signalFormatStr() const
{
	if(signalType() != E::SignalType::Analog)
	{
		return QString();
	}

	return E::valueToString<E::AnalogAppSignalFormat>(analogSignalFormat());
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::stateOffsetStr() const
{
	int offset = regValueAddr().offset();
	if (offset == BAD_ADDRESS)
	{
		return QString();
	}

	return QString::number(offset*2);
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::stateBitStr() const
{
	int bit = regValueAddr().bit();
	if (regValueAddr().offset() == BAD_ADDRESS || bit == BAD_ADDRESS)
	{
		return QString();
	}

	return QString::number(bit);
}

// -------------------------------------------------------------------------------------------------------------------

QString PS::Signal::stateStr() const
{
	QString str, formatStr;

	switch (signalType())
	{
		case E::SignalType::Analog:

			switch (analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:		formatStr = QString::asprintf("%%.%df", 0);					break;
				case E::AnalogAppSignalFormat::Float32:			formatStr = QString::asprintf("%%.%df", decimalPlaces());	break;
				default:
					assert(0);
			}

			str = QString::asprintf(formatStr.toLocal8Bit(), state());

			if (unit().isEmpty() == false)
			{
				str.append(" " + unit());
			}

			break;

		case E::SignalType::Discrete:

			str = state() == 0.0 ? "No (0)" : "Yes (1)";

			break;
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

double PS::Signal::state() const
{
	if (regBufAddr().offset() == BAD_ADDRESS || regBufAddr().bit() == BAD_ADDRESS)
	{
		return 0;
	}

	if (m_pValueData == nullptr)
	{
		return 0;
	}

	double state = 0;

	switch (signalType())
	{
		case E::SignalType::Analog:

			switch (analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						state = reverseUint32(*pDataPtr);
					}

					break;

				case E::AnalogAppSignalFormat::Float32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						float fState = 0;
						memcpy(&fState, &*pDataPtr, sizeof(float));
						state = static_cast<double>(reverseFloat(fState));
					}

					break;

				default:
					assert(0);
			}

			break;

		case E::SignalType::Discrete:
			{
				quint16* pDataPtr = reinterpret_cast<quint16*>(m_pValueData);
				if (pDataPtr == nullptr)
				{
					break;
				}

				int bitNo = regBufAddr().bit();

				if (bitNo >= 8)
				{
					bitNo -= 8;
				}
				else
				{
					bitNo += 8;
				}

				if ((*pDataPtr & (0x1 << bitNo)) != 0)
				{
					state = 1;
				}
			}
			break;
	}

	return state;
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Signal::setState(double state)
{
	if (regBufAddr().offset() == BAD_ADDRESS || regBufAddr().bit() == BAD_ADDRESS)
	{
		return false;
	}

	if (m_pValueData == nullptr)
	{
		return false;
	}

	bool signalChanged = false;

	switch (signalType())
	{
		case E::SignalType::Analog:

			switch (analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						quint32 iState = reverseUint32(static_cast<quint32>(state));
						*pDataPtr = iState;

						signalChanged = true;
					}
					break;

				case E::AnalogAppSignalFormat::Float32:
					{
						quint32* pDataPtr = reinterpret_cast<quint32*>(m_pValueData);
						if (pDataPtr == nullptr)
						{
							break;
						}

						float fState = reverseFloat(static_cast<float>(state));
						memcpy(pDataPtr, &fState, sizeof(float));

						signalChanged = true;
					}

					break;

				default:
					assert(0);
			}

			break;

		case E::SignalType::Discrete:
			{
				quint16* pDataPtr = reinterpret_cast<quint16*>(m_pValueData);
				if (pDataPtr == nullptr)
				{
					break;
				}

				int bitNo = regBufAddr().bit();

				if (bitNo >= 8)
				{
					bitNo -= 8;
				}
				else
				{
					bitNo += 8;
				}

				int iState = static_cast<int>(state);

				switch(iState)
				{
					case 0: *pDataPtr &= ~(0x1 << bitNo);		break;
					case 1: *pDataPtr |= (0x1 << bitNo);		break;
				}

				signalChanged = true;
			}
			break;
	}

	return signalChanged;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal& PS::Signal::operator=(const PS::Signal& from)
{
	m_signalMutex.lock();

		m_offset = from.m_offset;
		m_frameIndex = from.m_frameIndex;
		m_frameOffset = from.m_frameOffset;
		m_pValueData = from.m_pValueData;

		::Signal::operator=(from);

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

SignalBase::~SignalBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clear()
{
	QMutexLocker l(&m_signalMutex);

	m_signalList.clear();
	m_signalHashMap.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::count() const
{
	QMutexLocker l(&m_signalMutex);

	return TO_INT(m_signalList.size());
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::append(const PS::Signal& signal)
{
	if (signal.appSignalID().isEmpty() == true || signal.hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashMap.contains(signal.hash()) == true)
	{
		return -1;
	}

	m_signalList.push_back(signal);
	int index = TO_INT(m_signalList.size() - 1);

	m_signalHashMap.insert(signal.hash(), index);

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(const QString& appSignalID) const
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return nullptr;
	}

	return signalPtr(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return nullptr;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashMap.contains(hash) == false)
	{
		return nullptr;
	}

	int index = m_signalHashMap[hash];
	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return nullptr;
	}

	PS::Signal* pSignal = (PS::Signal*) &m_signalList[static_cast<quint64>(index)];
	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return nullptr;
	}

	PS::Signal* pSignal = (PS::Signal*) &m_signalList[static_cast<quint64>(index)];
	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal SignalBase::signal(const Hash& hash) const
{
	QMutexLocker l(&m_signalMutex);

	int index = m_signalHashMap[hash];
	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return PS::Signal();
	}

	return m_signalList[static_cast<quint64>(index)];
}


// -------------------------------------------------------------------------------------------------------------------

PS::Signal SignalBase::signal(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return PS::Signal();
	}

	return m_signalList[static_cast<quint64>(index)];
}


// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignal(int index, const PS::Signal &signal)
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return;
	}

	m_signalList[static_cast<quint64>(index)] = signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::saveSignalState(PS::Signal* pSignal)
{
	if (pSignal == nullptr)
	{
		return;
	}

	SignalState ss;

	ss.appSignalID = pSignal->appSignalID();
	ss.state = pSignal->state();

	m_signalStateList.push_back(ss);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::restoreSignalsState()
{
	for(SignalState& ss : m_signalStateList)
	{
		PS::Signal* pSignal = signalPtr(ss.appSignalID);
		if (pSignal == nullptr)
		{
			continue;
		}

		pSignal->setState(ss.state);
	}

	m_signalStateList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

SignalBase& SignalBase::operator=(const SignalBase& from)
{
	QMutexLocker l(&m_signalMutex);

	m_signalList = from.m_signalList;
	m_signalHashMap = from.m_signalHashMap;

	m_signalStateList = from.m_signalStateList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
