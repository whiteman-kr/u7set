#include "SignalConnectionBase.h"

#include "Database.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalConnection::SignalConnection() :
	m_hash(0)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnection::SignalConnection(const SignalConnection& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalConnection::isValid() const
{
	if (m_hash == UNDEFINED_HASH)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::clear()
{
	m_signalMutex.lock();

		for(int t = 0; t < MEASURE_IO_SIGNAL_TYPE_COUNT; t++)
		{
			m_pSignal[t] = nullptr;
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::setHash()
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

QString SignalConnection::typeStr() const
{
	if (m_type < 0 || m_type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return QString();
	}

	return SignalConnectionType[m_type];
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalConnection::appSignalID(int type) const
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

void SignalConnection::setAppSignalID(int type, const QString& appSignalID)
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

Metrology::Signal* SignalConnection::metrologySignal(int type) const
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

void SignalConnection::setMetrologySignal(int type, Metrology::Signal* pSignal)
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

void SignalConnection::initMetrologySignal()
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

SignalConnection& SignalConnection::operator=(const SignalConnection& from)
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

SignalConnectionBase::SignalConnectionBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::clear()
{
	m_connectionMutex.lock();

		m_connectionList.clear();

	m_connectionMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::count() const
{
	int count = 0;

	m_connectionMutex.lock();

		count = m_connectionList.count();

	m_connectionMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::sort()
{

}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::load()
{
	if (thePtrDB == nullptr)
	{
		return 0;
	}

	QTime responseTime;
	responseTime.start();

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_SIGNAL_CONNECTION);
	if (table == nullptr)
	{
		return false;
	}

	int readedRecordCount = 0;

	m_connectionMutex.lock();

		m_connectionList.resize(table->recordCount());

		readedRecordCount = table->read(m_connectionList.data());

	m_connectionMutex.unlock();

	table->close();

	qDebug() << "SignalConnectionBase::load() - Loaded signal connections: " << readedRecordCount << ", Time for load: " << responseTime.elapsed() << " ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalConnectionBase::save()
{
	if (thePtrDB == nullptr)
	{
		return false;
	}

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_SIGNAL_CONNECTION);
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

	m_connectionMutex.lock();

		writtenRecordCount = table->write(m_connectionList.data(), m_connectionList.count());

	m_connectionMutex.unlock();

	table->close();

	if (writtenRecordCount != count())
	{
		return false;
	}

	qDebug() << "SignalConnectionBase::save() - Written signal Connections: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::init()
{
	m_connectionMutex.lock();

		int count = m_connectionList.count();
		for(int i = 0; i < count; i++)
		{
			m_connectionList[i].initMetrologySignal();
		}

	m_connectionMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::empty()
{
	m_connectionMutex.lock();

		int count = m_connectionList.count();
		for(int i = 0; i < count; i++)
		{
			m_connectionList[i].clear();
		}

	m_connectionMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::append(const SignalConnection& connection)
{
	int index = -1;

	m_connectionMutex.lock();

		m_connectionList.append(connection);
		index = m_connectionList.count() - 1;

	m_connectionMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnection SignalConnectionBase::connection(int index) const
{
	SignalConnection signal;

	m_connectionMutex.lock();

		if (index >= 0 && index < m_connectionList.count())
		{
			signal = m_connectionList[index];
		}

	m_connectionMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::setSignal(int index, const SignalConnection& connection)
{
	m_connectionMutex.lock();

		if (index >= 0 && index < m_connectionList.count())
		{
			m_connectionList[index] = connection;
		}

	m_connectionMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::remove(int index)
{
	m_connectionMutex.lock();

		if (index >= 0 && index < m_connectionList.count())
		{
			m_connectionList.remove(index);
		}

	m_connectionMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::findIndex(int signalConnectionType, int measureIoType, Metrology::Signal* pSignal) const
{
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
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

	m_connectionMutex.lock();

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_connectionList[i].type() != signalConnectionType)
			{
				continue;
			}

			if (m_connectionList[i].metrologySignal(measureIoType) != pSignal)
			{
				continue;
			}

			foundIndex = i;

			break;
		}

	m_connectionMutex.unlock();

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::findIndex(const SignalConnection& signal) const
{
	int foundIndex = -1;

		m_connectionMutex.lock();

		int count = m_connectionList.count();

		for(int i = 0; i < count; i ++)
		{
			if (m_connectionList[i].hash() == signal.hash())
			{
				foundIndex = i;

				break;
			}
		 }

	m_connectionMutex.unlock();

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionBase& SignalConnectionBase::operator=(const SignalConnectionBase& from)
{
	m_connectionMutex.lock();

		m_connectionList = from.m_connectionList;

	m_connectionMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
