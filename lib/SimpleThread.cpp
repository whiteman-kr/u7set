#include "../include/SimpleThread.h"
#include <cassert>

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


SimpleThread::SimpleThread(SimpleThreadWorker* worker) :
	m_worker(worker)
{
	assert(m_worker != nullptr);
}


SimpleThread::~SimpleThread()
{
	m_thread.quit();
	m_thread.wait();
}


void SimpleThread::setWorker(SimpleThreadWorker* worker)
{
	if (worker == nullptr)
	{
		assert(false);
		return;
	}

	if (m_worker != nullptr)
	{
		assert(false);			// worker allready set!
		return;
	}

	m_worker = worker;
}


void SimpleThread::start()
{
	if (m_worker == nullptr)
	{
		assert(false);			// set worker before start!
		return;
	}

	m_worker->moveToThread(&m_thread);

	connect(&m_thread, &QThread::started, m_worker, &SimpleThreadWorker::slot_onThreadStarted);
	connect(&m_thread, &QThread::finished, m_worker, &SimpleThreadWorker::slot_onThreadFinished);

	beforeStart();

	m_thread.start();
}


void SimpleThread::quit()
{
	beforeQuit();

	m_thread.quit();
}


void SimpleThread::beforeStart()
{
}


void SimpleThread::beforeQuit()
{
}

