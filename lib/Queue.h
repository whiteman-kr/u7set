#pragma once
#include <QtCore>
#include <type_traits>
#include <cassert>
#include <atomic>

#include "SimpleMutex.h"

class QueueIndex
{
public:
	QueueIndex(int maxValue) :
		m_maxValue(maxValue)
	{
		assert(maxValue > 0);
	}

	int operator ++ (int)
	{
		m_index++;

		if (m_index == m_maxValue)
		{
			m_index = 0;
		}

		return m_index;
	}

	int operator += (int value)
	{
		m_index += value;

		if (m_index >= m_maxValue)
		{
			m_index %= m_maxValue;
		}

		return m_index;
	}

	int operator () () const { return m_index; }

	void reset() { m_index = 0; }
	void setMaxValue(int maxValue) { m_maxValue = maxValue; }
	void setValue(int value) { m_index = value; }

private:
	int m_index = 0;
	int m_maxValue = 0;
};

class QueueBase : public QObject
{
	Q_OBJECT

public:
	static const int MAX_QUEUE_MEMORY_SIZE = 50 * 1024 * 1024;			// 50 MBytes

public:
	QueueBase(QObject* parent, int itemSize, int queueSize);
	virtual ~QueueBase();

	int size() const { return m_size; }
	int maxSize() const { return m_maxSize; }

	bool isEmpty() const { return m_size == 0; }

	bool isNotEmpty() const { return m_size > 0; }

	bool isFull() const { return m_size == m_queueSize; }

	bool push(const char* item);
	bool pop(char* item);

	void clear();

	void lock() { m_mutex.lock(); }
	void unlock() { m_mutex.unlock(); }

	char* beginPush();
	bool completePush();

	char* beginPop();
	bool completePop();

	void resize(int newQueueSize);

signals:
	void queueNotEmpty();
	void queueFull();
	void queueEmpty();

protected:
	QMutex m_mutex;
	char* m_buffer = nullptr;

	int m_itemSize = 0;
	int m_queueSize = 0;

	int m_size = 0;								// current queue size
	int m_maxSize = 0;

	QueueIndex m_writeIndex;
	QueueIndex m_readIndex;

	int m_lostedCount = 0;
};


class LockFreeQueueBase
{
	//
	// One Writer - One Reader using only!!!
	//
public:
	static const int MAX_QUEUE_MEMORY_SIZE = 50 * 1024 * 1024;			// 50 MBytes

public:
	LockFreeQueueBase(int itemSize, int queueSize);
	virtual ~LockFreeQueueBase();

	bool push(const char* item);
	bool pop(char* item);

	char* beginPush();
	bool completePush();

	char* beginPop();
	bool completePop();

	void resize(int newQueueSize);					// not thread-safe operation!!!!

	bool isEmpty() const { return m_size.load() == 0; }
	bool isNotEmpty() const { return m_size.load() > 0; }
	bool isFull() const { return m_size.load() == m_queueSize; }
	int queueSize() const { return m_queueSize; }

	int size() const { return m_size.load(); }
	int maxSize() const { return m_maxSize.load(); }

private:
	char* m_buffer = nullptr;

	int m_itemSize = 0;
	int m_queueSize = 0;

	// vars modified by Writer only

	QueueIndex m_writeIndex;
	std::atomic<int> m_maxSize = { 0 };								// can be read from another thread
	int m_lostedCount = 0;

	// var modified by Reader only

	QueueIndex m_readIndex;

	// var modified both by Writer and Reader

	std::atomic<int> m_size = { 0 };								// current queue size
};


template <typename TYPE>
class Queue : public QueueBase
{
public:
	Queue(QObject* parent, int queueSize) :
		QueueBase(parent, sizeof(TYPE), queueSize)
	{
		// checking, that memcpy can be used to copy queue items of type TYPE
		//
		assert(std::is_trivially_copyable<TYPE>::value == true);
	}

	Queue(int queueSize) :
		QueueBase(nullptr, sizeof(TYPE), queueSize) 	{}

	virtual bool push(const TYPE* ptr) { return QueueBase::push(reinterpret_cast<const char*>(ptr)); }
	virtual bool pop(TYPE* ptr) { return QueueBase::pop(reinterpret_cast<char*>(ptr)); }

	TYPE* beginPush() { return reinterpret_cast<TYPE*>(QueueBase::beginPush()); }
	TYPE* beginPop() { return reinterpret_cast<TYPE*>(QueueBase::beginPop()); }
};


class QueueSignals : public QObject
{
	Q_OBJECT

signals:
	void notEmpty();
	void full();
	void empty();
};


template <typename TYPE>
class QueueOnList : public QueueSignals
{
public:
	QueueOnList(int maxSize, bool enableOverwrite);

