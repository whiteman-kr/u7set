#pragma once

#include <atomic>
#include <QtCore/QThread>

#if defined(Q_OS_WIN)
#include <intrin.h>
#endif

#include "WUtils.h"

class SpinLock
{
	Q_DISABLE_COPY_MOVE(SpinLock)

public:
	SpinLock()
	{
	}

	~SpinLock() noexcept
	{
		// The lock must not be held when the SpinLock object is destroyed.
		//
		const quintptr owner = m_ownerID.load(std::memory_order_relaxed);

		if (owner != 0)
		{
			qFatal("SpinLock is locked on destruction");
		}
	}

	void lock()
	{
		quintptr currentId = currentThreadId();

		Q_ASSERT(currentId != 0);

		quintptr owner = m_ownerID.load(std::memory_order_relaxed);

		if (owner == currentId)
		{
			qFatal("SpinLock is not recursive");
		}

		unsigned spin = 0;

		constexpr unsigned SHORT_SPIN_CTR = 128;
		constexpr unsigned YIELD_SPIN_CTR = 256;

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

			if (spin < SHORT_SPIN_CTR)
			{
#if defined(Q_CC_MSVC)
				_mm_pause();
#elif (defined(Q_CC_GNU) || defined(Q_CC_CLANG)) && defined(Q_PROCESSOR_X86)
				__builtin_ia32_pause();
#else
				QThread::yieldCurrentThread();
#endif
				continue;	// short active spin
			}

			if (spin < YIELD_SPIN_CTR)
			{
				QThread::yieldCurrentThread();
				continue;
			}

			QThread::msleep(1);
			spin = 0;
		}
	}

	[[nodiscard]] bool tryLock()
	{
		quintptr currentId = currentThreadId();

		quintptr expected = 0;

		return m_ownerID.compare_exchange_strong(expected, currentId,
												 std::memory_order_acquire,
												 std::memory_order_relaxed);
	}

	void unlock()
	{
		quintptr expected = currentThreadId();

		if (m_ownerID.compare_exchange_strong(
			expected, 0,
			std::memory_order_release,
			std::memory_order_relaxed) == false)
		{
			qFatal("SpinLock::unlock() from non-owner thread");
		}
	}

private:
	std::atomic<quintptr> m_ownerID { 0 };
};

class SpinLockGuard
{
public:
	explicit SpinLockGuard(SpinLock& spinLock) :
		m_spinLock(spinLock)
	{
		m_spinLock.lock();
	}

	// No copy or move allowed
	SpinLockGuard(const SpinLockGuard&) = delete;
	SpinLockGuard& operator=(const SpinLockGuard&) = delete;
	SpinLockGuard(SpinLockGuard&&) = delete;
	SpinLockGuard& operator=(SpinLockGuard&&) = delete;

	~SpinLockGuard() noexcept
	{
		m_spinLock.unlock();
	}

private:
	SpinLock& m_spinLock;
};

#define SLG_CONCAT_IMPL(a, b) a##b
#define SLG_CONCAT(a, b) SLG_CONCAT_IMPL(a,b)

#define AUTO_LOCK_BY_CURRENT_THREAD(spinLock)  \
			[[maybe_unused]] SpinLockGuard SLG_CONCAT(slg_, __LINE__)(spinLock)
