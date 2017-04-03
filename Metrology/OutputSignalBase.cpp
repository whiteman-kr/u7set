#include "OutputSignalBase.h"

#include "Database.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal() :
	m_hash(0)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal(const OutputSignal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::isValid() const
{
	if (m_hash == 0)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::clear()
{
	m_signalMutex.lock();

		for(int t = 0; t < MEASURE_IO_SIGNAL_TYPE_COUNT; t++)
		{
			m_pSignal[t] = nullptr;
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setHash()
{
	QString strID;

	m_signalMutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			strID.append(m_appSignalID[type]);
		}

		if (strID.isEmpty() == false)
		{
			m_hash = calcHash(strID);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignal::typeStr() const
{
	if (m_type < 0 || m_type >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	return OutputSignalType[m_type];
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignal::appSignalID(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	QString appSignalID;

	m_signalMutex.lock();

		appSignalID = m_appSignalID[type];

	m_signalMutex.unlock();

	return appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setAppSignalID(int type, const QString& appSignalID)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	m_signalMutex.lock();

		m_appSignalID[type] = appSignalID;

	m_signalMutex.unlock();

	setHash();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* OutputSignal::metrologySignal(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return nullptr;
	}

	Metrology::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		pSignal = m_pSignal[type];

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setMetrologySignal(int type, Metrology::Signal* pSignal)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	if (pSignal == nullptr)
	{
		return;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return;
	}

	m_signalMutex.lock();

		m_appSignalID[type] = param.appSignalID();

		m_pSignal[type] = pSignal;

	m_signalMutex.unlock();

	setHash();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::initMetrologySignal()
{
	m_signalMutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			if (m_appSignalID[type].isEmpty() == true)
			{
				continue;
			}

			Hash signalHash = calcHash(m_appSignalID[type]);
			if (signalHash == 0)
			{
				continue;
			}

			m_pSignal[type] = theSignalBase.signalPtr(signalHash);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal& OutputSignal::operator=(const OutputSignal& from)
{
	m_signalMutex.lock();

		m_index = from.m_index;

		m_type = from.m_type;

		m_hash = from.m_hash;

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_appSignalID[type] = from.m_appSignalID[type];
			m_pSignal[type] = from.m_pSignal[type];
		}

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase::OutputSignalBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::count() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::sort()
{

}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::load()
{
	if (thePtrDB == nullptr)
	{
		return 0;
	}

	QTime responseTime;
	responseTime.start();

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_OUTPUT_SIGNAL);
	if (table == nullptr)
	{
		return false;
	}

	int readedRecordCount = 0;

	m_signalMutex.lock();

		m_signalList.resize(table->recordCount());

		readedRecordCount = table->read(m_signalList.data());

	m_signalMutex.unlock();

	table->close();

	qDebug() << "OutputSignalBase::load() - Loaded output signals: " << readedRecordCount << ", Time for load: " << responseTime.elapsed() << " ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignalBase::save()
{
	if (thePtrDB == nullptr)
	{
		return false;
	}

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_OUTPUT_SIGNAL);
	if (table == nullptr)
	{
		return false;
	}

	if (table->clear() == false)
	{
		table->close();
		return false;
	}

	int writtenRecordCount = 0;

	m_signalMutex.lock();

		writtenRecordCount = table->write(m_signalList.data(), m_signalList.count());

	m_signalMutex.unlock();

	table->close();

	if (writtenRecordCount != count())
	{
		return false;
	}

	qDebug() << "OutputSignalBase::save() - Written output signals: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::init()
{
	m_signalMutex.lock();

		int count = m_signalList.count();
		for(int i = 0; i < count; i++)
		{
			m_signalList[i].initMetrologySignal();
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::empty()
{
	m_signalMutex.lock();

		int count = m_signalList.count();
		for(int i = 0; i < count; i++)
		{
			m_signalList[i].clear();
		}

	m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::append(const OutputSignal& signal)
{
	int index = -1;

	m_signalMutex.lock();

		m_signalList.append(signal);
		index = m_signalList.count() - 1;

	m_signalMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal OutputSignalBase::signal(int index) const
{
	OutputSignal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::setSignal(int index, const OutputSignal& signal)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index] = signal;
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(int index)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList.remove(index);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::findIndex(int outputSignalType, int measureIoType, Metrology::Signal* pSignal)
{
	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (measureIoType < 0 || measureIoType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (pSignal == nullptr)
	{
		assert(pSignal != nullptr);
		return -1;
	}

	int foundIndex = -1;

	m_signalMutex.lock();

		int count = m_signalList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_signalList[i].type() != outputSignalType)
			{
				continue;
			}

			if (m_signalList[i].metrologySignal(measureIoType) != pSignal)
			{
				continue;
			}

			foundIndex = i;

			break;
		}

	m_signalMutex.unlock();

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::findIndex(const OutputSignal& signal)
{
	int foundIndex = -1;

		m_signalMutex.lock();

		int count = m_signalList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_signalList[i].hash() == signal.hash())
			{
				foundIndex = i;

				break;
			}
		 }

	m_signalMutex.unlock();

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase& OutputSignalBase::operator=(const OutputSignalBase& from)
{
	m_signalMutex.lock();

		m_signalList = from.m_signalList;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
