#pragma once

#include <QThread>

class SimpleMutex
{
public:
	SimpleMutex();

	void lock();
	void lock(const QThread* currentThread);

	bool tryLock();
	bool tryLock(const QThread* currentThread);

	void unlock();
	void unlock(const QThread* currentThread);

private:
	std::atomic<const QThread*> m_currentOwner = { nullptr };
};

class SimpleMutexLocker
{
public:
	SimpleMutexLocker(SimpleMutex* mutex);
	SimpleMutexLocker(SimpleMutex* mutex, const QThread* currentThread);

	~SimpleMutexLocker();

private:
	SimpleMutex* m_simpleMutex;
	const QThread* m_currentThread = nullptr;
};


#define AUTO_LOCK_BY_THREAD(simpleMutex, thread) SimpleMutexLocker __simpleMutexLocker(&simpleMutex, thread); Q_UNUSED(__simpleMutexLocker);

#define AUTO_LOCK_BY_CURRENT_THREAD(simpleMutex) SimpleMutexLocker __simpleMutexLocker(&simpleMutex); Q_UNUSED(__simpleMutexLocker);

// -------------------------------------------------------------------------------------
//
// SimpleMutex class implementation
//
// -------------------------------------------------------------------------------------

inline SimpleMutex::SimpleMutex()
{
}

inline void SimpleMutex::lock()
{
	lock(QThread::currentThread());
}

inline void SimpleMutex::lock(const QThread* currentThread)
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

inline bool SimpleMutex::tryLock()
{
	return tryLock(QThread::currentThread());
}

inline bool SimpleMutex::tryLock(const QThread* currentThread)
{
	const QThread* expectedOwner = nullptr;

	return m_currentOwner.compare_exchange_strong(expectedOwner, currentThread);
}

inline void SimpleMutex::unlock()
{
	unlock(QThread::currentThread());
}

inline void SimpleMutex::unlock(const QThread* currentThread)
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

inline SimpleMutexLocker::SimpleMutexLocker(SimpleMutex* mutex) :
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

inline SimpleMutexLocker::SimpleMutexLocker(SimpleMutex* mutex, const QThread* currentThread) :
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

inline SimpleMutexLocker::~SimpleMutexLocker()
{
	if (m_simpleMutex == nullptr || m_currentThread == nullptr)
	{
		assert(false);
		return;
	}

	m_simpleMutex->unlock(m_currentThread);
}

