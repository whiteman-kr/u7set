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
		default:									assert(0);										break;
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
				default:										assert(0);										break;
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

				default: assert(0);
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

				default: assert(0); break;
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
	m_signalMutex.lock();

		m_signalList.clear();
		m_signalHashMap.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::count() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::readFromFile(const BuildInfo& buildInfo)
{
	clear();

	QString msgTitle = tr("Loading signals");

	if (buildInfo.buildDirPath().isEmpty() == true)
	{
		QString strError = tr("Please, input path to build directory!");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	// read Signals
	//
	QString signalsfile = buildInfo.buildFile(BUILD_FILE_TYPE_SIGNALS).path();
	if (signalsfile.isEmpty() == true)
	{
		QString strError = tr("Signals file %1 - path is empty!").arg(File::APP_SIGNALS_ASGS);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	QFile fileSignalsAsgs(signalsfile);
	if (fileSignalsAsgs.exists() == false)
	{
		QString strError = tr("File %1 is not found!").arg(signalsfile);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	if (fileSignalsAsgs.open(QIODevice::ReadOnly) == false)
	{
		QString strError = tr("File %1 is not opened!").arg(signalsfile);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	QByteArray&& signalsData = fileSignalsAsgs.readAll();
	QByteArray uncompressedData = qUncompress(signalsData);

	::Proto::AppSignalSet protoAppSignalSet;

	bool result = protoAppSignalSet.ParseFromArray(uncompressedData.constData(), uncompressedData.size());
	if (result == false)
	{
		return 0;
	}

	QString strProgressLabel;
	int signalCount = protoAppSignalSet.appsignal_size();

	#ifndef Q_CONSOLE_APP
		QProgressDialog* pprd = new QProgressDialog(tr("Loading signals..."), tr("&Cancel"), 0, signalCount);
		pprd->setMinimumDuration(0);
		pprd->setWindowTitle("Please Wait");

		for(int i = 0; i < signalCount; i++)
		{
			pprd->setValue(i) ;
			qApp->processEvents();
			if (pprd->wasCanceled())
			{
				 break;
			}

			const ::Proto::AppSignal& protoAppSignal = protoAppSignalSet.appsignal(i);

			PS::Signal signal;
			signal.serializeFrom(protoAppSignal);

			append(signal);
		}

		delete pprd;
	#else
		qDebug() << "Wait, please, signals are loading ...";

		for(int i = 0; i < signalCount; i++)
		{
			const ::Proto::AppSignal& protoAppSignal = protoAppSignalSet.appsignal(i);

			PS::Signal signal;
			signal.serializeFrom(protoAppSignal);

			append(signal);
		}
	#endif

	emit signalsLoaded();

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::append(const PS::Signal& signal)
{
	if (signal.appSignalID().isEmpty() == true || signal.hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(signal.hash()) == false)
		{
			m_signalList.append(signal);
			index = m_signalList.count() - 1;

			m_signalHashMap.insert(signal.hash(), index);
		}

	m_signalMutex.unlock();

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

	PS::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];
			if (index >= 0 && index < m_signalList.count())
			{
				pSignal = (PS::Signal*) &m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Signal* SignalBase::signalPtr(int index) const
{
	PS::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = (PS::Signal*) &m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}


// -------------------------------------------------------------------------------------------------------------------

PS::Signal SignalBase::signal(const Hash& hash) const
{
	PS::Signal signal;

	m_signalMutex.lock();

		int index = m_signalHashMap[hash];
		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}


// -------------------------------------------------------------------------------------------------------------------

PS::Signal SignalBase::signal(int index) const
{
	PS::Signal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}



// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignal(int index, const PS::Signal &signal)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index] = signal;
		}

	m_signalMutex.unlock();
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

	m_signalStateList.append(ss);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::restoreSignalsState()
{
	int count = m_signalStateList.count();
	for(int i = 0; i < count; i++)
	{
		PS::Signal* pSignal = signalPtr(m_signalStateList[i].appSignalID);
		if (pSignal == nullptr)
		{
			continue;
		}

		pSignal->setState(m_signalStateList[i].state);
	}

	m_signalStateList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

SignalBase& SignalBase::operator=(const SignalBase& from)
{
	m_signalMutex.lock();

		m_signalList = from.m_signalList;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
