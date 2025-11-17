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

QString SimpleThreadWorker::workerName() const
{
	return m_workerName;
}

void SimpleThreadWorker::enableLog(bool enable)
{
	m_enableLog = enable;
}

void SimpleThreadWorker::onThreadStarted()
{
}

void SimpleThreadWorker::onThreadFinished()
{
}

void SimpleThreadWorker::printFunction(const QString& func)
{
	log(QString("%1::%2").arg(workerName()).arg(func));
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

	bool expected = false;

	if (m_finished.compare_exchange_strong(expected, true) == false)
	{
		return;
	}

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
	if (m_enableLog)
	{
		qDebug() << C_STR(str);
	}
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

	if (!m_started)
	{
		std::lock_guard lg(m_mutex);
		for (auto* w : m_workers)
		{
			delete w; // они в главном потоке, родителя нет
		}
		m_workers.clear();
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
		log(QString("%1 addWorker ERROR!").arg(m_threadName));
		return;
	}

	m_workers.insert(worker);

	worker->enableLog(m_enableLog);

	m_threadName += QString(" %1").arg(worker->workerName());
}

void SimpleThread::setPriority(QThread::Priority priority)
{
	m_thread.setPriority(priority);
}

void SimpleThread::start(unsigned long time)
{
	if (m_started.exchange(true))
	{
		return;
	}

	{
		std::lock_guard lg(m_mutex);

		if (m_workers.empty())
		{
			m_started = false;
			log(QString("%1 NO workers!").arg(m_threadName));
			Q_ASSERT(false);
			return;
		}

		for(SimpleThreadWorker* worker : m_workers)
		{
			worker->enableLog(m_enableLog);
			worker->setThread(this);
			worker->moveToThread(&m_thread);

			connect(&m_thread, &QThread::started, worker, &SimpleThreadWorker::slot_onThreadStarted,
						static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
			connect(this, &SimpleThread::quitRequested, worker, &SimpleThreadWorker::slot_onThreadFinished,
						static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
		}
	}

	m_thread.start();

	//

	QElapsedTimer et;

	et.start();

	log(QString("%1 start").arg(m_threadName));

	std::unique_lock ul(m_mutex);

	bool startOk = m_condVar.wait_for(
		ul,
		std::chrono::microseconds(time),
		[this]() -> bool
		{
			return m_workers.empty();
		});

	ul.unlock();

	if (startOk == false)
	{
		log(QString("%1 NOT ALL workers started! pending %2 running %3").
			arg(m_threadName).arg(m_workers.size()).arg(m_runningWorkers.size()));
	}

	log(QString("%1 started %2 at %3 ms").
					  arg(m_threadName).arg(m_runningWorkers.size()).arg(et.elapsed()));
}

bool SimpleThread::quitAndWait(unsigned long time)
{
	Q_ASSERT(QThread::currentThread() != &m_thread);

	if (m_started == false)
	{
		log(QString("%1 NOT started!").arg(m_threadName));
		return true;
	}

	QElapsedTimer et;

	et.start();

	emit quitRequested();

	log(QString("%1 quitAndWait").arg(m_threadName));

	bool workersFinishedOk = false;

	std::unique_lock ul(m_mutex);

	workersFinishedOk = m_condVar.wait_for(
		ul,
		std::chrono::milliseconds(time),
		[this]() -> bool
		{
			return m_runningWorkers.empty();
		});

	ul.unlock();

	if (workersFinishedOk == false)
	{
		log(QString("%1 NOT ALL workers finished!").arg(m_threadName));
	}

	log(QString("%1 finished %2 at %3 ms").
					  arg(m_threadName).arg(m_finishedWorkersCount).arg(et.elapsed()));

	m_thread.quit();

	bool threadFinishedOk = m_thread.wait(3000);

	m_started = false;

	return workersFinishedOk && threadFinishedOk;
}

bool SimpleThread::isRunning() const
{
	return m_thread.isRunning();
}

bool SimpleThread::isFinished() const
{
	return m_thread.isFinished();
}

void SimpleThread::enableLog(bool enable)
{
	m_enableLog = enable;
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
	}

	m_condVar.notify_all();
}

void SimpleThread::workerFinished(SimpleThreadWorker* worker)
{
	if (worker == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	std::unique_lock ul(m_mutex);

	if (m_runningWorkers.contains(worker) == true)
	{
		m_runningWorkers.erase(worker);

		m_finishedWorkersCount++;

		ul.unlock();

		worker->deleteLater();
		m_condVar.notify_all();
		return;
	}

	if (m_workers.contains(worker))
	{
		// worker not started
		//
		m_workers.erase(worker);

		ul.unlock();

		delete worker;
		m_condVar.notify_all();
		return;
	}

	// WTF?
	//
	Q_ASSERT(false);
	ul.unlock();
	m_condVar.notify_all();
}

void SimpleThread::log(const QString& str)
{
	if (m_enableLog)
	{
		qDebug() << C_STR(str);
	}
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

