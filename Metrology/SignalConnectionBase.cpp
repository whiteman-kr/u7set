#include "SignalConnectionBase.h"

#include "Database.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalConnection::SignalConnection()
{
	m_handle.uint64 = 0;
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
	if (m_handle.type < 0 || m_handle.type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return false;
	}

	bool result = true;

	for(int t = 0; t < MEASURE_IO_SIGNAL_TYPE_COUNT; t++)
	{
		if (m_pSignal[t] == nullptr)
		{
			result = false;
			break;
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::clear()
{
	for(int t = 0; t < MEASURE_IO_SIGNAL_TYPE_COUNT; t++)
	{
		m_pSignal[t] = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::createHandle()
{
	for (int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		switch (type)
		{
			case MEASURE_IO_SIGNAL_TYPE_INPUT:

				if (m_pSignal[type] == nullptr)
				{
					m_handle.inputID = 0;
				}
				else
				{
					m_handle.inputID = static_cast<quint64>(m_pSignal[type]->param().ID());
				}

				break;

			case MEASURE_IO_SIGNAL_TYPE_OUTPUT:

				if (m_pSignal[type] == nullptr)
				{
					m_handle.outputID = 0;
				}
				else
				{
					m_handle.outputID = static_cast<quint64>(m_pSignal[type]->param().ID());
				}

				break;

			default:
				assert(0);
				break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalConnection::typeStr() const
{
	if (m_handle.type < 0 || m_handle.type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return QString("???");
	}

	return qApp->translate("SignalConnectionBase.h", SignalConnectionType[m_handle.type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalConnection::appSignalID(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	return m_appSignalID[type];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::setAppSignalID(int type, const QString& appSignalID)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	m_appSignalID[type] = appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* SignalConnection::signal(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return nullptr;
	}

	return m_pSignal[type];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::setSignal(int type, Metrology::Signal* pSignal)
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

	m_appSignalID[type] = param.appSignalID();

	m_pSignal[type] = pSignal;

	createHandle();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnection::initSignals()
{
	for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		if (m_appSignalID[type].isEmpty() == true)
		{
			continue;
		}

		m_pSignal[type] = theSignalBase.signalPtr(m_appSignalID[type]);
	}

	createHandle();
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnection& SignalConnection::operator=(const SignalConnection& from)
{
	m_index = from.m_index;

	m_handle = from.m_handle;

	for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		m_appSignalID[type] = from.m_appSignalID[type];
		m_pSignal[type] = from.m_pSignal[type];
	}

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
	QMutexLocker l(&m_connectionMutex);

	m_connectionList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::count() const
{
	QMutexLocker l(&m_connectionMutex);

	return m_connectionList.count();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::sort()
{
	QMutexLocker l(&m_connectionMutex);

	int connectionCount = m_connectionList.count();
	for( int i = 0; i < connectionCount - 1; i++ )
	{
		for( int j = i+1; j < connectionCount; j++ )
		{
			if ( m_connectionList[i].handle().uint64 > m_connectionList[j].handle().uint64 )
			{
				SignalConnection connection = m_connectionList[i];
				m_connectionList[i] = m_connectionList[j];
				m_connectionList[j] = connection;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::load()
{
	QElapsedTimer responseTime;
	responseTime.start();

	SqlTable* table = theDatabase.openTable(SQL_TABLE_SIGNAL_CONNECTION);
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

	qDebug() << __FUNCTION__ <<
				"Loaded signal connections: " <<
				readedRecordCount <<
				", Time for load: " <<
				responseTime.elapsed() <<
				" ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalConnectionBase::save()
{
	SqlTable* table = theDatabase.openTable(SQL_TABLE_SIGNAL_CONNECTION);
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

	qDebug() << __FUNCTION__ << "Written signal Connections: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::initSignals()
{
	QMutexLocker l(&m_connectionMutex);

	int count = m_connectionList.count();
	for(int i = 0; i < count; i++)
	{
		m_connectionList[i].initSignals();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::clearSignals()
{
	QMutexLocker l(&m_connectionMutex);

	int count = m_connectionList.count();
	for(int i = 0; i < count; i++)
	{
		m_connectionList[i].clear();
	}
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::append(const SignalConnection& connection)
{
	QMutexLocker l(&m_connectionMutex);

	m_connectionList.append(connection);

	return m_connectionList.count() - 1;
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnection SignalConnectionBase::connection(int index) const
{
	QMutexLocker l(&m_connectionMutex);

	if (index < 0 || index >= m_connectionList.count())
	{
		return SignalConnection();
	}

	return m_connectionList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::setConnection(int index, const SignalConnection& connection)
{
	QMutexLocker l(&m_connectionMutex);

	if (index < 0 || index >= m_connectionList.count())
	{
		return;
	}

	m_connectionList[index] = connection;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalConnectionBase::remove(int index)
{
	QMutexLocker l(&m_connectionMutex);

	if (index < 0 || index >= m_connectionList.count())
	{
		return;
	}

	m_connectionList.remove(index);
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::findIndex(int ioType, Metrology::Signal* pSignal) const
{
	if (ioType < 0 || ioType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (pSignal == nullptr)
	{
		assert(0);
		return -1;
	}

	int foundIndex = -1;

	QMutexLocker l(&m_connectionMutex);

	int count = m_connectionList.count();

	for(int i = 0; i < count; i ++)
	{
		if (m_connectionList[i].signal(ioType) != pSignal)
		{
			continue;
		}

		foundIndex = i;

		break;
	}

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::findIndex(int connectionType, int ioType, Metrology::Signal* pSignal) const
{
	if (connectionType < 0 || connectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (ioType < 0 || ioType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (pSignal == nullptr)
	{
		assert(0);
		return -1;
	}

	int foundIndex = -1;

	QMutexLocker l(&m_connectionMutex);

	int count = m_connectionList.count();

	for(int i = 0; i < count; i ++)
	{
		if (m_connectionList[i].type() != connectionType)
		{
			continue;
		}

		if (m_connectionList[i].signal(ioType) != pSignal)
		{
			continue;
		}

		foundIndex = i;

		break;
	}

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalConnectionBase::findIndex(const SignalConnection& connection) const
{
	int foundIndex = -1;

	QMutexLocker l(&m_connectionMutex);

	int count = m_connectionList.count();

	for(int i = 0; i < count; i ++)
	{
		if (m_connectionList[i].handle().uint64 == connection.handle().uint64)
		{
			foundIndex = i;

			break;
		}
	}

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

QVector<Metrology::Signal*> SignalConnectionBase::getOutputSignals(int connectionType, const QString& InputAppSignalID) const
{
	if (connectionType < 0 || connectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return QVector<Metrology::Signal*>();
	}

	if (InputAppSignalID.isEmpty() == true)
	{
		return QVector<Metrology::Signal*>();
	}

	QVector<Metrology::Signal*> outputSignalsList;

	QMutexLocker l(&m_connectionMutex);

	int count = m_connectionList.count();

	for(int i = 0; i < count; i ++)
	{
		const SignalConnection& connection = m_connectionList[i];

		if (connection.type() != connectionType)
		{
			continue;
		}

		if (connection.appSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT) == InputAppSignalID)
		{
			Metrology::Signal* pOutputSignal = m_connectionList[i].signal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
			if (pOutputSignal == nullptr || pOutputSignal->param().isValid() == false)
			{
				continue;
			}

			outputSignalsList.append(pOutputSignal);
		}
	}

	return outputSignalsList;
}

// -------------------------------------------------------------------------------------------------------------------

SignalConnectionBase& SignalConnectionBase::operator=(const SignalConnectionBase& from)
{
	QMutexLocker l(&m_connectionMutex);

	m_connectionList = from.m_connectionList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
