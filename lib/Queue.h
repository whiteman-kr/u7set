#pragma once
#include <QtCore>


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
		QueueBase(parent, sizeof(TYPE), queueSize) 	{}

	Queue(int queueSize) :
		QueueBase(nullptr, sizeof(TYPE), queueSize) 	{}

	virtual bool push(const TYPE* ptr) { return QueueBase::push(reinterpret_cast<const char*>(ptr)); }
	virtual bool pop(TYPE* ptr) { return QueueBase::pop(reinterpret_cast<char*>(ptr)); }

	TYPE* beginPush() { return reinterpret_cast<TYPE*>(QueueBase::beginPush()); }
};