	bool push(const TYPE& item);
	bool pop(TYPE* item);

	bool isEmpty();

	int size();

private:
	int m_maxSize = 0;
	bool m_enableOverwrite = false;

	QMutex m_mutex;

	QList<TYPE>	m_list;
};

template <typename TYPE>
QueueOnList<TYPE>::QueueOnList(int maxSize, bool enableOverwrite) :
	m_maxSize(maxSize),
	m_enableOverwrite(enableOverwrite)
{
	assert(m_maxSize > 0);
}

template <typename TYPE>
bool QueueOnList<TYPE>::push(const TYPE& item)
{
	bool result = true;

	m_mutex.lock();

	if (m_list.size() == m_maxSize)
	{
		if (m_enableOverwrite == true)
		{
			m_list.removeFirst();
			m_list.append(item);
		}
		else
		{
			result = false;
		}

		emit notEmpty();
		emit full();
	}
	else
	{
		m_list.append(item);

		emit notEmpty();

		if (m_list.size() == m_maxSize)
		{
			emit full();
		}
	}

	m_mutex.unlock();

	return result;
}


template <typename TYPE>
bool QueueOnList<TYPE>::pop(TYPE* item)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	bool result = true;

	m_mutex.lock();

	if (m_list.size() == 0)
	{
		result = false;
		emit empty();
	}
	else
	{
		*item = m_list.takeFirst();

		if (m_list.size() == 0)
		{
			emit empty();
		}
		else
		{
			emit notEmpty();
		}
	}

	m_mutex.unlock();

	return result;
}

template <typename TYPE>
bool QueueOnList<TYPE>::isEmpty()
{
	m_mutex.lock();

	bool empty = m_list.size() == 0;

	m_mutex.unlock();

	return empty;
}


template <typename TYPE>
class LockFreeQueue : public LockFreeQueueBase
{
public:
	LockFreeQueue(int queueSize) :
		LockFreeQueueBase(sizeof(TYPE), queueSize)
	{
		// checking, that memcpy can be used to copy queue items of type TYPE
		//
		assert(std::is_trivially_copyable<TYPE>::value == true);
	}

	virtual bool push(const TYPE* ptr) { return LockFreeQueueBase::push(reinterpret_cast<const char*>(ptr)); }
	virtual bool push(const TYPE& ref) { return LockFreeQueueBase::push(reinterpret_cast<const char*>(&ref)); }

	virtual bool pop(TYPE* ptr) { return LockFreeQueueBase::pop(reinterpret_cast<char*>(ptr)); }

	TYPE* beginPush() { return reinterpret_cast<TYPE*>(LockFreeQueueBase::beginPush()); }
	TYPE* beginPop() { return reinterpret_cast<TYPE*>(LockFreeQueueBase::beginPop()); }
};

template <typename T>
class FastThreadSafeQueue
{
	//
	// One Writer - One Reader using only!!!
	//
public:
	static const int MAX_QUEUE_MEMORY_SIZE = 50 * 1024 * 1024;			// 50 MBytes
	static const int MIN_QUEUE_SIZE = 3;

public:
	FastThreadSafeQueue(int queueSize);
	virtual ~FastThreadSafeQueue();

	void resize(int newSize);

	void push(const T& item, const QThread* thread, int* curSize = nullptr, int* curMaxSize = nullptr);
	bool pop(T* item, const QThread* thread);
	bool peekAt(int index, T* item, const QThread* thread);

	bool isFull(const QThread* thread) const;
	bool isEmpty(const QThread* thread) const;

	int queueSize(const QThread* thread) const;
	int size(const QThread* thread) const;
	int maxSize(const QThread* thread) const;

	T* beginPush(const QThread* thread);
	void completePush(const QThread* thread, int* curSize = nullptr, int* curMaxSize = nullptr);

	T* beginPop(const QThread* thread);
	void completePop(const QThread* thread);

	bool copyToBuffer(T* buffer, int bufferSizeInItems, int* copiedItemsCount, const QThread* thread);
	void nonDestructiveResize(int newQueueSize, const QThread* thread);

	void getSizes(int* curSize, int* curMaxSize, int* queueSize, const QThread* thread);

	void resetMaxSize(const QThread* thread);

	void clear(const QThread* thread);

private:
	int checkQueueSize(int newSize) const;
	void nonLockResizing(int newSize);

private:
	T* m_buffer = nullptr;
	int m_queueSize = 0;

	mutable SimpleMutex m_mutex;

	QueueIndex m_writeIndex;
	QueueIndex m_readIndex;
	int m_size = 0;								// current queue size

	qint64 m_lostedCount = 0;
	int m_maxSize = 0;

	bool m_pushIsBegan = false;
	bool m_popIsBegan = false;
};

