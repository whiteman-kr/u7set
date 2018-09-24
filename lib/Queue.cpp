#include <cassert>
#include "../lib/Queue.h"
#include "../lib/WUtils.h"

// -------------------------------------------------------------------------
//
// QueueBase class implementation
//
// -------------------------------------------------------------------------

QueueBase::QueueBase(QObject* parent, int itemSize, int queueSize) :
	QObject(parent),
	m_itemSize(itemSize),
	m_queueSize(queueSize),
	m_writeIndex(queueSize),
	m_readIndex(queueSize)
{
	assert(itemSize > 0);
	assert(queueSize > 0);
	assert(itemSize * queueSize < MAX_QUEUE_MEMORY_SIZE);		// limit to 50 Mb

	AUTO_LOCK(m_mutex)

	m_buffer = new char [itemSize * queueSize];
}


QueueBase::~QueueBase(void)
{
	AUTO_LOCK(m_mutex)

	delete [] m_buffer;

	m_buffer = nullptr;
}


bool QueueBase::push(const char* item)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	AUTO_LOCK(m_mutex);

	if (m_size == m_queueSize)
	{
		m_readIndex++;				// lost last item in queue
		m_size--;
		m_lostedCount++;
	}

	memcpy(m_buffer + m_writeIndex() * m_itemSize, item, m_itemSize);

	m_writeIndex++;
	m_size++;

	if (m_size > m_maxSize)
	{
		m_maxSize = m_size;
	}

	emit queueNotEmpty();

	if (m_size == m_queueSize)
	{
		emit queueFull();
	}

	return true;
}


bool QueueBase::pop(char* item)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	AUTO_LOCK(m_mutex);

	if (m_size == 0)
	{
		return false;
	}

	memcpy(item, m_buffer + m_readIndex() * m_itemSize, m_itemSize);

	m_readIndex++;
	m_size--;

	if (m_size == 0)
	{
		emit queueEmpty();
	}

	return true;
}


void QueueBase::clear()
{
	AUTO_LOCK(m_mutex);

	m_size = 0;

	m_writeIndex.reset();
	m_readIndex.reset();
}


char* QueueBase::beginPush()
{
	m_mutex.lock();

	if (m_size == m_queueSize)
	{
		m_readIndex++;				// lost last item in queue
		m_size--;
		m_lostedCount++;
	}

	return m_buffer + m_writeIndex() * m_itemSize;
}

bool QueueBase::completePush()
{
	m_writeIndex++;
	m_size++;

	if (m_size > m_maxSize)
	{
		m_maxSize = m_size;
	}

	emit queueNotEmpty();

	if (m_size == m_queueSize)
	{
		emit queueFull();
	}

	m_mutex.unlock();

	return true;
}

char* QueueBase::beginPop()
{
	m_mutex.lock();

	if (m_size == 0)
	{
		m_mutex.unlock();

		return nullptr;
	}

	return m_buffer + m_readIndex() * m_itemSize;
}

bool QueueBase::completePop()
{
	m_readIndex++;
	m_size--;

	if (m_size == 0)
	{
		emit queueEmpty();
	}

	m_mutex.unlock();

	return true;
}

void QueueBase::resize(int newQueueSize)
{
	assert(newQueueSize * m_itemSize < MAX_QUEUE_MEMORY_SIZE);

	AUTO_LOCK(m_mutex)

	delete [] m_buffer;

	m_size = 0;
	m_maxSize = 0;
	m_queueSize = newQueueSize;
	m_lostedCount = 0;

	m_buffer = new char [m_itemSize * m_queueSize];

	m_writeIndex.setMaxValue(newQueueSize);
	m_writeIndex.reset();

	m_readIndex.setMaxValue(newQueueSize);
	m_readIndex.reset();
}


// -------------------------------------------------------------------------
//
// LockFreeQueueBase class implementation
//
// One Writer - One Reader using only!!!
//
// -------------------------------------------------------------------------

