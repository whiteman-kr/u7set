#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <type_traits>

static void sleep_ms(int MS)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(MS));
}

// -------------------- SpinLock basic semantics --------------------

TEST(SpinLockTest, BasicLockUnlock)
{
	SpinLock lock;
	std::atomic<int> value { 0 };

	lock.lock();
	value.fetch_add(1, std::memory_order_relaxed);
	lock.unlock();

	EXPECT_EQ(value.load(), 1);
}

TEST(SpinLockTest, TryLock_FreeThenBusySameThread)
{
	SpinLock lock;

	// First attempt should succeed
	EXPECT_TRUE(lock.tryLock());

	// While held by the same thread, tryLock must fail
	EXPECT_FALSE(lock.tryLock());

	lock.unlock();
}

TEST(SpinLockTest, TryLock_FailsWhenLockedByAnotherThread)
{
	SpinLock lock;
	std::atomic<bool> ready { false };

	std::thread t
		(
			[&]()
			{
				lock.lock();
				ready.store(true, std::memory_order_release);
				sleep_ms(50);
				lock.unlock();
			}
			);

	while (!ready.load(std::memory_order_acquire))
	{
		std::this_thread::yield();
	}

	EXPECT_FALSE(lock.tryLock()); // currently owned by other thread

	t.join();

	EXPECT_TRUE(lock.tryLock()); // free again
	lock.unlock();
}

TEST(SpinLockTest, MutualExclusionUnderContention)
{
	SpinLock lock;

	std::atomic<int> in_cs { 0 };
	std::atomic<int> max_in_cs { 0 };
	std::atomic<int> counter { 0 };

	const int THREADS_COUNT = 8;
	const int ITERS_PER_THREAD = 2000;

	std::vector<std::thread> workers;
	workers.reserve(THREADS_COUNT);

	for (int i = 0; i < THREADS_COUNT; ++i)
	{
		workers.emplace_back
			(
				[&]()
				{
					for (int k = 0; k < ITERS_PER_THREAD; ++k)
					{
						lock.lock();

						int cur = in_cs.fetch_add(1, std::memory_order_acq_rel) + 1;
						int prev_max = max_in_cs.load(std::memory_order_relaxed);
						while (cur > prev_max && !max_in_cs.compare_exchange_weak(prev_max, cur,
																				  std::memory_order_relaxed))
						{
							/* retry */
						}

						int temp = counter.load(std::memory_order_relaxed);
						counter.store(temp + 1, std::memory_order_relaxed);

						in_cs.fetch_sub(1, std::memory_order_acq_rel);

						lock.unlock();
					}
				}
				);
	}

	for (auto &th : workers)
	{
		th.join();
	}

	EXPECT_EQ(counter.load(), THREADS_COUNT * ITERS_PER_THREAD);
	EXPECT_EQ(max_in_cs.load(), 1); // at most one thread in CS
}

TEST(SpinLockTest, SameThread_TryLockWhileHoldingReturnsFalse)
{
	SpinLock lock;

	lock.lock();
	EXPECT_FALSE(lock.tryLock());
	lock.unlock();
}

// -------------------- SpinLockGuard semantics --------------------

TEST(SpinLockGuardTest, RAIIUnlocksOnScopeExit)
{
	SpinLock lock;

	// Prove it is free
	EXPECT_TRUE(lock.tryLock());
	lock.unlock();

	{
		SpinLockGuard guard(lock);
		EXPECT_FALSE(lock.tryLock()); // held in scope
	}

	// After scope, must be free again
	EXPECT_TRUE(lock.tryLock());
	lock.unlock();
}

// Compile-time traits: guard is neither copyable nor movable
static_assert(!std::is_copy_constructible_v<SpinLockGuard>,  "SpinLockGuard must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<SpinLockGuard>,     "SpinLockGuard must not be copy-assignable");
static_assert(!std::is_move_constructible_v<SpinLockGuard>,  "SpinLockGuard must not be move-constructible");
static_assert(!std::is_move_assignable_v<SpinLockGuard>,     "SpinLockGuard must not be move-assignable");

// -------------------- Death tests (qFatal scenarios) --------------------
// These require gtest death test support. They will launch the statement in a subprocess.

TEST(SpinLockDeathTest, UnlockFromNonOwnerCrashes)
{
	// Entire scenario runs in the death-test subprocess
	EXPECT_DEATH(
		[]{
			SpinLock lock;
			std::atomic<bool> ready { false };

			std::thread owner
				(
					[&]()
					{
						lock.lock();
						ready.store(true, std::memory_order_release);
						// keep it locked while the main thread calls unlock()
						sleep_ms(200);
					}
					);

			while (!ready.load(std::memory_order_acquire))
			{
				std::this_thread::yield();
			}

			// This thread is not the owner -> qFatal inside unlock()
			lock.unlock();

			// Should never get here
			owner.detach(); // avoid join in case of unexpected pass
		}(),
		""
		);
}

TEST(SpinLockDeathTest, RecursiveLockCrashes)
{
	EXPECT_DEATH(
		[]{
			SpinLock lock;
			lock.lock();
			// Second lock from the same thread must qFatal ("not recursive")
			lock.lock();
		}(),
		""
		);
}

TEST(SpinLockDeathTest, DestroyWhileLockedCrashes)
{
	EXPECT_DEATH(
		[]{
			auto ptr = std::make_unique<SpinLock>();
			ptr->lock();
			// Destruction while locked must qFatal
			ptr.reset();
		}(),
		""
		);
}

// -------------------- Macro helper test (optional) --------------------

TEST(SpinLockMacroTest, AutoLockMacroScopesCorrectly)
{
	SpinLock lock;

	{
		AUTO_LOCK_BY_CURRENT_THREAD(lock);
		EXPECT_FALSE(lock.tryLock());
	}

	EXPECT_TRUE(lock.tryLock());
	lock.unlock();
}
