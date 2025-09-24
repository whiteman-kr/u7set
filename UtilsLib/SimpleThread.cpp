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
	// qDebug() << "Worker" << m_workerName << "created";
}

SimpleThreadWorker::~SimpleThreadWorker()
{
	// qDebug() << "Worker" << m_workerName << "deleted";
}

void SimpleThreadWorker::setWorkerName(const QString& workerName)
{
	m_workerName = workerName;
}

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
	qDebug() << C_STR(QString("SimpleThreadWorker::slot_onThreadStarted worker started %1").arg(workerName()));
	onThreadStarted();
	m_thread->workerStarted(this);
}

void SimpleThreadWorker::slot_onThreadFinished()
{
	onThreadFinished();
	m_thread->workerFinished(this);
	qDebug() << C_STR(QString("SimpleThreadWorker::slot_onThreadFinished worker finished %1").arg(workerName()));
}

void SimpleThreadWorker::setThread(SimpleThread* thread)
{
	Q_ASSERT(thread != nullptr && m_thread == nullptr);
	m_thread = thread;
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
}

void SimpleThread::addWorker(SimpleThreadWorker* worker)
{
	if (worker == nullptr ||
		m_workers.contains(worker))
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
	for(SimpleThreadWorker* worker : m_workers)
	{
		worker->setThread(this);
		worker->moveToThread(&m_thread);

		connect(&m_thread, &QThread::started, worker, &SimpleThreadWorker::slot_onThreadStarted);
		connect(&m_thread, &QThread::finished, worker, &SimpleThreadWorker::deleteLater);
		connect(this, &SimpleThread::quitRequested, worker, &SimpleThreadWorker::slot_onThreadFinished, Qt::QueuedConnection);
	}

	m_thread.start();

	//

	QElapsedTimer et;

	et.start();

	qDebug() << C_STR(QString("%1 start").arg(m_threadName));

	std::unique_lock ul(m_startCondVarMutex, std::defer_lock);

	ul.lock();

	m_startCondVar.wait(
		ul,
		[this]() -> bool
		{
			return m_startedWorkersCount == static_cast<int>(m_workers.size());
		});

	ul.unlock();

	qDebug() << C_STR(QString("%1 started %2 at %3 ms").
					  arg(m_threadName).arg(m_startedWorkersCount).arg(et.elapsed()));
}

bool SimpleThread::quitAndWait(unsigned long time)
{
	QElapsedTimer et;

	et.start();

	emit quitRequested();

	qDebug() << C_STR(QString("%1 quitAndWait").arg(m_threadName));

	bool finishedOk = false;

	std::unique_lock ul(m_finishCondVarMutex, std::defer_lock);

	ul.lock();

	finishedOk = m_finishCondVar.wait_for(
		ul,
		std::chrono::milliseconds(time),
		[this]() -> bool
		{
			return m_finishedWorkersCount == static_cast<int>(m_workers.size());
		});

	ul.unlock();

	qDebug() << C_STR(QString("%1 finished %2 at %3 ms").
					  arg(m_threadName).arg(m_finishedWorkersCount).arg(et.elapsed()));

	Q_ASSERT(finishedOk = true);

	m_thread.quit();

	finishedOk = m_thread.wait(3000);

	//Q_ASSERT(finishedOk == true);

	return finishedOk;
}

bool SimpleThread::isInterruptionRequested() const
{
	return m_thread.isInterruptionRequested();
}

bool SimpleThread::isRunning() const
{
	return m_thread.isRunning();
}

bool SimpleThread::isFinished() const
{
	return m_thread.isFinished();
}

int SimpleThread::finishedWorkersCount() const
{
	return m_finishedWorkersCount;
}

void SimpleThread::workerStarted(SimpleThreadWorker* worker)
{
	if (worker == nullptr || m_workers.contains(worker) == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_startCondVarMutex.lock();

	m_startedWorkersCount++;

	m_startCondVarMutex.unlock();

	m_startCondVar.notify_one();
}

void SimpleThread::workerFinished(SimpleThreadWorker* worker)
{
	if (worker == nullptr || m_workers.contains(worker) == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_finishCondVarMutex.lock();

	m_finishedWorkersCount++;

	m_finishCondVarMutex.unlock();

	m_finishCondVar.notify_one();
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

