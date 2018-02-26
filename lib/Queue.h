#pragma once
#include <QtCore>
#include <type_traits>
#include <cassert>


class QueueBase : public QObject
{
	Q_OBJECT

	class QueueIndex
	{
	private:
		int m_index = 0;
		int m_maxValue = 0;

	public:
		QueueIndex(int maxValue) :
			m_maxValue(maxValue) {}

		int operator ++ (int)
		{
			m_index++;

			if (m_index == m_maxValue)
			{
				m_index = 0;
			}

			return m_index;
		}

		int operator () () const { return m_index; }

		void reset() { m_index = 0; }

		void setMaxValue(int maxValue) { m_maxValue = maxValue; }
	};

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

	QueueIndex m_writeIndex;
	QueueIndex m_readIndex;

	int m_lostCount = 0;

public:
	QueueBase(QObject* parent, int itemSize, int queueSize);
	virtual ~QueueBase();

	int size() { return m_size; }

	bool isEmpty() { return m_size == 0; }

	bool isNotEmpty() { return m_size > 0; }

	bool push(const char* item);
	bool pop(char* item);

	void clear();

	void lock() { m_mutex.lock(); }
	void unlock() { m_mutex.unlock(); }

	char* beginPush();
	bool completePush();

	void resize(int newQueueSize);
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



