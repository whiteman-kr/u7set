#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
//
// SignalBase class implementation
//
// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
	QObject(parent)
{
}

void SignalBase::clear()
{
	clearSignalList();
	clearHashForRequestState();
}

int SignalBase::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

void SignalBase::clearSignalList()
{
	m_signalMutex.lock();

		m_signalHashMap.clear();
		m_signalList.clear();

	m_signalMutex.unlock();
}

int SignalBase::appendSignal(const AppSignal& param)
{
	if (param.appSignalID().isEmpty() == true || param.hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(param.hash()) == false)
		{
			m_signalList.append(TestSignal(param));
			index = m_signalList.count() - 1;

			m_signalHashMap.insert(param.hash(), index);
		}

	 m_signalMutex.unlock();

	return index;
}

TestSignal* SignalBase::signalPtr(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return nullptr;
	}

	return signalPtr(calcHash(appSignalID));
}

TestSignal* SignalBase::signalPtr(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return nullptr;
	}

	TestSignal* pSignal = nullptr;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				pSignal = &m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return pSignal;
}

TestSignal* SignalBase::signalPtr(int index)
{
	TestSignal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = &m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

TestSignal SignalBase::signal(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return TestSignal();
	}

	return signal(calcHash(appSignalID));
}

TestSignal SignalBase::signal(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return TestSignal();
	}

	TestSignal signal;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				signal = m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return signal;
}

TestSignal SignalBase::signal(int index)
{
	TestSignal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

AppSignal SignalBase::signalParam(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return AppSignal();
	}

	return signalParam(calcHash(appSignalID));
}

AppSignal SignalBase::signalParam(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return AppSignal();
	}

	AppSignal param;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				param = m_signalList[index].param();
			}
		}

	m_signalMutex.unlock();

	return param;
}

AppSignal SignalBase::signalParam(int index)
{
	AppSignal param;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			param = m_signalList[index].param();
		}

	m_signalMutex.unlock();

	return param;
}

void SignalBase::setSignalParam(const Hash& hash, const AppSignal& param)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return;
	}

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				m_signalList[index].setParam(param);

				emit updatedSignalParam(param.hash());
			}
		}

	m_signalMutex.unlock();
}

void SignalBase::setSignalParam(int index, const AppSignal& param)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index].setParam(param);

			emit updatedSignalParam(param.hash());
		}

	m_signalMutex.unlock();
}

AppSignalState SignalBase::signalState(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return AppSignalState();
	}

	return signalState(calcHash(appSignalID));
}

AppSignalState SignalBase::signalState(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return AppSignalState();
	}

	AppSignalState state;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				state = m_signalList[index].state();
			}
		}

	m_signalMutex.unlock();

	return state;
}

AppSignalState SignalBase::signalState(int index)
{
	AppSignalState state;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			state = m_signalList[index].state();
		}

	m_signalMutex.unlock();

	return state;
}

void SignalBase::setSignalState(const QString& appSignalID, const AppSignalState& state)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return;
	}

	setSignalState(calcHash(appSignalID), state);
}

void SignalBase::setSignalState(const Hash& hash, const AppSignalState &state)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return;
	}

	int index = -1;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				m_signalList[index].setState(state);
			}
		}

	m_signalMutex.unlock();
}

void SignalBase::setSignalState(int index, const AppSignalState& state)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index].setState(state);
		}

	m_signalMutex.unlock();
}


void SignalBase::clearHashForRequestState()
{
	m_stateMutex.lock();

		m_requestStateList.clear();

	m_stateMutex.unlock();
}

int SignalBase::hashForRequestStateCount() const
{
	int count = 0;

	m_stateMutex.lock();

		count = m_requestStateList.count();

	m_stateMutex.unlock();

	return count;
}

void SignalBase::appendHashForRequestState(const QVector<Hash>& hashList)
{
	m_stateMutex.lock();

		m_requestStateList.append(hashList);

	m_stateMutex.unlock();
}

Hash SignalBase::hashForRequestState(int index)
{
	Hash hash = UNDEFINED_HASH;

	m_stateMutex.lock();

		if (index >= 0 && index < m_requestStateList.count())
		{
			hash = m_requestStateList[index];
		}

	m_stateMutex.unlock();

	return hash;
}