template <typename T>
FastThreadSafeQueue<T>::FastThreadSafeQueue(int queueSize) :
	m_queueSize(1),
	m_readIndex(1),
	m_writeIndex(1)
{
	static_assert(std::is_trivially_copyable<T>::value == true);
	resize(queueSize);
}

template <typename T>
FastThreadSafeQueue<T>::~FastThreadSafeQueue()
{
	AUTO_LOCK_BY_CURRENT_THREAD(m_mutex);

	delete [] m_buffer;
}

template <typename T>
void FastThreadSafeQueue<T>::resize(int newSize)
{
	AUTO_LOCK_BY_CURRENT_THREAD(m_mutex);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	nonLockResizing(newSize);
}

template <typename T>
void FastThreadSafeQueue<T>::push(const T& item, const QThread* thread, int* curSize, int* curMaxSize)
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	if (m_size == m_queueSize)
	{
		// first queued item will be lost here
		//
		m_lostedCount++;

		m_readIndex++;
		m_size--;
	}

	memcpy(m_buffer + m_writeIndex(), &item, sizeof(T));

	m_writeIndex++;
	m_size++;

	if (curSize != nullptr)
	{
		*curSize = m_size;
	}

	if (m_size > m_maxSize)
	{
		m_maxSize = m_size;
	}

	if (curMaxSize != nullptr)
	{
		*curMaxSize = m_maxSize;
	}
}

template <typename T>
bool FastThreadSafeQueue<T>::pop(T* item, const QThread* thread)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	if (m_size == 0)
	{
		return false;
	}

	memcpy(item, m_buffer + m_readIndex(), sizeof(T));

	m_readIndex++;
	m_size--;

	return true;
}

template <typename T>
bool FastThreadSafeQueue<T>::peekAt(int index, T* item, const QThread* thread)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	if (index < 0 || index >= m_size)
	{
		assert(false);
		return false;
	}

	QueueIndex readIndex = m_readIndex;

	readIndex += index;

	memcpy(item, m_buffer + readIndex(), sizeof(T));

	return true;
}

template <typename T>
bool FastThreadSafeQueue<T>::isFull(const QThread* thread) const
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	return m_size == m_queueSize;
}

template <typename T>
bool FastThreadSafeQueue<T>::isEmpty(const QThread* thread) const
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	return m_size == 0;
}

template <typename T>
int FastThreadSafeQueue<T>::queueSize(const QThread* thread) const
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	return m_queueSize;
}


template <typename T>
int FastThreadSafeQueue<T>::size(const QThread* thread) const
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	return 	m_size;
}

template <typename T>
int FastThreadSafeQueue<T>::maxSize(const QThread* thread) const
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	return 	m_maxSize;
}

template <typename T>
T* FastThreadSafeQueue<T>::beginPush(const QThread* thread)
{
	m_mutex.lock(thread);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	m_pushIsBegan = true;

	if (m_size == m_queueSize)
	{
		// first queued item will be lost here
		//
		m_lostedCount++;

		m_readIndex++;
		m_size--;
	}

	return m_buffer + m_writeIndex();
}

template <typename T>
void FastThreadSafeQueue<T>::completePush(const QThread* thread, int* curSize, int* curMaxSize)
{
	assert(m_pushIsBegan == true);
	assert(m_popIsBegan == false);

	m_writeIndex++;
	m_size++;

	if (curSize != nullptr)
	{
		*curSize = m_size;
	}

	if (m_size > m_maxSize)
	{
		m_maxSize = m_size;
	}

	if (curMaxSize != nullptr)
	{
		*curMaxSize = m_maxSize;
	}

	m_pushIsBegan = false;

	m_mutex.unlock(thread);
}

template <typename T>
T* FastThreadSafeQueue<T>::beginPop(const QThread* thread)
{
	m_mutex.lock(thread);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	if (m_size == 0)
	{
		m_mutex.unlock(thread);
		return nullptr;
	}

	m_popIsBegan = true;

	return m_buffer + m_readIndex();
}

template <typename T>
void FastThreadSafeQueue<T>::completePop(const QThread* thread)
{
	assert(m_popIsBegan == true);
	assert(m_pushIsBegan == false);

	m_readIndex++;
	m_size--;

	m_popIsBegan = false;

	m_mutex.unlock(thread);
}