LockFreeQueueBase::LockFreeQueueBase(int itemSize, int queueSize) :
	m_itemSize(itemSize),
	m_queueSize(queueSize),
	m_writeIndex(queueSize),
	m_readIndex(queueSize)
{
	assert(itemSize > 0);
	assert(queueSize >= 1);
	assert(itemSize * queueSize < MAX_QUEUE_MEMORY_SIZE);		// limit to 50 Mb

	m_buffer = new char [itemSize * queueSize];
}

bool LockFreeQueueBase::push(const char* item)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	int curSize = m_size.load();

	if (curSize >= m_queueSize)
	{
		m_lostedCount++;					// item will not be queued (losted item)
		return false;
	}

	memcpy(m_buffer + m_writeIndex() * m_itemSize, item, m_itemSize);

	m_writeIndex++;

	int prevSize = m_size.fetch_add(1);		// m_size incremented by Writer only!

	assert(prevSize < m_queueSize);

	if (prevSize + 1 > m_maxSize)
	{
		m_maxSize.store(prevSize + 1);
	}

	return true;
}

bool LockFreeQueueBase::pop(char* item)
{
	if (item == nullptr)
	{
		assert(false);
		return false;
	}

	int curSize = m_size.load();

	if (curSize == 0)				// no items to pop
	{
		return false;
	}

	memcpy(item, m_buffer + m_readIndex() * m_itemSize, m_itemSize);

	m_readIndex++;

	m_size.fetch_sub(1);		// m_size decremented by Reader only!

	return true;
}

char* LockFreeQueueBase::beginPush()
{
	int curSize = m_size.load();

	assert(curSize <= m_queueSize);

	if (curSize == m_queueSize)
	{
		m_lostedCount++;					// item will not be queued (losted item)
		return nullptr;
	}

	return m_buffer + m_writeIndex() * m_itemSize;;
}

bool LockFreeQueueBase::completePush()
{
	m_writeIndex++;

	int prevSize = m_size.fetch_add(1);		// m_size incremented by Writer only!

	assert(prevSize < m_queueSize);

	if (prevSize + 1 > m_maxSize)
	{
		m_maxSize.store(prevSize + 1);
	}

	return true;
}

char* LockFreeQueueBase::beginPop()
{
	int curSize = m_size.load();

	if (curSize == 0)
	{
		return nullptr;
	}

	return m_buffer + m_readIndex() * m_itemSize;
}

bool LockFreeQueueBase::completePop()
{
	m_readIndex++;

	m_size.fetch_sub(1);		// m_size decremented by Reader only!

	return true;
}

void LockFreeQueueBase::resize(int newQueueSize)		// not thread-safe operation!!!!
{
	assert(newQueueSize * m_itemSize < MAX_QUEUE_MEMORY_SIZE);

	delete [] m_buffer;

	m_queueSize = newQueueSize;

	m_buffer = new char [m_itemSize * m_queueSize];

	m_readIndex.reset();
	m_readIndex.setMaxValue(newQueueSize);

	m_writeIndex.reset();
	m_writeIndex.setMaxValue(newQueueSize);

	m_size.store(0);
	m_maxSize.store(0);
}


FastQueueBase::FastQueueBase(int itemSize, int queueSize) :
	m_itemSize(itemSize),
	m_queueSize(queueSize),
	m_writeIndex(queueSize),
	m_readIndex(queueSize)
{
	if (m_queueSize <= 0)
	{
		assert(false);
		m_queueSize = 1;
	}

	if (m_itemSize <= 0)
	{
		assert(false);
	}

	m_buffer = allocateBuffer(m_queueSize * m_itemSize);
}

FastQueueBase::~FastQueueBase()
{
	delete [] m_buffer;
}

