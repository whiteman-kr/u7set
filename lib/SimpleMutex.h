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
