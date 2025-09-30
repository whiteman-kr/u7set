#pragma once

#include <atomic>

class SimpleMutex
{
public:
	SimpleMutex();

	void lock()	{ lock(getCurrentId()); }
	bool tryLock() { tryLock(getCurrentId()); }
	void unlock() { unlock(getCurrentId()); }

private:
	void lock(quintptr currentId)
	{
		Q_ASSERT(currentId != 0);

		quintptr owner = m_ownerID.load(std::memory_order_relaxed);
		Q_ASSERT(owner != currentId);	//	SimpleMutex is not recursive!

		quint64 spin = 0;

		for (;;)
		{
			quintptr expected = 0;

			if (m_ownerID.compare_exchange_weak(expected, currentId,
												std::memory_order_acquire,   // success
												std::memory_order_relaxed))  // failure
			{
				return;
			}

			spin++;

			if (spin < 64)
			{
				continue;	// short active spin
			}

			if (spin < 256)
			{
				QThread::yieldCurrentThread();
				continue;
			}

			QThread::msleep(1);
			spin = 0;
		}
	}

	bool tryLock(quintptr currentId)
	{
		Q_ASSERT(currentId != 0);

		quintptr expected = 0;

		return m_ownerID.compare_exchange_strong(expected, currentId,
												std::memory_order_acquire,
												std::memory_order_relaxed);
	}

	void unlock(quintptr currentId)
	{
		Q_ASSERT(currentId != 0);

		quintptr owner = m_ownerID.load(std::memory_order_relaxed);

		Q_ASSERT(owner == currentId);	// Unlock from non-owner thread

		m_ownerID.store(0, std::memory_order_release);
	}

	static inline quintptr getCurrentId()
	{
		return reinterpret_cast<quintptr>(QThread::currentThreadId());
	}

private:
	std::atomic<quintptr> m_ownerID { 0 };

	friend class SimpleMutexLocker;
};

class SimpleMutexLocker
{
public:
	explicit SimpleMutexLocker(SimpleMutex* mutex) :
		m_mutex(mutex),
		m_id(SimpleMutex::getCurrentId())
	{
		Q_ASSERT(m_mutex != nullptr);
		m_mutex->lock(m_id);
	}

	SimpleMutexLocker(SimpleMutex* mutex, quintptr id) :
		m_mutex(mutex), m_id(id)
	{
		Q_ASSERT(m_mutex != nullptr);
		Q_ASSERT(m_id != 0);
		m_mutex->lock(m_id);
	}

	SimpleMutexLocker(SimpleMutex* mutex, const QThread* thread) :
		m_mutex(mutex)
	{
		Q_ASSERT(m_mutex != nullptr);
		Q_ASSERT(thread != nullptr);
		m_id = reinterpret_cast<quintptr>(thread->currentThreadId());
		m_mutex->lock(m_id);
	}

	SimpleMutexLocker(SimpleMutexLocker&& other) noexcept
		: m_mutex(other.m_mutex), m_id(other.m_id)
	{
		other.m_mutex = nullptr;
		other.m_id = 0;
	}

	SimpleMutexLocker(const SimpleMutexLocker&) = delete;

	SimpleMutexLocker& operator=(SimpleMutexLocker&& other) noexcept
	{
		if (this != &other)
		{
			release();
			m_mutex = other.m_mutex;
			m_id = other.m_id;
			other.m_mutex = nullptr;
			other.m_id = 0;
		}
		return *this;
	}

	SimpleMutexLocker& operator=(const SimpleMutexLocker&) = delete;

	~SimpleMutexLocker()
	{
		release();
	}

private:
	void release()
	{
		if (m_mutex != nullptr && m_id != 0)
		{
			m_mutex->unlock(m_id);
			m_mutex = nullptr;
			m_id = 0;
		}
	}

private:
	SimpleMutex* m_mutex {nullptr};
	quintptr m_id { 0 };
};


#define AUTO_LOCK_BY_THREAD(simpleMutex, thread) SimpleMutexLocker __simpleMutexLocker(&simpleMutex, thread); Q_UNUSED(__simpleMutexLocker);

#define AUTO_LOCK_BY_CURRENT_THREAD(simpleMutex) SimpleMutexLocker __simpleMutexLocker(&simpleMutex); Q_UNUSED(__simpleMutexLocker);