template <typename T>
bool FastThreadSafeQueue<T>::copyToBuffer(T* buffer, int bufferSizeInItems, int* copiedItemsCount, const QThread* thread)
{
	if (buffer == nullptr || copiedItemsCount == nullptr)
	{
		assert(false);
		return false;
	}

	if (bufferSizeInItems <= 0)
	{
		assert(false);
		return false;
	}

	SimpleMutexLocker locker(&m_mutex, thread);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	*copiedItemsCount = 0;

	if (m_size == 0)
	{
		return false;
	}

	int itemsToCopy = bufferSizeInItems;

	if (m_size < itemsToCopy)
	{
		itemsToCopy = m_size;
	}

	if (m_readIndex() < m_writeIndex())
	{
		memcpy(buffer, m_buffer + m_readIndex(), itemsToCopy * sizeof(T));
	}
	else
	{
		int firstPartSize = m_queueSize - m_readIndex();

		memcpy(buffer, m_buffer + m_readIndex(), firstPartSize * sizeof(T));

		int secondPartSize = itemsToCopy - firstPartSize;

		if (secondPartSize > 0)
		{
			memcpy(buffer + firstPartSize, m_buffer, secondPartSize * sizeof(T));
		}
	}

	m_readIndex += itemsToCopy;
	m_size -= itemsToCopy;

	if (m_size == 0)
	{
		m_readIndex.reset();
		m_writeIndex.reset();
	}

	*copiedItemsCount = itemsToCopy;

	return true;
}

template <typename T>
void FastThreadSafeQueue<T>::nonDestructiveResize(int newQueueSize, const QThread* thread)
{
	SimpleMutexLocker locker(&m_mutex, thread);

	assert(m_pushIsBegan == false);
	assert(m_popIsBegan == false);

	if (newQueueSize == m_queueSize)
	{
		return;
	}

	if (m_size == 0)
	{
		nonLockResizing(newQueueSize);
		return;
	}

	newQueueSize = checkQueueSize(newQueueSize);

	// data copying required
	//

	int dSize = newQueueSize - m_size;

	if (dSize < 0)
	{
		// queue compression
		// if newQueueSize < m_size (dSize < 0), remove items from beginng of queue
		//
		m_readIndex += abs(dSize);
		m_size -= abs(dSize);
	}

	T* newBuffer = new T[newQueueSize];

	if (m_readIndex() < m_writeIndex())
	{
		memcpy(newBuffer, m_buffer + m_readIndex(), m_size * sizeof(T));
	}
	else
	{
		int firstPartSize = m_queueSize - m_readIndex();

		assert(firstPartSize > 0);

		memcpy(newBuffer, m_buffer + m_readIndex(), firstPartSize * sizeof(T));

		int secondPartSize = m_size - firstPartSize;

		if (secondPartSize > 0)
		{
			memcpy(newBuffer + firstPartSize, m_buffer + 0, secondPartSize * sizeof(T));
		}
	}

	delete [] m_buffer;
	m_buffer = newBuffer;

	m_readIndex.reset();
	m_readIndex.setMaxValue(newQueueSize);

	m_writeIndex.setValue(m_size);
	m_writeIndex.setMaxValue(newQueueSize);

	m_queueSize = newQueueSize;
	m_maxSize = m_size;
	m_lostedCount = 0;
}

template <typename T>
void FastThreadSafeQueue<T>::getSizes(int* curSize, int* curMaxSize, int* queueSize, const QThread* thread)
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	if (curSize != nullptr)
	{
		*curSize = m_size;
	}

	if (curMaxSize != nullptr)
	{
		*curMaxSize = m_maxSize;
	}

	if (queueSize != nullptr)
	{
		*queueSize = m_queueSize;
	}
}

template <typename T>
void FastThreadSafeQueue<T>::resetMaxSize(const QThread* thread)
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);
	m_maxSize = 0;
}

template <typename T>
int FastThreadSafeQueue<T>::checkQueueSize(int newSize) const
{
	if (newSize * sizeof(T) > MAX_QUEUE_MEMORY_SIZE)
	{
		assert(false);
		newSize = MAX_QUEUE_MEMORY_SIZE / sizeof(T);
	}

	if (newSize < MIN_QUEUE_SIZE)
	{
		assert(false);
		newSize = MIN_QUEUE_SIZE;
	}

	return newSize;
}

template <typename T>
void FastThreadSafeQueue<T>::clear(const QThread* thread)
{
	AUTO_LOCK_BY_THREAD(m_mutex, thread);

	m_readIndex.reset();
	m_writeIndex.reset();
	m_size = 0;
	m_maxSize = 0;
	m_lostedCount = 0;
}

template <typename T>
void FastThreadSafeQueue<T>::nonLockResizing(int newSize)
{
	m_queueSize = checkQueueSize(newSize);

	if (m_buffer != nullptr)
	{
		delete [] m_buffer;
		m_buffer = nullptr;
	}

	m_readIndex.setMaxValue(m_queueSize);
	m_readIndex.reset();

	m_writeIndex.setMaxValue(m_queueSize);
	m_writeIndex.reset();

	m_buffer = new T[m_queueSize];

	m_size = 0;
	m_maxSize = 0;
	m_lostedCount = 0;
}
