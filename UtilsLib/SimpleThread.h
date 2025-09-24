#pragma once

#include <atomic>
#include <unordered_set>

#include <QEventLoop>
#include <QThread>

class SimpleThread;

class SimpleThreadWorker : public QObject
{
	Q_OBJECT

public:
	SimpleThreadWorker(const QString& workerName = QString());
	virtual ~SimpleThreadWorker();

	void setWorkerName(const QString& workerName);
	QString workerName() const;

protected:
	virtual void onThreadStarted();
	virtual void onThreadFinished();

	void printFunction(const QString& func);

private slots:
	void slot_onThreadStarted();
	void slot_onThreadFinished();

private:
	void setThread(SimpleThread* thread);

private:
	QString m_workerName;
	SimpleThread* m_thread = nullptr;

	friend class SimpleThread;
};

#define PRINT_FUNC printFunction(__func__);


class SimpleThread : public QObject
{
	Q_OBJECT

public:
	SimpleThread();
	SimpleThread(SimpleThreadWorker* worker);
	virtual ~SimpleThread();

public:
	void addWorker(SimpleThreadWorker* worker);

	void setPriority(QThread::Priority priority);

	void start();
	bool quitAndWait(unsigned long time = ULONG_MAX);

	bool isInterruptionRequested() const;
	bool isRunning() const;
	bool isFinished() const;

	int finishedWorkersCount() const;

signals:
	void quitRequested();

private:
	void workerStarted(SimpleThreadWorker* worker);
	void workerFinished(SimpleThreadWorker* worker);

protected:
	QString m_threadName;

	QThread m_thread;
	std::unordered_set<SimpleThreadWorker*> m_workers;

	std::mutex m_startCondVarMutex;
	std::condition_variable m_startCondVar;
	int m_startedWorkersCount = 0;

	std::mutex m_finishCondVarMutex;
	std::condition_variable m_finishCondVar;
	int m_finishedWorkersCount = 0;

	friend class SimpleThreadWorker;
};

class RunOverrideThread : public QThread
{
public:
	bool isQuitRequested() const { return m_quitRequested.load(); }

	bool quitAndWait(unsigned long time = ULONG_MAX)
	{
		m_quitRequested.store(true);
		return wait(time);
	}

private:
	std::atomic<bool> m_quitRequested = { false };
};

class WaitForSignalHelper : public QObject
{
	Q_OBJECT

private:
	bool m_timeout = false;
	QEventLoop m_eventLoop;

private slots:
	void slot_timeout();

public:
	WaitForSignalHelper(const QObject* sender, const char* signal);

	bool wait(int milliseconds);			// return true if signal received before timeout
											// return false if timeout elapsed

	bool waitForever() { return wait(0); }
};

class ThreadWithQuit
{
public:
	void quit()
	{
		m_quitRequested = true;
		wakeupToQuit();
	}

	bool isQuitRequested() const { return m_quitRequested; }

	virtual void wakeupToQuit() {}

protected:
	std::atomic_bool m_quitRequested{false};
};