bool FastQueueBase::push(const char* item)
{
	TEST_PTR_RETURN_FALSE(item);

	if (m_size == m_queueSize)
	{
		m_readIndex++;				// lost last item in queue
	}

	memcpy(m_buffer + m_writeIndex() * m_itemSize, &item, m_itemSize);

	m_writeIndex++;

	// m_size - is not changed

	return true;
}

bool FastQueueBase::pop(char* item)
{
	TEST_PTR_RETURN_FALSE(item);

	if (m_size == 0)
	{
		return false;
	}

	memcpy(item, m_buffer + m_readIndex() * m_itemSize, m_itemSize);

	m_readIndex++;
	m_size--;

	return true;
}

void FastQueueBase::resizeAndCopy(int newQueueSize)
{
	if (newQueueSize <= 0)
	{
		assert(false);
		newQueueSize = 1;
	}

	if (newQueueSize == m_queueSize)
	{
		return;
	}

	if (m_size > 0)
	{
		// data copying required
		//

		if (newQueueSize < m_size)
		{
			// queue compression
			// if newQueueSize < m_size, remove items from beginng of queue
			//
			m_readIndex += m_size - newQueueSize;
			m_size = newQueueSize;
		}

		char* newBuffer = allocateBuffer(newQueueSize * m_itemSize);

		if (m_readIndex() < m_writeIndex())
		{
			memcpy(newBuffer, m_buffer + m_readIndex() * m_itemSize, m_size * m_itemSize);
		}
		else
		{
			int firstPartSize = m_queueSize - m_readIndex();
			int secondPartSize = m_size - firstPartSize;

			memcpy(newBuffer, m_buffer + m_readIndex() * m_itemSize, firstPartSize * m_itemSize);
			memcpy(newBuffer + firstPartSize * m_itemSize, m_buffer, secondPartSize * m_itemSize);
		}

		delete [] m_buffer;
		m_buffer = newBuffer;

		m_readIndex.reset();
		m_readIndex.setMaxValue(newQueueSize);

		m_writeIndex.setValue(m_size);
		m_writeIndex.setMaxValue(newQueueSize);

		m_queueSize = newQueueSize;
	}
	else
	{
		// no data copying required
		//
		delete [] m_buffer;
		m_buffer = allocateBuffer(newQueueSize * m_itemSize);

		m_readIndex.reset();
		m_readIndex.setMaxValue(newQueueSize);

		m_writeIndex.reset();
		m_writeIndex.setMaxValue(newQueueSize);

		m_queueSize = newQueueSize;
	}

}

bool FastQueueBase::copyToBuffer(char* buffer, int bufferSize, int* copiedDataSize)
{
	TEST_PTR_RETURN_FALSE(buffer);
	TEST_PTR_RETURN_FALSE(copiedDataSize);

	int itemsToCopy = bufferSize / m_itemSize;

	assert(itemsToCopy != 0);

	if (itemsToCopy == 0 || m_size == 0)
	{
		*copiedDataSize = 0;
		return false;
	}

	if (m_size < itemsToCopy)
	{
		itemsToCopy = m_size;
	}

	if (m_readIndex() < m_writeIndex())
	{
		memcpy(buffer, m_buffer + m_readIndex() * m_itemSize, itemsToCopy * m_itemSize);
	}
	else
	{
		int firstPartSize = m_queueSize - m_readIndex();
		int secondPartSize = m_size - firstPartSize;

		memcpy(buffer, m_buffer + m_readIndex() * m_itemSize, firstPartSize * m_itemSize);
		memcpy(buffer + firstPartSize * m_itemSize, m_buffer, secondPartSize * m_itemSize);
	}

	m_readIndex += itemsToCopy;
	m_size -= itemsToCopy;

	*copiedDataSize = itemsToCopy * m_itemSize;

	return true;
}

char* FastQueueBase::allocateBuffer(int sizeBytes)
{
	if (sizeBytes <= 0)
	{
		assert(false);
	}

	return new char [sizeBytes];
}

