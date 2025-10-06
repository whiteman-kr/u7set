#pragma once

#include <atomic>

class SpinLock
{
	Q_DISABLE_COPY_MOVE(SpinLock)

public:
	SpinLock()
	{
	}

	void lock()	{ lock(getCurrentId()); }

	bool tryLock() { return tryLock(getCurrentId()); }

	void unlock() { unlock(getCurrentId()); }

private:
	void lock(quintptr currentId)
	{
		Q_ASSERT(currentId != 0);

		quintptr owner = m_ownerID.load(std::memory_order_relaxed);
		Q_ASSERT(owner != currentId);	//	SpinLock is not recursive!

		unsigned spin = 0;

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

			if (spin < 128)
			{
				_mm_pause();
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

	[[nodiscard]] bool tryLock(quintptr currentId)
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

		quintptr expected = currentId;

		bool ok = m_ownerID.compare_exchange_strong(
			expected, 0,
			std::memory_order_release,
			std::memory_order_relaxed);

		Q_ASSERT(ok);		// Unlock from non-owner thread
	}

	static inline quintptr getCurrentId()
	{
		return reinterpret_cast<quintptr>(QThread::currentThreadId());
	}

private:
	std::atomic<quintptr> m_ownerID { 0 };

	friend class SpinLockGuard;
};

class SpinLockGuard
{
public:
	explicit SpinLockGuard(SpinLock* mutex) :
		m_mutex(mutex),
		m_id(SpinLock::getCurrentId())
	{
		Q_ASSERT(m_mutex != nullptr);
		m_mutex->lock(m_id);
	}

	SpinLockGuard(SpinLock* mutex, quintptr id) :
		m_mutex(mutex), m_id(id)
	{
		Q_ASSERT(m_mutex != nullptr);
		Q_ASSERT(m_id != 0);
		m_mutex->lock(m_id);
	}

	SpinLockGuard(SpinLockGuard&& other) noexcept
		: m_mutex(other.m_mutex), m_id(other.m_id)
	{
		other.m_mutex = nullptr;
		other.m_id = 0;
	}

	SpinLockGuard(const SpinLockGuard&) = delete;

	SpinLockGuard& operator=(SpinLockGuard&& other) noexcept
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

	SpinLockGuard& operator=(const SpinLockGuard&) = delete;

	~SpinLockGuard()
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
	SpinLock* m_mutex {nullptr};
	quintptr m_id { 0 };
};

#define AUTO_LOCK_BY_CURRENT_THREAD(simpleMutex) SpinLockGuard s_l_g(&simpleMutex); Q_UNUSED(s_l_g);
