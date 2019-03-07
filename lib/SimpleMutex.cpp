#include "SimpleMutex.h"

// -------------------------------------------------------------------------------------
//
// SimpleMutex class implementation
//
// -------------------------------------------------------------------------------------

SimpleMutex::SimpleMutex()
{
}

void SimpleMutex::lock()
{
	lock(QThread::currentThread());
}

void SimpleMutex::lock(const QThread* currentThread)
{
	bool result = false;

	do
	{
		const QThread* expectedOwner = nullptr;

		result = m_currentOwner.compare_exchange_strong(expectedOwner, currentThread);

		if (result == false)
		{
			QThread::yieldCurrentThread();
		}
	}
	while(result == false);
}

bool SimpleMutex::tryLock()
{
	return tryLock(QThread::currentThread());
}

bool SimpleMutex::tryLock(const QThread* currentThread)
{
	const QThread* expectedOwner = nullptr;

	return m_currentOwner.compare_exchange_strong(expectedOwner, currentThread);
}

void SimpleMutex::unlock()
{
	unlock(QThread::currentThread());
}

void SimpleMutex::unlock(const QThread* currentThread)
{
	const QThread* expectedOwner = currentThread;

	bool result = m_currentOwner.compare_exchange_strong(expectedOwner, nullptr);

	assert(result == true);

	Q_UNUSED(result);
}

// -------------------------------------------------------------------------------------
//
// SimpleMutexLocker class implementation
//
// -------------------------------------------------------------------------------------

SimpleMutexLocker::SimpleMutexLocker(SimpleMutex* mutex) :
	m_simpleMutex(mutex),
	m_currentThread(QThread::currentThread())
{
	if (m_simpleMutex == nullptr || m_currentThread == nullptr)
	{
		assert(false);
		return;
	}

	m_simpleMutex->lock(m_currentThread);
}

SimpleMutexLocker::SimpleMutexLocker(SimpleMutex* mutex, const QThread* currentThread) :
	m_simpleMutex(mutex),
	m_currentThread(currentThread)
{
	if (m_simpleMutex == nullptr || m_currentThread == nullptr)
	{
		assert(false);
		return;
	}

	m_simpleMutex->lock(m_currentThread);
}

SimpleMutexLocker::~SimpleMutexLocker()
{
	if (m_simpleMutex == nullptr || m_currentThread == nullptr)
	{
		assert(false);
		return;
	}

	m_simpleMutex->unlock(m_currentThread);
}
