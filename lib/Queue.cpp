#include <cassert>
#include "../lib/Queue.h"
#include "../lib/WUtils.h"


QueueBase::QueueBase(QObject* parent, int itemSize, int queueSize) :
	QObject(parent),
	m_itemSize(itemSize),
	m_queueSize(queueSize),
	m_writeIndex(queueSize),
	m_readIndex(queueSize)
{
	assert(itemSize > 0);
	assert(queueSize > 0);

	assert(itemSize * queueSize < 100 * 1024 * 1024);		// limit to 20Mb

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
		m_lostCount++;
	}

	memcpy(m_buffer + m_writeIndex() * m_itemSize, item, m_itemSize);

	m_writeIndex++;
	m_size++;

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
		m_lostCount++;
	}

	return m_buffer + m_writeIndex() * m_itemSize;
}


bool QueueBase::completePush()
{
	m_writeIndex++;
	m_size++;

	emit queueNotEmpty();

	if (m_size == m_queueSize)
	{
		emit queueFull();
	}

	m_mutex.unlock();

	return true;
}

void QueueBase::resize(int newQueueSize)
{
	assert(newQueueSize < 1024 * 1024);

	AUTO_LOCK(m_mutex)

	delete [] m_buffer;

	m_queueSize = newQueueSize;
	m_buffer = new char [m_itemSize * m_queueSize];

	m_writeIndex.reset();
	m_readIndex.reset();
}


