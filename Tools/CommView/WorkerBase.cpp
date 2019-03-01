#include "WorkerBase.h"

// -------------------------------------------------------------------------------------------------------------------

WorkerBase::WorkerBase(QObject *parent)
	: QObject(parent)
{

}

// -------------------------------------------------------------------------------------------------------------------

WorkerBase::~WorkerBase()
{

}

// -------------------------------------------------------------------------------------------------------------------

int WorkerBase::count() const
{
	int count = 0;

	m_mutex.lock();

		count = m_workerList.count();

	m_mutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void WorkerBase::clear()
{
}

// -------------------------------------------------------------------------------------------------------------------

int WorkerBase::append(SerialPortWorker* pWorker)
{
	if (pWorker == nullptr)
	{
		return -1;
	}

	int index = -1;

	m_mutex.lock();

		m_workerList.append(pWorker);
		index = m_workerList.count() - 1;

	m_mutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortWorker* WorkerBase::at(int index) const
{
	SerialPortWorker *pWorker = nullptr;

	m_mutex.lock();

		if (index >= 0 || index < m_workerList.count())
		{
			pWorker = m_workerList[index];
		}

	m_mutex.unlock();

	return pWorker;
}

// -------------------------------------------------------------------------------------------------------------------
