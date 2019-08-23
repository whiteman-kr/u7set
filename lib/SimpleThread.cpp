#include "../lib/SimpleThread.h"

#include <cassert>
#include <QTimer>
#include <QMetaMethod>


// -------------------------------------------------------------------------------------
//
// SimpleThreadWorker class implementation
//
// -------------------------------------------------------------------------------------


void SimpleThreadWorker::slot_onThreadStarted()
{
	onThreadStarted();
}


void SimpleThreadWorker::slot_onThreadFinished()
{
	onThreadFinished();
	deleteLater();
}


// -------------------------------------------------------------------------------------
//
// SimpleThread class implementation
//
// -------------------------------------------------------------------------------------


SimpleThread::SimpleThread()
{
}


SimpleThread::SimpleThread(SimpleThreadWorker* worker)
{
	addWorker(worker);
}


SimpleThread::~SimpleThread()
{
	m_thread.quit();
	m_thread.wait();

	foreach(SimpleThreadWorker* worker, m_workerList)
	{
		if (worker == nullptr)
		{
			assert(false);
			continue;
		}
	}
}


void SimpleThread::addWorker(SimpleThreadWorker* worker)
{
	if (m_thread.isRunning() == true)
	{
		// All workers should be added before the thread is started
		//
		assert(false);
		return;
	}

	if (worker == nullptr)
	{
		assert(false);
		return;
	}

	m_workerList.append(worker);
}


void SimpleThread::start()
{
	foreach(SimpleThreadWorker* worker, m_workerList)
	{
		if (worker == nullptr)
		{
			assert(false);
			continue;
		}

		worker->moveToThread(&m_thread);

		connect(&m_thread, &QThread::started, worker, &SimpleThreadWorker::slot_onThreadStarted);
		connect(&m_thread, &QThread::finished, worker, &SimpleThreadWorker::slot_onThreadFinished);
	}

	beforeStart();

	m_thread.start();
}


void SimpleThread::quit()
{
	beforeQuit();

	foreach(SimpleThreadWorker* worker, m_workerList)
	{
		if (worker == nullptr)
		{
			assert(false);
			continue;
		}

		worker->requestQuit();
	}

	m_thread.quit();
}


bool SimpleThread::wait(unsigned long time)
{
	return m_thread.wait(time);
}


bool SimpleThread::quitAndWait(unsigned long time)
{
	quit();
	return wait(time);
}

bool SimpleThread::isRunning() const
{
	return m_thread.isRunning();
}

bool SimpleThread::isFinished() const
{
	return m_thread.isFinished();
}

void SimpleThread::beforeStart()
{
}


void SimpleThread::beforeQuit()
{
}

// -------------------------------------------------------------------------------------
//
// WaitForSignalHelper class implementation
//
// -------------------------------------------------------------------------------------

WaitForSignalHelper::WaitForSignalHelper(const QObject* sender, const char* signal)
{
	connect(sender, signal, &m_eventLoop, SLOT(quit()));
}

void WaitForSignalHelper::slot_timeout()
{
	m_timeout = true;
	m_eventLoop.quit();
}

bool WaitForSignalHelper::wait(int milliseconds)
{
	QTimer timer;

	if (milliseconds != 0 )
	{
		connect(&timer, &QTimer::timeout, this, &WaitForSignalHelper::slot_timeout);
		timer.setInterval(milliseconds);
		timer.start();
	}
	else
	{
		// else, wait for ever!
	}

	m_timeout = false;

	m_eventLoop.exec();

	return !m_timeout;
}


