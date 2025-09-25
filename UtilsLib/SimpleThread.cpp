#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "SimpleThread.h"
#include "../UtilsLib/WUtils.h"

// -------------------------------------------------------------------------------------
//
// SimpleThreadWorker class implementation
//
// -------------------------------------------------------------------------------------

SimpleThreadWorker::SimpleThreadWorker(const QString& workerName) :
	m_workerName(workerName)
{
	log(QString("Worker %1 created").arg(m_workerName));
}

SimpleThreadWorker::~SimpleThreadWorker()
{
	log(QString("Worker %1 deleted").arg(m_workerName));
}

/*void SimpleThreadWorker::setWorkerName(const QString& workerName)
{
	m_workerName = workerName;
}*/

QString SimpleThreadWorker::workerName() const
{
	return m_workerName;
}

void SimpleThreadWorker::onThreadStarted()
{
}

void SimpleThreadWorker::onThreadFinished()
{
}

void SimpleThreadWorker::printFunction(const QString& func)
{
	qDebug() << C_STR(QString("%1::%2").arg(workerName()).arg(func));
}

void SimpleThreadWorker::slot_onThreadStarted()
{
	Q_ASSERT(m_thread);
	m_thread->workerStarted(this);
	onThreadStarted();
	log(QString("SimpleThreadWorker::slot_onThreadStarted worker started %1").arg(workerName()));
}

void SimpleThreadWorker::slot_onThreadFinished()
{
	Q_ASSERT(m_thread);
	if (m_finished == true)
	{
		return;
	}
	m_finished = true;
	log(QString("SimpleThreadWorker::slot_onThreadFinished worker finished %1").arg(workerName()));
	onThreadFinished();
	m_thread->workerFinished(this);
}

void SimpleThreadWorker::setThread(SimpleThread* thread)
{
	Q_ASSERT(thread != nullptr && m_thread == nullptr);
	m_thread = thread;
}

void SimpleThreadWorker::log(const QString& str)
{
	qDebug() << C_STR(str);
}

// -------------------------------------------------------------------------------------
//
// SimpleThread class implementation
//
// -------------------------------------------------------------------------------------

SimpleThread::SimpleThread() :
	m_threadName("SimpleThread:")
{
}

SimpleThread::SimpleThread(SimpleThreadWorker* worker) :
	m_threadName("SimpleThread:")
{
	addWorker(worker);
}

SimpleThread::~SimpleThread()
{
	if (m_started && m_thread.isRunning())
	{
		quitAndWait(3000);
	}
}

void SimpleThread::addWorker(SimpleThreadWorker* worker)
{
	std::lock_guard lg(m_mutex);

	if (worker == nullptr ||
		m_workers.contains(worker) ||
		m_started == true ||
		worker->parent() != nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_workers.insert(worker);

	m_threadName += QString(" %1").arg(worker->workerName());
}

void SimpleThread::setPriority(QThread::Priority priority)
{
	m_thread.setPriority(priority);
}

void SimpleThread::start()
{
	if (m_started.exchange(true))
	{
		Q_ASSERT(m_started == false);
		return;
	}

	{
		std::lock_guard lg(m_mutex);

		for(SimpleThreadWorker* worker : m_workers)
		{
			worker->setThread(this);
			worker->moveToThread(&m_thread);

			connect(&m_thread, &QThread::started, worker, &SimpleThreadWorker::slot_onThreadStarted, Qt::QueuedConnection);
			connect(&m_thread, &QThread::finished, worker, &SimpleThreadWorker::deleteLater);
			connect(this, &SimpleThread::quitRequested, worker, &SimpleThreadWorker::slot_onThreadFinished, Qt::QueuedConnection);
		}
	}

	m_thread.start();

	//

	QElapsedTimer et;

	et.start();

	log(QString("%1 start").arg(m_threadName));

	std::unique_lock ul(m_mutex);

	m_condVar.wait(
		ul,
		[this]() -> bool
		{
			return m_workers.empty();
		});

	ul.unlock();

	log(QString("%1 started %2 at %3 ms").
					  arg(m_threadName).arg(m_runningWorkersCount).arg(et.elapsed()));
}

bool SimpleThread::quitAndWait(unsigned long time)
{
	if (m_started == false)
	{
		Q_ASSERT(m_started == true);
		return true;
	}

	QElapsedTimer et;

	et.start();

	emit quitRequested();

	log(QString("%1 quitAndWait").arg(m_threadName));

	bool finishedOk = false;

	std::unique_lock ul(m_mutex);

	finishedOk = m_condVar.wait_for(
		ul,
		std::chrono::milliseconds(time),
		[this]() -> bool
		{
			return m_runningWorkers.empty();
		});

	ul.unlock();

	log(QString("%1 finished %2 at %3 ms").
					  arg(m_threadName).arg(m_finishedWorkersCount).arg(et.elapsed()));

	Q_ASSERT(finishedOk == true);

	m_thread.quit();

	finishedOk = m_thread.wait(3000);

	m_started = false;

	return finishedOk;
}

/*bool SimpleThread::isInterruptionRequested() const
{
	return m_thread.isInterruptionRequested();
}*/

bool SimpleThread::isRunning() const
{
	return m_thread.isRunning();
}

bool SimpleThread::isFinished() const
{
	return m_thread.isFinished();
}

void SimpleThread::workerStarted(SimpleThreadWorker* worker)
{
	{
		std::lock_guard lg(m_mutex);

		if (worker == nullptr || m_workers.contains(worker) == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_workers.erase(worker);
		m_runningWorkers.insert(worker);
		m_runningWorkersCount++;
	}

	m_condVar.notify_all();
}

void SimpleThread::workerFinished(SimpleThreadWorker* worker)
{
	{
		std::lock_guard lg(m_mutex);

		if (worker == nullptr || m_runningWorkers.contains(worker) == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_runningWorkers.erase(worker);

		m_runningWorkersCount--;
		m_finishedWorkersCount++;

		worker->deleteLater();
	}

	m_condVar.notify_all();
}

void SimpleThread::log(const QString& str)
{
	qDebug() << C_STR(str);
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

