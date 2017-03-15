#include "OutputSignalBase.h"

#include "Database.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal(const OutputSignal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::clear()
{
	m_hash = 0;

	m_type = OUTPUT_SIGNAL_TYPE_UNUSED;

	for(int k = 0; k < MEASURE_IO_SIGNAL_TYPE_COUNT; k++)
	{
		m_param[k].setAppSignalID(QString());
	}
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

bool OutputSignal::setHash()
{
	QString strID;

	for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		strID.append(m_appSignalID[type]);
	}

	if (strID.isEmpty() == true)
	{
		return false;
	}

	m_hash = calcHash(strID);
	if (m_hash == 0)
	{
		return false;
	}

	return true;
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

		setHash();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam OutputSignal::param(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return Metrology::SignalParam();
	}

	Metrology::SignalParam param;

	m_signalMutex.lock();

		param = m_param[type];

	m_signalMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setParam(int type, const Metrology::SignalParam& param)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	if (param.isValid() == false)\
	{
		return;
	}

	m_signalMutex.lock();

		m_appSignalID[type] = param.appSignalID();

		setHash();

		m_param[type] = param;

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::updateParam()
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

			m_param[type] = theSignalBase.signalParam(signalHash);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal& OutputSignal::operator=(const OutputSignal& from)
{
	m_signalID = from.m_signalID;
	m_hash = from.m_signalID;

	m_type = from.m_type;

	for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		m_appSignalID[type] = from.m_appSignalID[type];

		m_param[type] = from.m_param[type];
	}

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

int OutputSignalBase::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.size();

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

	if (writtenRecordCount != signalCount())
	{
		return false;
	}

	qDebug() << "OutputSignalBase::save() - Written output signals: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::appendSignal(const OutputSignal& signal)
{
	int index = -1;

	m_signalMutex.lock();

			m_signalList.append(signal);
			index = m_signalList.size() - 1;

	m_signalMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal OutputSignalBase::signal(int index) const
{
	OutputSignal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
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

		if (index >= 0 && index < m_signalList.size())
		{
			m_signalList[index] = signal;
		}

	m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(const OutputSignal& signal)
{
	int index = find(signal);

	return remove(index);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(int index)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
		{
			m_signalList.remove(index);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::find(int measureIoType, const Hash& hash, int outputSignalType)
{
	if (measureIoType < 0 || measureIoType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (hash == 0)
	{
		assert(hash != 0);
		return -1;
	}

	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	int foundIndex = -1;

	m_signalMutex.lock();

		int count = m_signalList.size();

		for(int i = 0; i < count; i ++)
		{
			const OutputSignal& signal = m_signalList[i];

			if (calcHash(m_signalList[i].appSignalID(measureIoType)) != hash)
			{
				continue;
			}

			if (signal.type() != outputSignalType)
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

int OutputSignalBase::find(const OutputSignal& signal)
{
	int foundIndex = -1;

		m_signalMutex.lock();

		int count = m_signalList.size();

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
